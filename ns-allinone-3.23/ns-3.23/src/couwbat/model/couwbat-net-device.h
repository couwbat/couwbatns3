#ifndef COUWBAT_NET_DEVICE_H
#define COUWBAT_NET_DEVICE_H

#include <map>
#include "ns3/net-device.h"

namespace ns3
{

class CouwbatMac;
class CouwbatPhy;
class Channel;
class Address;
class SpectrumDb;

/**
 * \ingroup couwbat
 * 
 * \brief Hold together all Couwbat-related objects.
 *
 * This class holds together ns3::CouwbatChannel, ns3::CouwbatPhy
 * and ns3::CouwbatMac.
 * 
 * Includes resolution table to convert IP addresses to MAC addresses
 * using manually added translation entries at the start of runtime
 * with AddTranslatonEntry().
 */
class CouwbatNetDevice : public NetDevice
{
public:
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);
  CouwbatNetDevice (); //!< Default constructor
  virtual ~CouwbatNetDevice (); //!< Destructor

  /**
   * Set the mac layer of this device.
   * \param mac The mac layer.
   */
  void SetMac (Ptr<CouwbatMac> mac);

  /**
   * Set the phy layer of this device.
   * \param phy The phy layer.
   */
  void SetPhy (Ptr<CouwbatPhy> phy);

  /**
   * Return the mac layer of this device.
   * \return The mac layer of this device.
   */
  Ptr<CouwbatMac> GetMac (void);

  /**
   * Return the phy layer of this device.
   * \return The phy layer of this device.
   */
  Ptr<CouwbatPhy> GetPhy (void);

  /**
   * \param node The node that contains the spectrum database to use.
   * 
   * Set the spectrum database. The database is accessed directly through
   * function calls. The communication between a CR-BS and the database
   * is NOT simulated.
   * 
   * TODO create a ServerSpectrumDbApplication and ClientSpectrumDbApplication
   */
  void UseDirectSpecDb (Ptr<Node> node);

  /**
   * \param specDb The spectrum database to use.
   * 
   * Set the spectrum database. The database is accessed directly through
   * function calls. The communication between a CR-BS and the database
   * is NOT simulated.
   * 
   * TODO create a ServerSpectrumDbApplication and ClientSpectrumDbApplication
   */
  void UseDirectSpecDb (Ptr<SpectrumDb> specDb);

  /**
   * Forward up a packet to upper ns3 layers.
   */
  void ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to);

  //inherited from NetDevice base class
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual Address GetMulticast (Ipv6Address addr) const;
  virtual bool IsBridge (void) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

  /**
   * Add an IP address to MAC address resolution entry
   */
  void AddTranslatonEntry (Ipv4Address from, Mac48Address to);
  
  /**
   * Translate an IP address to a MAC address.
   */
  Mac48Address TranslateAddress (Ipv4Address addr);

protected:
  /**
   * Initialize the mac, phy and NetDevice base class.
   */
  virtual void DoInitialize (void);

private:
  static const uint16_t MAX_MTU = 2304; //!< TODO rename this and find useful values; MTU value is not used at all and packets are forwarded as given, i.e. no splitting is implemented.
  uint16_t m_mtu;
  uint32_t m_ifIndex; //!< The interface index of this device.
  Ptr<SpectrumDb> m_specDb; //!< The spectrum db implementation for this device.
  Ptr<CouwbatMac> m_mac; //!< The mac layer.
  Ptr<CouwbatPhy> m_phy; //!< The phy layer.
  Ptr<Node> m_node; //!< The node of this device.
  NetDevice::ReceiveCallback m_forwardUp; //!< Method to call when forwarding received packets to the upper layer.
  std::map<Ipv4Address, Mac48Address> m_ipToMacTable; //!< IP to MAC translation table

};

} // namespace ns3
#endif /* COUWBAT_NET_DEVICE_H */
