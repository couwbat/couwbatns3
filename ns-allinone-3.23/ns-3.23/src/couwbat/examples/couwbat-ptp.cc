/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/couwbat-module.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/applications-module.h"
#include <limits>
#include <ctime>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CouwbatPtpExample");

/**
 * \file
 * \ingroup examples
 * couwbat-ptp is the main simulation scenario for the Couwbat MAC, with many configuration options available through command line parameters.
 * 
 * This scenario can include interaction between the most important components: CR-BS (\ref ns3::BsCouwbatMac), CR-STA (\ref ns3::StaCouwbatMac) and primary users (\ref ns3::TraceBasedPuNetDevice, \ref ns3::PrimaryUserNetDevice).
 * 
 * Execute with "--help" parameter for info on all parameters. These command line parameters allow the creation of a wide range of user-defined scenarios. The source code of the scenario can be easily tweaked for further customizations.
 */ 

int
main (int argc, char *argv[])
{
  uint32_t staCount = 1;
  uint32_t staMaxXY = 10;
  uint32_t appMode = 1;
  double durationSeconds = 5.0;
  unsigned int seed = std::time (0);
  bool macQueueLimitEnabled = Couwbat::mac_queue_limit_enabled;
  int macQueueLimitSize = Couwbat::mac_queue_limit_size;
  bool puEnabled = false;
  std::string puCsvFileName = "";
  uint32_t puCsvSamplingIntervalMs = 100;
  int puCsvStartingLine = 1;
  uint16_t streams = 1;
  unsigned int macDlUlSlotLimit = Couwbat::mac_dlul_slot_limit_size;

  CommandLine cmd;
  cmd.AddValue ("stas", "Number of CR-STAs.", staCount);
  cmd.AddValue ("dist", "One STA - Fixed distance between BS and STA; Multiple STAs - randomly positioned around BS with this as max distance", staMaxXY);
  cmd.AddValue ("d", "Simulation duration in seconds.", durationSeconds);
  cmd.AddValue ("mode", "0 (no applications), 1 or 2 (TCP stream), 3 (UDP stream)", appMode);
  cmd.AddValue ("seed", "Specify a particular seed for std::srand and ns3::RngSeedManager [std::time]", seed);
  cmd.AddValue ("queueLim", "Limit MAC TxQueue size", macQueueLimitEnabled);
  cmd.AddValue ("queueLimSize", "If queueLim is enabled, the maximum queue size in number of packets", macQueueLimitSize);
  cmd.AddValue ("pu", "Enable PUs", puEnabled);
  cmd.AddValue ("pucsv", "File name (and location) of PU spectrum trace file. If not specified, uses default PUs without trace files.", puCsvFileName);
  cmd.AddValue ("csvsi", "Sampling interval of PU spectrum trace file in ms", puCsvSamplingIntervalMs);
  cmd.AddValue ("csvln", "Line from which to start reading in trace file", puCsvStartingLine);
  cmd.AddValue ("streams", "Number of simultaneous TCP application streams", streams);
  cmd.AddValue ("dlUlSlotLim", "Limit number of MAC DL/UL slots to this number", macDlUlSlotLimit);

  if (argc < 2)
    {
      std::cout << "Running using default settings. Use the following"
          " command line arguments to change them.\n";
      cmd.PrintHelp (std::cout);
    }
  cmd.Parse (argc, argv);

  Time::SetResolution (Time::NS);
  Packet::EnablePrinting ();

  std::srand (seed);
  ns3::RngSeedManager::SetSeed (seed);

  Couwbat::mac_queue_limit_enabled = macQueueLimitEnabled;
  Couwbat::mac_queue_limit_size = macQueueLimitSize;
  Couwbat::mac_dlul_slot_limit_size = macDlUlSlotLimit;

  /*
   * SET UP LOGGING
   */
/*
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpClient", LOG_PREFIX_TIME);
  LogComponentEnable ("UdpClient", LOG_PREFIX_NODE);
  LogComponentEnable ("UdpClient", LOG_PREFIX_LEVEL);
  LogComponentEnable ("UdpClient", LOG_PREFIX_FUNC);

  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_PREFIX_TIME);
  LogComponentEnable ("UdpServer", LOG_PREFIX_NODE);
  LogComponentEnable ("UdpServer", LOG_PREFIX_LEVEL);
  LogComponentEnable ("UdpServer", LOG_PREFIX_FUNC);

  LogComponentEnable ("BsCouwbatMac", LOG_LEVEL_INFO);
  LogComponentEnable ("BsCouwbatMac", LOG_PREFIX_TIME);
  LogComponentEnable ("BsCouwbatMac", LOG_PREFIX_NODE);
  LogComponentEnable ("BsCouwbatMac", LOG_PREFIX_LEVEL);
  //LogComponentEnable ("BsCouwbatMac", LOG_PREFIX_FUNC);

  LogComponentEnable ("StaCouwbatMac", LOG_LEVEL_INFO);
  LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_TIME);
  LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_NODE);
  LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_LEVEL);
  //LogComponentEnable ("StaCouwbatMac", LOG_PREFIX_FUNC);

  LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("OnOffApplication", LOG_PREFIX_TIME);
  LogComponentEnable ("OnOffApplication", LOG_PREFIX_NODE);
  LogComponentEnable ("OnOffApplication", LOG_PREFIX_LEVEL);
  LogComponentEnable ("OnOffApplication", LOG_PREFIX_FUNC);

  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  LogComponentEnable ("PacketSink", LOG_PREFIX_TIME);
  LogComponentEnable ("PacketSink", LOG_PREFIX_NODE);
  LogComponentEnable ("PacketSink", LOG_PREFIX_LEVEL);
  LogComponentEnable ("PacketSink", LOG_PREFIX_FUNC);

  LogComponentEnable ("CouwbatInterferenceHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("CouwbatInterferenceHelper", LOG_PREFIX_TIME);
  LogComponentEnable ("CouwbatInterferenceHelper", LOG_PREFIX_NODE);
  LogComponentEnable ("CouwbatInterferenceHelper", LOG_PREFIX_LEVEL);
  LogComponentEnable ("CouwbatInterferenceHelper", LOG_PREFIX_FUNC);

  LogComponentEnable ("SimpleCouwbatPhy", LOG_LEVEL_INFO);
  LogComponentEnable ("SimpleCouwbatPhy", LOG_PREFIX_TIME);
  LogComponentEnable ("SimpleCouwbatPhy", LOG_PREFIX_NODE);
  LogComponentEnable ("SimpleCouwbatPhy", LOG_PREFIX_LEVEL);
  LogComponentEnable ("SimpleCouwbatPhy", LOG_PREFIX_FUNC);

  LogComponentEnable ("CouwbatPhyStateHelper", LOG_LEVEL_INFO);
  LogComponentEnable ("CouwbatPhyStateHelper", LOG_PREFIX_TIME);
  LogComponentEnable ("CouwbatPhyStateHelper", LOG_PREFIX_NODE);
  LogComponentEnable ("CouwbatPhyStateHelper", LOG_PREFIX_LEVEL);
  //LogComponentEnable ("CouwbatPhyStateHelper", LOG_PREFIX_FUNC);
*/
  LogComponentEnable ("CouwbatPtpExample", LOG_LEVEL_INFO);
  LogComponentEnable ("CouwbatPtpExample", LOG_PREFIX_TIME);
  LogComponentEnable ("CouwbatPtpExample", LOG_PREFIX_LEVEL);

  LogComponentEnable ("TraceBasedPuNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable ("TraceBasedPuNetDevice", LOG_PREFIX_TIME);
  LogComponentEnable ("TraceBasedPuNetDevice", LOG_PREFIX_LEVEL);
/*
  LogComponentEnable ("CouwbatTxQueue", LOG_LEVEL_DEBUG);
  LogComponentEnable ("CouwbatTxQueue", LOG_PREFIX_TIME);
  LogComponentEnable ("CouwbatTxQueue", LOG_PREFIX_NODE);
  LogComponentEnable ("CouwbatTxQueue", LOG_PREFIX_LEVEL);
  LogComponentEnable ("CouwbatTxQueue", LOG_PREFIX_FUNC);
*/

  /*
   * SET UP COUWBAT
   */

  CouwbatHelper couwbat;

  NodeContainer specDbNodes;
  specDbNodes.Create (1);
  couwbat.InstallSpectrumDb (specDbNodes.Get (0));

  /*
   *  create PUs
   */
  if (puEnabled)
    {
      if (puCsvFileName == "")
        {
          NodeContainer puNodes;
          puNodes.Create (2);
          Ptr<Node> pu;

          // PU 1
          pu = puNodes.Get (0);
          //NS_LOG_INFO ("Creating PU: id=" << pu->GetId () );
          couwbat.InstallPrimaryUser
            (
              pu,
              couwbat.CreateSpectrumMap (0, 32),
              CreateObjectWithAttributes<OnceOnOffModel>
                (
                  "FirstOn" , TimeValue (Seconds (2))
                )
            );

          // PU 2
          pu = puNodes.Get (1);
          //NS_LOG_INFO ("Creating PU: id=" << pu->GetId () );
          couwbat.InstallPrimaryUser
            (
              pu,
              couwbat.CreateSpectrumMap (1024, 32),
              CreateObjectWithAttributes<OnceOnOffModel>
                (
                  "FirstOn" , TimeValue (Seconds (1))
                )
            );
        }
      else
        {
          NodeContainer puNodes;
          puNodes.Create (1);
          Ptr<Node> pu = puNodes.Get (0);
          couwbat.InstallTraceBasedPu (
              pu, 0, puCsvFileName,
              MilliSeconds (puCsvSamplingIntervalMs), puCsvStartingLine);
        }
    }

  SimpleCouwbatChannelHelper channel = SimpleCouwbatChannelHelper::Default ();

  SimpleCouwbatPhyHelper phy = SimpleCouwbatPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  SimpleCouwbatMacHelper mac = SimpleCouwbatMacHelper::Default ();
  mac.SetType ("ns3::BsCouwbatMac");

  NodeContainer nodes;
  nodes.Create (1 + staCount);
  Ptr<Node> bsNode = nodes.Get (0);
  NodeContainer staNodes;
  for (uint32_t i = 0; i < staCount; ++i)
    {
      Ptr<Node> staNode = nodes.Get (i + 1);
      staNodes.Add (staNode);
    }

  Ptr<CouwbatNetDevice> bsNetDevice = couwbat.Install (phy, mac, bsNode);

  mac.SetType ("ns3::StaCouwbatMac");
  NetDeviceContainer staNetDevices;
  std::vector<Ptr<CouwbatNetDevice> > staCouwbatNetDevices;
  for (uint32_t i = 0; i < staCount; ++i)
    {
      Ptr<Node> staNode = nodes.Get (i + 1);
      Ptr<CouwbatNetDevice> staNetDevice = couwbat.Install (phy, mac, staNode);
      staNetDevices.Add (staNetDevice);
      staCouwbatNetDevices.push_back (staNetDevice);
    }

  NetDeviceContainer devices;
  devices.Add (bsNetDevice);
  devices.Add (staNetDevices);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Ipv4Address bsIpAddr = interfaces.GetAddress (0);
  std::vector<Ipv4Address> staIpAddrs;
  for (uint32_t i = 0; i < staCount; ++i)
    {
      Ipv4Address staIpAddr = interfaces.GetAddress (i + 1);
      staIpAddrs.push_back (staIpAddr);

      Ptr<CouwbatNetDevice> staNetDevice = staCouwbatNetDevices[i];
      bsNetDevice->AddTranslatonEntry (staIpAddr, staNetDevice->GetMac ()->GetAddress ());

      staNetDevice->AddTranslatonEntry (bsIpAddr, bsNetDevice->GetMac ()->GetAddress ());
    }


  // Set TCP defaults
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1452));
  // Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue (0));
  Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (1024000000));
  Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (1024000000));
  // Config::SetDefault("ns3::TcpSocket::SlowStartThreshold", UintegerValue (1000000));


  //////////////////////
  // Mobility
  //////////////////////
  MobilityHelper mobility;

  // STA random position parameters
  std::ostringstream staRandomPropertiesX;
  std::ostringstream staRandomPropertiesY;
  std::ostringstream staRandomPropertiesZ;

  // NB: Multiple Nodes/STAs at the exact same coordinates crashes the simulation for some reason. This workaround prevents that
  // while allowing to set an exact distance between BS and one STA for e.g. testing SNR. Multiple STAs are placed
  // randomly around the BS within the distance specified across X and Y coordinates.
  if (staCount > 1)
    {
      staRandomPropertiesX << "ns3::UniformRandomVariable[Min=-"<<staMaxXY<<"|Max="<<staMaxXY<<"|Stream=" << std::rand () << "]";
      staRandomPropertiesY << "ns3::UniformRandomVariable[Min=-"<<staMaxXY<<"|Max="<<staMaxXY<<"|Stream=" << std::rand () << "]";
    }
  else
    {
      staRandomPropertiesX << "ns3::UniformRandomVariable[Min="<<staMaxXY<<"|Max="<<staMaxXY<<"|Stream=" << std::rand () << "]";
      staRandomPropertiesY << "ns3::UniformRandomVariable[Min=0|Max=0"<<"|Stream=" << std::rand () << "]";
    }

  staRandomPropertiesZ << "ns3::UniformRandomVariable[Min=1|Max=1"
          "|Stream=" << std::rand () << "]";

  mobility.SetPositionAllocator (
      "ns3::RandomBoxPositionAllocator",
      "X", StringValue (staRandomPropertiesX.str ()),
      "Y", StringValue (staRandomPropertiesY.str ()),
      "Z", StringValue (staRandomPropertiesZ.str ())
      );
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (staNodes);

  // BS random position parameters
  std::ostringstream bsRandomPropertiesX;
  std::ostringstream bsRandomPropertiesY;
  std::ostringstream bsRandomPropertiesZ;
  bsRandomPropertiesX << "ns3::UniformRandomVariable[Min=0|Max=0"
      "|Stream=" << std::rand () << "]";
  bsRandomPropertiesY << "ns3::UniformRandomVariable[Min=0|Max=0"
        "|Stream=" << std::rand () << "]";
  bsRandomPropertiesZ << "ns3::UniformRandomVariable[Min=26|Max=26"
          "|Stream=" << std::rand () << "]";

  mobility.SetPositionAllocator (
      "ns3::RandomBoxPositionAllocator",
      "X", StringValue (bsRandomPropertiesX.str ()),
      "Y", StringValue (bsRandomPropertiesY.str ()),
      "Z", StringValue (bsRandomPropertiesZ.str ())
      );
  mobility.Install (bsNode);

  //////////////////////
  // Print mobility info
  //////////////////////
  bool gnuplot = true;
  Ptr<MobilityModel> bsMobility = bsNode->GetObject<MobilityModel> ();

  // set up gnuplot file
  std::ofstream gnuplotFile;
  if (gnuplot)
    {
      gnuplotFile.open ("~gnuplotdata.tmp", std::ios::out | std::ios::trunc);
      gnuplotFile << bsMobility->GetPosition ().x << " "
          << bsMobility->GetPosition ().y << "\n";
    }

  // print positions
  char letter = 'A';
  NS_LOG_INFO ("CR-BS " << bsNode->GetId () << "(" << letter++
               << ") position " << bsMobility->GetPosition ());

  for (uint32_t j = 0; j < staCount; ++j)
    {
      Ptr<MobilityModel> staMobility = staNodes.Get (j)->GetObject<MobilityModel> ();
      NS_LOG_INFO ("CR-STA " << staNodes.Get (j)->GetId () << "("
                   << letter++ << ") position: "
                   << staMobility->GetPosition ()
                   << ", distance from BS="
                   << staMobility->GetDistanceFrom (bsMobility));

      if (gnuplot)
        {
          gnuplotFile << staMobility->GetPosition ().x << " "
              << staMobility->GetPosition ().y << "\n";
        }
    }

  if (gnuplot)
    {
      gnuplotFile.close ();
      std::ostringstream plotCommands;
      plotCommands << "set terminal dumb;"
          " set nokey; set xrange [-"<<staMaxXY<<":"<<staMaxXY<<"]; set yrange [-"<<staMaxXY<<":"<<staMaxXY<<"];"
          " plot '~gnuplotdata.tmp' every ::0::0";
      for (uint32_t k = 1; k <= staCount; ++k)
        {
          plotCommands << ", '~gnuplotdata.tmp' every ::" << k << "::"<< k;
        }
      int systemRet = std::system (("gnuplot -e \"" + plotCommands.str () + "\"").c_str ());
      std::remove ("~gnuplotdata.tmp");
      if (systemRet)
        {
          std::cerr << "gnuplot error, returned " << systemRet << std::endl;
        }
    }


  /*
   * SET UP APPLICATIONS
   *
   * Instructions:
   * 1) Install receiver application to staNode/bsNode
   * 2) Install sender application to staNode/bsNode (destination IP is staIpAddr/bsIpAddr)
   * 3) Optionally set app start times to e.g. 1 sec to allow STA to associate first (~0.7 sec)
   */

  ApplicationContainer allSinkApps;
  ApplicationContainer allSourceApps;

  NS_LOG_INFO ("appMode: " << appMode);
  switch (appMode)
  {
    case 1:
    case 2:
      {
        /*
         * TCP stream variant 1
         */
          // Info
          NS_LOG_INFO("BS IP:  " << bsIpAddr);
          NS_LOG_INFO("STA IP: " << staIpAddrs[0]);

          // Install server (receiver) on all nodes
          uint16_t sinkPort;
          for (sinkPort=10; sinkPort < 10+streams; sinkPort ++ )
            {
              // Install server (receiver) on BS and STA(s)
              PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
              ApplicationContainer sinkApps = packetSinkHelper.Install (nodes);

              // Set the start time
              int start = 0.0;
              int stop = durationSeconds;
              sinkApps.Start (Seconds (start));
              sinkApps.Stop (Seconds (stop));

              allSinkApps.Add (sinkApps);

              ApplicationContainer sourceApps;

              OnOffHelper onOffHelper ("ns3::TcpSocketFactory",  Address ());
              onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
              onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
              onOffHelper.SetAttribute ("DataRate", StringValue ("20Mbps"));
              onOffHelper.SetAttribute ("PacketSize", UintegerValue(1452));

              // Install client (sender to BSs) on STAs
              onOffHelper.SetAttribute ("Remote", AddressValue (InetSocketAddress (bsIpAddr, sinkPort)));
              sourceApps.Add (onOffHelper.Install (staNodes));

              onOffHelper.SetAttribute ("DataRate", StringValue ("80Mbps"));
              // Install client (sender to all STAs) on BS
              for (uint32_t i = 0; i < staCount; ++i)
                {
                  onOffHelper.SetAttribute ("Remote", AddressValue (InetSocketAddress (staIpAddrs[i], sinkPort)));
                  sourceApps.Add (onOffHelper.Install (bsNode));
                }
              sourceApps.Start (Seconds (start+1.0));
              sourceApps.Stop (Seconds (stop));

              allSourceApps.Add (sourceApps);
            }
      }
      break;

    case 3:
      {
        /*
         * UDP stream
         */
        // Install server (receiver) on all nodes
        UdpServerHelper server (9);
        allSinkApps = server.Install (nodes);
        allSinkApps.Start (Seconds (1.0));
        allSinkApps.Stop (Seconds (durationSeconds));

        // Install client (sender to BS) on all STAs
        UdpClientHelper client (bsIpAddr, 9);
        client.SetAttribute ("Interval", TimeValue (MicroSeconds (50)));
        client.SetAttribute ("PacketSize", UintegerValue (1452));
        client.SetAttribute ("MaxPackets", UintegerValue (std::numeric_limits<uint32_t>::max()));
        allSourceApps = client.Install (staNodes);
        // Install client (sender to all STAs) on BS
        for (uint32_t i = 0; i < staCount; ++i)
          {
            client.SetAttribute ("RemoteAddress", AddressValue (staIpAddrs[i]));
            allSourceApps.Add (client.Install (bsNode));
          }
        allSourceApps.Start (Seconds (1.0));
        allSourceApps.Stop (Seconds (durationSeconds));
      }
      break;

    default:
      break;

  }

  /*
   * RUN SIMULATION
   */

  Simulator::Stop (Seconds (durationSeconds));
  Simulator::Run ();

  /*
   * PRINT RESULTS
   */

  std::cout << "\n\nPRINT RESULTS FOR MODE="<<appMode<<":\n\n";

  if (appMode == 1 || appMode == 2)
    {
      for (ApplicationContainer::Iterator it = allSinkApps.Begin (); it != allSinkApps.End (); ++it)
        {
          Ptr<PacketSink> sinkApp = DynamicCast<PacketSink> (*it);
          std::cout << "Total Bytes received by application (" << sinkApp << ") on node " << sinkApp->GetNode ()->GetId ()
              << ": " << sinkApp->GetTotalRx () << std::endl;
        }
    }
  else if (appMode == 3)
    {
      Ptr<UdpServer> sink1 = DynamicCast<UdpServer> (allSinkApps.Get (0));
      std::cout << "Total UL Packets BS Received: " << sink1->GetReceived () << std::endl;

      for (uint32_t i = 0; i < staCount; ++i)
        {
          Ptr<UdpServer> sink2 = DynamicCast<UdpServer> (allSinkApps.Get (i + 1));
          std::cout << "Total DL Packets STA "<< i + 1 <<" Received: " << sink2->GetReceived () << std::endl;
        }
    }

  Simulator::Destroy ();

  return 0;
}

