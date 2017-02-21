#include "sta-couwbat-mac.h"
#include "ns3/log.h"
#include "couwbat-packet-helper.h"
#include "couwbat.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("StaCouwbatMac");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (StaCouwbatMac);

TypeId
StaCouwbatMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::StaCouwbatMac")
    .SetParent<CouwbatMac> ()
    .SetGroupName("Couwbat")
    .AddConstructor<StaCouwbatMac> ()
  ;
  return tid;
}

StaCouwbatMac::StaCouwbatMac ()
{
  NS_LOG_FUNCTION (this);

  m_rxOkCallback = MakeCallback (&StaCouwbatMac::RxOk, this);
}

StaCouwbatMac::~StaCouwbatMac ()
{
  NS_LOG_FUNCTION (this);
}

void
StaCouwbatMac::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  if (m_netlinkMode)
    {
      NS_LOG_DEBUG("StaCouwbatMac running in netlink mode");
    }
  else
    {
      m_phy->SetReceiveOkCallback (m_rxOkCallback);
  }

  m_associated = false;

  // Initialise constant packet sizes
  std::vector<uint32_t> subchannels;
  subchannels.push_back (0);
//  Ptr<Packet> pss = CouwbatPacketHelper::CreatePss(m_address, subchannels);
//  m_pssSizeBytes = pss->GetSize ();
  m_pssSizeBytes = 30;

  Ptr<Packet> assoc = CouwbatPacketHelper::CreateEmpty (
      COUWBAT_FC_CONTROL_STA_ASSOC,
      m_address,
      m_currentBsAddr,
      0);
  m_assocSizeBytes = assoc->GetSize ();

  m_seq = std::rand ();

  m_useBackupCc = false;

  m_xstate_pss_rx_scheduled[1] = false;
  m_xstate_map_rx_scheduled[1] = false;
  m_xstate_pss_rx_scheduled[0] = false;
  m_xstate_map_rx_scheduled[0] = false;

  SetState (CR_STA_INIT);
}

void
StaCouwbatMac::SetState (const CrStaState newState)
{
  NS_LOG_FUNCTION (this << newState);
  m_state = newState;
}

void
StaCouwbatMac::StartScanning ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("CR-STA " << m_address << (m_associated ? " lost connection to BS," : "") << " initiating full scan");

  SetState (CR_STA_SCAN);
  m_associated = false;
  m_useBackupCc = false;
  m_scannedPss.clear ();

  m_currentScanEndedSfCnt = m_sfCnt + 1 + Couwbat::GetNumberOfSubchannels ();

  m_currentScanCcId = 0;
  ScanRxPss (m_currentScanCcId);
  ++m_currentScanCcId;
}

void
StaCouwbatMac::ScanRxPss (uint32_t subch)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("CR-STA " << m_address << " scan for PSS on subchannel " << subch);

  std::vector<CouwbatMCS> pssMcs = std::vector<CouwbatMCS> (1, Couwbat::GetDefaultMcs ());
  double padding = 0;
  uint16_t len = NecessarySymbolsForBytes(m_pssSizeBytes, 1, pssMcs, padding);
  NS_ASSERT (padding == 0);

  CouwbatMetaHeader rxSchedMh;
  rxSchedMh.m_ofdm_sym_len = len;
  rxSchedMh.m_ofdm_sym_offset = 0;
  rxSchedMh.m_flags = CW_CMD_WIFI_EXTRA_ZERO_RX;
  rxSchedMh.m_allocatedSubChannels.set (subch, true);
  rxSchedMh.m_ofdm_sym_sframe_count = m_sfCnt + 1;
  rxSchedMh.m_MCS[subch] = Couwbat::GetDefaultMcs ();

  Ptr<Packet> rxSched = Create<Packet> ();
  Send (rxSchedMh, rxSched);
  m_xstate_pss_rx_scheduled[0] = true;
}

void
StaCouwbatMac::SelectCrBs (void)
{
  NS_LOG_FUNCTION (this);
  if (m_scannedPss.empty ())
    {
      NS_LOG_INFO ("CR-STA " << m_address << " couldn't find any BS");
      StartScanning ();
      return;
    }

  std::vector<ScannedPss>::iterator i = m_scannedPss.begin ();
  ScannedPss bestPss = *(i++);
//  Take first result
  m_scannedPss.clear ();

  CouwbatMacHeader header;
  bestPss.pss->RemoveHeader (header);
  CouwbatPssHeader pss;
  bestPss.pss->RemoveHeader (pss);

  NS_LOG_INFO ("CR-STA " << m_address << " selecting CR-BS "
               << header.GetSource ());

  uint32_t ccId = 0;
  for (uint32_t i = 0; i < Couwbat::GetNumberOfSubchannels (); ++i)
    {
      if (bestPss.mh.m_allocatedSubChannels.test (i))
  {
    ccId = i;
    break;
  }
    }
  m_ccId = ccId;

  m_currentBsAddr = header.GetSource ();
  SetState (CR_STA_ASSOC);

  m_pssTimeout = Couwbat::STA_PSS_TIMEOUT_SF;

  // schedule next PSS RX
  double padding;
  CouwbatMetaHeader rxSchedMh;
  rxSchedMh.m_flags = CW_CMD_WIFI_EXTRA_ZERO_RX;
  rxSchedMh.m_ofdm_sym_sframe_count = m_sfCnt + 1;
  rxSchedMh.m_ofdm_sym_offset = 0;
  std::vector<CouwbatMCS> pssMcs = std::vector<CouwbatMCS> (1, Couwbat::GetDefaultMcs ());
  rxSchedMh.m_ofdm_sym_len = NecessarySymbolsForBytes(m_pssSizeBytes, 1, pssMcs, padding);
  NS_ASSERT (padding == 0);
  rxSchedMh.m_allocatedSubChannels.set (m_ccId, true);
  rxSchedMh.m_MCS[m_ccId] = Couwbat::GetDefaultMcs ();
  Ptr<Packet> rxSched = Create<Packet> ();
  Send (rxSchedMh, rxSched);
  m_xstate_pss_rx_scheduled[0] = true;
}

void
StaCouwbatMac::SendAssoc ()
{
  NS_LOG_FUNCTION (this);

  double padding;
  uint32_t guard = Couwbat::GetAlohaNrGuardSymbols ();
  std::vector<CouwbatMCS> nbMcs = std::vector<CouwbatMCS> (1, Couwbat::GetDefaultMcs ());
  uint32_t pssTxDurationSymb = NecessarySymbolsForBytes (m_pssSizeBytes, 1, nbMcs, padding);
  NS_ASSERT (padding == 0);
  uint32_t assocTxDurationSymb = NecessarySymbolsForBytes(m_assocSizeBytes, 1, nbMcs, padding);
  NS_ASSERT (padding == 0);

  // Random contention slot
  uint32_t offset = std::rand () % Couwbat::GetContentionSlotCount ();
  uint32_t txOffsetSymb = pssTxDurationSymb + guard + offset * (assocTxDurationSymb + guard);

  NS_LOG_DEBUG ("CR-STA " << m_address << " sending association request"
               " using contention slot " << offset << ", txOffsetSymb=" << txOffsetSymb);

  CouwbatMetaHeader assocMh;
  assocMh.m_flags = CW_CMD_WIFI_EXTRA_TX;
  assocMh.m_ofdm_sym_sframe_count = m_sfCnt + 1;
  assocMh.m_ofdm_sym_offset = txOffsetSymb;
  assocMh.m_ofdm_sym_len = NecessarySymbolsForBytes(m_assocSizeBytes, 1, nbMcs, padding);
  NS_ASSERT (padding == 0);
  assocMh.m_allocatedSubChannels.set (m_ccId, true);
  assocMh.m_MCS[m_ccId] = Couwbat::GetDefaultMcs ();
  Ptr<Packet> assoc = CouwbatPacketHelper::CreateEmpty(COUWBAT_FC_CONTROL_STA_ASSOC, m_address, m_currentBsAddr, 0);
  Send (assocMh, assoc);
}

void
StaCouwbatMac::SendSfStart (const CouwbatMetaHeader &mh)
{
  NS_LOG_INFO ("CR-STA " << m_address << " sending SF_START: scheduling in superframe " << mh.m_ofdm_sym_sframe_count << " finished");
  if (m_netlinkMode)
    {
      /*
       * send SF_START when all scheduling for this superframe is done
       */

      Ptr<Packet> sfsPacket = Create<Packet> ();
      CouwbatMetaHeader sfs;
      sfs.m_flags = CW_CMD_WIFI_EXTRA_ZERO_SF_START;
      sfs.m_ofdm_sym_sframe_count = mh.m_ofdm_sym_sframe_count;
      Send (sfs, sfsPacket);
    }
}

bool
StaCouwbatMac::FilterPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  // Get header
  CouwbatMacHeader header;
  if (packet->GetSize () < header.GetHeaderSize ()) return true;
  packet->PeekHeader (header);

  // Check for valid FCS
  if (header.GetFrameType () != COUWBAT_FC_CONTROL_MAP
      && header.GetFrameType () != COUWBAT_FC_DATA_DL)
    {
      CouwbatFcsTrailer trailer;
      if (packet->GetSize () < trailer.GetTrailerSize ()) return true;
      packet->RemoveTrailer (trailer);
      if (!trailer.CheckFcs (packet, 0))
        {
          NS_LOG_DEBUG ("FCS mismatch");
          return true; // FCS doesn't match
        }
    }

  if (header.GetDestination () != m_address
      && header.GetDestination () != Mac48Address("ff:ff:ff:ff:ff:ff"))
    {
      NS_LOG_DEBUG ("Wrong destination");
      return true; // not for us
    }

  if ((m_associated || m_state != CR_STA_SCAN)
      && header.GetSource () != m_currentBsAddr)
    {
      NS_LOG_DEBUG ("From other BS");
      return true; // not from our BS
    }

  return false;
}

void
StaCouwbatMac::RxOk (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  CouwbatMetaHeader mh;
  packet->RemoveHeader (mh);
  uint32_t mh_flag = mh.m_flags;
  switch (mh_flag)
    {
    case CW_CMD_WIFI_EXTRA_ZERO_SF_START:
      RxOkHandleZeroSfStart (mh, packet);
      break;

    case CW_CMD_WIFI_EXTRA_RX:
      {
        std::string ps = packet->ToString ();
        if (ps != "")
          {
            NS_LOG_INFO ("CR-STA " << m_address << " RX: {" << ps << "}");
          }
        RxOkHandleExtraRx (mh, packet);
      }
      break;

    default:
      break;
    }
}

void
StaCouwbatMac::RxOkHandleZeroSfStart (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  m_sfCnt = mh.m_ofdm_sym_sframe_count;
  m_txHistory.SuperframeTick (m_txQueue);

  // Update extra state information
  m_xstate_pss_rx_scheduled[1] = m_xstate_pss_rx_scheduled[0];
  m_xstate_map_rx_scheduled[1] = m_xstate_map_rx_scheduled[0];
  m_xstate_pss_rx_scheduled[0] = false;
  m_xstate_map_rx_scheduled[0] = false;

  switch (m_state)
  {
    case CR_STA_INIT:
      SetState (CR_STA_SCAN);
      StartScanning ();
      break;

    case CR_STA_SCAN:
      if (m_sfCnt >= m_currentScanEndedSfCnt)
        {
          // Scan ended
          SelectCrBs ();
        }
      else
        {
          // Scan in progress
          if (m_currentScanCcId < Couwbat::GetNumberOfSubchannels ())
            {
              ScanRxPss (m_currentScanCcId);
              ++m_currentScanCcId;
            }
        }
      break;

    case CR_STA_ASSOC:
    case CR_STA_OPERATING:
      {
        // Check PSS not received timeout
        if (m_pssTimeout <= 0)
          {
            if (m_backupCc.fields.newChNumber != m_ccId && m_useBackupCc)
              {
                NS_LOG_INFO("CR-STA " << m_address << " PSS not received timeout reached, switching CC to known backup CC");
                m_ccId = m_backupCc.fields.newChNumber;
                m_useBackupCc = false;
                m_pssTimeout = Couwbat::STA_PSS_TIMEOUT_SF;
              }
            else
              {
                NS_LOG_INFO("CR-STA " << m_address << " PSS not received timeout reached, starting full scan");
                StartScanning ();
                break;
              }
          }
        --m_pssTimeout;

        // Check for scheduled CC change to backup CC
        if (m_backupCc.fields.chSwitchActive && m_useBackupCc)
          {
            if (m_backupCc.fields.chSwitchCounter == 0)
              {
                NS_LOG_INFO("CR-STA " << m_address << " performing scheduled CC change to backup CC");
                m_ccId = m_backupCc.fields.newChNumber;
                m_backupCc.fields.chSwitchActive = 0;
                m_useBackupCc = false;
              }
            else
              {
                --m_backupCc.fields.chSwitchCounter;
              }
          }

        // schedule next PSS RX
        double padding;
        CouwbatMetaHeader rxPssSched;
        rxPssSched.m_flags = CW_CMD_WIFI_EXTRA_ZERO_RX;
        rxPssSched.m_ofdm_sym_sframe_count = m_sfCnt + 1;
        rxPssSched.m_ofdm_sym_offset = 0;
        std::vector<CouwbatMCS> nbMcs = std::vector<CouwbatMCS> (1, Couwbat::GetDefaultMcs ());
        rxPssSched.m_ofdm_sym_len = NecessarySymbolsForBytes(m_pssSizeBytes, 1, nbMcs, padding);
        NS_ASSERT (padding == 0);
        rxPssSched.m_allocatedSubChannels.set (m_ccId, true);
        rxPssSched.m_MCS[m_ccId] = Couwbat::GetDefaultMcs ();
        Ptr<Packet> rxSched = Create<Packet> ();
        Send (rxPssSched, rxSched);
        m_xstate_pss_rx_scheduled[0] = true;
      }
      break;

    default:
      break;
  }

  // SF_START to PHY
  if (m_xstate_pss_rx_scheduled[1] == false && m_xstate_map_rx_scheduled[1] == false)
    {
      // Will not receive PSS or MAP => will not need to schedule anything else in this SF, send SF_START immediately
      SendSfStart (mh);
    }
}

void
StaCouwbatMac::RxOkHandleExtraRx (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

//  NS_LOG_INFO ("CR-STA " << m_address << " RxOkHandleExtraRx() metaheader: " << mh);

  if (FilterPacket (packet)) // TODO xstate not handled
    {
      NS_LOG_DEBUG ("Dropped packet " << packet->ToString());

      if (mh.m_ofdm_sym_offset == 0)
        {
          // Dropped PSS
          NS_ASSERT (m_xstate_pss_rx_scheduled[1] == true);
          if (m_xstate_map_rx_scheduled[1] == false)
            {
              // PSS reception has failed, will not receive MAP => will not need to schedule anything else in this SF, send SF_START
              SendSfStart (mh);
            }
        }

      if (mh.m_ofdm_sym_offset == 54) // TODO hardcoded
        {
          // Dropped MAP
          NS_ASSERT (m_xstate_map_rx_scheduled[1] == true);
          SendSfStart (mh);
          // MAP reception has failed => will not need to schedule anything else in this SF, send SF_START
        }

      return;
    }

  switch (m_state)
  {
    case CR_STA_SCAN:
      {
        CouwbatMacHeader header;
        packet->PeekHeader (header);
        if (header.GetFrameType () == COUWBAT_FC_CONTROL_PSS)
          {
            RxSavePssDetails (mh, packet);
          }

        NS_ASSERT (m_xstate_pss_rx_scheduled[1] == true);
        SendSfStart (mh);
      }
      break;

    case CR_STA_ASSOC:
      {
        CouwbatMacHeader header;
        packet->PeekHeader (header);
        switch (header.GetFrameType ())
        {
          case COUWBAT_FC_CONTROL_PSS:
            RxProcessPss (mh, packet);
            NS_ASSERT (m_xstate_pss_rx_scheduled[1] == true);
            if (m_xstate_map_rx_scheduled[1] == false)
              {
                // Will not receive MAP after this PSS => will not need to schedule anything else in this SF, send SF_START
                SendSfStart (mh);
              }
            break;
          case COUWBAT_FC_CONTROL_MAP:
            RxProcessMap (mh, packet);
            NS_ASSERT (m_xstate_map_rx_scheduled[1] == true);
            // Received MAP => will not need to schedule anything else in this SF, send SF_START
            SendSfStart (mh);
            break;
          default:
            break;
        }
      }
      break;

    case CR_STA_OPERATING:
      {
        CouwbatMacHeader header;
        packet->PeekHeader (header);
        switch (header.GetFrameType ())
        {
          case COUWBAT_FC_CONTROL_PSS:
            RxProcessPss (mh, packet);
            NS_ASSERT (m_xstate_pss_rx_scheduled[1] == true);
            if (m_xstate_map_rx_scheduled[1] == false)
              {
                // Will not receive MAP after this PSS => will not need to schedule anything else in this SF, send SF_START
                SendSfStart (mh);
              }
            break;
          case COUWBAT_FC_CONTROL_MAP:
            RxProcessMap (mh, packet);
            NS_ASSERT (m_xstate_map_rx_scheduled[1] == true);
            // Received MAP => will not need to schedule anything else in this SF, send SF_START
            SendSfStart (mh);
            break;
          case COUWBAT_FC_DATA_DL:
            RxProcessData (mh, packet);
            break;
          default:
            break;
        }
      }
      break;

    default:
      break;
  }
}

void
StaCouwbatMac::RxSavePssDetails (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  ScannedPss sp;
  sp.mh = mh;
  sp.pss = packet;
  m_scannedPss.push_back (sp);
}

void
StaCouwbatMac::RxProcessPss (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  switch (m_state)
  {
    case CR_STA_ASSOC:
      NS_LOG_FUNCTION ("CR-STA " << m_address << " received PSS from BS with which it wants to associate");
      SendAssoc ();
      break;

    case CR_STA_OPERATING:
      NS_LOG_LOGIC ("CR-STA " << m_address << " received PSS from associated BS!");
      break;

    default:
      NS_LOG_ERROR ("StaCouwbatMac::RxProcessPss() executed in incompatible state");
      return;
      break;
  }

  // Reset timeout
  m_pssTimeout = Couwbat::STA_PSS_TIMEOUT_SF;

  CouwbatMacHeader header;
  packet->RemoveHeader (header);
  CouwbatPssHeader pss;
  packet->RemoveHeader (pss);
  uint32_t wbChCnt = 0;
  for (uint32_t i = 0; i < Couwbat::GetNumberOfSubchannels (); ++i)
    {
      if (pss.m_allocation.test (i))
        {
          ++wbChCnt;
        }
    }

  // Update backup CC info
  m_backupCc = pss.m_backupChannel;
  m_useBackupCc = true;

  // schedule next MAP RX
  CouwbatMetaHeader rxMapSched;
  rxMapSched.m_flags = CW_CMD_WIFI_EXTRA_ZERO_RX;
  rxMapSched.m_ofdm_sym_sframe_count = m_sfCnt + 1;
  rxMapSched.m_ofdm_sym_offset = 54; // TODO hardcoded
  NS_LOG_LOGIC ("wbChCnt: " << wbChCnt );
  rxMapSched.m_ofdm_sym_len = pss.m_pssMaintain.fields.mapLength;
  rxMapSched.m_allocatedSubChannels = pss.m_allocation;
  for (uint32_t i = 0; i < Couwbat::GetNumberOfSubchannels (); ++i)
    {
      if (pss.m_allocation.test (i))
        {
          rxMapSched.m_MCS[i] = Couwbat::GetDefaultMcs ();
        }
    }
  Ptr<Packet> rxSched = Create<Packet> ();
  Send (rxMapSched, rxSched);
  m_xstate_map_rx_scheduled[0] = true;
}

void
StaCouwbatMac::RxProcessMap (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
  {
    case CR_STA_ASSOC:
      NS_LOG_LOGIC ("CR-STA " << m_address << " received MAP from BS with which it wants to associate");
      break;

    case CR_STA_OPERATING:
      NS_LOG_LOGIC ("CR-STA " << m_address << " received MAP from associated BS!");
      break;

    default:
      NS_LOG_ERROR ("StaCouwbatMac::RxProcessMap() executed in incompatible state");
      return;
      break;
  }

  // Extract DL and UL subpackets
  std::vector<Ptr<Packet> > mapDl;
  std::vector<Ptr<Packet> > mapUl;
  bool mapOkay = CouwbatPacketHelper::GetMapSubpackets (packet, mapDl, mapUl); // TODO Handle map errors

  if (!mapOkay)
    {
      NS_LOG_INFO ("CR-STA " << m_address << " MAP is faulty, cannot schedule RX/TX");
      return;
    }

  // Iterate over DL subpackets
  bool found_us = false;
  for (uint32_t dlIndex = 0; dlIndex < mapDl.size (); ++dlIndex)
    {
      CouwbatMapSubpacket dlMapSubp;
      mapDl[dlIndex]->RemoveHeader (dlMapSubp);
      if (dlMapSubp.m_ie_id != m_address)
        {
          // Not for us
          continue;
        }

      found_us = true;

      if (dlMapSubp.m_ofdm_count == 0)
        {
          continue;
        }

      uint8_t dlMcs[Couwbat::MAX_SUBCHANS];
      dlMapSubp.GetMcs (dlMcs);

      // Schedule RX of DL data
      CouwbatMetaHeader rxDownlinkDataSched;
      rxDownlinkDataSched.m_flags = CW_CMD_WIFI_EXTRA_ZERO_RX;
      rxDownlinkDataSched.m_ofdm_sym_sframe_count = m_sfCnt + 1;
      rxDownlinkDataSched.m_ofdm_sym_offset = dlMapSubp.m_ofdm_offset;
      rxDownlinkDataSched.m_ofdm_sym_len = dlMapSubp.m_ofdm_count;
      rxDownlinkDataSched.m_allocatedSubChannels = mh.m_allocatedSubChannels;
      std::copy (dlMcs, dlMcs + Couwbat::MAX_SUBCHANS, rxDownlinkDataSched.m_MCS);
      Ptr<Packet> rxSched = Create<Packet> ();
      Send (rxDownlinkDataSched, rxSched);
    }

  NS_LOG_DEBUG ("Before StaCouwbatMac::RxProcessMap: TxQueueSize=" << GetTxQueueSize(m_currentBsAddr));

  // Iterate over UL subpackets
  uint32_t ulIndex = 0;
  for (uint32_t i = 0; i < mapUl.size (); ++i)
    {
      CouwbatMapSubpacket ulMapSubp;
      mapUl[i]->RemoveHeader (ulMapSubp);
      if (ulMapSubp.m_ie_id != m_address)
        {
          // Not for us
          continue;
        }

      found_us = true;

      // Create UL data packet and schedule TX of UL data
      uint8_t ulMcs[Couwbat::MAX_SUBCHANS];
      ulMapSubp.GetMcs (ulMcs);

      uint16_t ack;
      if (ulIndex < m_lastSeq.size ())
        {
          ack = m_lastSeq[ulIndex];
        }
      else // Slot count has increased
        {
          ack = 0; // TODO What value to put here? Maybe needs a reserved SEQ number for "not an ACK"
        }

      uint32_t subchCount = 0;
      for (uint32_t i = 0; i < Couwbat::MAX_SUBCHANS; ++i)
        {
          if (mh.m_allocatedSubChannels.test(i))
            {
              ++subchCount;
            }
        }

      std::vector<CouwbatMCS> ulMcsVector;
      for (uint32_t i = 0; i < Couwbat::MAX_SUBCHANS; ++i)
        {
          if (ulMcs[i] != COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE)
            {
              ulMcsVector.push_back ((CouwbatMCS) ulMcs[i]);
            }
        }
      NS_ASSERT (ulMcsVector.size () == subchCount);

      uint32_t maxSizeBytes = TransmittableBytesWithSymbols (ulMapSubp.m_ofdm_count, subchCount, ulMcsVector);
      NS_ASSERT (maxSizeBytes == floor (maxSizeBytes));

      std::vector<uint8_t> noCqi = std::vector<uint8_t> (Couwbat::GetNumberOfSubchannels (), 255); // max CQI value used as CQI N/A
      std::vector<uint8_t> &cqi = (ulIndex < m_lastCqi.size ()) ? m_lastCqi[ulIndex] : noCqi;

      std::vector<Ptr<Packet> > dummyHistory;
      Ptr<Packet> ulPack = CouwbatPacketHelper::CreateUlDataPacket(
          m_address, m_currentBsAddr, m_seq, m_txQueue, maxSizeBytes,
          ack, cqi, dummyHistory
          // txHistory retransmission disabled
          //*m_txHistory.GetNewList (m_currentBsAddr, m_seq)
          );
      ++m_seq;
      ++ulIndex;

      if (!ulPack)
        {
          NS_LOG_WARN ("Uplink packet creation failed, skipping transmission in this uplink slot.");
          continue;
        }

      NS_LOG_DEBUG ("UL padding=" << maxSizeBytes - ulPack->GetSize ());
      int paddingSize = maxSizeBytes - ulPack->GetSize ();

      // Add real padding
      uint8_t *buf = new uint8_t[paddingSize];
      Ptr<Packet> padPacket = Create<Packet> (buf, paddingSize);
      delete[] buf;
      ulPack->AddAtEnd (padPacket);

      double padding;
      CouwbatMetaHeader ulMetaheader;
      ulMetaheader.m_flags = CW_CMD_WIFI_EXTRA_TX;
      ulMetaheader.m_ofdm_sym_sframe_count = m_sfCnt + 1;
      ulMetaheader.m_ofdm_sym_offset = ulMapSubp.m_ofdm_offset;
      ulMetaheader.m_allocatedSubChannels = mh.m_allocatedSubChannels;
      std::copy (ulMcs, ulMcs + Couwbat::MAX_SUBCHANS, ulMetaheader.m_MCS);
      ulMetaheader.m_ofdm_sym_len = NecessarySymbolsForBytes(ulPack->GetSize (), subchCount, ulMcsVector, padding);
      NS_ASSERT (padding == 0);
      NS_LOG_DEBUG ("CR-STA " << m_address  << " scheduling UL data packet:");
      NS_LOG_DEBUG ("(" << ulMetaheader << ") " << ulPack->ToString ());
      Send (ulMetaheader, ulPack);
    }

  NS_LOG_DEBUG ("After StaCouwbatMac::RxProcessMap: TxQueueSize=" << GetTxQueueSize(m_currentBsAddr));

  // reset saved CQI and SEQ from last superframe
  m_lastCqi.clear ();
  m_lastSeq.clear ();

  if (m_state == CR_STA_ASSOC)
    {
      if (!found_us)
        {
          NS_LOG_LOGIC ("CR-STA " << m_address << " no assoc confirmation in MAP");
        }
      else
        {
          // We are in map
          m_associated = true;
          SetState (CR_STA_OPERATING);
          NS_LOG_INFO ("CR-STA " << m_address  << " has received confirmation for the association and is now associated");
        }
    }

}

void
StaCouwbatMac::RxProcessData (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  NS_LOG_INFO ("CR-STA " << m_address
     << " has received a data packet (" << packet->GetSize () << " B)");

  std::vector<uint8_t> cqi;
  cqi.assign (mh.m_CQI, mh.m_CQI + Couwbat::MAX_SUBCHANS);
  m_lastCqi.push_back (cqi);

  CouwbatMacHeader couwbatHeader;
  packet->PeekHeader (couwbatHeader);
  m_lastSeq.push_back (couwbatHeader.GetSequence ());

  Couwbat1ByteHeader ack;
  std::vector<Ptr<Packet> > data;
  bool fcsCorrect = CouwbatPacketHelper::GetPayload (packet, data, 0, &ack);
//  NS_LOG_DEBUG ("StaCouwbatMac::RxProcessData() packet ref count: " << packet->GetReferenceCount ());
  if (!fcsCorrect) return;

  // Register ACKed entry
  // txHistory retransmission disabled
//  m_txHistory.RegisterAck (couwbatHeader.GetSource (), ack.GetVal ());

  for (std::vector<Ptr<Packet> >::iterator i = data.begin ();
        i != data.end (); ++i)
    {
      ForwardUp(*i, m_currentBsAddr, m_address);
    }
}

void
StaCouwbatMac::Send (CouwbatMetaHeader mh, Ptr<Packet> packet, std::string message)
{
  NS_LOG_FUNCTION (this);
  if (!message.empty ()) NS_LOG_DEBUG ("CR-STA " << m_address << " send:" << message);
  NS_LOG_LOGIC ("CR-STA " << m_address << " TO PHY: {ns3::CouwbatMetaHeader(" << mh << ") " << packet->ToString () << "}\n");

  if (m_netlinkMode)
    {
      m_cwnlSendCallback (mh, packet);
    }
  else
    {
      packet->AddHeader (mh);
      m_phy->SendPacket (packet);
    }
}

void
StaCouwbatMac::Enqueue (Ptr<Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this);
  m_txQueue.Enqueue (to, packet);
}

int
StaCouwbatMac::GetTxQueueSize (const Mac48Address dest)
{
  return m_txQueue.GetQueueSize (dest);
}

Callback<void,Ptr<Packet> >
StaCouwbatMac::GetRxOkCallback ()
{
  NS_LOG_FUNCTION (this);
  return m_rxOkCallback;
}

void
StaCouwbatMac::SetCwnlSendCallback (Callback<int, CouwbatMetaHeader, Ptr<Packet> > cb)
{
  NS_LOG_FUNCTION (this);
  m_cwnlSendCallback = cb;
}

} // namespace ns3

