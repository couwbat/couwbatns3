#ifndef COUWBAT_MODE_H
#define COUWBAT_MODE_H

#include <vector>
#include <ostream>
#include <stdint.h>

namespace ns3 {

/**
 * \ingroup couwbat
 * 
 * This enumeration defines the modulation classes
 */
enum CouwbatModulationClass
{
  COUWBAT_MOD_CLASS_UNKNOWN = 0, //!< Modulation class unknown or unspecified. A Mode with this COUWBAT_MOD_CLASS_UNKNOWN has not been properly initialised.
  COUWBAT_MOD_CLASS_OFDM //!< so far only OFDM is supported
};

/**
 * \ingroup couwbat
 * 
 * This enumeration defines the various MCS used for the OFDM transmission modes.
 */
enum CouwbatMCS
{
  // Base 3-bit MCS
  COUWBAT_MCS_SUBCARRIER_NOT_AVAILABLE = 0, //!< Indicates that this subchannel/subcarrier remains unused
  COUWBAT_MCS_QPSK_1_2,
  COUWBAT_MCS_QPSK_3_4,
  COUWBAT_MCS_16QAM_1_2,
  COUWBAT_MCS_16QAM_3_4,
  COUWBAT_MCS_64QAM_1_2,
  COUWBAT_MCS_64QAM_2_3,
  COUWBAT_MCS_64QAM_3_4,

  // MCS without puncturing
  COUWBAT_MCS_QPSK,
  COUWBAT_MCS_16QAM,
  COUWBAT_MCS_64QAM,

  // Extra/legacy MCS
  COUWBAT_MCS_BPSK_1_2 = 50,
  COUWBAT_MCS_64QAM_5_6,
  COUWBAT_MCS_256QAM_3_4,
  COUWBAT_MCS_256QAM_5_6,
};

/**
 * \ingroup couwbat
 * 
 * Container for PHY parameters such as modulation class, PHY rate, MCS and subchannels
 */
class CouwbatMode
{
public:
  CouwbatMode ();
  CouwbatMode (enum CouwbatModulationClass modClass,
               bool isMandatory,
               std::vector<uint32_t> subchannels,
               std::vector<enum CouwbatMCS> mcs);

  /**
   * \returns the number of Hz used by this signal
   */
  std::vector<uint32_t> GetSubchannels (void) const;

  /**
   * \returns the physical bit rate of this signal.
   *
   * If a transmission mode uses 1/2 FEC, and if its
   * data rate is 3Mbs, the phy rate is 6Mbs
   */
  uint64_t GetPhyRate (void) const;

  /**
   * \returns the coding rate of this transmission mode
   */
  std::vector<enum CouwbatMCS> GetMCS (void) const;

  /**
   * \returns true if this mode is a mandatory mode, false
   *          otherwise.
   */
  bool IsMandatory (void) const;

  /**
   *
   * \returns the Modulation Class
   */
  enum CouwbatModulationClass GetModulationClass () const;

private:
  enum CouwbatModulationClass m_modClass;
  bool m_isMandatory;
  std::vector<uint32_t> m_subchannels;
  std::vector<enum CouwbatMCS> m_mcs;
  uint32_t m_phyRate;
};

bool operator == (const CouwbatMode &a, const CouwbatMode &b); //!< Compre two modes for equality
bool operator != (const CouwbatMode &a, const CouwbatMode &b); //!< Compare two modes for inequality
std::ostream & operator << (std::ostream & os, const CouwbatMode &mode); //!< Print a mode
//std::istream & operator >> (std::istream &is, CouwbatMode &mode);

} // namespace ns3

#endif /* COUWBAT_MODE_H */
