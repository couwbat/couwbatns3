#include "simple-couwbat-channel.h"
#include "simple-couwbat-phy.h"
#include "ns3/network-module.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/pointer.h"
#include "ns3/object-factory.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/couwbat-packet-helper.h" // for printing of std::vector<double>
#include "ns3/couwbat.h"

NS_LOG_COMPONENT_DEFINE ("SimpleCouwbatChannel");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (SimpleCouwbatChannel);

TypeId
SimpleCouwbatChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleCouwbatChannel")
    .SetParent<CouwbatChannel> ()
    .SetGroupName("Couwbat")
    .AddConstructor<SimpleCouwbatChannel> ()
    .AddAttribute ("PropagationDelayModel", "A pointer to the propagation delay model attached to this channel.",
                   PointerValue (),
                   MakePointerAccessor (&SimpleCouwbatChannel::m_delay),
                   MakePointerChecker<PropagationDelayModel> ())
  ;
  return tid;
}

SimpleCouwbatChannel::SimpleCouwbatChannel ()
{
  NS_LOG_FUNCTION (this);
}

SimpleCouwbatChannel::~SimpleCouwbatChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_phyList.clear ();
}

void
SimpleCouwbatChannel::SetPropagationLossModel (std::vector<Ptr<PropagationLossModel> > loss)
{
  m_loss = loss;
}
void
SimpleCouwbatChannel::SetPropagationDelayModel (Ptr<PropagationDelayModel> delay)
{
  m_delay = delay;
}

void
SimpleCouwbatChannel::Send (Ptr<SimpleCouwbatPhy> sender, Ptr<const Packet> packet, double txPowerDbm,
		CouwbatTxVector txVector) const
{

  NS_LOG_INFO ("SimpleCouwbatChannel::Send()");

  Ptr<MobilityModel> senderMobility = sender->GetMobility ()->GetObject<MobilityModel> ();
  NS_ASSERT (senderMobility != 0);
  uint32_t j = 0;
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++, j++)
    {
      if (sender != (*i))
        {
          // For now don't account for inter channel interference
//          if ((*i)->GetChannelNumber () != sender->GetChannelNumber ())
//            {
//              continue;
//            }

          Ptr<MobilityModel> receiverMobility = (*i)->GetMobility ()->GetObject<MobilityModel> ();
          NS_ASSERT (receiverMobility != 0);

          Time delay = m_delay->GetDelay (senderMobility, receiverMobility);

          std::vector<double> rxPowerDbm;
          const std::vector<uint32_t> subch = txVector.GetMode ().GetSubchannels ();

          for (uint32_t i = 0; i < subch.size (); ++i)
            {
                // if we are sending on this subchannel, calculate RxPowerDbm
                rxPowerDbm.push_back (m_loss[subch[i]]->CalcRxPower (txPowerDbm, senderMobility, receiverMobility));
            }

          NS_LOG_INFO ("propagation: txPower=" << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, " <<
                        "distance=" << senderMobility->GetDistanceFrom (receiverMobility) << "m, delay=" << delay);

          Ptr<Packet> copy = packet->Copy ();
          Ptr<Object> dstNetDevice = m_phyList[j]->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode ()->GetId ();
            }
          Simulator::ScheduleWithContext (dstNode,
                                          delay, &SimpleCouwbatChannel::Receive, this,
                                          j, copy, rxPowerDbm, txVector);
        }
    }
    NS_LOG_INFO ("Successfully sent on channel: '" << packet->ToString () << "'");
}

void
SimpleCouwbatChannel::Receive (uint32_t i, Ptr<Packet> packet, std::vector<double> rxPowerDbm,
                          CouwbatTxVector txVector) const
{
  NS_LOG_INFO ("SimpleCouwbatChannel::Receive() => " << i);

  m_phyList[i]->StartReceivePacket (packet, rxPowerDbm, txVector);
}

uint32_t
SimpleCouwbatChannel::GetNDevices (void) const
{
  return m_phyList.size ();
}
Ptr<NetDevice>
SimpleCouwbatChannel::GetDevice (uint32_t i) const
{
  return m_phyList[i]->GetDevice ()->GetObject<NetDevice> ();
}

void
SimpleCouwbatChannel::Add (Ptr<SimpleCouwbatPhy> phy)
{
  m_phyList.push_back (phy);
}

int64_t
SimpleCouwbatChannel::AssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  for (uint32_t i = 0; i < m_loss.size (); ++i)
    {
      currentStream += m_loss[i]->AssignStreams (currentStream);
    }
  return (currentStream - stream);
}

} // namespace ns3
