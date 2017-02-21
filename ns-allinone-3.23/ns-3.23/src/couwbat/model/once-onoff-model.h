#ifndef ONCE_ONOFF_MODEL_H
#define ONCE_ONOFF_MODEL_H

#include "ns3/onoff-model.h"

namespace ns3
{

/**
 * \brief Implement an OnOffModel that turns the PU on at a
 * specified time and off at a specified time.
 * \ingroup couwbat
 *
 * The OnceOnOffModel turns a PU on once a specified time and off once
 * at a specified time.
 */
class OnceOnOffModel : public OnOffModel
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  OnceOnOffModel (); //!< Default constructor
  ~OnceOnOffModel (); //!< Destructor

  /**
   * Initialize and start this model.
   * Inherited from OnOffModel.
   */
  void Start (void);

private:
  /**
   * Turn the PU on.
   */
  void TurnOn (void);

  /**
   * Turn the PU off.
   */
  void TurnOff(void);

  bool m_runIndefinitely; //!< True if the PU will not be turned off.
};

} // namespace ns3

#endif /* ONCE_ONOFF_MODEL_H */ 
