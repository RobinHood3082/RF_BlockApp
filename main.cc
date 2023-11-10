#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/integer.h"

#include "ndnBlockchainApp.hpp"
#include "ipBlockchainApp.hpp"

namespace ns3 {
int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);

  int startTime = 0, endTime = 60;

  AnnotatedTopologyReader topologyReader ("", 25);
  topologyReader.SetFileName ("scratch/RF_BlockApp/topology/topology.txt");
  topologyReader.Read ();

  NodeContainer allNodes = topologyReader.GetNodes ();
  cout << "Number of Nodes: " << allNodes.size () << endl;

  bool ndn_flag = false;
  // change this flag to run on udp

  if (ndn_flag)
    {
      LogComponentEnable ("ndnBlockchainApp", LOG_LEVEL_DEBUG);

      ndn::StackHelper ndnHelper;
      ndnHelper.SetDefaultRoutes (true);
      ndnHelper.setPolicy ("nfd::cs::lru");
      ndnHelper.setCsSize (100);

      ndnHelper.InstallAll ();

      ndn::StrategyChoiceHelper::InstallAll ("/ndn.blockchain",
                                             "/localhost/nfd/strategy/multicast");

      ndn::GlobalRoutingHelper ndnRoutingHelper;
      ndnRoutingHelper.InstallAll ();

      ndn::AppHelper appHelper ("ndnBlockchainApp");

      vector<Ptr<ndnBlockchainApp>> apps;
      // for (auto& node : allNodes) {
      //     std::string nodeName = Names::FindName(node);
      //     if (nodeName[0] == 'r') continue;

      //     appHelper.SetAttribute("NodeName", StringValue(nodeName));
      //     appHelper.SetAttribute("cssz", IntegerValue(100));
      //     ApplicationContainer tempApps = appHelper.Install(node);
      //     // std::cout << apps.Get(1)->GetTypeId().GetName() << std::endl;
      //     apps.Add(tempApps.Get(0));

      //     std::string prefix = "/ndn.blockchain/" + nodeName;
      //     ndnRoutingHelper.AddOrigins(prefix, node);
      //     // cout << nodeName << endl;
      // }

      for (auto &node : allNodes)
        {
          std::string nodeName = Names::FindName (node);

          Ptr<ndnBlockchainApp> app = CreateObject<ndnBlockchainApp> ();
          app->SetStartTime (Seconds (startTime));
          app->SetStopTime (Seconds (endTime));
          app->SetAttribute ("NodeName", StringValue (nodeName));
          app->SetAttribute ("cssz", IntegerValue (100));

          apps.push_back (app);

          if (nodeName[0] == 'r')
            continue;

          node->AddApplication (app);
          std::string prefix = "/ndn.blockchain/" + nodeName;
          ndnRoutingHelper.AddOrigins (prefix, node);
        }

      ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes ();

      Simulator::Stop (Seconds (endTime));
      AnimationInterface anim ("scratch/RF_BlockApp/output/animation.xml");

      // std::cout << "App name is: ";
      // std::cout << allNodes.Get(108)->Get << std::endl;

      // SCHEDULER START
      // std::cout << "App: " << apps[108]->GetTypeId().GetName() << std::endl;

      for (int i = 108; i <= 162; i++)
        {
          Simulator::Schedule (Seconds (2.0 + i * 0.035), &ndnBlockchainApp::sendInitRequest,
                               apps[i], std::string ("user-144"));
        }

      Simulator::Schedule (Seconds (10.000), &ndnBlockchainApp::NewTransaction, apps[108],
                           std::string ("user-145 Donald"));
      Simulator::Schedule (Seconds (10.005), &ndnBlockchainApp::NewTransaction, apps[109],
                           std::string ("user-146 Clinton"));
      Simulator::Schedule (Seconds (10.010), &ndnBlockchainApp::NewTransaction, apps[110],
                           std::string ("user-226 Russel"));
      Simulator::Schedule (Seconds (10.015), &ndnBlockchainApp::NewTransaction, apps[111],
                           std::string ("user-157 Andrew"));
      Simulator::Schedule (Seconds (10.020), &ndnBlockchainApp::NewTransaction, apps[112],
                           std::string ("user-197 Donald"));

      Simulator::Schedule (Seconds (10.030), &ndnBlockchainApp::NewBlock, apps[91]);

      Simulator::Schedule (Seconds (12.020), &ndnBlockchainApp::NewTransaction, apps[112],
                           std::string ("user-197 Donald"));
      Simulator::Schedule (Seconds (13.000), &ndnBlockchainApp::NewTransaction, apps[108],
                           std::string ("user-145 Donald"));
      Simulator::Schedule (Seconds (14.005), &ndnBlockchainApp::NewTransaction, apps[109],
                           std::string ("user-146 Clinton"));
      Simulator::Schedule (Seconds (15.010), &ndnBlockchainApp::NewTransaction, apps[110],
                           std::string ("user-226 Russel"));

      Simulator::Schedule (Seconds (20.500), &ndnBlockchainApp::NewBlock, apps[91]);

      Simulator::Schedule (Seconds (50.000), &ndnBlockchainApp::ShowResults, apps[108]);

      // SCHEDULER END
      Simulator::Run ();
      Simulator::Destroy ();
    }
  else
    {
      auto allLinks = topologyReader.GetLinks ();
      InternetStackHelper internetHelper;
      internetHelper.Install (allNodes);

      topologyReader.AssignIpv4Addresses ("10.0.0.0");

      vector<Ptr<ipBlockchainApp>> apps;
      for (int i = 0; i < allNodes.size (); i++)
        {
          Ptr<Node> node = allNodes.Get (i);
          std::string nodeName = Names::FindName (node);

          Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
          Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1, 0);
          Ipv4Address ipAddr = iaddr.GetLocal ();

          cout << i << " " << nodeName << " " << ipAddr << endl;

          if (nodeName[0] == 'r')
            continue;

          Ptr<ipBlockchainApp> udp = CreateObject<ipBlockchainApp> ();
          udp->SetStartTime (Seconds (startTime));
          udp->SetStopTime (Seconds (endTime));
          udp->SetAttribute ("NodeName", StringValue (nodeName));

          stringstream ss;
          ss << ipAddr;
          string ipAddrStr;
          ss >> ipAddrStr;

          udp->SetAttribute ("m_thisAddress", StringValue (ipAddrStr));

          allNodes.Get (i)->AddApplication (udp);
          apps.push_back (udp);
        }

      Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
      Packet::EnablePrinting ();

      // Ptr<ipBlockchainApp> udp = CreateObject<ipBlockchainApp> ();

      // udp->SetStartTime (Seconds (0));
      // udp->SetStopTime (Seconds (20));

      // Ptr<Ipv4> ipv4 = allNodes.Get (100)->GetObject<Ipv4> ();
      // Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1, 0);
      // Ipv4Address ipAddr = iaddr.GetLocal ();
      // stringstream ss;
      // ss << ipAddr;
      // string ipAddrStr;
      // ss >> ipAddrStr;
      // udp->SetAttribute ("m_thisAddress", StringValue (ipAddrStr));

      // allNodes.Get (100)->AddApplication (udp);

      // Ipv4Address destIp ("10.1.17.2");

      Simulator::Schedule (Seconds (20), &ipBlockchainApp::NewTransaction, apps[108],
                           std::string ("user-145 Donald"));

      // std::string hello = "hello1";
      // Ptr<Packet> packet1 = Create<Packet> ((uint8_t *) hello.c_str (), hello.size () + 1);
      // Simulator::Schedule (Seconds (20), &ipBlockchainApp::SendPacket, apps[100], packet1, destIp,
      //                      80);

      Simulator::Run ();
      Simulator::Destroy ();
    }

  return 0;
}
} // namespace ns3

int
main (int argc, char *argv[])
{
  return ns3::main (argc, argv);
}