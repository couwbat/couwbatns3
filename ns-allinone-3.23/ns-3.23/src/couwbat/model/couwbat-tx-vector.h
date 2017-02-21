#ifndef COUWBAT_TX_VECTOR_H
#define COUWBAT_TX_VECTOR_H

#include "couwbat-mode.h"
#include <ostream>

namespace ns3 {


/**
 * \ingroup couwbat
 * 
 * This class mimics the TXVECTOR which is to be
 * passed to the PHY in order to define the parameters which are to be
 * used for a transmission.
 * 
 * Original code base taken from corresponding class in ns3 Wi-Fi module.
 */
class CouwbatTxVector
{
public:
  CouwbatTxVector ();
  /**
   * Create a TXVECTOR with the given parameters.
   *
   * \param mode CouwbatMode - mcs mode
   * \param powerLevel transmission power level
   */
  CouwbatTxVector (CouwbatMode mode, uint8_t powerLevel);
  /**
   *  \returns the txvector payload mode
   */
  CouwbatMode GetMode (void) const;
  /**
  * Sets the selected payload transmission mode
  *
  * \param mode
  */
  void SetMode (CouwbatMode mode);
  /**
   *  \returns the transmission power level
   */
  uint8_t GetTxPowerLevel (void) const;
  /**
   * Sets the selected transmission power level
   *
   * \param powerlevel
   */
  void SetTxPowerLevel (uint8_t powerlevel);

private:

  CouwbatMode m_mode;         /**< The DATARATE parameter in Table 15-4.
                           It is the value that will be passed
                           to PMD_RATE.request */
  uint8_t  m_txPowerLevel;  /**< The TXPWR_LEVEL parameter in Table 15-4.
                           It is the value that will be passed
                           to PMD_TXPWRLVL.request */

};

/**
 * Serialize CouwbatTxVector to the given ostream.
 *
 * \param os
 * \param v
 * \return ostream
 */
std::ostream & operator << (std::ostream & os,const CouwbatTxVector &v);

bool operator == (const CouwbatTxVector& first, const CouwbatTxVector& second);

} // namespace ns3

#endif // COUWBAT_TX_VECTOR_H
