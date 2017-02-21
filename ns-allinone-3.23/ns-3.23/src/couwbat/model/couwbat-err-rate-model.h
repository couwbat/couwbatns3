#ifndef COUWBAT_ERROR_RATE_MODEL_H
#define COUWBAT_ERROR_RATE_MODEL_H

#include <stdint.h>
#include "couwbat-mode.h"
#include "ns3/object.h"

namespace ns3 {
/**
 * \ingroup couwbat
 * \brief The interface for couwbat error models. Original code base taken from corresponding class in ns3 Wi-Fi module.
 *
 */
class CouwbatErrorRateModel : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * \param txMode a specific transmission mode
   * \param ber a target ber
   * \returns the snr which corresponds to the requested
   *          ber.
   */
  double CalculateSnr (CouwbatMode txMode, enum CouwbatMCS mcs, double ber) const;

  /**
   * A pure virtual method that must be implemented in the subclass.
   * This method returns the probability that the given 'chuck' of the
   * packet will be successfully received by the PHY.
   *
   * A chuck can be viewed as a part of a packet with equal SNR.
   * The probability of successfully receiving the chunk depends on
   * the mode, the SNR, and the size of the chunk.
   *
   * \param mode the Wi-Fi mode the chunk is sent
   * \param snr the SNR of the chunk
   * \param nbits the number of bits in this chunk
   * \return probability of successfully receiving the chunk
   */
  virtual double GetChunkSuccessRate (CouwbatMode mode, CouwbatMCS mcs, double snr, uint32_t nbits) const;

private:
  /**
   * Return the logarithm of the given value to base 2.
   *
   * \param val
   * \return the logarithm of val to base 2.
   */
  double Log2 (double val) const;
  /**
   * Return BER of BPSK with the given parameters.
   *
   * \param snr snr value
   * \param signalSpread
   * \param phyRate
   * \return BER of BPSK at the given SNR
   */
  double GetBpskBer (double snr, uint32_t signalSpread, uint32_t phyRate) const;
  /**
   * Return BER of QAM-m with the given parameters.
   *
   * \param snr snr value
   * \param m
   * \param signalSpread
   * \param phyRate
   * \return BER of BPSK at the given SNR
   */
  double GetQamBer (double snr, unsigned int m, uint32_t signalSpread, uint32_t phyRate) const;
  /**
   * Return k!
   *
   * \param k
   * \return k!
   */
  uint32_t Factorial (uint32_t k) const;
  /**
   * Return Binomial distribution for a given k, p, and n
   *
   * \param k
   * \param p
   * \param n
   * \return a Binomial distribution
   */
  double Binomial (uint32_t k, double p, uint32_t n) const;
  /**
   * \param ber
   * \param d
   * \return double
   */
  double CalculatePdOdd (double ber, unsigned int d) const;
  /**
   * \param ber
   * \param d
   * \return double
   */
  double CalculatePdEven (double ber, unsigned int d) const;
  /**
   * \param ber
   * \param d
   * \return double
   */
  double CalculatePd (double ber, unsigned int d) const;
  /**
   * \param snr
   * \param nbits
   * \param signalSpread
   * \param phyRate
   * \param dFree
   * \param adFree
   * \return double
   */
  double GetFecBpskBer (double snr, double nbits,
                        uint32_t signalSpread, uint32_t phyRate,
                        uint32_t dFree, uint32_t adFree) const;
  /**
   * \param snr
   * \param nbits
   * \param signalSpread
   * \param phyRate
   * \param m
   * \param dfree
   * \param adFree
   * \param adFreePlusOne
   * \return double
   */
  double GetFecQamBer (double snr, uint32_t nbits,
                       uint32_t signalSpread,
                       uint32_t phyRate,
                       uint32_t m, uint32_t dfree,
                       uint32_t adFree, uint32_t adFreePlusOne) const;
};

} // namespace ns3

#endif /* COUWBAT_ERROR_RATE_MODEL_H */
