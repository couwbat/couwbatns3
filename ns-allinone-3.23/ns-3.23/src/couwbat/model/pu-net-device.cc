#include "pu-net-device.h"
#include "spectrum-map.h"
#include "spectrum-db.h"
#include "onoff-model.h"
#include "ns3/network-module.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("PrimaryUserNetDevice");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (PrimaryUserNetDevice);

TypeId
PrimaryUserNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::PrimaryUserNetDevice")
    .SetParent<Object> ()
    .AddConstructor<PrimaryUserNetDevice> ()
    .SetGroupName("Couwbat")
  ;
  return tid;
}

PrimaryUserNetDevice::PrimaryUserNetDevice (void)
  : m_specMap (0),
    m_specDb (0),
    m_onOffModel (0),
    m_node (0)
{
  NS_LOG_FUNCTION (this);
}

PrimaryUserNetDevice::~PrimaryUserNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

void
PrimaryUserNetDevice::SetSpectrumMap (Ptr<SpectrumMap> map)
{
  NS_LOG_FUNCTION (this << map);
  NS_LOG_INFO ("PU using spectrum " << map);
  m_specMap = map;
}

void
PrimaryUserNetDevice::SetSpectrumDb (Ptr<SpectrumDb> db)
{
  NS_LOG_FUNCTION (this << db);
  m_specDb = db;
}

void
PrimaryUserNetDevice::SetOnOffModel (Ptr<OnOffModel> model)
{
  NS_LOG_FUNCTION (this << model);
  m_onOffModel = model;
}

void
PrimaryUserNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_node = node;
}

Ptr<Node>
PrimaryUserNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

void
PrimaryUserNetDevice::Start (void)
{
  NS_LOG_FUNCTION (this);

  if (!m_onOffModel)
    {
      NS_LOG_INFO("No OnOffModel available for PU, default TurnOn.");
      TurnOn ();
    }
  else
    {
      m_onOffModel->Start ();
    }
}

void
PrimaryUserNetDevice::TurnOff (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("PU leaving spectrum:  " << m_specMap);
  m_specDb->LeaveSpectrum (m_specMap);
}

void
PrimaryUserNetDevice::TurnOn (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("PU occupying spectrum: " << m_specMap);
  m_specDb->OccupySpectrum (m_specMap);
}

} // namespace ns3
