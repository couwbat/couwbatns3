#ifndef SIMPLE_COUWBAT_HELPER_H
#define SIMPLE_COUWBAT_HELPER_H

#include "couwbat-helper.h"
#include "ns3/core-module.h"

namespace ns3
{

class SimpleCouwbatChannel;
class Node;
class CouwbatNetDevice;

/**
 * \ingroup helper
 * 
 * \brief Manage and create couwbat channel objects with the simple model.
 */
class SimpleCouwbatChannelHelper
{
public:

  /**
   * Create a channel helper without any parameter set. The user must set
   * them all to be able to call Create later.
   */
  SimpleCouwbatChannelHelper ();

  /**
   * Create a channel helper in a default working state.
   */
  static SimpleCouwbatChannelHelper Default (void);

  /**
   * \returns a new channel
   *
   * Create a channel based on the configuration parameters set previously.
   */
  Ptr<SimpleCouwbatChannel> Create (void) const;
};


/**
 * \ingroup helper
 * 
 * \brief Manage and create couwbat phy objects with the simple model.
 */
class SimpleCouwbatPhyHelper : public CouwbatPhyHelper
{
public:
  /**
   * Create a phy helper without any parameter set. The user must set
   * them all to be able to call Install later.
   */
  SimpleCouwbatPhyHelper ();

  /**
   * Create a phy helper in a default working state.
   * \return phy helper in a default working state.
   */
  static SimpleCouwbatPhyHelper Default (void);

  /**
   * \param channel the channel to associate to this helper
   *
   * Every PHY created by a call to Install is associated to this channel.
   */
  void SetChannel (Ptr<SimpleCouwbatChannel> channel);

private:
  /**
   * \param node the node on which we wish to create a wifi PHY
   * \param device the device within which this PHY will be created
   * \returns a newly-created PHY object.
   *
   * This method implements the pure virtual method defined in \ref ns3::CouwbatPhyHelper.
   */
  virtual Ptr<CouwbatPhy> Create (Ptr<Node> node, Ptr<CouwbatNetDevice> device) const;

  ObjectFactory m_phyObjectFactory;
  Ptr<SimpleCouwbatChannel> m_channel;

};

/**
 * \ingroup helper
 * 
 * \brief Manage and create couwbat mac object with the simple model.
 */
class SimpleCouwbatMacHelper : public CouwbatMacHelper
{
public:
  /**
   * Create a mac helper without any parameters set. The user must set
   * them all to be able to call Install later.
   */
  SimpleCouwbatMacHelper ();

  /**
   * Create a mac helper in a default working state.
   * \return the a mac helper i a default working state.
   */
  static SimpleCouwbatMacHelper Default (void);

  /**
   * \param type the type to install
   * \param n0 the name of the attribute to set
   * \param v0 the value of the attribute to set
   * \param n1 the name of the attribute to set
   * \param v1 the value of the attribute to set
   * \param n2 the name of the attribute to set
   * \param v2 the value of the attribute to set
   * \param n3 the name of the attribute to set
   * \param v3 the value of the attribute to set
   * \param n4 the name of the attribute to set
   * \param v4 the value of the attribute to set
   * \param n5 the name of the attribute to set
   * \param v5 the value of the attribute to set
   * \param n6 the name of the attribute to set
   * \param v6 the value of the attribute to set
   * \param n7 the name of the attribute to set
   * \param v7 the value of the attribute to set
   *
   * Every MAC created by a call to Install has this mac type.
   *
   * All the attributes specified in this method should exist
   * in the requested mac.
   */
  virtual void SetType 
    (
      std::string type,
      std::string n0 = "", const AttributeValue &v0 = EmptyAttributeValue (),
      std::string n1 = "", const AttributeValue &v1 = EmptyAttributeValue (),
      std::string n2 = "", const AttributeValue &v2 = EmptyAttributeValue (),
      std::string n3 = "", const AttributeValue &v3 = EmptyAttributeValue (),
      std::string n4 = "", const AttributeValue &v4 = EmptyAttributeValue (),
      std::string n5 = "", const AttributeValue &v5 = EmptyAttributeValue (),
      std::string n6 = "", const AttributeValue &v6 = EmptyAttributeValue (),
      std::string n7 = "", const AttributeValue &v7 = EmptyAttributeValue ()
    );

private:
  /**
   * \param device the net device this mac will reside in
   * \returns a newly-created MAC object.
   *
   * This method implements the pure virtual method defined in \ref ns3::CouwbatMacHelper.
   */
  virtual Ptr<CouwbatMac> Create (Ptr<CouwbatNetDevice> device) const;

  ObjectFactory m_macObjectFactory;
};

} // namespace ns3

#endif /* SIMPLE_COUWBAT_HELPER_H */

