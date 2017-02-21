#ifndef RANDOM_ONOFF_MODEL_H
#define RANDOM_ONOFF_MODEL_H

#include "ns3/onoff-model.h"

namespace ns3
{

class Time;

/**
 * \brief An OnOffModel that turns the PU periodically on
 * and off for random periods specified by a random variable.
 *
 * \ingroup couwbat
 *
 * The Attribute RandomVariable defines how long to keep the
 * PU on and off in each period after the first on until
 * the final off.
 */
class RandomOnOffModel : public OnOffModel
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  RandomOnOffModel ();  //!< Default constructor
  ~RandomOnOffModel (); //!< Destructor

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

  bool m_runIndefinitely; //!< True if the PU will run indefinitely.
  Ptr<RandomVariableStream> m_randVar; //!< The random variable that returns the time (in seconds) for how long the primary user stays on/off in each period.
};

} // namespace ns3

#endif /* RANDOM_ONOFF_MODEL_H */ 


