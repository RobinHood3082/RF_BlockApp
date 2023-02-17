#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "bits/stdc++.h"
#include "ns3/integer.h"

#include "ndnBlockchainApp.hpp"

namespace ns3 {
int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    int startTime = 0, endTime = 10;

    AnnotatedTopologyReader topologyReader("", 25);
    topologyReader.SetFileName("scratch/RF_BlockApp/topology/topology.txt");
    topologyReader.Read();

    NodeContainer allNodes = topologyReader.GetNodes();

    bool ndn_flag = true;
    // change this flag to run on udp

    if (ndn_flag) {
        LogComponentEnable("ndnBlockchainApp", LOG_LEVEL_INFO);

        ndn::StackHelper ndnHelper;
        ndnHelper.SetDefaultRoutes(true);
        ndnHelper.setPolicy("nfd::cs::lru");
        ndnHelper.setCsSize(100);

        ndnHelper.InstallAll();

        ndn::StrategyChoiceHelper::InstallAll("/ndn.blockchain", "/localhost/nfd/strategy/multicast");

        ndn::GlobalRoutingHelper ndnRoutingHelper;
        ndnRoutingHelper.InstallAll();

        ndn::AppHelper appHelper("ndnBlockchainApp");

        vector <Ptr<ndnBlockchainApp>> apps;
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

        for (auto& node : allNodes) {
            std::string nodeName = Names::FindName(node);

            Ptr <ndnBlockchainApp> app = CreateObject <ndnBlockchainApp>();
            app->SetStartTime(Seconds(startTime));
            app->SetStopTime(Seconds(endTime));
            app->SetAttribute("NodeName", StringValue(nodeName));
            app->SetAttribute("cssz", IntegerValue(100));

            apps.push_back(app);

            if (nodeName[0] == 'r') continue;

            node->AddApplication(app);
            std::string prefix = "/ndn.blockchain/" + nodeName;
            ndnRoutingHelper.AddOrigins(prefix, node);
        }

        ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();

        Simulator::Stop(Seconds(endTime));
        AnimationInterface anim("scratch/RF_BlockApp/output/animation.xml");

        // std::cout << "App name is: ";
        // std::cout << allNodes.Get(108)->Get << std::endl;

        // SCHEDULER START
        // std::cout << "App: " << apps[108]->GetTypeId().GetName() << std::endl;
        Simulator::Schedule(Seconds(2), &ndnBlockchainApp::sendInitRequest, apps[108], std::string("user-144"));

        // SCHEDULER END
        Simulator::Run();
        Simulator::Destroy();
    } 
    else {

    }

    return 0;
}
}

int
main(int argc, char* argv[]) {
    return ns3::main(argc, argv);
}