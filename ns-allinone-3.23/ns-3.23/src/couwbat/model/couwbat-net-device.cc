#include "couwbat-net-device.h"
#include "spectrum-db.h"
#include "couwbat-mac.h"
#include "couwbat-phy.h"
#include "couwbat.h"
#include "couwbat-channel.h"
#include "ns3/network-module.h"
#include "ns3/log.h"
#include <stdexcept>
#include <cstdlib>
#include "ns3/ipv4-header.h"

NS_LOG_COMPONENT_DEFINE ("CouwbatNetDevice");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (CouwbatNetDevice);

TypeId
CouwbatNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CouwbatNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<CouwbatNetDevice> ()
    .SetGroupName ("Couwbat")
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (MAX_MTU),
                   MakeUintegerAccessor (&CouwbatNetDevice::SetMtu,
                                         &CouwbatNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> (1,MAX_MTU))
  ;
  return tid;
}

CouwbatNetDevice::CouwbatNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

CouwbatNetDevice::~CouwbatNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

void
CouwbatNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_mac->Initialize ();
  m_phy->Initialize ();
  NetDevice::DoInitialize ();
}

void
CouwbatNetDevice::SetMac (Ptr<CouwbatMac> mac)
{
  NS_LOG_FUNCTION (this << mac);
  m_mac = mac;
}

void
CouwbatNetDevice::SetPhy (Ptr<CouwbatPhy> phy)
{
  NS_LOG_FUNCTION (this << phy);
  m_phy = phy;
}

Ptr<CouwbatMac>
CouwbatNetDevice::GetMac (void)
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

Ptr<CouwbatPhy>
CouwbatNetDevice::GetPhy (void)
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void
CouwbatNetDevice::UseDirectSpecDb (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  UseDirectSpecDb (node->GetObject<SpectrumDb> ());
}

void
CouwbatNetDevice::UseDirectSpecDb (Ptr<SpectrumDb> specDb)
{
  NS_LOG_FUNCTION (this << specDb);
  m_specDb = specDb;
}

void
CouwbatNetDevice::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << from);
  NS_LOG_INFO (packet->ToString ());
  if (m_forwardUp.IsNull ())
    {
      NS_LOG_WARN ("CouwbatNetDevice::m_forwardUp null callback");
      return;
    }

  LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  enum NetDevice::PacketType type;
  if (to.IsBroadcast ())
    {
      type = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      type = NetDevice::PACKET_MULTICAST;
    }
  else if (to == m_mac->GetAddress ())
    {
      type = NetDevice::PACKET_HOST;
    }
  else
    {
      type = NetDevice::PACKET_OTHERHOST;
    }

  if (type != NetDevice::PACKET_OTHERHOST)
    {
      bool callbackSuccessful = m_forwardUp (this, packet, llc.GetType (), from);
      NS_LOG_INFO ("ForwardUp callback" << (callbackSuccessful ? "" : " not") << " successful");
    }
}

void 
CouwbatNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
  m_ifIndex = index;
}

uint32_t 
CouwbatNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ifIndex;
}

Ptr<Channel> 
CouwbatNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
CouwbatNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_mac->SetAddress (Mac48Address::ConvertFrom (address));
}

Address
CouwbatNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mac->GetAddress ();
}
  
bool
CouwbatNetDevice::SetMtu (const uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  if (mtu > MAX_MTU)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}

uint16_t 
CouwbatNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}
  
bool 
CouwbatNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  if (!m_phy || !m_mac) return false;
  return true;
}

void 
CouwbatNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  // TODO to be implemented
}

bool 
CouwbatNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
CouwbatNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address::GetBroadcast ();
}

bool
CouwbatNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

Address
CouwbatNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address::GetMulticast (multicastGroup);
}

Address 
CouwbatNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address::GetMulticast (addr);
}

bool
CouwbatNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}
bool
CouwbatNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool 
CouwbatNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (Mac48Address::IsMatchingType (dest));

  Ipv4Header iph;
  packet->PeekHeader (iph);
  Mac48Address realTo = TranslateAddress(iph.GetDestination ());

  if (realTo == Mac48Address::GetBroadcast ())
    {
      NS_LOG_ERROR ("CouwbatNetDevice::Send(): MAC address for IP " << iph.GetDestination () << " unknown, send failed.");
      return false;
    }

  if (Couwbat::mac_queue_limit_enabled && GetMac ()->GetTxQueueSize (realTo) > Couwbat::mac_queue_limit_size)
    {
      return false;
    }

  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);

  m_mac->Enqueue (packet, realTo);
  return true;
}
  
bool 
CouwbatNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this);
  // TODO to be implemented
  return false;
}

Ptr<Node> 
CouwbatNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}
  
void 
CouwbatNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

bool
CouwbatNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void
CouwbatNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  m_forwardUp = cb;
}

void 
CouwbatNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
  // TODO to be implemented
}
  
bool 
CouwbatNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void
CouwbatNetDevice::AddTranslatonEntry (Ipv4Address from, Mac48Address to)
{
  m_ipToMacTable[from] = to;
}

Mac48Address
CouwbatNetDevice::TranslateAddress (Ipv4Address addr)
{
  try
    {
      return m_ipToMacTable.at (addr);
    }
  catch (const std::out_of_range& oor)
    {
    }
  return Mac48Address::GetBroadcast ();
}

} // namespace ns3
