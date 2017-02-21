#include "once-onoff-model.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "pu-net-device.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("OnceOnOffModel");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (OnceOnOffModel);

TypeId
OnceOnOffModel::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::OnceOnOffModel")
    .SetParent<OnOffModel> ()
    .AddConstructor<OnceOnOffModel> ()
    .SetGroupName("Couwbat")
  ;
  return tid;
}

OnceOnOffModel::OnceOnOffModel (void)
{
  NS_LOG_FUNCTION (this);
}

OnceOnOffModel::~OnceOnOffModel ()
{
  NS_LOG_FUNCTION (this);
}

void
OnceOnOffModel::Start (void)
{
  NS_LOG_FUNCTION (this);

  // schedule the first on
  Simulator::Schedule (GetFirstOn () , &OnceOnOffModel::TurnOn , this);

  // check if running indefinitely
  m_runIndefinitely = (GetFinalOff () <= Seconds (0));
  if (m_runIndefinitely)
    {
      NS_LOG_INFO ("FinalOff <= 0, running indefinitely");
    }
  else
    {
      Simulator::Schedule (GetFinalOff (), &OnceOnOffModel::TurnOff, this);
    }

}

void
OnceOnOffModel::TurnOn(void)
{
  NS_LOG_FUNCTION (this);
  GetPuNetDevice () -> TurnOn ();
  if (m_runIndefinitely)
    {
      NS_LOG_INFO ("keeping PU on for (rest of simulation time)");
    }
  else
    {
      NS_LOG_INFO ("keeping PU on for " << GetFinalOff () - GetFirstOn ());
    }
}

void
OnceOnOffModel::TurnOff (void)
{
  NS_LOG_FUNCTION (this);
  GetPuNetDevice () -> TurnOff ();
  NS_LOG_INFO ("keeping PU off for (rest of simulation time), FinalOff reached");
}

} // namespace ns3

