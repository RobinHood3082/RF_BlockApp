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

namespace ns3 {
int main(int argc, char *argv[]) {
    CommandLine cmd;
    cmd.Parse(argc, argv);

    AnnotatedTopologyReader topologyReader("", 25);
    topologyReader.SetFileName("scratch/RF_BlockApp/topology/1755.r0-conv-annotated.txt");
    topologyReader.Read();

    NodeContainer allNodes = topologyReader.GetNodes();

    bool ndn = true;
    // change this flag to run on udp

    if (ndn) {
        ndn::StackHelper ndnHelper;
        ndnHelper.SetDefaultRoutes(true);
        ndnHelper.setPolicy("nfd::cs::lru");
        ndnHelper.setCsSize(100);
        ndnHelper.InstallAll();

        ndn::StrategyChoiceHelper::InstallAll("/ndn.blockchain", "/localhost/nfd/strategy/multicast");

        ndn::GlobalRoutingHelper ndnRoutingHelper;
        ndnRoutingHelper.InstallAll();

        ndn::AppHelper appHelper("BlockChainApp");
        for (auto& node : allNodes) {
            std::string nodeName = Names::FindName(node);
            appHelper.SetAttribute("NodeName", StringValue(nodeName));
            appHelper.SetAttribute("cssz", IntegerValue(100));
            appHelper.Install(node);

            std::string prefix = "/ndn.blockchain/" + nodeName;
            ndnRoutingHelper.AddOrigins(prefix, node);
        }

        ndn::GlobalRoutingHelper::CalculateAllPossibleRoutes();

        Simulator::Stop(Seconds(60.0));
        AnimationInterface anim("scratch/RF_BlockApp/output/animation.xml");

        Simulator::Run();
        Simulator::Destroy();
    } 
    else {

    }
}
}

int
main(int argc, char* argv[]) {
    return ns3::main(argc, argv);
}