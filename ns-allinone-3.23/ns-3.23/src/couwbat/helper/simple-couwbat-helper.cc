#include "simple-couwbat-helper.h"
#include "ns3/simple-couwbat-channel.h"
#include "ns3/simple-couwbat-phy.h"
#include "ns3/couwbat-net-device.h"
#include "ns3/network-module.h"
#include "ns3/couwbat-mac.h"
#include "ns3/log.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/couwbat.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/okumura-hata-propagation-loss-model.h"

NS_LOG_COMPONENT_DEFINE ("SimpleCouwbatHelper");

namespace ns3
{

/*
 * ==============================================
 * SimpleCouwbatChannelHelper
 * ==============================================
 */
SimpleCouwbatChannelHelper::SimpleCouwbatChannelHelper ()
{
  NS_LOG_FUNCTION (this);
}

SimpleCouwbatChannelHelper
SimpleCouwbatChannelHelper::Default (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  SimpleCouwbatChannelHelper helper;
  return helper;
}

Ptr<SimpleCouwbatChannel>
SimpleCouwbatChannelHelper::Create () const
{
  NS_LOG_FUNCTION (this);
  Ptr<SimpleCouwbatChannel> channel = CreateObject<SimpleCouwbatChannel> ();
  Ptr<PropagationDelayModel> propDelay = CreateObject<ConstantSpeedPropagationDelayModel> ();
  std::vector<Ptr<PropagationLossModel> > propLoss;
  double sch0CenterFreq = Couwbat::GetSch0CenterFreq ();
  double schFreqSpacing = Couwbat::GetSCFrequencySpacing () * Couwbat::GetNumberOfSubcarriersPerSubchannel ();
  for (uint32_t i = 0; i < Couwbat::GetNumberOfSubchannels (); ++i)
    {
      double freq = sch0CenterFreq + (i * schFreqSpacing);

      // instantiate a PropagationLossModel for every subchannel

      //      HATA URBAN model for cellular planning
      //      Frequency (MHz) 150 to 1500MHz
      //      Base station height 30-200m
      //      Mobile station height 1-10m
      //      Distance 1-20km
      Ptr<OkumuraHataPropagationLossModel> model = CreateObject<OkumuraHataPropagationLossModel> ();
      model->SetAttribute ("Frequency", DoubleValue (freq));
      model->SetAttribute ("Environment", EnumValue (ns3::SmallCity));
      model->SetAttribute ("CitySize", EnumValue (ns3::UrbanEnvironment));

      Ptr<NakagamiPropagationLossModel> model2 = CreateObject<NakagamiPropagationLossModel> ();

      model->SetNext (model2);

      propLoss.push_back (model);
    }
  channel->SetPropagationDelayModel (propDelay);
  channel->SetPropagationLossModel (propLoss);

  return channel;
}


/*
 * ==============================================
 * SimpleCouwbatPhyHelper
 * ==============================================
 */
SimpleCouwbatPhyHelper::SimpleCouwbatPhyHelper ()
  : m_channel (0)
{
  NS_LOG_FUNCTION (this);
  m_phyObjectFactory.SetTypeId ("ns3::SimpleCouwbatPhy");
}

SimpleCouwbatPhyHelper
SimpleCouwbatPhyHelper::Default (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  SimpleCouwbatPhyHelper helper;
  return helper;
}

void
SimpleCouwbatPhyHelper::SetChannel (Ptr<SimpleCouwbatChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);
  m_channel = channel;
}

Ptr<CouwbatPhy>
SimpleCouwbatPhyHelper::Create (Ptr<Node> node, Ptr<CouwbatNetDevice> device ) const
{
  NS_LOG_FUNCTION (this << node << device);
  Ptr<SimpleCouwbatPhy> phy = m_phyObjectFactory.Create<SimpleCouwbatPhy> ();
  phy->SetChannel (m_channel);
  phy->SetMobility (node);
  phy->SetDevice (device);
  std::vector<Ptr<CouwbatErrorRateModel> > erms;
  for (uint32_t i = 0; i < Couwbat::GetNumberOfSubchannels (); ++i)
    {
      // instantiate a CouwbatErrorRateModel for every subchannel
      Ptr<CouwbatErrorRateModel> e = CreateObject<CouwbatErrorRateModel> ();
      erms.push_back (e);
    }
  phy->SetErrorRateModel (erms);
  return phy;
}

/*
 * ==============================================
 * SimpleCouwbatMacHelper
 * ==============================================
 */
SimpleCouwbatMacHelper::SimpleCouwbatMacHelper ()
{
  NS_LOG_FUNCTION (this);
}

SimpleCouwbatMacHelper
SimpleCouwbatMacHelper::Default (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  SimpleCouwbatMacHelper helper;
  return helper;
}

void
SimpleCouwbatMacHelper::SetType 
(
  std::string type,
  std::string n0, const AttributeValue &v0,
  std::string n1, const AttributeValue &v1,
  std::string n2, const AttributeValue &v2,
  std::string n3, const AttributeValue &v3,
  std::string n4, const AttributeValue &v4,
  std::string n5, const AttributeValue &v5,
  std::string n6, const AttributeValue &v6,
  std::string n7, const AttributeValue &v7
)
{
  NS_LOG_FUNCTION (this << type);
  m_macObjectFactory.SetTypeId (type);
  m_macObjectFactory.Set (n0, v0);
  m_macObjectFactory.Set (n1, v1);
  m_macObjectFactory.Set (n2, v2);
  m_macObjectFactory.Set (n3, v3);
  m_macObjectFactory.Set (n4, v4);
  m_macObjectFactory.Set (n5, v5);
  m_macObjectFactory.Set (n6, v6);
  m_macObjectFactory.Set (n7, v7);
}

Ptr<CouwbatMac>
SimpleCouwbatMacHelper::Create (Ptr<CouwbatNetDevice> device) const
{
  NS_LOG_FUNCTION (this);
  Ptr<CouwbatMac> mac = m_macObjectFactory.Create<CouwbatMac> ();
  mac->SetDevice (device);
  Ptr<CouwbatPhy> phy = device->GetPhy ();
  if (!phy)
    {
      NS_FATAL_ERROR ("Setting mac->m_phy to null pointer");
    }
  mac->SetPhy (phy);
  return mac;
}

} // namespace ns3
