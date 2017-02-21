/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "couwbat-helper.h"
#include "ns3/pu-net-device.h"
#include "ns3/trace-based-pu-net-device.h"
#include "ns3/network-module.h"
#include "ns3/spectrum-db.h"
#include "ns3/spectrum-map.h"
#include "ns3/once-onoff-model.h"
#include "ns3/network-module.h"
#include "ns3/couwbat-net-device.h"
#include "ns3/couwbat-mac.h"
#include "ns3/couwbat-phy.h"
#include "ns3/bs-couwbat-mac.h"
#include "ns3/spectrum-manager.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"

NS_LOG_COMPONENT_DEFINE("CouwbatHelper");

namespace ns3 {

CouwbatPhyHelper::~CouwbatPhyHelper ()
{
  NS_LOG_FUNCTION (this);
}

CouwbatMacHelper::~CouwbatMacHelper ()
{
  NS_LOG_FUNCTION (this);
}

CouwbatHelper::CouwbatHelper (void)
  : m_specDbNode (0)
{
  NS_LOG_FUNCTION (this);
}

CouwbatHelper::~CouwbatHelper (void)
{
  NS_LOG_FUNCTION (this);
}

Ptr<SpectrumDb>
CouwbatHelper::InstallSpectrumDb (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  Ptr<SpectrumDb> specDb;
  if (m_specDbNode != 0)
    {
      NS_LOG_WARN ("You are trying to install a "
        << "Couwbat Spectrum Database on more "
        << "than one node (not supported), the "
        << "first node will be used instead.");
    }
  else
    {
      // install the spectrum db to the node
      specDb = CreateObject<SpectrumDb> ();
      node->AggregateObject (specDb);
      m_specDbNode = node;
    }
  return specDb;
}

Ptr<PrimaryUserNetDevice>
CouwbatHelper::InstallPrimaryUser (Ptr<Node> node, Ptr<SpectrumMap> map, Ptr<OnOffModel> onOffModel)
{
  NS_LOG_FUNCTION (this << node << map << onOffModel);
  Ptr<PrimaryUserNetDevice> puNetDevice;
  Ptr<SpectrumDb> specDb;
  if (m_specDbNode == 0)
    {
      NS_LOG_ERROR ("To install a primary user "
        << "a spectrum database must be installed first.");
    }
  else
    {
      // create PU net device
      puNetDevice = CreateObject<PrimaryUserNetDevice> ();

      // install spectrum map
      puNetDevice->SetSpectrumMap (map);

      // install spectrum database
      specDb = m_specDbNode->GetObject<SpectrumDb> ();
      puNetDevice->SetSpectrumDb (specDb);

      // install onoff modell
      NS_LOG_DEBUG("node: "<<node<<", puNetDevice: "<<puNetDevice<<", onOffModel: "<<onOffModel);
      puNetDevice->SetOnOffModel (onOffModel);
      onOffModel->SetPuNetDevice (puNetDevice);

      // install PU net device on the node
      puNetDevice->SetNode (node);
      node->AggregateObject (puNetDevice);

      // start the PU
      Simulator::ScheduleWithContext (node->GetId (), Seconds (0), &PrimaryUserNetDevice::Start, puNetDevice);
    }
  return puNetDevice;
}

Ptr<TraceBasedPuNetDevice>
CouwbatHelper::InstallTraceBasedPu (
    Ptr<Node> node, Ptr<OnOffModel> onOffModel, std::string fileName,
    Time samplingInterval, int startingLine)
{
  NS_LOG_FUNCTION (this << node << onOffModel << fileName << samplingInterval << startingLine);
  Ptr<TraceBasedPuNetDevice> puNetDevice;
  Ptr<SpectrumDb> specDb;
  if (m_specDbNode == 0)
    {
      NS_LOG_ERROR ("To install a primary user "
        << "a spectrum database must be installed first.");
    }
  else
    {
      // create PU net device
      puNetDevice = CreateObject<TraceBasedPuNetDevice> ();

      // set CSV
      puNetDevice->SetSpectrumTraceFile (fileName, samplingInterval);

      // install spectrum database
      specDb = m_specDbNode->GetObject<SpectrumDb> ();
      puNetDevice->SetSpectrumDb (specDb);

      // install onoff modell
      NS_LOG_DEBUG("node: "<<node<<", puNetDevice: "<<puNetDevice<<", onOffModel: "<<onOffModel);
      //puNetDevice->SetOnOffModel (onOffModel);
      //onOffModel->SetPuNetDevice (puNetDevice);

      // install PU net device on the node
      puNetDevice->SetNode (node);
      node->AggregateObject (puNetDevice);

      // start the PU
      if (samplingInterval > Time ())
        {
          Simulator::ScheduleWithContext (node->GetId (), Seconds (0), &TraceBasedPuNetDevice::Start,
                                      puNetDevice, startingLine);
        }
    }
  return puNetDevice;
}

Ptr<CouwbatNetDevice>
CouwbatHelper::Install
  (
    const CouwbatPhyHelper &phyHelper,
    const CouwbatMacHelper &macHelper,
    Ptr<Node> node
  ) const
{
  NS_LOG_FUNCTION (this << "somePhyHelper" << "someMacHelper" << node);

  // create a couwbat network device
  Ptr<CouwbatNetDevice> netDevice = CreateObject<CouwbatNetDevice> ();

  // connect phy
  Ptr<CouwbatPhy> phy = phyHelper.Create (node, netDevice);
  netDevice->SetPhy (phy);

  // connect mac
  Ptr<CouwbatMac> mac = macHelper.Create (netDevice);
  mac->SetAddress (Mac48Address::Allocate ());
  netDevice->SetMac (mac);

  // add a spectrum manager to the device if the mac is a base station
  /*
   * TODO consider building an own helper for the SpectrumManager.
   * The helper could configure the SpectrumManager to use different
   * types of spectrum database implementations (e.g. direct object access
   * or simulated communication via an client spectrum database application).
   *
   */
  Ptr<BsCouwbatMac> bsMac = mac->GetObject<BsCouwbatMac> ();
  if (bsMac)
    {
      NS_LOG_INFO ("Created a CR-BS, adding SpectrumManager");
      Ptr<SpectrumManager> spectrumManager = CreateObject<SpectrumManager> ();
      Ptr<SpectrumDb> specDb = m_specDbNode->GetObject<SpectrumDb> ();
      spectrumManager->SetSpectrumDb (specDb);
      netDevice->AggregateObject (spectrumManager);
    }

  // connect device with node
  node->AddDevice (netDevice); // AddDevice schedules DoInitialize of the netDevice at time 0

  return netDevice;
}

Ptr<SpectrumMap>
CouwbatHelper::CreateSpectrumMap (uint32_t start_subcarrier, uint32_t n_subcarriers)
{
  NS_LOG_FUNCTION (this << start_subcarrier << n_subcarriers);
  Ptr<SpectrumMap> map = CreateObject<SpectrumMap> ();
  map->SetSpectrum (start_subcarrier, n_subcarriers);
  return map;
}

Ptr<OnceOnOffModel>
CouwbatHelper::CreateOnceOnOffModel (TimeValue firstOn, TimeValue finalOff)
{
  //NS_LOG_FUNCTION (this << firstOn << finalOff); // There is some problem with logging this
  Ptr<OnceOnOffModel> onoff = CreateObject<OnceOnOffModel> ();
  onoff->SetAttribute ("FirstOn" , firstOn );
  onoff->SetAttribute ("FinalOff", finalOff);
  return onoff;
}

void
CouwbatHelper::EnableLogComponents (void)
{
  LogComponentEnable ("CouwbatHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleCouwbatHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("SpectrumDb", LOG_LEVEL_ALL);
  LogComponentEnable ("PrimaryUserNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("SpectrumMap", LOG_LEVEL_ALL);
  LogComponentEnable ("OnceOnOffModel", LOG_LEVEL_ALL);
  LogComponentEnable ("PeriodicOnOffModel", LOG_LEVEL_ALL);
  LogComponentEnable ("RandomOnOffModel", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleCouwbatHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("CouwbatChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleCouwbatChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("CouwbatPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleCouwbatPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("CouwbatMac", LOG_LEVEL_ALL);
  LogComponentEnable ("BsCouwbatMac", LOG_LEVEL_ALL);
  LogComponentEnable ("StaCouwbatMac", LOG_LEVEL_ALL);
  LogComponentEnable ("CouwbatNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("SpectrumManager", LOG_LEVEL_ALL);
}

} // namespace ns3

