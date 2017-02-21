#ifndef SPECTRUM_DB_H
#define SPECTRUM_DB_H

#include "ns3/core-module.h"

namespace ns3
{

class SpectrumMap;

/**
 * \brief A database that collects spectrum usage of primary users.
 * \ingroup couwbat
 *
 * The spectrum database collects spectrum usage of different primary users
 * and keeps a summary spectrum map which is the sum of all spectrum usage.
 *
 * TODO bugfix, if two PUs overlap in spectrum usage and one of them diasppers/vacates
 * the spectrum it's whole map is declared free, although the other still would occupy parts of that map.
 *
 */
class SpectrumDb : public Object
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  SpectrumDb (void); //!< Default constructor
  ~SpectrumDb (); //!< Destructor

  /**
   * Inform the database about the spectrum being used.
   * \param map A map of the spectrum being used.
   */
  void OccupySpectrum (Ptr<SpectrumMap> map);

  /**
   * Inform the database about the spectrum to free.
   * \param map A map of the spectrum to free.
   */
  void LeaveSpectrum (Ptr<SpectrumMap> map);

  /**
   * Return a new spectrum map that has the occupied subcarriers set. 
   * \return A new spectrum map that has the occupied subcarriers set.
   */
  Ptr<SpectrumMap> GetOccupiedSpectrum (void) const;

private:
  Ptr<SpectrumMap> m_specMap; //<! the summary of the used spectrum
}; // class SpectrumDb

} // namespace ns3

#endif /* SPECTRUM_DB_H */
