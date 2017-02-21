#include "couwbat-tx-vector.h"

namespace ns3 {

CouwbatTxVector::CouwbatTxVector ()
  : m_txPowerLevel (0)
{
}

CouwbatTxVector::CouwbatTxVector (CouwbatMode mode, uint8_t powerLevel)
  : m_mode (mode),
    m_txPowerLevel (powerLevel)
{
}

CouwbatMode
CouwbatTxVector::GetMode (void) const
{
  return m_mode;
}
uint8_t
CouwbatTxVector::GetTxPowerLevel (void) const
{
  return m_txPowerLevel;
}
void
CouwbatTxVector::SetMode (CouwbatMode mode)
{
  m_mode=mode;
}
void
CouwbatTxVector::SetTxPowerLevel (uint8_t powerlevel)
{
  m_txPowerLevel=powerlevel;
}

std::ostream & operator << ( std::ostream &os, const CouwbatTxVector &v)
{
  os << "mode:" << v.GetMode() << " txpwrlvl:" << v.GetTxPowerLevel();
  return os;
}

bool operator == (const CouwbatTxVector& first, const CouwbatTxVector& second)
{
  return first.GetMode () == second.GetMode ();
}

} // namespace ns3
