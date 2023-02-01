#include "BlockChainApp.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

#include "bits/stdc++.h"
#include "ns3/string.h"
#include "ns3/integer.h"

using namespace std;

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("ndnBlockchainApp");
NS_OBJECT_ENSURE_REGISTERED(ndnBlockchainApp);

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> dist(0, std::numeric_limits<uint32_t>::max());

TypeId ndnBlockchainApp::GetTypeId() {
    auto checker = MakeIntegerChecker<int64_t>();
    static TypeId tid = TypeId("ns3::ndnBlockchainApp")
                        .AddConstructor<ndnBlockchainApp>()
                        .SetParent<ndn::App>()
                        .AddAttribute("NodeName", 
                                        "Name of the Node",
                                        StringValue("/"),
                                        MakeStringAccessor(&ndnBlockchainApp::nameOfNode),
                                        MakeStringChecker())
                        .AddAttribute("cssz", 
                                        "CS Size", 
                                        IntegerValue((int64_t) 20), 
                                        MakeIntegerAccessor(&ndnBlockchainApp::cssz), 
                                        checker);

    return tid;
}

void ndnBlockchainApp::StartApplication() {
    originBlock.sz = 0;
    blockChain.push_back(originBlock);
    temporaryBlock.hashLast = originBlock.hashThis;

    NS_LOG_INFO("ndnBlockchainApp start: " << nameOfNode);
    ndn::App::StartApplication();
    std::string route = "/ndn.blockchain";
    ndn::FibHelper::AddRoute(GetNode(), route, m_face, 0);

    if (nameOfNode[0] == 'm') appMode = MINER;
    else appMode = USER;
}

void ndnBlockchainApp::StopApplication() {
    ndn::App::StopApplication();
}

void ndnBlockchainApp::SendInterest(std::shared_ptr<const ndn::Interest> interest) {
    m_transmittedInterests(interest, this, m_face);
    m_appLink->onReceiveInterest(*interest);
}

void ndnBlockchainApp::OnInterest(std::shared_ptr<const ndn::Interest> interest) {
    ndn::App::OnInterest(interest);
}

std::string decode_data(std::shared_ptr<const ns3::ndn::Data> data) {
    auto& contentBlock = data->getContent();
    std::stringstream ss;
    std::string content, temp;

    ss << contentBlock;
    ss >> temp;

    int status = 0;
    for (int i = 0, sz = temp.size(); i < sz; i++) {
        if (temp[i] == '[') {
            status = SIZE_BEGIN;
        } else if (temp[i] == ']') {
            status = SIZE_END;
        } else if (temp[i] == '=') {
            status = CONTENT_BEGIN;
        } else {
            if (status != CONTENT_BEGIN) continue;
            std::string hexStr;
            hexStr += temp[i];
            i++;
            hexStr += temp[i];

            char c = stoi(hexStr, 0, 16);
            content += c;
        }
    }

    return content;
}

void ndnBlockchainApp::OnData(std::shared_ptr<const ndn::Data> data) {
    App::OnData(data);
}

void ndnBlockchainApp::verifyBlock(int hashVal) { // DONE
    if (verifyCount[hashVal] > minerList.size() / 2) return;

    // VRIFIED, sppse
    BBlock& newBlock = blockStore[hashVal];
    verifyCount[hashVal]++;

    // broadcast verification
    for (auto rcpnt : minerList) {
        if (rcpnt == nameOfNode) continue;

        std::string prefix = "/ndn.blockchain/" + rcpnt;
        prefix += "/" + std::to_string(newBlock.hashThis);
        prefix += "/" + nameOfNode;
        prefix += "/verify";

        auto newinterest = std::make_shared<ndn::Interest>(prefix);
        newinterest->setNonce(dist(rng));
        newinterest->setInterestLifetime(ndn::time::seconds(40000));
        this->SendInterest(newinterest);
    }

    if (verifyCount[hashVal] > minerList.size() / 2) {
        this->addBlock(newBlock);
        this->BroadcastBlockToUsers(newBlock);
    }
}

void ndnBlockchainApp::addBlock(const BBlock& newBlock) { // DONE
    blockChain.push_back(newBlock);
    trList.clear();
}

void ndnBlockchainApp::ShowResults() {
    std::string output;
    std::map <std::string, int> voteCount;

    for (auto& b : blockChain) {
        if (b.hashThis == 0) continue; // if originBlock, skip
        for (int i = 0; i < b.sz; i++) {
            voteCount[b.trDest[i]]++;
        }
    }

    output += "Results on " + nameOfNode;
    output += "\n";

    output += "<candidate>\t<vote_count>\n";
    for (auto [s, c] : voteCount) {
        output += s + "\t";
        output += std::to_string(c) +"\n";
    }

    output += "---\n";
    NS_LOG_INFO(output);
}

void ndnBlockchainApp::NewTransaction(string trxData) {
    istringstream is(trxData);
    string user, rcpnt;

    is >> user;
    is >> rcpnt;

    this->BroadcastTransaction(user, rcpnt);
}

void ndnBlockchainApp::BroadcastTransaction(string sender, string reciever) {
    for (auto& miner : minerList) {
        string prefix = "/ndn.blockchain/" + miner + "/" + reciever;
        prefix += "/" + nameOfNode;
        prefix += "/vote";
    
        auto interest = std::make_shared<ndn::Interest>(prefix);
        interest->setNonce(dist(rng));
        interest->setInterestLifetime(ndn::time::seconds(40000));
        this->SendInterest(interest);
    }
}

void ndnBlockchainApp::BroadcastBlockToUsers(BBlock newBlock) {
    for (auto rcpnt : userList) {
        if (rcpnt == nameOfNode) continue;

        std::string tmpPrefix = "/ndn.blockchain/";
        tmpPrefix += rcpnt + "/";
        tmpPrefix += newBlock.creator + "/";
        tmpPrefix += std::to_string(blockChain.size() - 1) + "/";
        tmpPrefix += "block_ready";

        auto interest = std::make_shared<ndn::Interest>(tmpPrefix);
        interest->setNonce(dist(rng));
        interest->setInterestLifetime(ndn::time::seconds(40000));
        this->SendInterest(interest);
    }
}

void ndnBlockchainApp::BroadcastBlockToMiners(BBlock newBlock) {
    for (auto rcpnt : minerList) {
        if (rcpnt == nameOfNode) continue;

        std::string tmpPrefix = "/ndn.blockchain/";
        tmpPrefix += rcpnt + "/";
        tmpPrefix += newBlock.creator + "/";
        tmpPrefix += std::to_string(blockChain.size() - 1) + "/";
        tmpPrefix += "block_ready";

        auto interest = std::make_shared<ndn::Interest>(tmpPrefix);
        interest->setNonce(dist(rng));
        interest->setInterestLifetime(ndn::time::seconds(40000));
        this->SendInterest(interest);
    }
}

void ndnBlockchainApp::OnInterest(std::shared_ptr<const ndn::Interest> interest) {
    ndn::App::OnInterest(interest);

    std::string interestName = "";
    std::stringstream ss;
    ss << interest->getName();
    ss >> interestName;

    for (char& c : interestName) if (c == '/') c = ' ';
    std::istringstream is(interestName);
    std::string word;
    std::vector <std::string> vec;

    while (is >> word) vec.push_back(word);

    if (vec.back() == "init") return;
    else if (vec.back() == "verify") {
        int hashVal = std::stoi(vec[2]);
        this->verifyBlock(hashVal);
        return;
    }

    if (vec.back() == "vote") {
        std::string voteRecipient = vec[2], byUser = vec[3];

        trList.push_back(std::make_pair(byUser, voteRecipient));
    }

    else if (vec.back() == "block_ready_ack") {
        BBlock& last = blockChain.back();
        std::string blockContent;

        blockContent += last.creator;
        blockContent += " " + std::to_string(last.trSource.size());
        blockContent += " " + std::to_string(last.hashLast);
        blockContent += " " + std::to_string(last.hashThis);
        blockContent += " " + std::to_string(last.pow);

        for (int i = 0; i < last.trSource.size(); i++) {
            blockContent += " " + last.trSource[i];
            blockContent += " " + last.trDest[i];
        }

        auto data = std::make_shared<ndn::Data>(interest->getName());
        data->setFreshnessPeriod(ndn::time::milliseconds(400000));
        data->setContent(std::make_shared< ::ndn::Buffer>(blockContent.begin(), blockContent.end()));
        ndn::StackHelper::getKeyChain().sign(*data);

//            NS_LOG_DEBUG("Sending Data for " << data->getName() << " from " << nameOfNode);
        m_transmittedDatas(data, this, m_face);
        m_appLink->onReceiveData(*data);
    }

    else if (vec.back() == "block_ready") {
        std::string prefix = "/ndn.blockchain/";
        prefix += vec[2] + "/";
        prefix += vec[3] + "/";
        prefix += "block_ready_ack";

        auto newinterest = std::make_shared<ndn::Interest>(prefix);
        newinterest->setNonce(dist(rng));
        newinterest->setInterestLifetime(ndn::time::seconds(40000));
        this->SendInterest(newinterest);
    }
}

void ndnBlockchainApp::NewBlock() {
    temporaryBlock.creator = nameOfNode;
    temporaryBlock.hashLast = blockChain.back().hashThis;
    temporaryBlock.hashThis = temporaryBlock.hashLast + 2;
    temporaryBlock.pow = 5 * blockChain.size();
    temporaryBlock.sz = trList.size();

    NS_LOG_INFO(nameOfNode << " generated Proof-of-work for block " << blockChain.size());

    temporaryBlock.trSource.clear();
    temporaryBlock.trDest.clear();
    for (auto [u, v] : trList) {
        temporaryBlock.trSource.push_back(u);
        temporaryBlock.trDest.push_back(v);
    }

    trList.clear();
    blockChain.push_back(temporaryBlock);

    this->BroadcastBlockToMiners(temporaryBlock);
}
}