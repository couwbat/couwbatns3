#include "couwbat-intf-helper.h"
#include "couwbat-phy.h"
#include "couwbat-err-rate-model.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>
#include "couwbat.h"
#include "couwbat-packet-helper.h"

NS_LOG_COMPONENT_DEFINE ("CouwbatInterferenceHelper");

namespace ns3 {

double
WToDbm (double w)
{
  return 10.0 * std::log10 (w * 1000.0);
}

/****************************************************************
 *       Phy event class
 ****************************************************************/

CouwbatInterferenceHelper::Event::Event (uint32_t size, CouwbatMode payloadMode,
                                  Time duration, double rxPower, CouwbatTxVector txVector, uint32_t mcsIndex)
  : m_size (size),
    m_payloadMode (payloadMode),
    m_startTime (Simulator::Now ()),
    m_endTime (m_startTime + duration),
    m_rxPowerW (rxPower),
    m_txVector (txVector),
    m_mcsIndex (mcsIndex)
{
  NS_LOG_FUNCTION (this << size << payloadMode << duration
                   << rxPower << txVector.GetTxPowerLevel());
}
CouwbatInterferenceHelper::Event::~Event ()
{
  NS_LOG_FUNCTION (this);
}

Time
CouwbatInterferenceHelper::Event::GetDuration (void) const
{
  NS_LOG_FUNCTION (this);
  return m_endTime - m_startTime;
}
Time
CouwbatInterferenceHelper::Event::GetStartTime (void) const
{
  NS_LOG_FUNCTION (this);
  return m_startTime;
}
Time
CouwbatInterferenceHelper::Event::GetEndTime (void) const
{
  NS_LOG_FUNCTION (this);
  return m_endTime;
}
double
CouwbatInterferenceHelper::Event::GetRxPowerW (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxPowerW;
}
uint32_t
CouwbatInterferenceHelper::Event::GetSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}
CouwbatMode
CouwbatInterferenceHelper::Event::GetPayloadMode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_payloadMode;
}

CouwbatTxVector
CouwbatInterferenceHelper::Event::GetTxVector (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txVector;
}

uint32_t
CouwbatInterferenceHelper::Event::GetMcsIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mcsIndex;
}


/****************************************************************
 *       Class which records SNIR change events for a
 *       short period of time.
 ****************************************************************/

CouwbatInterferenceHelper::NiChange::NiChange (Time time, double delta)
  : m_time (time),
    m_delta (delta)
{
  NS_LOG_FUNCTION (this << time << delta);
}
Time
CouwbatInterferenceHelper::NiChange::GetTime (void) const
{
  NS_LOG_FUNCTION (this);
  return m_time;
}
double
CouwbatInterferenceHelper::NiChange::GetDelta (void) const
{
  NS_LOG_FUNCTION (this);
  return m_delta;
}
bool
CouwbatInterferenceHelper::NiChange::operator < (const CouwbatInterferenceHelper::NiChange& o) const
{
  NS_LOG_FUNCTION (this);
  return (m_time < o.m_time);
}

/****************************************************************
 *       The actual CouwbatInterferenceHelper
 ****************************************************************/

CouwbatInterferenceHelper::CouwbatInterferenceHelper ()
  : m_errorRateModel (0),
    m_firstPower (0.0),
    m_rxing (false)
{
  NS_LOG_FUNCTION (this);
}
CouwbatInterferenceHelper::~CouwbatInterferenceHelper ()
{
  NS_LOG_FUNCTION (this);
  EraseEvents ();
  m_errorRateModel = 0;
}

TypeId CouwbatInterferenceHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatInterferenceHelper")
    .SetParent<Object> ()
  ;
  return tid;
}

Ptr<CouwbatInterferenceHelper::Event>
CouwbatInterferenceHelper::Add (uint32_t size, CouwbatMode payloadMode, uint32_t mcsIndex,
                         Time duration, double rxPowerW, CouwbatTxVector txVector)
{
  NS_LOG_FUNCTION (this << size << payloadMode << duration << rxPowerW
                   << txVector.GetTxPowerLevel());
  Ptr<CouwbatInterferenceHelper::Event> event;

  event = Create<CouwbatInterferenceHelper::Event> (size,
                                             payloadMode,
                                             duration,
                                             rxPowerW,
                                             txVector,
                                             mcsIndex);
  AppendEvent (event);
  return event;
}


void
CouwbatInterferenceHelper::SetNoiseFigure (double value)
{
  NS_LOG_FUNCTION (this << value);
  m_noiseFigure = value;
}

double
CouwbatInterferenceHelper::GetNoiseFigure (void) const
{
  NS_LOG_FUNCTION (this);
  return m_noiseFigure;
}

void
CouwbatInterferenceHelper::SetErrorRateModel (Ptr<CouwbatErrorRateModel> rate)
{
  NS_LOG_FUNCTION (this);
  m_errorRateModel = rate;
}

Ptr<CouwbatErrorRateModel>
CouwbatInterferenceHelper::GetErrorRateModel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_errorRateModel;
}

Time
CouwbatInterferenceHelper::GetEnergyDuration (double energyW)
{
  NS_LOG_FUNCTION (this << energyW);
  Time now = Simulator::Now ();
  double noiseInterferenceW = 0.0;
  Time end = now;
  noiseInterferenceW = m_firstPower;
  for (NiChanges::const_iterator i = m_niChanges.begin (); i != m_niChanges.end (); i++)
    {
      noiseInterferenceW += i->GetDelta ();
      end = i->GetTime ();
      if (end < now)
        {
          continue;
        }
      if (noiseInterferenceW < energyW)
        {
          break;
        }
    }
  return end > now ? end - now : MicroSeconds (0);
}

void
CouwbatInterferenceHelper::AppendEvent (Ptr<CouwbatInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << event);
  Time now = Simulator::Now ();
  if (!m_rxing)
    {
      NiChanges::iterator nowIterator = GetPosition (now);
      for (NiChanges::iterator i = m_niChanges.begin (); i != nowIterator; i++)
        {
          m_firstPower += i->GetDelta ();
        }
      m_niChanges.erase (m_niChanges.begin (), nowIterator);
      m_niChanges.insert (m_niChanges.begin (), NiChange (event->GetStartTime (), event->GetRxPowerW ()));
    }
  else
    {
      AddNiChangeEvent (NiChange (event->GetStartTime (), event->GetRxPowerW ()));
    }
  AddNiChangeEvent (NiChange (event->GetEndTime (), -event->GetRxPowerW ()));

}


double
CouwbatInterferenceHelper::CalculateSnr (double signal, double noiseInterference, CouwbatMode mode) const
{
  NS_LOG_FUNCTION (this << signal << noiseInterference << mode);
  // thermal noise at 290K in J/s = W
  static const double BOLTZMANN = 1.3803e-23;
  // Nt is the power of thermal noise in W
  uint32_t bw = Couwbat::GetSCFrequencySpacing();
  double Nt = BOLTZMANN * 290.0 * bw;
  // receiver noise Floor (W) which accounts for thermal noise and non-idealities of the receiver
  double noiseFloor = m_noiseFigure * Nt;
  NS_LOG_INFO ("CalculateSnr(): Nt="<<Nt<<", noiseInterference="<<noiseInterference<<", noiseFloor="<<noiseFloor);
  double noise = noiseFloor + noiseInterference;
  double snrDb = 10.0 * std::log10(signal / noise); // in dB micha
  double snr = signal / noise;
  NS_LOG_INFO ("CalculateSnr():"
               <<" signal="<<signal<<"W"
               <<", signal="<<WToDbm (signal)<<"dBm"
               <<", noise="<<noise<<"W"
               <<", noise="<<WToDbm (noise)<<"dBm"
               <<", snr="<<snr
               <<", snrDb="<<snrDb);
  return snr;
}

double
CouwbatInterferenceHelper::CalculateNoiseInterferenceW (Ptr<CouwbatInterferenceHelper::Event> event, NiChanges *ni) const
{
  NS_LOG_FUNCTION (this);
  double noiseInterference = m_firstPower;
  NS_ASSERT (m_rxing);
  for (NiChanges::const_iterator i = m_niChanges.begin () + 1; i != m_niChanges.end (); i++)
    {
      if ((event->GetEndTime () == i->GetTime ()) && event->GetRxPowerW () == -i->GetDelta ())
        {
          break;
        }
      ni->push_back (*i);
    }
  ni->insert (ni->begin (), NiChange (event->GetStartTime (), noiseInterference));
  ni->push_back (NiChange (event->GetEndTime (), 0));
  std::vector<double> nis;
  for (NiChanges::const_iterator i = ni->begin (); i != ni->end (); ++i)
    {
      nis.push_back ((*i).GetDelta ());
    }
  NS_LOG_INFO ("CalculateNoiseInterferenceW(): noiseInterference="<<noiseInterference
               <<", ni="<<nis);
  return noiseInterference;
}

double
CouwbatInterferenceHelper::CalculateChunkSuccessRate (double snir, Time duration, CouwbatMode mode, enum CouwbatMCS mcs) const
{
  NS_LOG_FUNCTION (this << snir << duration << mode);
  if (duration == NanoSeconds (0))
    {
      NS_LOG_INFO ("CalculateChunkSuccessRate(): "<<1.0);
      return 1.0;
    }
  uint32_t rate = mode.GetPhyRate ();
  uint64_t nbits = (uint64_t)(rate * duration.GetSeconds ());
//  double nbits = (rate * duration.GetSeconds ()); // nbits as double instead

  NS_LOG_INFO ("CalculateChunkSuccessRate(): calling GetChunkSuccessRate(mode="<<mode<<",snir="<<snir<<",nbits="<<nbits<<"), duration="<<duration.GetSeconds ()<<"s, rate="<<rate);
  double csr = m_errorRateModel->GetChunkSuccessRate (mode, mcs, snir, (uint32_t)nbits);
  NS_LOG_INFO ("CalculateChunkSuccessRate(): csr="<<csr);
  return csr;
}

double
CouwbatInterferenceHelper::CalculatePer (Ptr<const CouwbatInterferenceHelper::Event> event, NiChanges *ni, SnrPer *snrPer) const
{
  NS_LOG_FUNCTION (this);
  double psr = 1.0; /* Packet Success Rate */
  NiChanges::iterator j = ni->begin ();
  Time previous = (*j).GetTime ();
  CouwbatMode payloadMode = event->GetPayloadMode ();
  CouwbatMCS mcs = payloadMode.GetMCS ()[event->GetMcsIndex ()];

  //CouwbatMode headerMode = -1;//CouwbatPhy::GetPlcpHeaderMode (payloadMode);
  Time plcpHeaderStart = (*j).GetTime ();// + MicroSeconds (CouwbatPhy::GetPlcpPreambleDurationMicroSeconds (payloadMode, preamble)); //packet start time+ preamble
  Time plcpHsigHeaderStart=plcpHeaderStart;//+ MicroSeconds (CouwbatPhy::GetPlcpHeaderDurationMicroSeconds (payloadMode, preamble));//packet start time+ preamble+L SIG
  Time plcpHtTrainingSymbolsStart = plcpHsigHeaderStart;// + MicroSeconds (CouwbatPhy::GetPlcpHtSigHeaderDurationMicroSeconds (payloadMode, preamble));//packet start time+ preamble+L SIG+HT SIG
  Time plcpPayloadStart =plcpHtTrainingSymbolsStart;// + MicroSeconds (CouwbatPhy::GetPlcpHtTrainingSymbolDurationMicroSeconds (payloadMode, preamble,event->GetTxVector())); //packet start time+ preamble+L SIG+HT SIG+Training
  double noiseInterferenceW = (*j).GetDelta ();
  NS_LOG_INFO ("CalculatePerMD2(): noiseInterferenceW="<<noiseInterferenceW);
  double powerW = event->GetRxPowerW ();
    j++;
  while (ni->end () != j)
    {
      Time current = (*j).GetTime ();
      NS_LOG_INFO ("CalculatePer(): iteration, current="<<current<<", previous="<<previous);
      NS_ASSERT (current >= previous);
      //Case 1: Both prev and curr point to the payload
      if (previous >= plcpPayloadStart)
        {
          double snr = CalculateSnr (powerW, noiseInterferenceW, payloadMode);
          NS_LOG_INFO ("CalculatePer(): called CalculateSnr (powerW="<<powerW
                       <<", niW="<<noiseInterferenceW<<", mode="<<payloadMode
                       << ") = "<<snr);
          double csr = CalculateChunkSuccessRate (snr, current - previous, payloadMode, mcs);
          NS_LOG_INFO ("CalculatePer(): called CalculateChunkSuccessRate (snr="<<snr
                                 <<", duration="<<current - previous<<", mode="<<payloadMode
                                 << ") = "<<csr);
          psr *= csr;
          NS_LOG_INFO ("CalculatePer(): psr="<<psr);

          if (snr > snrPer->maxSnr) snrPer->maxSnr = snr;
          if (snr < snrPer->minSnr) snrPer->minSnr = snr;
        }

      noiseInterferenceW += (*j).GetDelta ();
      previous = (*j).GetTime ();
      NS_LOG_INFO ("CalculatePer(): noiseInterferenceW="<<noiseInterferenceW);
      j++;
    }

  double per = 1 - psr;
  NS_LOG_INFO ("CalculatePer(): psr="<<psr<<", per="<<per);
  return per;
}


struct CouwbatInterferenceHelper::SnrPer
CouwbatInterferenceHelper::CalculateSnrPer (Ptr<CouwbatInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this);
  NiChanges ni;
  double noiseInterferenceW = CalculateNoiseInterferenceW (event, &ni);
  // SNR at the start of the packet
  double snr = CalculateSnr (event->GetRxPowerW (),
                             noiseInterferenceW,
                             event->GetPayloadMode ());

  struct SnrPer snrPer;
  snrPer.minSnr = snr;
  snrPer.maxSnr = snr;

  /* calculate the SNIR at the start of the packet and accumulate
   * all SNIR changes in the snir vector.
   */
  double per = CalculatePer (event, &ni, &snrPer);

  snrPer.per = per;
  NS_LOG_INFO ("CalculateSnrPer(): snr="<<snr<<", per="<<per);
  return snrPer;
}

void
CouwbatInterferenceHelper::EraseEvents (void)
{
  NS_LOG_FUNCTION (this);
  m_niChanges.clear ();
  m_rxing = false;
  m_firstPower = 0.0;
}
CouwbatInterferenceHelper::NiChanges::iterator
CouwbatInterferenceHelper::GetPosition (Time moment)
{
  NS_LOG_FUNCTION (this << moment);
  return std::upper_bound (m_niChanges.begin (), m_niChanges.end (), NiChange (moment, 0));

}
void
CouwbatInterferenceHelper::AddNiChangeEvent (NiChange change)
{
  NS_LOG_FUNCTION (this);
  m_niChanges.insert (GetPosition (change.GetTime ()), change);
}
void
CouwbatInterferenceHelper::NotifyRxStart ()
{
  m_rxing = true;
}
void
CouwbatInterferenceHelper::NotifyRxEnd ()
{
  m_rxing = false;
}
} // namespace ns3
