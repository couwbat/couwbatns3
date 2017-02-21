#ifndef ONOFF_MODEL_H
#define ONOFF_MODEL_H

#include "ns3/core-module.h"

namespace ns3
{

class PrimaryUserNetDevice;

/**
 * \brief Abstract class to turn a PU on and off.
 * \ingroup couwbat
 *
 * Inherit from this class to create an OnOffModel that
 * turn a PU on and off according to a specific pattern.
 * This class offers methods to interface with the PU and
 * attributes to specify the first time when to turn the PU
 * on and the last time when to turn it off.
 */
class OnOffModel : public Object
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * Initialize the OnOffModel and start it.
   */
  virtual void Start (void) = 0;

  /**
   * Return the PU that this OnOffModel turns on and off.
   * \param puNetDevice The PU that this OnOffModel turns on and off.
   */
  void SetPuNetDevice (Ptr<PrimaryUserNetDevice> puNetDevice);

  /**
   * Return the PU that this OnOffModel turns on and off.
   * \return The PU that this OnOffModel turns on and off.
   */
  Ptr<PrimaryUserNetDevice> GetPuNetDevice (void) const;

  /**
   * Return the time when to first turn the PU on.
   * \return The time when to first turn the PU on.
   */
  Time GetFirstOn (void) const;

  /**
   * Return the time when to finally turn the PU off.
   * \return The time when to finally turn the PU off.
   */
  Time GetFinalOff (void) const;

private:
  Ptr<PrimaryUserNetDevice> m_puNetDevice; //!< the PU to turn on and off
  Time m_firstOn; //!< The time when to first turn the PU on.
  Time m_finalOff; //!< The time when to finally turn the PU off.
};

} // namespace ns3

#endif /* ONOFF_MODEL_H */ 

