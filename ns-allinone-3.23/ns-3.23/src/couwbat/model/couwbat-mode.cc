#include "couwbat-mode.h"
#include "couwbat.h"
#include "ns3/assert.h"
#include "ns3/fatal-error.h"

namespace ns3 {

/**
 * Check if the two CouwbatModes are identical.
 *
 * \param a CouwbatMode
 * \param b CouwbatMode
 * \return true if the two CouwbatModes are identical,
 *         false otherwise
 */
bool operator == (const CouwbatMode &a, const CouwbatMode &b)
{
  return a.GetSubchannels () == b.GetSubchannels ()
      && a.GetPhyRate () == b.GetPhyRate ()
      && a.GetMCS () == b.GetMCS ()
      && (a.IsMandatory () == b.IsMandatory ())
      && a.GetModulationClass () == b.GetModulationClass ();
}

bool operator != (const CouwbatMode &a, const CouwbatMode &b)
{
  return !(a == b);
}

/**
 * Serialize CouwbatMode to ostream (human-readable).
 *
 * \param os std::ostream
 * \param mode
 * \return std::ostream
 */
std::ostream & operator << (std::ostream & os, const CouwbatMode &mode)
{
  return os << "SomeCouwbatMode";
}

/**
 * Serialize CouwbatMode from istream (human-readable).
 *
 * \param is std::istream
 * \param mode
 * \return std::istream
 */
//std::istream & operator >> (std::istream &is, CouwbatMode &mode)
//{
//  return is;
//}

CouwbatMode::CouwbatMode ()
  : m_modClass (COUWBAT_MOD_CLASS_UNKNOWN),
    m_isMandatory (false),
    m_mcs (COUWBAT_MCS_QPSK_1_2),
    m_phyRate (0)
{
}

CouwbatMode::CouwbatMode (enum CouwbatModulationClass modClass,
                          bool isMandatory,
                          std::vector<uint32_t> subchannels,
                          std::vector<enum CouwbatMCS> mcs)
  : m_modClass (modClass),
    m_isMandatory (isMandatory),
    m_subchannels (subchannels),
    m_mcs (mcs)
{
  NS_ASSERT (subchannels.size () == mcs.size ());
  m_phyRate = 0;
  for (unsigned int i = 0; i < mcs.size (); ++i)
    {
      int bits_per_symb = 0;
      switch (mcs[i])
        {
        case COUWBAT_MCS_BPSK_1_2:
          bits_per_symb = 0.5;
          break;
        case COUWBAT_MCS_QPSK_1_2:
          bits_per_symb = 1;
          break;
        case COUWBAT_MCS_QPSK_3_4:
          bits_per_symb = 1.5;
          break;
        case COUWBAT_MCS_QPSK:
          bits_per_symb = 2;
          break;
        case COUWBAT_MCS_16QAM_1_2:
          bits_per_symb = 2;
          break;
        case COUWBAT_MCS_16QAM_3_4:
          bits_per_symb = 3;
          break;
        case COUWBAT_MCS_16QAM:
          bits_per_symb = 4;
          break;
        case COUWBAT_MCS_64QAM_2_3:
          bits_per_symb = 4;
          break;
        case COUWBAT_MCS_64QAM_3_4:
          bits_per_symb = 4.5;
          break;
        case COUWBAT_MCS_64QAM_5_6:
          bits_per_symb = 5;
          break;
        case COUWBAT_MCS_64QAM:
          bits_per_symb = 6;
          break;
        case COUWBAT_MCS_256QAM_3_4:
          bits_per_symb = 6;
          break;
        case COUWBAT_MCS_256QAM_5_6:
          bits_per_symb = 5*8/6;
          break;
        case COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE:
        default:
          bits_per_symb = 0;
          break;
        }

      m_phyRate += bits_per_symb
          * Couwbat::GetNumberOfDataSubcarriersPerSubchannel()
          / (Couwbat::GetSymbolDuration() * 1.0e-6);
    }
}

std::vector<uint32_t>
CouwbatMode::GetSubchannels (void) const
{
  return m_subchannels;
}

uint64_t
CouwbatMode::GetPhyRate (void) const
{
  return m_phyRate;
}

std::vector<enum CouwbatMCS>
CouwbatMode::GetMCS (void) const
{
  return m_mcs;
}

bool
CouwbatMode::IsMandatory (void) const
{
  return m_isMandatory;
}
enum CouwbatModulationClass
CouwbatMode::GetModulationClass () const
{
  return m_modClass;
}

} // namespace ns3
