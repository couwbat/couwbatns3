#include "couwbat-phy-state-helper.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "couwbat-meta-header.h"
#include "couwbat-mac.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>

NS_LOG_COMPONENT_DEFINE ("CouwbatPhyStateHelper");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CouwbatPhyStateHelper)
  ;

TypeId
CouwbatPhyStateHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatPhyStateHelper")
    .SetParent<Object> ()
    .AddConstructor<CouwbatPhyStateHelper> ()
    .AddTraceSource ("State",
                     "The state of the PHY layer",
                     MakeTraceSourceAccessor (&CouwbatPhyStateHelper::m_stateLogger))
    .AddTraceSource ("RxOk",
                     "A packet has been received successfully.",
                     MakeTraceSourceAccessor (&CouwbatPhyStateHelper::m_rxOkTrace))
    .AddTraceSource ("RxError",
                     "A packet has been received unsuccessfully.",
                     MakeTraceSourceAccessor (&CouwbatPhyStateHelper::m_rxErrorTrace))
    .AddTraceSource ("Tx", "Packet transmission is starting.",
                     MakeTraceSourceAccessor (&CouwbatPhyStateHelper::m_txTrace))
  ;
  return tid;
}

CouwbatPhyStateHelper::CouwbatPhyStateHelper ()
  : m_rxing (false),
    m_endTx (Seconds (0)),
    m_endRx (Seconds (0)),
    m_endCcaBusy (Seconds (0)),
    m_endSwitching (Seconds (0)),
    m_startTx (Seconds (0)),
    m_startRx (Seconds (0)),
    m_startCcaBusy (Seconds (0)),
    m_startSwitching (Seconds (0)),
    m_previousStateChangeTime (Seconds (0))
{
  NS_LOG_FUNCTION (this);
}

void
CouwbatPhyStateHelper::SetReceiveOkCallback (CouwbatPhy::RxOkCallback callback)
{
  m_rxOkCallback = callback;
}
void
CouwbatPhyStateHelper::SetReceiveErrorCallback (CouwbatPhy::RxErrorCallback callback)
{
  m_rxErrorCallback = callback;
}
void
CouwbatPhyStateHelper::RegisterListener (CouwbatPhyListener *listener)
{
  m_listeners.push_back (listener);
}

bool
CouwbatPhyStateHelper::IsStateIdle (void)
{
  return (GetState () == CouwbatPhy::IDLE);
}
bool
CouwbatPhyStateHelper::IsStateBusy (void)
{
  return (GetState () != CouwbatPhy::IDLE);
}
bool
CouwbatPhyStateHelper::IsStateRx (void)
{
  return (GetState () == CouwbatPhy::RX);
}
bool
CouwbatPhyStateHelper::IsStateTx (void)
{
  return (GetState () == CouwbatPhy::TX);
}
bool
CouwbatPhyStateHelper::IsStateSwitching (void)
{
  return (GetState () == CouwbatPhy::SWITCHING);
}


Time
CouwbatPhyStateHelper::GetStateDuration (void)
{
  return Simulator::Now () - m_previousStateChangeTime;
}

Time
CouwbatPhyStateHelper::GetDelayUntilIdle (void)
{
  Time retval;

  switch (GetState ())
    {
    case CouwbatPhy::RX:
      retval = m_endRx - Simulator::Now ();
      break;
    case CouwbatPhy::TX:
      retval = m_endTx - Simulator::Now ();
      break;
    //case CouwbatPhy::CCA_BUSY:
    //  retval = m_endCcaBusy - Simulator::Now ();
    //  break;
    case CouwbatPhy::SWITCHING:
      retval = m_endSwitching - Simulator::Now ();
      break;
    case CouwbatPhy::IDLE:
      retval = Seconds (0);
      break;
    default:
      NS_FATAL_ERROR ("Invalid CouwbatPhy state.");
      retval = Seconds (0);
      break;
    }
  retval = Max (retval, Seconds (0));
  return retval;
}

Time
CouwbatPhyStateHelper::GetLastRxStartTime (void) const
{
  return m_startRx;
}

enum CouwbatPhy::State
CouwbatPhyStateHelper::GetState (void)
{
  if (m_endTx > Simulator::Now ())
    {
      return CouwbatPhy::TX;
    }
  else if (m_rxing)
    {
      return CouwbatPhy::RX;
    }
  else if (m_endSwitching > Simulator::Now ())
    {
      return CouwbatPhy::SWITCHING;
    }
//  else if (m_endCcaBusy > Simulator::Now ())
//    {
//      return CouwbatPhy::CCA_BUSY;
//    }
  else
    {
      return CouwbatPhy::IDLE;
    }
}


void
CouwbatPhyStateHelper::NotifyTxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyTxStart (duration);
    }
}
void
CouwbatPhyStateHelper::NotifyRxStart (Time duration)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxStart (duration);
    }
}
void
CouwbatPhyStateHelper::NotifyRxEndOk (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndOk ();
    }
}
void
CouwbatPhyStateHelper::NotifyRxEndError (void)
{
  for (Listeners::const_iterator i = m_listeners.begin (); i != m_listeners.end (); i++)
    {
      (*i)->NotifyRxEndError ();
    }
}

void
CouwbatPhyStateHelper::SwitchToTx (Time txDuration, Ptr<const Packet> packet, CouwbatMode txMode,
                                uint8_t txPower)
{
  m_txTrace (packet);
  NotifyTxStart (txDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case CouwbatPhy::RX:
      /* The packet which is being received as well
       * as its endRx event are cancelled by the caller.
       */
      m_rxing = false;
      m_stateLogger (m_startRx, now - m_startRx, CouwbatPhy::RX);
      m_endRx = now;
      break;
    case CouwbatPhy::IDLE:
      //LogPreviousIdleAndCcaBusyStates ();
      break;
    case CouwbatPhy::SWITCHING:
    default:
      NS_FATAL_ERROR ("Invalid CouwbatPhy state.");
      break;
    }
  m_stateLogger (now, txDuration, CouwbatPhy::TX);
  m_previousStateChangeTime = now;
  m_endTx = now + txDuration;
  m_startTx = now;
}
void
CouwbatPhyStateHelper::SwitchToRx (Time rxDuration)
{
  NS_ASSERT (IsStateIdle ());
  NS_ASSERT (!m_rxing);
  NotifyRxStart (rxDuration);
  Time now = Simulator::Now ();
  switch (GetState ())
    {
    case CouwbatPhy::IDLE:
      //LogPreviousIdleAndCcaBusyStates ();
      break;
    case CouwbatPhy::SWITCHING:
    case CouwbatPhy::RX:
    case CouwbatPhy::TX:
      NS_FATAL_ERROR ("Invalid CouwbatPhy state.");
      break;
    }
  m_previousStateChangeTime = now;
  m_rxing = true;
  m_startRx = now;
  m_endRx = now + rxDuration;
  NS_ASSERT (IsStateRx ());
}

void
CouwbatPhyStateHelper::SwitchFromRxEndOk (Ptr<Packet> packet, std::vector<double> snrW, CouwbatMode mode)
{
  NS_LOG_DEBUG (this << "SwitchFromRxEndOk, " << "snrs[]");
  m_rxOkTrace (packet);
  NotifyRxEndOk ();
  DoSwitchFromRx ();

  if (!m_rxOkCallback.IsNull ())
    {
      Time now = Simulator::Now ();

      static const uint32_t sfDuration = Couwbat::GetSuperframeDuration ();

      const int64_t currentSfStart =
          (now.GetMicroSeconds () / sfDuration) * sfDuration;

      const uint32_t currentSymbol = (now.GetMicroSeconds () - currentSfStart + Couwbat::GetSymbolDuration () / 2)
	      / Couwbat::GetSymbolDuration ();

      uint32_t ofs = (m_startRx.GetMicroSeconds () - currentSfStart + Couwbat::GetSymbolDuration () / 2)
	      / Couwbat::GetSymbolDuration ();

      uint32_t len = ((now - m_startRx).GetMicroSeconds () + Couwbat::GetSymbolDuration () / 2)
	      / Couwbat::GetSymbolDuration ();

      // fill out meta header
      CouwbatMetaHeader mh;
      mh.m_flags = CW_CMD_WIFI_EXTRA_RX;
      mh.m_ofdm_sym_sframe_count = m_phy->GetSfCnt ();
      mh.m_ofdm_sym_offset = ofs;
      mh.m_ofdm_sym_len = len;
      mh.m_allocatedSubChannels.reset ();
      // Default to value CQI N/A
      memset (mh.m_CQI, 255, Couwbat::GetNumberOfSubchannels ());
      const std::vector<uint32_t> &subchannels = mode.GetSubchannels ();
      NS_LOG_DEBUG ("GetSubchannels: " << subchannels.size ());
      for (uint32_t i = 0; i < subchannels.size (); ++i)
        {
          mh.m_allocatedSubChannels.set(subchannels[i], true);
          mh.m_MCS[subchannels[i]] = (uint8_t) mode.GetMCS ()[i];
          double snrSubch = 10.0 * std::log10(snrW[subchannels[i]]);
          double clampedSnrSubch = std::max (0.0, std::min (snrSubch, 255.0));
          mh.m_CQI[subchannels[i]] = clampedSnrSubch;
        }

      NS_LOG_INFO ("X_metaheader: {" << mh << "}");

      // receive only if correctly scheduled
      while (!m_rxQueue.empty ())
        {
          // compare to inferred metaheader to determine if packet RX is successful
          // schedMh is the RX scheduling metaheader, mh is the RX event that triggered this call
          pshRxQueue rq = m_rxQueue.front ();
          CouwbatMetaHeader schedMh = rq.mh;

          // check if next schedule is for future superframe
          if (schedMh.m_ofdm_sym_sframe_count > m_phy->GetSfCnt ())
            {
              // keep in queue and ignore this RX event
              NS_LOG_DEBUG ("ignoring RX event - nothing scheduled for this superframe");
              NS_LOG_DEBUG ("schedMh vs mh: m_ofdm_sym_sframe_count " << schedMh.m_ofdm_sym_sframe_count << " " << m_phy->GetSfCnt ()
                                                        << ", currentSymbol: " << currentSymbol << " " << (schedMh.m_ofdm_sym_offset + schedMh.m_ofdm_sym_len)
                                                        << ", m_allocatedSubChannels " << schedMh.m_allocatedSubChannels << " " << mode.GetSubchannels ().size ());
              return;
            }

          // discard if it is overdue
          if (schedMh.m_ofdm_sym_sframe_count < m_phy->GetSfCnt ()
              || currentSymbol > (schedMh.m_ofdm_sym_offset + schedMh.m_ofdm_sym_len))
            {
              NS_LOG_DEBUG ("discarding overdue RX schedule");
              NS_LOG_DEBUG ("schedMh vs mh: m_ofdm_sym_sframe_count " << schedMh.m_ofdm_sym_sframe_count << " " << m_phy->GetSfCnt ()
                                          << ", currentSymbol: " << currentSymbol << " " << (schedMh.m_ofdm_sym_offset + schedMh.m_ofdm_sym_len)
                                          << ", m_allocatedSubChannels " << schedMh.m_allocatedSubChannels << " " << mode.GetSubchannels ().size ());
              m_rxQueue.pop ();
              continue;
            }

          // schedMh can now only be for current superframe, compare parameters with this RX event
          // check for mismatched offset, subchannels, frequency band or length

          // do nothing if the start receive offset symbol is different
          if (schedMh.m_ofdm_sym_offset > mh.m_ofdm_sym_offset)
            {
              NS_LOG_DEBUG ("ignoring RX event - nothing scheduled for this symbol");
              NS_LOG_DEBUG ("schedMh vs mh: m_ofdm_sym_offset " << schedMh.m_ofdm_sym_offset << " " << mh.m_ofdm_sym_offset
                            << ", m_ofdm_sym_len: " << schedMh.m_ofdm_sym_len << " " << mh.m_ofdm_sym_len);
              return;
            }

          // WILL be handled, remove from queue
          m_rxQueue.pop ();

          if (schedMh.m_ofdm_sym_offset != mh.m_ofdm_sym_offset
              || schedMh.m_ofdm_sym_len != mh.m_ofdm_sym_len
              || schedMh.m_allocatedSubChannels != mh.m_allocatedSubChannels
              || schedMh.m_frequency_band != mh.m_frequency_band
              || !std::equal (schedMh.m_MCS, schedMh.m_MCS + Couwbat::GetNumberOfSubchannels(), mh.m_MCS))
            {
              // This superframe is out of sync, fail to receive due to RX schedule mismatch
              NS_LOG_DEBUG ("offset, sym_len, subch or band mismatch => out-of-sync");
              NS_LOG_DEBUG ("schedMh vs mh: m_ofdm_sym_offset " << schedMh.m_ofdm_sym_offset << " " << mh.m_ofdm_sym_offset
                            << ", m_ofdm_sym_len: " << schedMh.m_ofdm_sym_len << " " << mh.m_ofdm_sym_len
                            << ", m_allocatedSubChannels " << schedMh.m_allocatedSubChannels << " " << mh.m_allocatedSubChannels
                            << ", m_frequency_band " << schedMh.m_frequency_band << " " << mh.m_frequency_band);

              return;
            }

          NS_LOG_DEBUG ("RX successful");

          // Cancel dummy packet forward up
          Simulator::Cancel (rq.e);

          packet->AddHeader (mh);
          m_rxOkCallback (packet);
          return;
        }
      NS_LOG_DEBUG ("ignoring RX event - not scheduled");
    }
  NS_LOG_WARN ("m_rxOkCallback is null");
}
void
CouwbatPhyStateHelper::SwitchFromRxEndError (Ptr<const Packet> packet, std::vector<double> snr)
{
  // TODO Implement Metaheader here, this function is called in case of RxEndError in PHY
  m_rxErrorTrace (packet);
  NotifyRxEndError ();
  DoSwitchFromRx ();
  if (!m_rxErrorCallback.IsNull ())
    {
      m_rxErrorCallback (packet);
    }
}

void
CouwbatPhyStateHelper::DoSwitchFromRx (void)
{
  NS_ASSERT (IsStateRx ());
  NS_ASSERT (m_rxing);

  Time now = Simulator::Now ();
  m_stateLogger (m_startRx, now - m_startRx, CouwbatPhy::RX);
  m_previousStateChangeTime = now;
  m_rxing = false;

  NS_ASSERT (IsStateIdle ());// || IsStateCcaBusy ());
}

void
CouwbatPhyStateHelper::SetPhy (Ptr<SimpleCouwbatPhy> phy)
{
  m_phy = phy;
}

void
CouwbatPhyStateHelper::EnqueueRx (CouwbatMetaHeader mh)
{
  NS_LOG_DEBUG ("Enqueue {" << mh << "}");
  pshRxQueue rq;
  rq.mh = mh;

  // Schedule a check and dummy forward up shortly after SwitchFromRxEndOk would be called.
  // This event is cancelled if the RX is successful
  const Time now = Simulator::Now ();
  static const uint32_t sfDuration = Couwbat::GetSuperframeDuration ();
  static const uint32_t symbDuration = Couwbat::GetSymbolDuration ();
  const int64_t currentSfStart = (now.GetMicroSeconds () / sfDuration) * sfDuration;
  const int64_t currentSfCnt = m_phy->GetSfCnt ();
  const int64_t sfOffset = mh.m_ofdm_sym_sframe_count - currentSfCnt;
  const int64_t symbOffset = mh.m_ofdm_sym_offset;
  const int64_t lenOffset = mh.m_ofdm_sym_len + 1;

  Time schedEndRxTime = MicroSeconds (currentSfStart + (sfOffset * sfDuration) + (symbOffset + lenOffset) * symbDuration);
  EventId e = Simulator::Schedule (schedEndRxTime - now, &CouwbatPhyStateHelper::CheckScheduleRxStatus, this, mh);
  rq.e = e;
  m_rxQueue.push (rq);
}

void
CouwbatPhyStateHelper::CheckScheduleRxStatus (CouwbatMetaHeader mh)
{
  NS_LOG_FUNCTION (mh);
  uint32_t byteCount = MhGetBytes (mh);
  ForwardUpDummy (mh, byteCount);
}

void
CouwbatPhyStateHelper::ForwardUpDummy (CouwbatMetaHeader mh, uint32_t sizeBytes)
{
  NS_LOG_FUNCTION (mh << sizeBytes);

  uint8_t* buf = new uint8_t[sizeBytes];
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd == -1)
    {
      NS_LOG_ERROR ("Error opening /dev/urandom");
    }
  else
    {
      read(fd, buf, sizeBytes);
      close (fd);
    }

  Ptr<Packet> packet = Create<Packet> (buf, sizeBytes);
  delete[] buf;
  mh.m_flags = CW_CMD_WIFI_EXTRA_RX;
  packet->AddHeader (mh);
  NS_LOG_INFO ("Forwarding up dummy");
  m_rxOkCallback (packet);
}

uint32_t
CouwbatPhyStateHelper::MhGetBytes (CouwbatMetaHeader mh)
{
  NS_LOG_FUNCTION (mh);

  uint16_t symbCount = mh.m_ofdm_sym_len;
  const uint8_t* mcs = mh.m_MCS;
  std::vector<CouwbatMCS> mcsVector;
  uint32_t subchCount = 0;
  for (uint32_t i = 0; i < Couwbat::MAX_SUBCHANS; ++i)
    {
      if (mcs[i] != COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE)
        {
          mcsVector.push_back ((CouwbatMCS) mcs[i]);
        }
      if (mh.m_allocatedSubChannels.test (i))
        {
          ++subchCount;
        }
    }
  NS_ASSERT (mcsVector.size () == subchCount);

  uint32_t byteCount = CouwbatMac::TransmittableBytesWithSymbols(symbCount, subchCount, mcsVector);
  return byteCount;
}

} // namespace ns3
