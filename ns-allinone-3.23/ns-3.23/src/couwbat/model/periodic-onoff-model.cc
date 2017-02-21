#include "periodic-onoff-model.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "pu-net-device.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("PeriodicOnOffModel");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (PeriodicOnOffModel);

TypeId
PeriodicOnOffModel::GetTypeId (void)
{
  static TypeId tid = TypeId("ns3::PeriodicOnOffModel")

    .SetParent<OnOffModel> ()

    .AddConstructor<PeriodicOnOffModel> ()

    .SetGroupName("Couwbat")

    .AddAttribute ("PeriodOn",
                   "Periodic duration the primary user stays turned on. Gets turned off afterwards.",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&PeriodicOnOffModel::m_pOn),
                   MakeTimeChecker ())

    .AddAttribute ("PeriodOff",
                   "Periodic duration the primary user stays turned off. Gets turned on afterwards.",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&PeriodicOnOffModel::m_pOff),
                   MakeTimeChecker ())
    ;
  return tid;
}

PeriodicOnOffModel::PeriodicOnOffModel (void)
{
  NS_LOG_FUNCTION (this);
}

PeriodicOnOffModel::~PeriodicOnOffModel ()
{
  NS_LOG_FUNCTION (this);
}

void
PeriodicOnOffModel::Start (void)
{
  NS_LOG_FUNCTION (this);

  // check if running indefinitely
  m_runIndefinitely = (GetFinalOff () <= Seconds (0));
  if (m_runIndefinitely)
    {
      NS_LOG_INFO ("FinalOff <= 0, running indefinitely");
    }

  // schedule first on
  Simulator::Schedule (GetFirstOn (), &PeriodicOnOffModel::TurnOn, this);
}

void
PeriodicOnOffModel::TurnOn (void)
{
  NS_LOG_FUNCTION (this);
  GetPuNetDevice () -> TurnOn ();

  // schedule the off
  Time relativeFinalOff = GetFinalOff () - Simulator::Now ();
  if ( m_runIndefinitely || (GetFirstOn () < relativeFinalOff) ) 
    { // turn off after m_pOn time
      NS_LOG_INFO ("keeping PU on for " << GetFirstOn ());
      Simulator::Schedule (m_pOff, &PeriodicOnOffModel::TurnOff, this);
    }
  else
    { // schedule the final off (by conversion to relative time)
      NS_LOG_INFO ("keeping PU on for " << relativeFinalOff);
      Simulator::Schedule (relativeFinalOff, &PeriodicOnOffModel::TurnOff, this);
    }
}

void
PeriodicOnOffModel::TurnOff (void)
{
  NS_LOG_FUNCTION (this);
  GetPuNetDevice () -> TurnOff ();

  // schedule the next on
  Time relativeFinalOff = GetFinalOff () - Simulator::Now ();
  if ( m_runIndefinitely || (GetFinalOff () < relativeFinalOff) ) 
    { // turn on after m_pOff time
      NS_LOG_INFO ("keeping PU off for " << m_pOff);
      Simulator::Schedule (m_pOn, &PeriodicOnOffModel::TurnOn, this);
    }
  else
    {
      // no need to turn it on again, since we reached our final off
      NS_LOG_INFO ("keeping PU off for (rest of simulation time), FinalOff reached");
    }
}

} // namespace ns3

