#include "bs-couwbat-mac.h"
#include "spectrum-manager.h"
#include "couwbat.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include <algorithm>
#include <numeric>

NS_LOG_COMPONENT_DEFINE ("BsCouwbatMac");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (BsCouwbatMac);

TypeId
BsCouwbatMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BsCouwbatMac")

    .SetParent<CouwbatMac> ()

    .AddConstructor<BsCouwbatMac> ()

    .SetGroupName ("Couwbat")
  ;
  return tid;
}

BsCouwbatMac::BsCouwbatMac (void)
: m_ccSelected (false)
{
  NS_LOG_FUNCTION (this);

  m_rxOkCallback = MakeCallback (&BsCouwbatMac::RxOk, this);
}

BsCouwbatMac::~BsCouwbatMac (void)
{
  NS_LOG_FUNCTION (this);
  DoDispose ();
}

bool
BsCouwbatMac::IsLinkUp (void) const
{
  return true;
}

void
BsCouwbatMac::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  if (m_netlinkMode)
    {
      NS_LOG_DEBUG("BsCouwbatMac running in netlink mode");
    }
  else
    {
      m_specManager = GetDevice ()->GetObject<SpectrumManager> ();
      if (!m_specManager)
        {
          NS_FATAL_ERROR ("Initialising base station without a spectrum manager!");
        }

      if (!m_phy)
        {
          NS_FATAL_ERROR ("Initialising base station without a PHY!");
        }

      m_phy->SetReceiveOkCallback (m_rxOkCallback);
  }

  // Initialise constant packet sizes
  Ptr<Packet> pss = Create<Packet> ();
  CouwbatPssHeader pssHeader;
  pss->AddHeader (pssHeader);
  CouwbatPacketHelper::CreateCouwbatControlPacket(pss, COUWBAT_FC_CONTROL_PSS, m_address, Mac48Address("ff:ff:ff:ff:ff:ff"), 0);
  m_pssSizeBytes = pss->GetSize ();

  Ptr<Packet> assoc = CouwbatPacketHelper::CreateEmpty (
      COUWBAT_FC_CONTROL_STA_ASSOC,
      m_address,
      m_address,
      0);
  m_assocSizeBytes = assoc->GetSize ();

  m_ccSelected = false;

  // TODO use different sequences of SEQ numbers for different STAs
  m_seq = std::rand ();

  m_restoreBackupCcTempStasTimeout = -1;

}

bool
BsCouwbatMac::SelectNewCcNoBackup (void)
{
  NS_LOG_FUNCTION (this);
  m_allocatedNbSubChannels[0].reset ();
  bool selected = false;
  for (uint32_t ccId = 0; ccId < Couwbat::GetNumberOfSubchannels (); ++ccId)
    {
      if (m_specManager->IsCcFree (ccId))
        {
          m_allocatedNbSubChannels[0].set (ccId, true);
          m_ccId[0] = ccId;
          m_ccSelected = true;
          selected = true;
          break;
        }
    }

  if (!selected)
    {
      // Couldn't find a free CC
      return false;
    }

  SelectBackupCc ();

  return true;
}

void BsCouwbatMac::SelectBackupCc (void)
{
  // Select backup CC
  m_backupCc.fields.chSwitchActive = false;
  for (uint32_t ccId = 0; ccId < Couwbat::GetNumberOfSubchannels (); ++ccId)
    {
      if (m_specManager->IsCcFree (ccId) && ccId != m_ccId[0])
        {
          // Found backup CC
          m_backupCc.fields.newChNumber = ccId;
          NS_LOG_INFO ("CR-BS " << m_address << " selected new backup CC: " << ccId);
          return;
        }
    }

  // Backup CC not found, set to active CC
  m_backupCc.fields.newChNumber = m_ccId[0];
}

bool
BsCouwbatMac::UpdateWidebandDetails (const CouwbatMetaHeader &mh)
{
  NS_LOG_FUNCTION (this);
  m_allocatedWbSubChannels[0].reset ();
  int count = 0;
  for (uint32_t ccId = 0; ccId < Couwbat::GetNumberOfSubchannels (); ++ccId)
    {
      if (m_specManager->IsCcFree (ccId))
        {
          // Avoid low CQI wideband subchannels according to STA feedback during CR-BS wideband channel selection per superframe
          bool avoid = false;

          if (Couwbat::mac_avoid_low_cqi_wb_subchannels) // Only do this if functionality is enabled
            {
              int entriesRead = 0;
              int entriesAgainst = 0;
              std::vector<uint8_t> cqis;

              for (unsigned int i = 0; i < m_associatedStas[0].size (); ++i)
                {
                  // Iterate over all associated STAs
                  Mac48Address addr = m_associatedStas[0][i];

                  staCqiHist_t &hi = m_cqiHist[addr];
                  for (unsigned int j = 0; j < hi.size (); ++j)
                    {
                      // Iterate over entries from the STA received in last superframe
                      cqiHistEntry_t &entry = hi[j];
                      NS_ASSERT (entry.src == addr);
                      if (entry.sframe_count != (mh.m_ofdm_sym_sframe_count - 1))
                        {
                          // NOTE: Due to insertion order into staCqiHist_t, only older than 1 superframe entries follow. Stop looking at entries.
                          // These older entries could be useful for future enhancements to this algorithm.
                          break;
                        }

                      if (entry.cqi[ccId] == 255)
                        {
                          // Ignore CQI N/As (e.g. due to previously unused subchannel)
                          continue;
                        }

                      ++entriesRead;
                      cqis.push_back (entry.cqi[ccId]);
                      if (entry.cqi[ccId] < Couwbat::mac_below_value_avoid_low_cqi_wb_subchannels)
                        {
                          // Entry is below minimum CQI value. Count as "against this subchannel"
                          ++entriesAgainst;
                        }
                    }
                }

              if (entriesRead > 0)
                {
                  // Percentage of negative feedback
                  double ratio = double(entriesAgainst) / double (entriesRead);

                  // If ratio exceeds the threshold percentage, skip using this subchannel despite it being unoccupied according to SpecDb
                  if (ratio >= Couwbat::mac_against_threshold_avoid_low_cqi_wb_subchannels)
                    {
                      // Print info and list of CQI feedback values that lead to this decision
                      std::ostringstream ss;
                      ss << "Read " << entriesRead << " total CQI feedback entries from STA(s) regarding subchannel "
                                   << ccId << ", entries against: " << entriesAgainst;

                      ss << " => Avoiding unoccupied wideband subchannel " << ccId << " due to negative CQI feedback from STA(s): {";
                      std::copy (cqis.begin(), cqis.end() - 1, std::ostream_iterator<int>(ss, ","));
                      ss << int(cqis.back ());
                      ss << "}";
                      NS_LOG_INFO (ss.str ());

                      avoid = true;
                    }
                }
            }

          if (!avoid)
            {
              m_allocatedWbSubChannels[0].set (ccId, true);
              ++count;
            }
        }
    }
  m_wbSubchannelCnt[0] = count;
  std::string wbSubChannelString = m_allocatedWbSubChannels[0].to_string ();
  std::reverse (wbSubChannelString.begin (), wbSubChannelString.end ());
  NS_LOG_INFO ("CR-BS " << m_address << " using " << count << " wideband subchannels: " << wbSubChannelString);
  return count > 0;
}

bool
BsCouwbatMac::SelectCc (void)
{
  NS_LOG_FUNCTION (this);

  if (m_ccSelected)
    {
      NS_LOG_INFO ("CR-BS " << m_address << ": previous control channel "
                   << m_ccId[0] << " is no longer free");

      // Switch to backup CC if possible
      if (m_specManager->IsCcFree (m_backupCc.fields.newChNumber) && m_associatedStasBackupCcTemp.empty ())
        {
          // Switch to backup because it is available
          m_lastCcChange = Simulator::Now ();
          m_ccId[0] = m_backupCc.fields.newChNumber;
          SelectBackupCc ();
          m_associatedStasBackupCcTemp = m_associatedStas[0];
          m_associatedStas[0].clear ();
          m_restoreBackupCcTempStasTimeout = 4;
          m_allocatedNbSubChannels[0].reset ();
          m_allocatedNbSubChannels[0].set (m_ccId[0], true);

          NS_LOG_INFO ("CR-BS " << m_address
                 << " switching to backup CC " << m_ccId[0]);

          // Selected CC successfully
          return true;
        }
    }

  m_ccSelected = false;

  m_associatedStas[0].clear ();

  // select a free CC
  if (SelectNewCcNoBackup ())
    {
      NS_LOG_INFO ("CR-BS " << m_address
             << " selected new CC (backup CC not usable) " << m_ccId[0]);
      m_lastCcChange = Simulator::Now ();
      m_ccSelected = true;
      m_restoreBackupCcTempStasTimeout = -1;
      m_associatedStasBackupCcTemp.clear ();

      // Selected CC successfully
      return true;
    }

  // Could not select CC
  return false;
}

void
BsCouwbatMac::StartSuperframe (const CouwbatMetaHeader &mh)
{
  NS_LOG_DEBUG ("\n\n\nSUPERFRAME STARTING ### " << mh.m_ofdm_sym_sframe_count << "\n--------------------");
  NS_LOG_FUNCTION (this);

  // Update state history arrays
  // TODO: optimise by using linked list instead of array to avoid all the copying?
  m_associatedStas[2] = m_associatedStas[1];
  m_associatedStas[1] = m_associatedStas[0];
  m_pssHistory[2] = m_pssHistory[1];
  m_pssHistory[1] = m_pssHistory[0];
  m_downlinkMapSubpacketHistory[1] = m_downlinkMapSubpacketHistory[0];
  m_downlinkMapSubpacketHistory[0].clear ();
  m_uplinkMapSubpacketHistory[1] = m_uplinkMapSubpacketHistory[0];
  m_uplinkMapSubpacketHistory[0].clear ();
  m_mapPaddingBytes[2] = m_mapPaddingBytes[1];
  m_mapPaddingBytes[1] = m_mapPaddingBytes[0];
  m_mapSizeSymbols[2] = m_mapSizeSymbols[1];
  m_mapSizeSymbols[1] = m_mapSizeSymbols[0];
  m_mapUlDlSlotsPerSta[2] = m_mapUlDlSlotsPerSta[1];
  m_mapUlDlSlotsPerSta[1] = m_mapUlDlSlotsPerSta[0];
  m_ccId[2] = m_ccId[1];
  m_ccId[1] = m_ccId[0];
  m_allocatedNbSubChannels[2] = m_allocatedNbSubChannels[1];
  m_allocatedNbSubChannels[1] = m_allocatedNbSubChannels[0];
  m_allocatedWbSubChannels[2] = m_allocatedWbSubChannels[1];
  m_allocatedWbSubChannels[1] = m_allocatedWbSubChannels[0];
  m_wbSubchannelCnt[2] = m_wbSubchannelCnt[1];
  m_wbSubchannelCnt[1] = m_wbSubchannelCnt[0];
  m_staDataMcs[1] = m_staDataMcs[0];
  m_staDataMcs[0].clear ();
  m_txHistory.SuperframeTick (m_txQueue);

  // Check if CC is free, select new CC or abort superframe if no CC is available
  if (!m_ccSelected || !m_specManager->IsCcFree (m_ccId[0]))
    {
      if (!SelectCc ())
        {
          // No free CC available
          NS_LOG_INFO ("CR-BS " << m_address << " cannot find free subchannel");
          return;
        }
    }

  // Check if backup CC is free and update accordingly
  if (!m_specManager->IsCcFree (m_backupCc.fields.newChNumber))
    {
      SelectBackupCc ();
    }

  // Handle associated STAs
  while (!m_newStas.empty ())
    {
      m_associatedStas[0].push_back (m_newStas.back ());
      m_newStas.pop_back ();
    }

  if (m_restoreBackupCcTempStasTimeout > 0)
    {
      --m_restoreBackupCcTempStasTimeout;
      if (m_restoreBackupCcTempStasTimeout == 0)
        {
          while (!m_associatedStasBackupCcTemp.empty ())
            {
              m_associatedStas[0].push_back (m_associatedStasBackupCcTemp.back ());
              m_associatedStasBackupCcTemp.pop_back ();
            }
          m_restoreBackupCcTempStasTimeout = -1;
        }
    }

  CleanCqiHist (mh.m_ofdm_sym_sframe_count);

  // Check availability of wideband channels and update accordingly
  if (!UpdateWidebandDetails (mh))
    {
      // No free wideband subchannels available, abort superframe
      NS_LOG_INFO ("CR-BS " << m_address << " cannot find free channels for wideband phase");
      return;
    }


  NS_LOG_LOGIC ("CR-BS " << m_address << " superframe starting");

  // Print extra info
  std::ostringstream ss;
  ss << "CR-BS " << m_address << " current parameters: CC=" << m_ccId[0];
  ss << ", BackupCC=" << m_backupCc.fields.newChNumber;
  ss << ", AssociatedStas={";
  for (size_t i = 0; i < m_associatedStas[0].size (); ++i)
    {
      ss << m_associatedStas[0][i] << ",";
    }
  ss << "}";
  if (!m_netlinkMode)
    {
      ss << ", TimeSinceLastCcChange=" << Simulator::Now () - m_lastCcChange;
    }
  NS_LOG_INFO (ss.str ());

  // Do work

  SendPss (mh);
  RecvAssoc (mh);
  SendMap (mh);
  SendDl (mh);
  RecvUl (mh);
  SendSfStart (mh);
}

void
BsCouwbatMac::SendPss (const CouwbatMetaHeader &mhSfStart)
{
  NS_LOG_LOGIC ("CR-BS " << m_address << " starting PSS transmission on subchannel " << m_ccId[0]);

  unsigned int stas = m_associatedStas[0].size ();
  unsigned int symbWideband = 2446; // TODO should this be hard coded? Maybe add to couwbat.h?

  for (std::vector<Mac48Address>::iterator macIterator = m_associatedStas[0].begin (); macIterator != m_associatedStas[0].end (); ++macIterator)
    {
      // Get optimal MCS values for each subchannel to use in data phase for this STA
      m_staDataMcs[0].push_back (GetOptimalMcs (*macIterator, m_allocatedWbSubChannels[0], mhSfStart));
    }

  BsCouwbatMac::MapLengthRetType ml = GetMapLength (stas, m_wbSubchannelCnt[0], symbWideband, m_staDataMcs[0]);

  m_mapPaddingBytes[0] = ml.mapPaddingBytes;
  m_mapSizeSymbols[0] = ml.mapSymbols;
  m_mapUlDlSlotsPerSta[0] = ml.ulDlSlotsPerSta;

  CouwbatPssHeader pssHeader;
  pssHeader.m_allocation = m_allocatedWbSubChannels[0];
  pssHeader.m_backupChannel = m_backupCc;
  pssHeader.m_pssMaintain.fields.pssFragments = 1;
  pssHeader.m_pssMaintain.fields.mapLength = m_mapSizeSymbols[0];

  Ptr<Packet> pss = Create<Packet> ();
  pss->AddHeader (pssHeader);

  CouwbatPacketHelper::CreateCouwbatControlPacket(pss, COUWBAT_FC_CONTROL_PSS, m_address, Mac48Address("ff:ff:ff:ff:ff:ff"), 0);
  std::vector<CouwbatMCS> pssMcs = std::vector<CouwbatMCS> (1, Couwbat::GetDefaultMcs ());
  double pssPaddingBytes = 0;
  const uint32_t pssTxDurationSymb = NecessarySymbolsForBytes (m_pssSizeBytes, 1, pssMcs, pssPaddingBytes);
  NS_ASSERT (pssPaddingBytes == 0);

  CouwbatMetaHeader mh;
  mh.m_flags = CW_CMD_WIFI_EXTRA_TX;
  mh.m_ofdm_sym_sframe_count = mhSfStart.m_ofdm_sym_sframe_count + 1;
  mh.m_ofdm_sym_offset = 0;
  mh.m_ofdm_sym_len = pssTxDurationSymb;
  mh.m_allocatedSubChannels = m_allocatedNbSubChannels[0];
  mh.m_MCS[m_ccId[0]] = Couwbat::GetDefaultMcs ();

  NS_LOG_DEBUG ("schedule PSS");
  Send (mh, pss);

  m_pssHistory[0] = pssHeader;
}

void
BsCouwbatMac::RecvAssoc (const CouwbatMetaHeader &mhSfStart)
{
  NS_LOG_LOGIC ("CR-BS " << m_address << " scheduling assoc RX");

  std::vector<CouwbatMCS> nbMcs = std::vector<CouwbatMCS> (1, Couwbat::GetDefaultMcs ());
  double padding;
  const uint32_t pssTxDurationSymb = NecessarySymbolsForBytes(m_pssSizeBytes, 1, nbMcs, padding);
  const uint32_t assocTxDurationSymb = NecessarySymbolsForBytes(m_assocSizeBytes, 1, nbMcs, padding);
  NS_ASSERT (padding == 0);
  const uint32_t guard = Couwbat::GetAlohaNrGuardSymbols ();

  CouwbatMetaHeader schedRxMh;
  schedRxMh.m_flags = CW_CMD_WIFI_EXTRA_ZERO_RX;
  schedRxMh.m_ofdm_sym_sframe_count = mhSfStart.m_ofdm_sym_sframe_count + 1;
  schedRxMh.m_ofdm_sym_len = assocTxDurationSymb;
  schedRxMh.m_ofdm_sym_offset = pssTxDurationSymb + Couwbat::GetAlohaNrGuardSymbols ();
  schedRxMh.m_allocatedSubChannels = m_allocatedNbSubChannels[0];
  schedRxMh.m_MCS[m_ccId[0]] = Couwbat::GetDefaultMcs ();

  for (uint32_t i = 0; i < Couwbat::GetContentionSlotCount (); ++i)
    {
      Ptr<Packet> schedRx = Create<Packet> ();
      NS_LOG_DEBUG ("schedule assoc RX");
      Send (schedRxMh, schedRx);

      schedRxMh.m_ofdm_sym_offset += assocTxDurationSymb + guard;
    }
}

void
BsCouwbatMac::SendMap (const CouwbatMetaHeader &mhSfStart)
{
  m_uplinkMapSubpacketHistory[0].clear ();
  m_downlinkMapSubpacketHistory[0].clear ();

  const unsigned int staCount = m_associatedStas[1].size ();
  if (staCount < 1) // no STAs, don't send MAP
    {
      return;
    }

  // Get an MCS vector for narrowband (control) phase with 1 subchannel and default MCS
  const std::vector<CouwbatMCS> narrowbandMcs = std::vector<CouwbatMCS> (1, Couwbat::GetDefaultMcs ());

  double padding;
  // pssTxDurationSymb: 12 = 10 + 2 (preamble)
  const double pssTxDurationSymb = NecessarySymbolsForBytes (m_pssSizeBytes, 1, narrowbandMcs, padding);
  NS_ASSERT (padding == 0); // implies pssTxDurationSymb is whole number

  // singleAssocTxDurationSymb: 8 = 6 + 2 (preamble)
  const double singleAssocTxDurationSymb = NecessarySymbolsForBytes (m_assocSizeBytes, 1, narrowbandMcs, padding);
  NS_ASSERT (padding == 0); // implies singleAssocTxDurationSymb is whole number

  const unsigned int alohaGuardSymb = Couwbat::GetAlohaNrGuardSymbols ();
  const unsigned int widebandBaseOffsetSymb = pssTxDurationSymb + Couwbat::GetContentionSlotCount () * (singleAssocTxDurationSymb + alohaGuardSymb) + alohaGuardSymb;

  // sf_symbols: 2500
  const unsigned int superframeTotalSymb = Couwbat::GetSuperframeDuration() / Couwbat::GetSymbolDuration();

  // total wideband symbols: 2446. TODO inconsistent: hardcoded in SendPss, but calculated here
  const unsigned int widebandTotalSymb = superframeTotalSymb - widebandBaseOffsetSymb;

  const unsigned int widebandGuardSymb = Couwbat::GetSuperframeGuardSymbols ();

  unsigned int mapLengthExtraSymbols = 0;
  if (m_mapSizeSymbols[0] > m_mapSizeSymbols[1] && m_mapSizeSymbols[1] != 0 && m_mapUlDlSlotsPerSta[1] != 0)
    {
      // MAP now takes more symbols to transmit than previously. We need to compensate for this
      // by increasing downlinkOffset and reducing dataPhaseUsableSymb accordingly
      // using mapLengthExtraSymbols
      mapLengthExtraSymbols = m_mapSizeSymbols[0] - m_mapSizeSymbols[1];
    }

  // Subtract: guard time between MAP and data phase + guard time at the end of data phase before next PSS + 1x MAP TX symbol count
  const unsigned int dataPhaseUsableSymb = widebandTotalSymb - (2 * widebandGuardSymb) - m_mapSizeSymbols[1] - mapLengthExtraSymbols;

  const double downlinkSymb = std::floor (Couwbat::GetDataPhaseDownlinkPortion () * dataPhaseUsableSymb);
  const double uplinkSymb = dataPhaseUsableSymb - downlinkSymb;

  // Total number of DL and UL symbols per STA, remove a few to serve as guard
  const double staDownlinkSymb = downlinkSymb / staCount - widebandGuardSymb;
  const double staUplinkSymb = uplinkSymb / staCount - widebandGuardSymb;

  // Number of DL and UL symbols per burst for each STA
  const uint16_t staUplinkSymbPerBurst = staUplinkSymb / m_mapUlDlSlotsPerSta[1];
  const uint16_t staDownlinkSymbPerBurst = staDownlinkSymb / m_mapUlDlSlotsPerSta[1];

  // Offset accumulator variables for the following loop as we create the MAP subpackets
  uint16_t downlinkOffset = widebandBaseOffsetSymb + m_mapSizeSymbols[1] + widebandGuardSymb + mapLengthExtraSymbols;
  uint16_t uplinkOffset = downlinkOffset + downlinkSymb;

  // Save MAP subpackets for each STA here
  std::vector<Ptr<Packet> > downlinkMapSubpackets;
  std::vector<Ptr<Packet> > uplinkMapSubpackets;

  NS_LOG_INFO ("MAP DL/UL slot count: " << m_mapUlDlSlotsPerSta[1]);

  // generate MAP subpackets for each STA and add them to above vectors
  for (std::vector<Mac48Address>::iterator macIterator = m_associatedStas[1].begin (); macIterator != m_associatedStas[1].end (); ++macIterator)
    {
      // Get optimal MCS values for each subchannel to use in data phase for this STA
      uint8_t dataMcs[Couwbat::MAX_SUBCHANS];
      std::vector<CouwbatMCS> &dataMcsVector = m_staDataMcs[1][macIterator - m_associatedStas[1].begin ()];
      unsigned int vec_ind = 0;
      for (unsigned int i = 0; i < Couwbat::MAX_SUBCHANS; ++i)
        {
          if (m_allocatedWbSubChannels[1].test (i))
            {
              dataMcs[i] = dataMcsVector[vec_ind++];
            }
          else
            {
              dataMcs[i] = COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE;
            }
        }

      /*
       * UPLINK
       */
      {
        // TODO guards between bursts from/to same STA not needed?

        for (uint32_t burstIndex = 0; burstIndex < m_mapUlDlSlotsPerSta[1]; ++burstIndex)
          {
            uint16_t uplinkOfdmCount = staUplinkSymbPerBurst;
            double transmittableBytesUplink = TransmittableBytesWithSymbols (uplinkOfdmCount, m_wbSubchannelCnt[1], dataMcsVector);
            while (transmittableBytesUplink != floor (transmittableBytesUplink))
              {
                --uplinkOfdmCount;
                transmittableBytesUplink = TransmittableBytesWithSymbols (uplinkOfdmCount, m_wbSubchannelCnt[1], dataMcsVector);
              }
            NS_ASSERT (uplinkOfdmCount > 0);

            // Uplink map subpacket
            CouwbatMapSubpacket ulHeader;
            ulHeader.m_ie_id = *macIterator;
            ulHeader.m_ofdm_offset = uplinkOffset;
            ulHeader.m_ofdm_count = uplinkOfdmCount;
            NS_LOG_DEBUG ("ulHeader uplinkOffset=" << uplinkOffset << ", uplinkOfdmCount=" << uplinkOfdmCount);
            ulHeader.SetAmc (dataMcs);
            Ptr<Packet> ulSubp = Create<Packet> ();
            ulSubp->AddHeader(ulHeader);
            uplinkMapSubpackets.push_back (ulSubp);

            m_uplinkMapSubpacketHistory[0].push_back (ulHeader);

            uplinkOffset += uplinkOfdmCount;
          }
        uplinkOffset += widebandGuardSymb;
      }

      /*
       * DOWNLINK
       */
      {
        // TODO guards between bursts from/to same STA not needed?

        for (uint32_t burstIndex = 0; burstIndex < m_mapUlDlSlotsPerSta[1]; ++burstIndex)
          {
            uint16_t downlinkOfdmCount = staDownlinkSymbPerBurst;
            double transmittableBytesDownlink = TransmittableBytesWithSymbols (downlinkOfdmCount, m_wbSubchannelCnt[1], dataMcsVector);
            while (transmittableBytesDownlink != floor (transmittableBytesDownlink))
              {
                --downlinkOfdmCount;
                transmittableBytesDownlink = TransmittableBytesWithSymbols (downlinkOfdmCount, m_wbSubchannelCnt[1], dataMcsVector);
              }
            NS_ASSERT (downlinkOfdmCount > 0);

            // Downlink map subpacket
            CouwbatMapSubpacket dlHeader;
            dlHeader.m_ie_id = *macIterator;
            NS_LOG_DEBUG ("dlHeader downlinkOffset: " << downlinkOffset << ", downlinkOfdmCount=" << downlinkOfdmCount);
            dlHeader.m_ofdm_offset = downlinkOffset;
            dlHeader.m_ofdm_count = downlinkOfdmCount;
            dlHeader.SetAmc (dataMcs);
            Ptr<Packet> dlSubp = Create<Packet> ();
            dlSubp->AddHeader(dlHeader);
            downlinkMapSubpackets.push_back (dlSubp);

            m_downlinkMapSubpacketHistory[0].push_back (dlHeader);

            downlinkOffset += dlHeader.m_ofdm_count;
          }

        downlinkOffset += widebandGuardSymb;

      }
    }

  // Create MAP and send
  Ptr<Packet> map = CouwbatPacketHelper::CreateMap (m_address, downlinkMapSubpackets, uplinkMapSubpackets);
  // Add real padding
  uint8_t *buf = new uint8_t[m_mapPaddingBytes[1]];
  Ptr<Packet> padPacket = Create<Packet> (buf, m_mapPaddingBytes[1]);
  delete[] buf;
  map->AddAtEnd (padPacket);

  CouwbatMetaHeader mapTxMh;
  mapTxMh.m_flags = CW_CMD_WIFI_EXTRA_TX;
  mapTxMh.m_ofdm_sym_sframe_count = mhSfStart.m_ofdm_sym_sframe_count + 1;
  mapTxMh.m_ofdm_sym_offset = widebandBaseOffsetSymb;
  mapTxMh.m_ofdm_sym_len = m_mapSizeSymbols[1];
  mapTxMh.m_allocatedSubChannels = m_pssHistory[1].m_allocation;

  double msz = NecessarySymbolsForBytes (map->GetSize (), m_wbSubchannelCnt[1], std::vector<CouwbatMCS> (m_wbSubchannelCnt[1], Couwbat::GetDefaultMcs ()), padding);
  NS_ASSERT_MSG (m_mapSizeSymbols[1] == msz, "m_mapSizeSymbols[1]: " << m_mapSizeSymbols[1] << "; NecessarySymbolsForBytes: " << msz);
  NS_ASSERT (padding == 0);

  for (uint32_t subCh = 0; subCh < Couwbat::GetNumberOfSubchannels (); ++subCh)
    {
      if (m_pssHistory[1].m_allocation.test (subCh)) mapTxMh.m_MCS[subCh] = Couwbat::GetDefaultMcs ();
    }

  NS_LOG_DEBUG ("Created MAP to send (" << msz << " symbols) ");
  Send (mapTxMh, map);
}

void
BsCouwbatMac::SendDl (const CouwbatMetaHeader &mhSfStart)
{
  for (unsigned i = 0; i < m_associatedStas[0].size (); ++i)
    {
      NS_LOG_DEBUG ("Before BsCouwbatMac::SendDl: TxQueueSize(" << m_associatedStas[0][i] << ")=" << GetTxQueueSize(m_associatedStas[0][i]));
    }

  /*
   * send DOWNLINK
   */
  std::map <Mac48Address, uint8_t> nrDlPacketsCreated;
  for (uint32_t dlMapSubpIndex = 0; dlMapSubpIndex < m_downlinkMapSubpacketHistory[1].size (); ++dlMapSubpIndex)
    {
      CouwbatMapSubpacket *subp = &m_downlinkMapSubpacketHistory[1][dlMapSubpIndex];
      uint16_t downlinkOffset = subp->m_ofdm_offset;
      uint16_t downlinkSymbolCount = subp->m_ofdm_count;
      Mac48Address dest = subp->m_ie_id;

      uint8_t mcs[Couwbat::MAX_SUBCHANS];
      subp->GetMcs (mcs);

      std::vector<CouwbatMCS> mcsVector;
      for (uint32_t i = 0; i < Couwbat::MAX_SUBCHANS; ++i)
        {
          if (mcs[i] != COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE)
            {
              mcsVector.push_back ((CouwbatMCS) mcs[i]);
            }
        }
      NS_ASSERT (mcsVector.size () == m_wbSubchannelCnt[2]);

      uint8_t ack;
      if (dlMapSubpIndex < m_lastSeq[dest].size ())
        {
          ack = m_lastSeq[dest][dlMapSubpIndex];
        }
      else
        {
          ack = 0; // TODO What value to put here? Maybe needs a reserved SEQ number for "not an ACK"
        }

      // Data packet
      double maxSizeBytes = TransmittableBytesWithSymbols (downlinkSymbolCount, m_wbSubchannelCnt[2], mcsVector);
      NS_ASSERT (maxSizeBytes == floor (maxSizeBytes));
      std::vector<Ptr<Packet> > dummyHistory;
      Ptr<Packet> dlPack = CouwbatPacketHelper::CreateDlDataPacket(
          m_address,
          dest,
          m_seq,
          m_txQueue,
          maxSizeBytes,
          ack,
          // txHistory retransmission disabled
//          *m_txHistory.GetNewList (dest, m_seq)
          dummyHistory
          );
      ++m_seq;

      if (!dlPack)
        {
          NS_LOG_WARN ("Downlink packet creation failed, skipping transmission in this downlink slot.");
          continue;
        }

      NS_LOG_DEBUG ("DL padding=" << maxSizeBytes - dlPack->GetSize ());
      NS_LOG_DEBUG ("DL pack size=" << dlPack->GetSize ());
      // Add real padding
      int paddingSize = maxSizeBytes - dlPack->GetSize ();
      uint8_t *buf = new uint8_t[paddingSize];
      Ptr<Packet> padPacket = Create<Packet> (buf, paddingSize);
      delete[] buf;
      dlPack->AddAtEnd (padPacket);

      // Downlink data packet TX metaheader
      double padding;
      CouwbatMetaHeader dlMh;
      dlMh.m_flags = CW_CMD_WIFI_EXTRA_TX;
      dlMh.m_ofdm_sym_sframe_count = mhSfStart.m_ofdm_sym_sframe_count + 1;
      dlMh.m_ofdm_sym_offset = downlinkOffset;
      dlMh.m_ofdm_sym_len = NecessarySymbolsForBytes (dlPack->GetSize (), m_wbSubchannelCnt[2], mcsVector, padding);
      NS_ASSERT (padding == 0);
      dlMh.m_allocatedSubChannels = m_pssHistory[2].m_allocation;
      std::copy (mcs, mcs + Couwbat::MAX_SUBCHANS, dlMh.m_MCS);

      NS_LOG_DEBUG ("DATA downlinkDataPackets TX scheduling:");
      NS_LOG_DEBUG ("(" << dlMh << ") " << dlPack->ToString ());
      Send (dlMh, dlPack);
    }
  m_lastSeq.clear ();

  for (unsigned i = 0; i < m_associatedStas[0].size (); ++i)
    {
      NS_LOG_DEBUG ("After BsCouwbatMac::SendDl: TxQueueSize(" << m_associatedStas[0][i] << ")=" << GetTxQueueSize(m_associatedStas[0][i]));
    }
}

void
BsCouwbatMac::RecvUl (const CouwbatMetaHeader &mhSfStart)
{
  /*
   * schedule UPLINK receipt
   */
  for (uint32_t ulMapSubpIndex = 0; ulMapSubpIndex < m_uplinkMapSubpacketHistory[1].size (); ++ulMapSubpIndex)
    {
      CouwbatMapSubpacket *subp = &m_uplinkMapSubpacketHistory[1][ulMapSubpIndex];
      uint16_t uplinkOffset = subp->m_ofdm_offset;
      uint16_t uplinkSymbolCount = subp->m_ofdm_count;

      uint8_t mcs[Couwbat::MAX_SUBCHANS];
      subp->GetMcs (mcs);

      // Uplink RX metaheader
      CouwbatMetaHeader ulMh;
      ulMh.m_flags = CW_CMD_WIFI_EXTRA_ZERO_RX;
      ulMh.m_ofdm_sym_sframe_count = mhSfStart.m_ofdm_sym_sframe_count + 1;
      ulMh.m_ofdm_sym_offset = uplinkOffset;
      ulMh.m_ofdm_sym_len = uplinkSymbolCount;
      ulMh.m_allocatedSubChannels = m_pssHistory[2].m_allocation;
      std::copy (mcs, mcs + Couwbat::MAX_SUBCHANS, ulMh.m_MCS);

      Ptr<Packet> rxSched = Create<Packet> ();
      NS_LOG_DEBUG ("DATA uplinkMetaheaders RX scheduling:");
      Send (ulMh, rxSched);

    }
}

void
BsCouwbatMac::SendSfStart (const CouwbatMetaHeader &mh)
{
  NS_LOG_INFO ("CR-BS " << m_address << " sending SF_START: scheduling in superframe " << mh.m_ofdm_sym_sframe_count << " finished");

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
BsCouwbatMac::CheckControlPacket (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  // Check if control packet (fixed length, no padding) is valid
  CouwbatFcsTrailer trailer;
  if (packet->GetSize () < trailer.GetTrailerSize ()) return true;
  packet->RemoveTrailer (trailer);
  if (!trailer.CheckFcs (packet, 0))
    {
      NS_LOG_DEBUG ("Filtered control packet " << packet->ToString());
      return true; // FCS doesn't match
    }

  return false;
}

void
BsCouwbatMac::RxOkHandleExtraRx (const CouwbatMetaHeader &mh, Ptr<Packet> packet)
{
//  NS_LOG_INFO ("CR-BS " << m_address << " RxOkHandleExtraRx() metaheader: " << mh);

  // check for correct assoc FCS
  if (mh.m_allocatedSubChannels == m_allocatedNbSubChannels[2])
    {
      if (CheckControlPacket (packet)) return;
    }

  std::string ps = packet->ToString ();
  if (ps != "") NS_LOG_DEBUG("CR-BS " << m_address << " RX: {" << ps << "}");

  // Get header
  CouwbatMacHeader ch;
  if (packet->GetSize () < ch.GetHeaderSize ()) return;
  packet->PeekHeader (ch);
  Mac48Address src = ch.GetSource ();

  if (ch.GetDestination () != m_address
  && ch.GetDestination () != Mac48Address("ff:ff:ff:ff:ff:ff"))
      return; // not for us

  // Receive association requests
  couwbat_frame_t ch_ftype = ch.GetFrameType ();
  switch (ch_ftype)
  {
    case COUWBAT_FC_CONTROL_STA_ASSOC:
      NS_LOG_INFO ("CR-BS " << m_address
       << " received association request from "
       << src);
      AddSta(src);
      break;

    case COUWBAT_FC_DATA_UL:
      {
        NS_LOG_INFO ("CR-BS " << m_address
               << " has received a data packet from "
               << src
               << " (" << packet->GetSize () << " B)");

        CouwbatUlBurstHeader ulHeader;

        std::vector<Ptr<Packet> > data;
        bool fcsCorrect = CouwbatPacketHelper::GetPayload (packet, data, &ulHeader);
//        NS_LOG_DEBUG ("BsCouwbatMac::RxOkHandleExtraRx() packet ref count: " << packet->GetReferenceCount ());
        if (!fcsCorrect) return;

        // Register ACKed entry
        // txHistory retransmission disabled
//        m_txHistory.RegisterAck (src, ulHeader.m_ack);

        m_lastSeq[src].push_back (ch.GetSequence ());

        AddCqiHist (src, ulHeader.m_cqi, mh.m_ofdm_sym_sframe_count);

        for (std::vector<Ptr<Packet> >::iterator i = data.begin ();
              i != data.end (); ++i)
          {
            ForwardUp(*i, src, ch.GetDestination ());
          }
      }
      break;

    default:
      break;
  }
}

void
BsCouwbatMac::RxOkHandleZeroSfStart (const CouwbatMetaHeader &mh, Ptr<Packet> packet)
{
  StartSuperframe (mh);
}

void
BsCouwbatMac::RxOk (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);

  CouwbatMetaHeader mh;
  packet->RemoveHeader (mh);

  uint32_t mh_flag = mh.m_flags;
  switch (mh_flag)
    {
    case CW_CMD_WIFI_EXTRA_ZERO_SF_START:
      RxOkHandleZeroSfStart(mh, packet);
      break;

    case CW_CMD_WIFI_EXTRA_RX:
      RxOkHandleExtraRx(mh, packet);
      break;

    default:
      break;
    }
}

void
BsCouwbatMac::AddSta (Mac48Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  // Don't add if already present
  if (std::find (m_associatedStas[0].begin (), m_associatedStas[0].end (), addr)
      == m_associatedStas[0].end ())
    {
      m_newStas.push_back (addr);
      NS_LOG_INFO ("CR-BS " << m_address
       << " added STA "
       << addr);
    }
}

void
BsCouwbatMac::Enqueue (Ptr<Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this);
  m_txQueue.Enqueue (to, packet);
}

void
BsCouwbatMac::Send (CouwbatMetaHeader mh, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC ("CR-BS " << m_address << " TO PHY: {ns3::CouwbatMetaHeader(" << mh << ") " << packet->ToString () << "}\n");
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

std::vector<CouwbatMCS>
BsCouwbatMac::GetOptimalMcs (const Mac48Address &dest, std::bitset<Couwbat::MAX_SUBCHANS> allocatedSubchannels, const CouwbatMetaHeader &mh)
{
  NS_LOG_FUNCTION (this);

  std::vector<CouwbatMCS> ret;

  staCqiHist_t &hi = m_cqiHist[dest];

  for (uint32_t ccId = 0; ccId < Couwbat::MAX_SUBCHANS; ++ccId)
    {
      if (allocatedSubchannels.test (ccId))
        {
          // Make an intelligent decision
          if (hi.empty ())
            {
              // No history present, use default MCS
              ret.push_back (Couwbat::GetDefaultMcs ());
            }
          else
            {
              double cqiSum = 0;
              int entriesRead = 0;

              for (unsigned int j = 0; j < hi.size (); ++j)
                {
                  // Iterate over CQI entries

                  cqiHistEntry_t &entry = hi[j];
                  NS_ASSERT (entry.src == dest);
                  // Only look at CQI entries received in last superframe
                  if (entry.sframe_count != (mh.m_ofdm_sym_sframe_count - 1))
                    {
                      // NOTE: Due to insertion order into staCqiHist_t, only older than 1 superframe entries follow. Stop looking at entries.
                      // These older entries could be useful for future enhancements to this algorithm.
                      break;
                    }

                  if (entry.cqi[ccId] == 255)
                    {
                      // Ignore CQI N/As (e.g. due to previously unused subchannel)
                      continue;
                    }

                  ++entriesRead;
                  cqiSum += entry.cqi[ccId];
                }

              double avgCqi = 0;
              if (entriesRead > 0)
                {
                  avgCqi = cqiSum / entriesRead;
                }

              // SNR levels roughly correspond to 802.11a/g client MCS taken from:
              // https://dl.dropboxusercontent.com/u/8644251/Revolution%20Wi-Fi%20MCS%20to%20SNR.pdf

              CouwbatMCS m;
              if (avgCqi >= 22)
                {
                  m = COUWBAT_MCS_64QAM_3_4;
                }
              else if (avgCqi >= 20)
                {
                  m = COUWBAT_MCS_64QAM_2_3;
                }
              else if (avgCqi >= 18)
                {
                  m = COUWBAT_MCS_64QAM_1_2;
                }
              else if (avgCqi >= 14)
                {
                  m = COUWBAT_MCS_16QAM_3_4;
                }
              else if (avgCqi >= 9)
                {
                  m = COUWBAT_MCS_16QAM_1_2;
                }
              else if (avgCqi >= 6)
                {
                  m = COUWBAT_MCS_QPSK_3_4;
                }
             else if (avgCqi >= Couwbat::mac_below_value_avoid_low_cqi_wb_subchannels)
                {
                  m = Couwbat::GetDefaultMcs ();
                }
             else
               {
                 NS_LOG_INFO ("STA " << dest << " combined CQI for subchannel " << ccId << " is below the"
                     " minimum acceptable value. But it has been decided to use this subchannel despite that."
                     " Using default, most reliable MCS for the STA in this case");
                 m = Couwbat::GetDefaultMcs ();
               }

              ret.push_back (m);
            }
        }
    }

  return ret;
}

/*
 * GetMapLength tries to get the number of slots for the maximum length bursts
 */
BsCouwbatMac::MapLengthRetType
BsCouwbatMac::GetMapLength (unsigned int stas, unsigned int subchannels, unsigned int symbWideband, const std::vector<std::vector<CouwbatMCS> > &dataMcs)
{
  NS_ASSERT (dataMcs.size () == stas);

  // Set starting/base values
  int minMapBytes = 18 + stas * 2 * 34 + 2; // for 1 DL/UL burst per STA
  std::vector<CouwbatMCS> mapMcs (subchannels, Couwbat::GetDefaultMcs());
  double mapPadding = 0;
  double mapSymbols = std::ceil (NecessarySymbolsForBytes (minMapBytes, subchannels, mapMcs, mapPadding));
  int dlOverheadBytes = 20;
  int ulOverheadBytes = 84;

  double alloc = 0; // Total number of symbols allocated for wideband data transfer
  double totalSymbPerDlUlSlot = 0; // Number of symbols needed for one pair of DL and UL slots for each STA
  
  // Ensure that there is at least 1 DL/UL slot; if not enough bandwidth, reduce number of assumed payloads
  int triesCount = 0;
  do
    {
      int payloadSizeBytes = 1518; // Assumed payload size
      int delimiterOverheadBytes = 4; // Couwbat overhead per payload (from MPDU delimiter)
      
      // Reduce number of packets after each try
      int downlinkPayloadCount = 16 - triesCount;
      int uplinkPayloadCount = 4 - (triesCount / 4);
      
      if (downlinkPayloadCount <= 0 || uplinkPayloadCount <= 0)
        {
          alloc = symbWideband + 1; // set error state value
          break;
        }
      
      // Total size of each Couwbat data frame assuming target payload sizes and counts (max burst length)
      // with ratio 0.8 / 0.2
      int downlinkFrameSizeBytes = dlOverheadBytes + (payloadSizeBytes + delimiterOverheadBytes) * downlinkPayloadCount;
      int uplinkFrameSizeBytes = ulOverheadBytes + (payloadSizeBytes + delimiterOverheadBytes) * uplinkPayloadCount;
      
      // Need to calculate the number of symbols for each STA separately due to possibly different MCS between STAs
      std::vector<double> symbPerDlVector; // Number of symbols per DL frame, one entry for each STA
      std::vector<double> symbPerUlVector; // Number of symbols per UL frame, one entry for each STA
      for (unsigned int i = 0; i < dataMcs.size (); ++i)
        {
          NS_ASSERT (dataMcs[i].size () == subchannels);
          
          double padding;
          
          double d = std::ceil (NecessarySymbolsForBytes (downlinkFrameSizeBytes, subchannels, dataMcs[i], padding));
          symbPerDlVector.push_back (d);
          
          double u = std::ceil (NecessarySymbolsForBytes (uplinkFrameSizeBytes, subchannels, dataMcs[i], padding));
          symbPerUlVector.push_back (u);
        }
      double symbPerDlVectorSum = std::accumulate (symbPerDlVector.begin(), symbPerDlVector.end (), 0);
      double symbPerUlVectorSum = std::accumulate (symbPerUlVector.begin(), symbPerUlVector.end (), 0);
      totalSymbPerDlUlSlot = symbPerDlVectorSum + symbPerUlVectorSum;

      alloc = mapSymbols + totalSymbPerDlUlSlot;
      ++triesCount;
    }
  while (alloc >= symbWideband);
  
  /**
   * Number of DL/UL slots per STA
   * 1 if above attemp was successful, the normal case, bandwidth for at least 1 DL and UL slot for each STA
   * 0 if above attempt failed, no data transmission possible, bandwidth too low
   */
  unsigned int ulDlSlotsPerSta = (alloc >= symbWideband) ? 0 : 1;
  
  // Try to add as many as possible additional slots using the above parameters
  if (stas > 0 && ulDlSlotsPerSta >= 1)
    {
      // Keep trying to add slots until total number of available symbols in superframe is exceeded by allocated symbols for data transmission
      while (alloc < symbWideband)
        {
          unsigned int ulDlSlotsPerStaNew = ulDlSlotsPerSta + 1;

          if (ulDlSlotsPerStaNew > Couwbat::mac_dlul_slot_limit_size) break; // Limit max slots for purposes of testing and reducing output to terminal

          unsigned int minMapBytesNew = 18 + stas * 2 * 34 * ulDlSlotsPerStaNew + 2;
          double mapPaddingNew;
          double mapSymbolsNew = std::ceil (NecessarySymbolsForBytes (minMapBytesNew, subchannels, mapMcs, mapPaddingNew));
          double allocNew = mapSymbolsNew + totalSymbPerDlUlSlot * ulDlSlotsPerStaNew;

          if (allocNew < symbWideband)
            {
              minMapBytes = minMapBytesNew;
              mapSymbols = mapSymbolsNew;
              alloc = allocNew;
              mapPadding = mapPaddingNew;
              ++ulDlSlotsPerSta;
            }
          else
            {
              break;
            }
        }
    }

  double wasted = (1.0 - (mapSymbols + totalSymbPerDlUlSlot * ulDlSlotsPerSta) / symbWideband) * 100.0;

  NS_ASSERT (floor (mapPadding) == mapPadding);
  mapSymbols = std::ceil (mapSymbols);

  BsCouwbatMac::MapLengthRetType ret;
  ret.ulDlSlotsPerSta = ulDlSlotsPerSta;
  ret.wasted = wasted;
  ret.mapSymbols = mapSymbols;
  ret.mapSizeBytes = minMapBytes;
  ret.mapPaddingBytes = mapPadding;
  return ret;
}

void
BsCouwbatMac::AddCqiHist (Mac48Address source, uint8_t cqi[], uint32_t sframe_count)
{
  NS_LOG_FUNCTION (this << source << sframe_count);

  bool allBlank = true;
  for (unsigned int i = 0; i < Couwbat::GetNumberOfSubchannels(); ++i)
    {
      if (cqi[i] != 255)
        {
          allBlank = false;
          break;
        }
    }

  if (allBlank)
    {
      return;
    }

  // Add an entry to the CQI history

  std::ostringstream ss;
  ss << "CR-BS " << m_address << " adding CQI from " << source
      << " sframe " << sframe_count
      << " to history, cqi[]={" << (int) cqi[0];
  for (unsigned int i = 1; i < Couwbat::MAX_SUBCHANS; ++i)
    {
      ss << "," << (int) cqi[i];
    }
  ss << "}";
  NS_LOG_INFO (ss.str ());

  cqiHistEntry_t entry;
  entry.src = source;
  entry.sframe_count = sframe_count;
  entry.cqi.assign (cqi, cqi + Couwbat::MAX_SUBCHANS);

  staCqiHist_t &hi = m_cqiHist[source];
  hi.push_front (entry);
}

int
BsCouwbatMac::GetTxQueueSize (const Mac48Address dest)
{
  return m_txQueue.GetQueueSize (dest);
}

void
BsCouwbatMac::SetSpectrumManager (Ptr<SpectrumManager> sm)
{
  NS_LOG_FUNCTION (this << sm);
  m_specManager = sm;
}

Callback<void,Ptr<Packet> >
BsCouwbatMac::GetRxOkCallback ()
{
  NS_LOG_FUNCTION (this);
  return m_rxOkCallback;
}

void
BsCouwbatMac::SetCwnlSendCallback (Callback<int, CouwbatMetaHeader, Ptr<Packet> > cb)
{
  NS_LOG_FUNCTION (this);
  m_cwnlSendCallback = cb;
}

void
BsCouwbatMac::CleanCqiHist (uint32_t sframe_count)
{
  NS_LOG_FUNCTION (this << sframe_count);

  // Delete older CQI history entries

  static const unsigned int sfCutoffDifference = 3;

  typedef std::map<Mac48Address, staCqiHist_t>::iterator it_type;
  for (it_type iterator = m_cqiHist.begin(); iterator != m_cqiHist.end(); ++iterator)
    {
      // iterator->first == key (Mac48Address)
      // iterator->second == value: (staCqiHist_t)

      staCqiHist_t &hi = iterator->second;

      if (std::find (m_associatedStas[0].begin (), m_associatedStas[0].end (), iterator->first)
            == m_associatedStas[0].end ())
        {
          // STA no longer associated, delete all history
          hi.clear ();
          continue;
        }

      // STA is still present and associated
      for (staCqiHist_t::iterator it2 = hi.begin (); it2 != hi.end (); )
        {
          cqiHistEntry_t &entry = *it2;
          if (entry.sframe_count < (sframe_count - sfCutoffDifference)) // TODO sframe_count wraparound would cause a bug here
            {
              it2 = hi.erase (it2);
            }
          else
            {
              ++it2;
            }
        }

      NS_LOG_LOGIC ("staCqiHist_t size for " << iterator->first << " after erase: " << hi.size ());
    }
}

} // namespace ns3
