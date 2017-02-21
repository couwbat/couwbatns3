#include "simple-couwbat-phy.h"
#include "simple-couwbat-channel.h"
#include "couwbat-mode.h"
#include "couwbat-phy-state-helper.h"
#include "couwbat-err-rate-model.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/enum.h"
#include "ns3/pointer.h"
#include "ns3/net-device.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/boolean.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <numeric>
#include "couwbat-packet-helper.h" // for printing of std::vector<double>
#include "couwbat.h"
#include "couwbat-net-device.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("SimpleCouwbatPhy");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (SimpleCouwbatPhy);

TypeId
SimpleCouwbatPhy::GetTypeId (void)
{

  static TypeId tid = TypeId ("ns3::SimpleCouwbatPhy")
    .SetParent<CouwbatPhy> ()
    .SetGroupName("Couwbat")
    .AddConstructor<SimpleCouwbatPhy> ()
    .AddAttribute ("EnergyDetectionThreshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to detect the signal.",
                   DoubleValue (-146.0),
                   MakeDoubleAccessor (&SimpleCouwbatPhy::SetEdThreshold,
                                       &SimpleCouwbatPhy::GetEdThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("CcaMode1Threshold",
                   "The energy of a received signal should be higher than "
                   "this threshold (dbm) to allow the PHY layer to declare CCA BUSY state",
                   DoubleValue (-99.0),
                   MakeDoubleAccessor (&SimpleCouwbatPhy::SetCcaMode1Threshold,
                                       &SimpleCouwbatPhy::GetCcaMode1Threshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxGain",
                   "Transmission gain (dB).",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&SimpleCouwbatPhy::SetTxGain,
                                       &SimpleCouwbatPhy::GetTxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxGain",
                   "Reception gain (dB).",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&SimpleCouwbatPhy::SetRxGain,
                                       &SimpleCouwbatPhy::GetRxGain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerLevels",
                   "Number of transmission power levels available between "
                   "TxPowerStart and TxPowerEnd included.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SimpleCouwbatPhy::m_nTxPower),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("TxPowerEnd",
                   "Maximum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&SimpleCouwbatPhy::SetTxPowerEnd,
                                       &SimpleCouwbatPhy::GetTxPowerEnd),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxPowerStart",
                   "Minimum available transmission level (dbm).",
                   DoubleValue (16.0206),
                   MakeDoubleAccessor (&SimpleCouwbatPhy::SetTxPowerStart,
                                       &SimpleCouwbatPhy::GetTxPowerStart),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxNoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0 (usually 290 K)\"."
                   " For",
                   DoubleValue (7),
                   MakeDoubleAccessor (&SimpleCouwbatPhy::SetRxNoiseFigure,
                                       &SimpleCouwbatPhy::GetRxNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("State", "The state of the PHY layer",
                   PointerValue (),
                   MakePointerAccessor (&SimpleCouwbatPhy::m_state),
                   MakePointerChecker<CouwbatPhyStateHelper> ())
    .AddAttribute ("Frequency", "The operating frequency.",
                   UintegerValue (2407),
                   MakeUintegerAccessor (&SimpleCouwbatPhy::GetFrequency,
                                        &SimpleCouwbatPhy::SetFrequency),
                   MakeUintegerChecker<uint32_t> ())
   ;
  return tid;
}

SimpleCouwbatPhy::SimpleCouwbatPhy ()
  :  m_endRxEvent (),
     m_channelStartingFrequency (0)
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
  m_state = CreateObject<CouwbatPhyStateHelper> ();
  m_state->SetPhy (this);
  for (uint32_t i = 0; i < Couwbat::GetNumberOfSubchannels (); ++i)
    {
      m_interference.push_back (CreateObject<CouwbatInterferenceHelper> ());
    }
  NS_ASSERT (m_interference.size () == Couwbat::GetNumberOfSubchannels ());
  m_sfCnt = 0;
}

SimpleCouwbatPhy::~SimpleCouwbatPhy ()
{
  NS_LOG_FUNCTION (this);
}


void
SimpleCouwbatPhy::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
//  m_deviceRateSet.clear ();
  m_deviceMcsSet.clear();
  m_device = 0;
  m_mobility = 0;
  m_state = 0;
  for (uint32_t i = 0; i < m_interference.size (); ++i)
    {
      m_interference[i] = 0;
    }
}

//void
//SimpleCouwbatPhy::ConfigureStandard ()
//{
//  NS_LOG_FUNCTION (this);
//  m_channelStartingFrequency = 5e3; // 5.000 GHz
//
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate6Mbps ());
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate9Mbps ());
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate12Mbps ());
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate18Mbps ());
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate24Mbps ());
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate36Mbps ());
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate48Mbps ());
////  m_deviceRateSet.push_back (WifiPhy::GetOfdmRate54Mbps ());
//}


void
SimpleCouwbatPhy::SetRxNoiseFigure (double noiseFigureDb)
{
  NS_LOG_FUNCTION (this << noiseFigureDb);
  for (uint32_t i = 0; i < m_interference.size (); ++i)
    {
      NS_ASSERT (m_interference[i] != 0);
      m_interference[i]->SetNoiseFigure (DbToRatio (noiseFigureDb));
    }
}
void
SimpleCouwbatPhy::SetTxPowerStart (double start)
{
  NS_LOG_FUNCTION (this << start);
  m_txPowerBaseDbm = start;
}
void
SimpleCouwbatPhy::SetTxPowerEnd (double end)
{
  NS_LOG_FUNCTION (this << end);
  m_txPowerEndDbm = end;
}
void
SimpleCouwbatPhy::SetNTxPower (uint32_t n)
{
  NS_LOG_FUNCTION (this << n);
  m_nTxPower = n;
}
void
SimpleCouwbatPhy::SetTxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_txGainDb = gain;
}
void
SimpleCouwbatPhy::SetRxGain (double gain)
{
  NS_LOG_FUNCTION (this << gain);
  m_rxGainDb = gain;
}
void
SimpleCouwbatPhy::SetEdThreshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_edThresholdW = DbmToW (threshold);
}
void
SimpleCouwbatPhy::SetCcaMode1Threshold (double threshold)
{
  NS_LOG_FUNCTION (this << threshold);
  m_ccaMode1ThresholdW = DbmToW (threshold);
}
void
SimpleCouwbatPhy::SetErrorRateModel (const std::vector<Ptr<CouwbatErrorRateModel> > &rate)
{
  NS_LOG_FUNCTION (this);
  if (rate.size () != m_interference.size ())
    {
      NS_FATAL_ERROR ("SetErrorRateModel: vector size mismatch");
      return;
    }
  for (uint32_t i = 0; i < m_interference.size (); ++i)
    {
      NS_ASSERT (m_interference[i] != 0);
      m_interference[i]->SetErrorRateModel (rate[i]);
    }
}

void
SimpleCouwbatPhy::SetMobility (Ptr<Object> mobility)
{
  NS_LOG_FUNCTION (this);
  m_mobility = mobility;
}

double
SimpleCouwbatPhy::GetRxNoiseFigure (void) const
{
  NS_ASSERT (m_interference[0] != 0);
  return m_interference[0]->GetNoiseFigure ();
}
double
SimpleCouwbatPhy::GetTxPowerStart (void) const
{
  return m_txPowerBaseDbm;
}
double
SimpleCouwbatPhy::GetTxPowerEnd (void) const
{
  return m_txPowerEndDbm;
}
double
SimpleCouwbatPhy::GetTxGain (void) const
{
  return m_txGainDb;
}
double
SimpleCouwbatPhy::GetRxGain (void) const
{
  return m_rxGainDb;
}

double
SimpleCouwbatPhy::GetEdThreshold (void) const
{
  return WToDbm (m_edThresholdW);
}

double
SimpleCouwbatPhy::GetCcaMode1Threshold (void) const
{
  return WToDbm (m_ccaMode1ThresholdW);
}

std::vector<Ptr<CouwbatErrorRateModel> >
SimpleCouwbatPhy::GetErrorRateModel (void) const
{
  std::vector<Ptr<CouwbatErrorRateModel> > ret;
  for (uint32_t i = 0; i < m_interference.size (); ++i)
    {
      NS_ASSERT (m_interference[i] != 0);
      ret.push_back (m_interference[i]->GetErrorRateModel ());
    }
  return ret;
}

Ptr<Object>
SimpleCouwbatPhy::GetMobility (void)
{
  return m_mobility;
}

// unused function
double
SimpleCouwbatPhy::CalculateSnr (CouwbatMode txMode, double ber) const
{
  NS_LOG_FUNCTION (this);
  std::vector<uint32_t> subchannels = txMode.GetSubchannels ();
  double totalSnr = 0;
  for (uint32_t i = 0; i < subchannels.size (); ++i)
    {
      NS_ASSERT (m_interference[subchannels[i]] != 0);
      totalSnr += m_interference[subchannels[i]]->GetErrorRateModel ()->CalculateSnr (txMode, txMode.GetMCS ()[i], ber);
    }
  return totalSnr / subchannels.size ();
}

Ptr<CouwbatChannel>
SimpleCouwbatPhy::GetChannel (void) const
{
  return m_channel;
}
void
SimpleCouwbatPhy::SetChannel (Ptr<SimpleCouwbatChannel> channel)
{
  m_channel = channel;
  m_channel->Add (this);
}

void
SimpleCouwbatPhy::SetReceiveOkCallback (RxOkCallback callback)
{
  m_state->SetReceiveOkCallback (callback);
  m_rxOkCallback = callback;
}

void
SimpleCouwbatPhy::SetReceiveErrorCallback (RxErrorCallback callback)
{
  m_state->SetReceiveErrorCallback (callback);
  m_rxErrorCallback = callback;
}

void
SimpleCouwbatPhy::StartReceivePacket (Ptr<Packet> packet,
                                 std::vector<double> rxPowerDbm,
                                 CouwbatTxVector txVector)
{
  NS_LOG_LOGIC ("SimpleCouwbatPhy::StartReceivePacket(): " << rxPowerDbm << ", nSubch="<< txVector.GetMode().GetSubchannels ().size());
  NS_LOG_FUNCTION (this << packet << rxPowerDbm << txVector.GetMode().GetSubchannels());
  std::vector<double> rxPowerW;
  for (uint32_t i = 0; i < rxPowerDbm.size (); ++i)
    {
      rxPowerDbm[i] += m_rxGainDb;
      rxPowerW.push_back (DbmToW (rxPowerDbm[i]));
    }
  NS_LOG_LOGIC ("SimpleCouwbatPhy::StartReceivePacket(): rxPowerW="<<rxPowerW<<", rxPowerDbm="<<rxPowerDbm);
  Time rxDuration = CalculateTxDuration (packet->GetSize (), txVector);
  CouwbatMode txMode = txVector.GetMode();
  Time endRx = Simulator::Now () + rxDuration;

  std::vector<Ptr<CouwbatInterferenceHelper::Event> > events;
  std::vector<uint32_t> subchannels = txMode.GetSubchannels ();
  for (uint32_t i = 0; i < subchannels.size (); ++i)
    {
      NS_ASSERT (m_interference[subchannels[i]] != 0);
      NS_LOG_LOGIC ("SimpleCouwbatPhy::StartReceivePacket(): call m_interference["<<subchannels[i]<<"]->Add()");
      Ptr<CouwbatInterferenceHelper::Event> e = m_interference[subchannels[i]]->Add (packet->GetSize (),
                                  txMode,
                                  i,
                                  rxDuration,
                                  rxPowerW[i],
                  txVector);  // we need it to calculate duration of HT training symbols
      events.push_back (e);
    }
  switch (m_state->GetState ())
    {
    case SimpleCouwbatPhy::SWITCHING:
      NS_LOG_INFO ("drop packet because of channel switching");
      NotifyRxDrop (packet);
      /*
       * Packets received on the upcoming channel are added to the event list
       * during the switching state. This way the medium can be correctly sensed
       * when the device listens to the channel for the first time after the
       * switching e.g. after channel switching, the channel may be sensed as
       * busy due to other devices' tramissions started before the end of
       * the switching.
       */
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          // that packet will be noise _after_ the completion of the
          // channel switching.
          //goto maybeCcaBusy;
        }
      break;
    case SimpleCouwbatPhy::RX:
      NS_LOG_INFO ("drop packet because already in Rx (power=" <<
                    rxPowerW << "W)");
      NotifyRxDrop (packet);
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          // that packet will be noise _after_ the reception of the
          // currently-received packet.
          //goto maybeCcaBusy;
        }
      break;
    case SimpleCouwbatPhy::TX:
      NS_LOG_INFO ("drop packet because already in Tx (power=" <<
                    rxPowerW << "W)");
      NotifyRxDrop (packet);
      if (endRx > Simulator::Now () + m_state->GetDelayUntilIdle ())
        {
          // that packet will be noise _after_ the transmission of the
          // currently-transmitted packet.
          //goto maybeCcaBusy;
        }
      break;
    case SimpleCouwbatPhy::IDLE:
      bool cond = true;
      // TODO EnergyDetectionThreshold can be ignored

      for (uint32_t k = 0; k < txVector.GetMode().GetSubchannels().size (); ++k)
        {
          if (rxPowerW[k] < m_edThresholdW)
            {
              cond = false;
              break;
            }
        }

      if (cond)
        {
          NS_LOG_INFO ("sync to signal (power=" << rxPowerW << "W), packet: "
                       << packet->ToString ());
          // sync to signal
          m_state->SwitchToRx (rxDuration);
          NS_ASSERT (m_endRxEvent.IsExpired ());
          NotifyRxBegin (packet);
          for (uint32_t i = 0; i < subchannels.size (); ++i)
            {
              NS_ASSERT (m_interference[subchannels[i]] != 0);
              m_interference[subchannels[i]]->NotifyRxStart ();
            }
          m_endRxEvent = Simulator::Schedule (rxDuration, &SimpleCouwbatPhy::EndReceive, this,
                                              packet,
                                              events);
        }
      else
        {
          NS_LOG_INFO ("drop packet because signal power too Small (" <<
                        rxPowerW << "<" << m_edThresholdW << ")");
          NotifyRxDrop (packet);
        }
      break;
    }

  return;

}

void
SimpleCouwbatPhy::SendPacket (Ptr<Packet> packet)
{
  // dummy values that will be ignored due to presence of metaheader:
  std::vector<uint32_t> subchannels;
  subchannels.push_back (0);
  std::vector<enum CouwbatMCS> mcs;
  mcs.push_back (Couwbat::GetDefaultMcs ());
  CouwbatMode txMode = CouwbatMode (COUWBAT_MOD_CLASS_OFDM, true, subchannels, mcs);
  CouwbatTxVector txVector = CouwbatTxVector (txMode, Couwbat::GetTxPowerBS());
  SendPacketMh (packet, txMode, txVector, true);
}

void
SimpleCouwbatPhy::SendPacketMh (Ptr<Packet> packet, CouwbatMode txMode, CouwbatTxVector txVector, bool metaheaderExists)
{
  NS_LOG_DEBUG ("Packet content " << packet->ToString());

  if (metaheaderExists) {
      CouwbatMetaHeader mh;
      packet->RemoveHeader (mh);

      if (mh.m_ofdm_sym_sframe_count <= m_sfCnt)
        {
          return;
        }

      if (mh.m_flags == CW_CMD_WIFI_EXTRA_TX)
        {
          // convert metaheader to CouwbatMode and CouwbatTxVector format
          std::vector<uint32_t> subchannels;
          std::vector<CouwbatMCS> mcs;
          for (uint32_t i = 0; i < Couwbat::GetNumberOfSubchannels (); ++i)
            {
              if (mh.m_allocatedSubChannels.test(i))
                {
                  subchannels.push_back (i);
                  mcs.push_back ((enum CouwbatMCS)mh.m_MCS[i]);
                }
            }
          CouwbatMode txMode = CouwbatMode (COUWBAT_MOD_CLASS_OFDM, true, subchannels, mcs);
          CouwbatTxVector txVector = CouwbatTxVector (txMode, Couwbat::GetTxPowerBS());


          // Calculate TX time and schedule
          // int times are in microseconds
          static const uint32_t sfDuration = Couwbat::GetSuperframeDuration ();

          const int64_t currentSfStart =
              (Simulator::Now ().GetMicroSeconds () / sfDuration) * sfDuration;

          const int64_t sendTime =
              currentSfStart + (mh.m_ofdm_sym_sframe_count - m_sfCnt) * sfDuration
              + mh.m_ofdm_sym_offset * Couwbat::GetSymbolDuration ();

          Time txDurationTime = CalculateTxDuration (packet->GetSize (), txVector);
          unsigned int txDurationSymbols = (txDurationTime.GetMicroSeconds () + Couwbat::GetSymbolDuration () / 2)
          / Couwbat::GetSymbolDuration ();
          NS_ASSERT (mh.m_ofdm_sym_len == txDurationSymbols);

          Simulator::Schedule (
              MicroSeconds(sendTime) - Simulator::Now(),
              &SimpleCouwbatPhy::SendPacketMh,
              this,
              packet,
              txMode,
              txVector,
              false);
        }

      else if (mh.m_flags == CW_CMD_WIFI_EXTRA_ZERO_RX)
        {
          m_state->EnqueueRx (mh);
        }
      return;
  }

  NS_LOG_FUNCTION (this << packet << txMode << (uint32_t)txVector.GetTxPowerLevel());
  NS_LOG_INFO ("PHY sending packet " << packet);

  /* Transmission can happen if:
   *  - we are syncing on a packet. It is the responsibility of the
   *    MAC layer to avoid doing this but the PHY does nothing to
   *    prevent it.
   *  - we are idle
   */
  NS_ASSERT (!m_state->IsStateTx () && !m_state->IsStateSwitching ());

  Time txDuration = CalculateTxDuration (packet->GetSize (), txVector);

  NS_LOG_LOGIC ("SimpleCouwbatPhy::SendPacket(), tx_duration= " << txDuration << ", size=" << packet->GetSize () << ", subch=" << txMode.GetSubchannels());

  if (m_state->IsStateRx ())
    {
      m_endRxEvent.Cancel ();
      for (uint32_t i = 0; i < m_interference.size (); ++i)
        {
          NS_ASSERT (m_interference[i] != 0);
          m_interference[i]->NotifyRxEnd ();
        }
    }
  NotifyTxBegin (packet);
  m_state->SwitchToTx (txDuration, packet, txVector.GetMode(),  txVector.GetTxPowerLevel());
  m_channel->Send (this, packet, GetPowerDbm ( txVector.GetTxPowerLevel()) + m_txGainDb, txVector);
}

//uint32_t
//SimpleCouwbatPhy::GetNModes (void) const
//{
//  return m_deviceRateSet.size ();
//}
CouwbatMode
SimpleCouwbatPhy::GetMode (uint32_t mode) const
{
  NS_FATAL_ERROR ("not implemented because incompatible with model");
  return CouwbatMode ();
}
uint32_t
SimpleCouwbatPhy::GetNTxPower (void) const
{
  return m_nTxPower;
}


void
SimpleCouwbatPhy::RegisterListener (CouwbatPhyListener *listener)
{
  m_state->RegisterListener (listener);
}

bool
SimpleCouwbatPhy::IsStateIdle (void)
{
  return m_state->IsStateIdle ();
}
bool
SimpleCouwbatPhy::IsStateBusy (void)
{
  return m_state->IsStateBusy ();
}
bool
SimpleCouwbatPhy::IsStateRx (void)
{
  return m_state->IsStateRx ();
}
bool
SimpleCouwbatPhy::IsStateTx (void)
{
  return m_state->IsStateTx ();
}
bool
SimpleCouwbatPhy::IsStateSwitching (void)
{
  return m_state->IsStateSwitching ();
}

Time
SimpleCouwbatPhy::GetStateDuration (void)
{
  return m_state->GetStateDuration ();
}
Time
SimpleCouwbatPhy::GetDelayUntilIdle (void)
{
  return m_state->GetDelayUntilIdle ();
}

Time
SimpleCouwbatPhy::GetLastRxStartTime (void) const
{
  return m_state->GetLastRxStartTime ();
}

double
SimpleCouwbatPhy::DbToRatio (double dB) const
{
  double ratio = std::pow (10.0, dB / 10.0);
  return ratio;
}

double
SimpleCouwbatPhy::DbmToW (double dBm) const
{
  double mW = std::pow (10.0, dBm / 10.0);
  return mW / 1000.0;
}

double
SimpleCouwbatPhy::WToDbm (double w) const
{
  return 10.0 * std::log10 (w * 1000.0);
}

double
SimpleCouwbatPhy::RatioToDb (double ratio) const
{
  return 10.0 * std::log10 (ratio);
}

double
SimpleCouwbatPhy::GetEdThresholdW (void) const
{
  return m_edThresholdW;
}

double
SimpleCouwbatPhy::GetPowerDbm (uint8_t power) const
{
  NS_ASSERT (m_txPowerBaseDbm <= m_txPowerEndDbm);
  NS_ASSERT (m_nTxPower > 0);
  double dbm;
  if (m_nTxPower > 1)
    {
      dbm = m_txPowerBaseDbm + power * (m_txPowerEndDbm - m_txPowerBaseDbm) / (m_nTxPower - 1);
    }
  else
    {
      NS_ASSERT_MSG (m_txPowerBaseDbm == m_txPowerEndDbm, "cannot have TxPowerEnd != TxPowerStart with TxPowerLevels == 1");
      dbm = m_txPowerBaseDbm;
    }
  return dbm;
}

void
SimpleCouwbatPhy::EndReceive (Ptr<Packet> packet, std::vector<Ptr<CouwbatInterferenceHelper::Event> > events)
{
  NS_LOG_FUNCTION (this << packet);
  NS_ASSERT (IsStateRx ());
  for (uint32_t i = 0; i < events.size (); ++i)
    {
      NS_ASSERT (events[i]->GetEndTime () == Simulator::Now ());
    }

  NS_LOG_LOGIC ("SimpleCouwbatPhy::EndReceive()");
  std::vector<uint32_t> subchannels = events[0]->GetTxVector ().GetMode ().GetSubchannels ();
  NS_ASSERT (subchannels.size () == events.size ());
  double snrPersPerTotal = 0;
  double snrPersSnrTotal = 0;
  double snrPersSnrTotalMax = 0;
  std::vector<double> allSnr = std::vector<double> (Couwbat::GetNumberOfSubchannels (), 0.0);
  std::vector<double> allSnrMax = std::vector<double> (Couwbat::GetNumberOfSubchannels (), 0.0);
  NS_LOG_LOGIC ("receiving on subchannels=" << subchannels);
  for (uint32_t i = 0; i < events.size (); ++i)
    {
      NS_ASSERT (m_interference[subchannels[i]] != 0);
      NS_LOG_LOGIC ("SimpleCouwbatPhy::EndReceive(): call m_interference["<<subchannels[i]<<"]->CalculateSnrPer()");
      struct CouwbatInterferenceHelper::SnrPer s = m_interference[subchannels[i]]->CalculateSnrPer (events[i]);
      snrPersSnrTotal += s.minSnr;
      snrPersSnrTotalMax += s.maxSnr;
      snrPersPerTotal += s.per;
      allSnr[subchannels[i]] += s.minSnr;
      allSnrMax[subchannels[i]] += s.maxSnr;
    }
  for (uint32_t i = 0; i < m_interference.size (); ++i)
    {
      NS_ASSERT (m_interference[i] != 0);
      m_interference[i]->NotifyRxEnd ();
    }

  struct CouwbatInterferenceHelper::SnrPer snrPer;
  snrPer.minSnr = snrPersSnrTotal / events.size ();
  snrPer.maxSnr = snrPersSnrTotalMax / events.size ();
  snrPer.per = snrPersPerTotal / events.size ();
  NS_LOG_LOGIC ("SimpleCouwbatPhy::EndReceive(): avgSnr="<<snrPer.minSnr<<", avgPer="<<snrPer.per);

  if (!Couwbat::sinrPerSubchannelCallback.IsNull()
      && ((subchannels.size () > 1 && Couwbat::m_sinrPerSubchannelCallbackWideband)
          || !Couwbat::m_sinrPerSubchannelCallbackWideband))
    {
      Ptr<CouwbatNetDevice> netDevice = m_device->GetObject<CouwbatNetDevice> ();
      if (!Couwbat::m_sinrPerSubchannelCallbackIdEnabled
          || (netDevice && netDevice->GetNode ()->GetId () == Couwbat::m_sinrPerSubchannelCallbackNodeId))
        {
          CouwbatMacHeader header;
          packet->PeekHeader (header);
          if (!Couwbat::m_sinrPerSubchannelCallbackEnableMacFilter
              || (Couwbat::m_sinrPerSubchannelCallbackEnableMacFilter
                  && header.GetSource () == Couwbat::m_sinrPerSubchannelCallbackMacFilterAddress))
          Couwbat::sinrPerSubchannelCallback (allSnr, allSnrMax, snrPer.per);
        }
    }

  NS_LOG_DEBUG ("mcs=" << "someMCS" /*(events[0]->GetPayloadMode ().GetMCS ())*/ << ", #subchans=" << (events[0]->GetPayloadMode ().GetSubchannels ().size()) <<
                ", snr=" << snrPer.minSnr << ", per=" << snrPer.per << ", size=" << packet->GetSize ());
  double random = m_random->GetValue ();
  NS_LOG_INFO ("SimpleCouwbatPhy::EndReceive(): random="<<random);
  
  // ignore interference model result
  if ( 1 )// random > snrPer.per)
    {
      NotifyRxEnd (packet);
      m_state->SwitchFromRxEndOk (packet, allSnr, events[0]->GetPayloadMode ());
      NS_LOG_INFO ("Channel successfully delivered packet to device: " << packet->ToString ());
    }
  else
    {
      /* failure. */
      NotifyRxDrop (packet);
      m_state->SwitchFromRxEndError (packet, allSnr);
      NS_LOG_INFO ("Channel failed to deliver packet to device");
          //": '" << packet->ToString () << "' - snr too low");
    }
}

int64_t
SimpleCouwbatPhy::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}

void
SimpleCouwbatPhy::SetFrequency (uint32_t freq)
{
  m_channelStartingFrequency = freq;
}

uint32_t
SimpleCouwbatPhy::GetFrequency (void) const
{
  return m_channelStartingFrequency;
}

uint32_t
SimpleCouwbatPhy::GetBssMembershipSelector (uint32_t selector) const
{
  return  m_bssMembershipSelectorSet[selector];
}
//CouwbatModeList
//SimpleCouwbatPhy::GetMembershipSelectorModes(uint32_t selector)
//{
//  //uint32_t id=GetBssMembershipSelector(selector);
//  CouwbatModeList supportedmodes;
////  if (id == HT_PHY)
////  {
////    //mandatory MCS 0 to 7
////     supportedmodes.push_back (WifiPhy::GetOfdmRate6_5MbpsBW20MHz ());
////     supportedmodes.push_back (WifiPhy::GetOfdmRate13MbpsBW20MHz ());
////     supportedmodes.push_back (WifiPhy::GetOfdmRate19_5MbpsBW20MHz ());
////     supportedmodes.push_back (WifiPhy::GetOfdmRate26MbpsBW20MHz ());
////     supportedmodes.push_back (WifiPhy::GetOfdmRate39MbpsBW20MHz ());
////     supportedmodes.push_back (WifiPhy::GetOfdmRate52MbpsBW20MHz ());
////     supportedmodes.push_back (WifiPhy::GetOfdmRate58_5MbpsBW20MHz ());
////     supportedmodes.push_back (WifiPhy::GetOfdmRate65MbpsBW20MHz ());
////  }
//  return supportedmodes;
//}
uint8_t
SimpleCouwbatPhy::GetNMcs (void) const
{
  return  m_deviceMcsSet.size ();
}
uint8_t
SimpleCouwbatPhy::GetMcs (uint8_t mcs) const
{
  return  m_deviceMcsSet[mcs];
}

void
SimpleCouwbatPhy::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::ScheduleNow (&SimpleCouwbatPhy::SfTrigger, this);
}

void SimpleCouwbatPhy::SfTrigger (void)
{
  m_sfCnt++;
  Ptr<Packet> zeroSFpacket = Create<Packet> ();
  CouwbatMetaHeader mh;
  mh.m_flags = CW_CMD_WIFI_EXTRA_ZERO_SF_START;
  mh.m_ofdm_sym_sframe_count = m_sfCnt;

  zeroSFpacket->AddHeader(mh);
  m_rxOkCallback(zeroSFpacket);
  Simulator::Schedule (MicroSeconds(Couwbat::GetSuperframeDuration()),&SimpleCouwbatPhy::SfTrigger, this);
}

void
SimpleCouwbatPhy::SetDevice (Ptr<Object> device)
{
  NS_LOG_FUNCTION (this << device);
  m_device = device;
}

Ptr<Object>
SimpleCouwbatPhy::GetDevice (void) const
{
  NS_LOG_FUNCTION (this);
  return m_device;
}


} // namespace ns3
