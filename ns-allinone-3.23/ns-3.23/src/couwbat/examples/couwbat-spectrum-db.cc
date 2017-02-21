/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/couwbat-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CouwbatSpectrumDbExample");

/**
 * \file
 * \ingroup examples
 * couwbat-spectrum-db is a simulation scenario which demonstrates the spectrum manager (\ref ns3::SpectrumManager), spectrum database (\ref ns3::SpectrumDb) and simple primary user components (\ref ns3::PrimaryUserNetDevice).
 * 
 * Execute with "--help" parameter for info on all parameters.
 */ 

int
main (int argc, char *argv[])
{
  /*
   * ===========================
   * 0. Default Logging options.
   * ===========================
   */
  LogComponentEnable ("CouwbatSpectrumDbExample", LOG_LEVEL_INFO);
  LogComponentEnable ("SpectrumDb", LOG_LEVEL_INFO);
  LogComponentEnable ("PrimaryUserNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("Couwbat", LOG_LEVEL_ALL);

  /*
   * =======================
   * 1. Commandline options.
   * =======================
   */
  bool verbose = false;
  uint32_t duration = 10; // in seconds

  CommandLine cmd;
  cmd.AddValue ("verbose", "Use all couwbat related log componets if true.", verbose);
  cmd.AddValue ("duration", "duration of the simulation in seconds", duration);

  cmd.Parse (argc,argv);

  /*
   * ==============================
   * 2. Prepare the couwbat helper.
   * ==============================
   */
  CouwbatHelper couwbat;

  // turn on couwbat logging if requested
  if (verbose)
    {
      couwbat.EnableLogComponents ();
    }

  /*
   * ==================================
   * 3. Create the simulation scenario.
   * ==================================
   */

  /*
   * 3.0
   *  general couwbat settings
   */
  /*
  Couwbat::SetCenterFreq ();
  Couwbat::SetFFTDuration ();
  Couwbat::SetSCFrequencySpacing ();
  Couwbat::SetSymbolDuration ();
  Couwbat::SetCPDuration ();
  Couwbat::SetNumberOfSubcarriers ();
  Couwbat::SetNumberOfSubchannels ();
  Couwbat::SetNumberOfSubcarriersPerSubchannel ();
  Couwbat::SetNumberOfDataSubcarriersPerSubchannel ();
  Couwbat::SetSuperframeDuration ();
  Couwbat::SetNarrowbandPhaseDuration ();
  Couwbat::SetWidebandPhaseDuration ();
  */

  /*
   * 3.1
   *  create node that holds the spectrum database
   *  The couwbat helper will remember the spectrum db node
   *  and give it to primary users that will be created afterwards.
   */
  NodeContainer specDbNodes;
  specDbNodes.Create (1);
  couwbat.InstallSpectrumDb (specDbNodes.Get (0) );

  /*
   * 3.2
   *  create primary users
   */
  NodeContainer puNodes;
  puNodes.Create (3);

  Ptr<Node> pu;

  pu = puNodes.Get (0);
  NS_LOG_INFO ("Creating PU: id="<< pu->GetId () );
  couwbat.InstallPrimaryUser
    (
      pu,
      couwbat.CreateSpectrumMap (8, 4),
      CreateObjectWithAttributes<OnceOnOffModel>
        (
          "FirstOn" , TimeValue (Seconds (1)),
          "FinalOff", TimeValue (Seconds (3))
        )
    );

  pu = puNodes.Get (1);
  NS_LOG_INFO ("Creating PU: id="<< pu->GetId () );
  couwbat.InstallPrimaryUser
    (
      pu,
      couwbat.CreateSpectrumMap (4, 2),
      CreateObjectWithAttributes<PeriodicOnOffModel>
        (
          "FirstOn" , TimeValue (Seconds (2)),
          "FinalOff", TimeValue (Seconds (0)),
          "PeriodOn" , TimeValue (Seconds (1)),
          "PeriodOff", TimeValue (Seconds (2))
        )
    );

  pu = puNodes.Get (2);
  NS_LOG_INFO ("Creating PU: id="<< pu->GetId () );
  couwbat.InstallPrimaryUser
    (
      pu,
      couwbat.CreateSpectrumMap (0, 2),
      CreateObjectWithAttributes<RandomOnOffModel>
        (
          "FirstOn" , TimeValue (Seconds (3)),
          "FinalOff", TimeValue (Seconds (8)),
          "RandomVariable", PointerValue (CreateObjectWithAttributes<UniformRandomVariable>
            (
             "Min", DoubleValue (1),
             "Max", DoubleValue (2)
            ))
        )
    );


  /*
   * ======================
   * 4. Run the simulation.
   * ======================
   */
  Simulator::Stop (Seconds (duration + 0.1));
  NS_LOG_INFO ("---------------------");
  NS_LOG_INFO (" Starting Simulation");
  NS_LOG_INFO ("---------------------");
  Simulator::Run ();
  NS_LOG_INFO ("--------------------------------------------");
  NS_LOG_INFO ("Simulation finished after " << Simulator::Now ());
  NS_LOG_INFO ("--------------------------------------------");
  Simulator::Destroy ();
  return 0;
}


