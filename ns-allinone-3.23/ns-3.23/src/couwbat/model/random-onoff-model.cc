#include "random-onoff-model.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "pu-net-device.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("RandomOnOffModel");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (RandomOnOffModel);

TypeId
RandomOnOffModel::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::RandomOnOffModel")

    .SetParent<OnOffModel> ()

    .AddConstructor<RandomOnOffModel> ()

    .SetGroupName("Couwbat")

    .AddAttribute ("RandomVariable",
                   "The random variable that returns the time (in seconds) for how long the primary user stays on/off in each period.",
                   StringValue("ns3::UniformRandomVariable"),
                   MakePointerAccessor (&RandomOnOffModel::m_randVar),
                   MakePointerChecker<RandomVariableStream> ())
    ;
  return tid;
}

RandomOnOffModel::RandomOnOffModel (void)
{
  NS_LOG_FUNCTION (this);
}

RandomOnOffModel::~RandomOnOffModel ()
{
  NS_LOG_FUNCTION (this);
}

void
RandomOnOffModel::Start (void)
{
  NS_LOG_FUNCTION (this);

  // check if running indefinitely
  m_runIndefinitely = (GetFinalOff () <= Seconds (0));
  if (m_runIndefinitely)
    {
      NS_LOG_INFO ("FinalOff <= 0, running indefinitely");
    }

  // schedule first on with random delay
  Time rand = Seconds (m_randVar->GetValue ());
  Simulator::Schedule (GetFirstOn () + rand, &RandomOnOffModel::TurnOn, this);
}

void
RandomOnOffModel::TurnOn (void)
{
  NS_LOG_FUNCTION (this);
  GetPuNetDevice () -> TurnOn ();

  // schedule the off
  Time rand = Seconds (m_randVar->GetValue ());
  Time relativeFinalOff = GetFinalOff () - Simulator::Now ();
  if ( m_runIndefinitely || (rand < relativeFinalOff) ) 
    { // turn off after random time
      NS_LOG_INFO ("keeping PU on for " << rand);
      Simulator::Schedule (rand, &RandomOnOffModel::TurnOff, this);
    }
  else
    { // schedule the final off (by conversion to relative time)
      NS_LOG_INFO ("keeping PU on for " << relativeFinalOff);
      Simulator::Schedule (relativeFinalOff, &RandomOnOffModel::TurnOff, this);
    }
}

void
RandomOnOffModel::TurnOff (void)
{
  NS_LOG_FUNCTION (this);
  GetPuNetDevice () -> TurnOff ();

  // schedule the next on
  Time rand = Seconds (m_randVar->GetValue ());
  Time relativeFinalOff = GetFinalOff () - Simulator::Now ();
  if ( m_runIndefinitely || (rand < relativeFinalOff) ) 
    { // turn on after random time
      NS_LOG_INFO ("keeping PU off for " << rand);
      Simulator::Schedule (rand, &RandomOnOffModel::TurnOn, this);
    }
  else
    {
      // no need to turn it on again, since final off is reached
      NS_LOG_INFO ("keeping PU off for (rest of simulation time), FinalOff reached");
    }
}

} // namespace ns3

