#include "onoff-model.h"
#include "pu-net-device.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("OnOffModel");

namespace ns3
{

TypeId
OnOffModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::OnOffModel")

    .SetParent<Object> ()

    .SetGroupName("Couwbat")

    .AddAttribute ("FirstOn",
                   "Time when to first turn on the primary user.",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&OnOffModel::m_firstOn),
                   MakeTimeChecker ())

    .AddAttribute ("FinalOff",
                   "Time when to finally turn off the primary user. If set to 0 PU runs indefinitely.",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&OnOffModel::m_finalOff),
                   MakeTimeChecker ())

  ;
  //no .AddConstructor<...> () here because this is an abstract class (Start method is virtual)
  return tid;
}

void
OnOffModel::SetPuNetDevice (Ptr<PrimaryUserNetDevice> puNetDevice)
{
  NS_LOG_FUNCTION (this << puNetDevice);
  m_puNetDevice = puNetDevice;
}

Ptr<PrimaryUserNetDevice> 
OnOffModel::GetPuNetDevice (void) const
{
  NS_LOG_FUNCTION (this);
  return m_puNetDevice;
}

Time
OnOffModel::GetFirstOn (void) const
{
  NS_LOG_FUNCTION (this);
  return m_firstOn;
}

Time
OnOffModel::GetFinalOff (void) const
{
  NS_LOG_FUNCTION (this);
  return m_finalOff;
}

} // namespace ns3


