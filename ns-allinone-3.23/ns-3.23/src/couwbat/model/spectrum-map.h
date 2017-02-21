#ifndef SPECTRUM_MAP_H
#define SPECTRUM_MAP_H

#include "ns3/core-module.h"
#include <vector>

namespace ns3
{

/**
 * \brief Represent a spectrum by a vector of bits for each subcarrier.
 * \ingroup couwbat
 *
 * The spectrum map has Couwbat::GetNumberOfSubcarriers() subcarriers.
 */
class SpectrumMap : public Object
{
public :
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * Construct a SpectrumMap with Couwbat::GetNumberOfSubcarriers() subcarriers.
   */
  SpectrumMap (void); //!< Default constructor
  ~SpectrumMap (); //!< Destructor

  /**
   * \param start_subcarrier The first subcarrier to occupy.
   * \param nr_subcarriers The number of subcarriers to occupy.
   *
   * Occupie the given number of subcarriers in the spectrum, starting at
   * the given first subcarrier index.
   */
  void SetSpectrum (uint32_t start_subcarrier, uint32_t nr_subcarriers);

  /**
   * Return the value of the subcarrier at the requested position.
   * \param pos The requested subcarrier.
   * \return The value of the subcarrier at the requested position.
   */
  bool at (uint32_t pos);

  /**
   * \param otherMap An other map to do a logical OR with.
   * Perform a logical subcarrierwise OR between this spectrum
   * map and the other map and save the result in this map.
   */
  void OR (Ptr<SpectrumMap> otherMap);

  /**
   * \param otherMap An other map to do a logical XOR with.
   * Perform a logical subcarrierwise XOR between this spectrum
   * map and the other map and save the result in this map.
   */
  void XOR (Ptr<SpectrumMap> otherMap);

  std::vector<bool> m_map; // TODO make this private
}; // class SpectrumMap

std::ostream &operator << (std::ostream &os, const SpectrumMap &map);
std::ostream &operator << (std::ostream &os, const Ptr<SpectrumMap> &map);
std::istream &operator >> (std::istream &is, SpectrumMap &map);

ATTRIBUTE_HELPER_HEADER (SpectrumMap);

} // namespace ns3

#endif /* SPECTRUM_MAP_H */
