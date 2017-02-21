/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/couwbat-module.h"
#include "ns3/mobility-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("CouwbatStaAloneExample");

/**
 * \file
 * \ingroup examples
 * couwbat-sta-alone is a simulation scenario which demonstrates a standalone CR-STA MAC (\ref ns3::StaCouwbatMac)
 * 
 * Execute with "--help" parameter for info on all parameters.
 */ 

int
main (int argc, char *argv[])
{
  /*****************************
   * 0. Default Logging options.
   *****************************/
  LogComponentEnable ("CouwbatStaAloneExample", LOG_LEVEL_INFO);

  LogComponentEnable ("StaCouwbatMac", LOG_LEVEL_INFO);
  LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_TIME);
  LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_NODE);
  LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_LEVEL);

  /********************************
   * 1. Prepare the couwbat helper.
   ********************************/
  CouwbatHelper couwbat;

  /************************************
   * 2. Create the simulation scenario.
   ************************************/

  /*
   * Uses default Couwbat parameters
   */

  /**
   * Set up channel and devices
   */
  // channel
  SimpleCouwbatChannelHelper channel = SimpleCouwbatChannelHelper::Default ();

  // phy
  SimpleCouwbatPhyHelper phy = SimpleCouwbatPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  // mac
  SimpleCouwbatMacHelper mac = SimpleCouwbatMacHelper::Default ();

  /*
   * Create a CR-STA
   */
  mac.SetType ("ns3::StaCouwbatMac");
  NodeContainer staNodes;
  staNodes.Create (1);
  for (NodeContainer::Iterator i = staNodes.Begin (); i != staNodes.End (); ++i)
    {
      Ptr<Node> staNode = *i;
      NS_LOG_INFO ("Creating CR-STA: id=" << staNode->GetId ());
      couwbat.Install (phy, mac, staNode);
    }

  /************************
   * 3. Run the simulation.
   ************************/
  Simulator::Stop (MilliSeconds (1970));
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



