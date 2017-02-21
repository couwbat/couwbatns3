#ifndef PU_NET_DEVICE_H
#define PU_NET_DEVICE_H

#include "ns3/core-module.h"

namespace ns3
{

class SpectrumMap;
class SpectrumDb;
class OnOffModel;
class Node;

/**
 * \brief Model for a Primary User that occupies a spectrum
 * according to a OnOffModel and inform the SpectrumDb
 * about the usage.
 *
 * \ingroup couwbat
 *
 * The PrimaryUserNetDevice uses a definable spectrum that it
 * occupies and leaves according to a definable OnOffModel.
 * The PrimaryUserNetDevice informs a spectrum database via direct
 * memory access when it occupies and leaves the spectrum.
 */
class PrimaryUserNetDevice : public Object
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  PrimaryUserNetDevice (void); //!< Default constructor
  ~PrimaryUserNetDevice (); //!< Destructor

  /**
   * Set the spectrum to occupy.
   * \param map The spectrum to occupy.
   */
  void SetSpectrumMap (Ptr<SpectrumMap> map);

  /**
   * Set the spectrum database.
   * \param db The spectrum database.
   */
  void SetSpectrumDb (Ptr<SpectrumDb> db);

  /**
   * Set the on-off model.
   * \param The on-off model.
   */
  void SetOnOffModel (Ptr<OnOffModel> model);

  /**
   * Set the node that this PU is associated to.
   * \param node The node that this PU is associated to.
   */
  void SetNode (Ptr<Node> node);

  /**
   * Return the node that this PU is associated to.
   * \return The node that this PU is associated to.
   */
  Ptr<Node> GetNode (void) const;

  /**
   * Initialize this PU and start operation.
   */
  void Start (void);

  /**
   * Informs the Spectrum database about the used Spectrum.
   */
  void TurnOn (void);

  /**
   * Informs the spectrum database about leaving the used spectrum.
   */
  void TurnOff (void);

private :
  Ptr<SpectrumMap> m_specMap; //!< The spectrum to occupy.
  Ptr<SpectrumDb> m_specDb; //!< The spectrum database.
  Ptr<OnOffModel> m_onOffModel; //!< The on-off model.
  Ptr<Node> m_node; //!< The node that this PU is associated to.
}; // class PrimaryUserNetDevice

} // namespace ns3

#endif /* PU_NET_DEVICE_H */
