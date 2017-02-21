#ifndef PERIODIC_ONOFF_MODEL_H
#define PERIODIC_ONOFF_MODEL_H

#include "ns3/onoff-model.h"

namespace ns3
{

class Time;

/**
 * \brief An OnOffModel that turns the PU periodically on
 * and off according to some specified on and off periods.
 *
 * \ingroup couwbat
 *
 * The Attributes PeriodOn and PeriodOff define how long
 * to keep the PU on and off in each period after the first
 * on until the final off.
 */
class PeriodicOnOffModel : public OnOffModel
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  PeriodicOnOffModel (); //!< Default constructor
  ~PeriodicOnOffModel (); //!< Destructor

  /**
   * Initialize and start this model.
   * Inherited from OnOffModel.
   */
  void Start (void);

private:

  /**
   * Turn the PU on and schedule the next time when to turn if off.
   */
  void TurnOn (void);

  /**
   * Turn the PU off and schedule the next time when to turn if on.
   */
  void TurnOff (void);

  Time m_pOn; //!< Periodic duration the primary user stays turned on. Gets turned off afterwards.
  Time m_pOff; //!< Periodic duration the primary user stays turned off. Gets turned on afterwards.
  bool m_runIndefinitely; //!< True if the PU will run indefinitely.
};

} // namespace ns3

#endif /* PERIODIC_ONOFF_MODEL_H */ 

