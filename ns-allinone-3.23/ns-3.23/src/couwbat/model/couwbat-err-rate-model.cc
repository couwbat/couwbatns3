#include <cmath>
#include "couwbat-err-rate-model.h"
#include "couwbat.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CouwbatErrorRateModel)
  ;

TypeId CouwbatErrorRateModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatErrorRateModel")
    .SetParent<Object> ()
  ;
  return tid;
}

double
CouwbatErrorRateModel::CalculateSnr (CouwbatMode txMode, enum CouwbatMCS mcs, double ber) const
{
  // This is a very simple binary search.
  double low, high, precision;
  low = 1e-25;
  high = 1e25;
  precision = 1e-12;
  while (high - low > precision)
    {
      NS_ASSERT (high >= low);
      double middle = low + (high - low) / 2;
      if ((1 - GetChunkSuccessRate (txMode, mcs, middle, 1)) > ber)
        {
          low = middle;
        }
      else
        {
          high = middle;
        }
    }
  return low;
}

double
CouwbatErrorRateModel::Log2 (double val) const
{
  return std::log (val) / std::log (2.0);
}
double
CouwbatErrorRateModel::GetBpskBer (double snr, uint32_t signalSpread, uint32_t phyRate) const
{
  double EbNo = snr * signalSpread / phyRate;
  double z = std::sqrt (EbNo);
  double ber = 0.5 * erfc (z);
  //NS_LOG_INFO ("bpsk snr=" << snr << " ber=" << ber);
  return ber;
}
double
CouwbatErrorRateModel::GetQamBer (double snr, unsigned int m, uint32_t signalSpread, uint32_t phyRate) const
{
  double EbNo = snr * signalSpread / phyRate;
  double z = std::sqrt ((1.5 * Log2 (m) * EbNo) / (m - 1.0));
  double z1 = ((1.0 - 1.0 / std::sqrt (m)) * erfc (z));
  double z2 = 1 - std::pow ((1 - z1), 2.0);
  double ber = z2 / Log2 (m);
  //NS_LOG_INFO ("Qam m=" << m << " rate=" << phyRate << " snr=" << snr << " ber=" << ber);
  return ber;
}
uint32_t
CouwbatErrorRateModel::Factorial (uint32_t k) const
{
  uint32_t fact = 1;
  while (k > 0)
    {
      fact *= k;
      k--;
    }
  return fact;
}
double
CouwbatErrorRateModel::Binomial (uint32_t k, double p, uint32_t n) const
{
  double retval = Factorial (n) / (Factorial (k) * Factorial (n - k)) * std::pow (p, static_cast<double> (k)) * std::pow (1 - p, static_cast<double> (n - k));
  return retval;
}
double
CouwbatErrorRateModel::CalculatePdOdd (double ber, unsigned int d) const
{
  NS_ASSERT ((d % 2) == 1);
  unsigned int dstart = (d + 1) / 2;
  unsigned int dend = d;
  double pd = 0;

  for (unsigned int i = dstart; i < dend; i++)
    {
      pd += Binomial (i, ber, d);
    }
  return pd;
}
double
CouwbatErrorRateModel::CalculatePdEven (double ber, unsigned int d) const
{
  NS_ASSERT ((d % 2) == 0);
  unsigned int dstart = d / 2 + 1;
  unsigned int dend = d;
  double pd = 0;

  for (unsigned int i = dstart; i < dend; i++)
    {
      pd +=  Binomial (i, ber, d);
    }
  pd += 0.5 * Binomial (d / 2, ber, d);

  return pd;
}

double
CouwbatErrorRateModel::CalculatePd (double ber, unsigned int d) const
{
  double pd;
  if ((d % 2) == 0)
    {
      pd = CalculatePdEven (ber, d);
    }
  else
    {
      pd = CalculatePdOdd (ber, d);
    }
  return pd;
}

double
CouwbatErrorRateModel::GetFecBpskBer (double snr, double nbits,
                                   uint32_t signalSpread, uint32_t phyRate,
                                   uint32_t dFree, uint32_t adFree) const
{
  double ber = GetBpskBer (snr, signalSpread, phyRate);
  if (ber == 0.0)
    {
      return 1.0;
    }
  double pd = CalculatePd (ber, dFree);
  double pmu = adFree * pd;
  pmu = std::min (pmu, 1.0);
  double pms = std::pow (1 - pmu, nbits);
  return pms;
}

double
CouwbatErrorRateModel::GetFecQamBer (double snr, uint32_t nbits,
                                  uint32_t signalSpread,
                                  uint32_t phyRate,
                                  uint32_t m, uint32_t dFree,
                                  uint32_t adFree, uint32_t adFreePlusOne) const
{
  double ber = GetQamBer (snr, m, signalSpread, phyRate);
  if (ber == 0.0)
    {
      return 1.0;
    }
  /* first term */
  double pd = CalculatePd (ber, dFree);
  double pmu = adFree * pd;
  /* second term */
  pd = CalculatePd (ber, dFree + 1);
  pmu += adFreePlusOne * pd;
  pmu = std::min (pmu, 1.0);
  double pms = std::pow (1 - pmu, static_cast<double> (nbits));
  return pms;
}

double
CouwbatErrorRateModel::GetChunkSuccessRate (CouwbatMode mode, enum CouwbatMCS mcs, double snr, uint32_t nbits) const
{
  if (mode.GetModulationClass () == COUWBAT_MOD_CLASS_OFDM)
    {
	  // we are using different subchannels
	  std::vector<uint32_t> subchans = mode.GetSubchannels();

	  // we need the bandwidth
	  uint32_t bw = Couwbat::GetSCFrequencySpacing();

	  switch (mcs)
	    {
	    case COUWBAT_MCS_BPSK_1_2:
            return GetFecBpskBer (snr,
                                  nbits,
                                  bw, // signal spread
                                  mode.GetPhyRate (), // phy rate
                                  10, // dFree
                                  11 // adFree
                                  );

	      break;
	    case COUWBAT_MCS_QPSK_1_2:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 4,  // m
                                 10, // dFree
                                 11, // adFree
                                 0   // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_QPSK_3_4:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 4, // m
                                 5, // dFree
                                 8, // adFree
                                 31 // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_16QAM_1_2:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 16, // m
                                 10, // dFree
                                 11, // adFree
                                 0   // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_16QAM_3_4:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 16, // m
                                 5,  // dFree
                                 8,  // adFree
                                 31  // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_64QAM_2_3:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 64, // m
                                 6,  // dFree
                                 1,  // adFree
                                 16  // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_64QAM_3_4:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 64, // m
                                 5,  // dFree
                                 8,  // adFree
                                 31  // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_64QAM_5_6:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 64, // m
                                 5,  // dFree
                                 8,  // adFree
                                 31  // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_256QAM_3_4:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 256, // m
                                 6,  // dFree
                                 1,  // adFree
                                 16  // adFreePlusOne
                                 );
	      break;
	    case COUWBAT_MCS_256QAM_5_6:
            return GetFecQamBer (snr,
                                 nbits,
                                 bw, // signal spread
                                 mode.GetPhyRate (), // phy rate
                                 256, // m
                                 6,  // dFree
                                 1,  // adFree
                                 16  // adFreePlusOne
                                 );
	      break;

	    default:
	      break;
	    }
    }

  return 0;
}
} // namespace ns3
