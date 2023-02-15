#ifndef BCAPP_HPP
#define BCAPP_HPP

#include "BBlock.hpp"
#include "ns3/ndnSIM/apps/ndn-app.hpp"

using namespace std;

enum AppMode {MINER, USER};

namespace ns3 {
class ndnBlockchainApp : public ndn::App {
public:
    BBlock originBlock;
    AppMode appMode;

    std::string nameOfNode;
    int64_t cssz = 20;
    std::vector <std::string> minerList = {
        "miner1",
        "miner2",
        "miner3"
    };

    std::vector <std::string> userList = {
        "user1",
        "user2",
        "user3",
        "user4",
        "user5",
        "user6",
        "user7",
        "user8",
        "user9"
    };

    std::vector <std::string> votes = {
        "01 user1 Ash",
        "02 user2 Ash",
        "03 user3 Robin",
        "04 user4 Brock",
        "05 user5 Brock",
        "06 user6 Damian",
        "07 user7 Damian",
        "08 user8 Damian",
        "09 user9 Ash"
    };

    // suffix list for interest packet
    /// /vote : new vote
    /// /<miner_number>/<block number>/block_ready : to let everyone know that new block is generated
    /// /<miner_number>/<block number>/block_ready_ack : acknowledgement
    /// /<miner_number>/<block number>/verify : block verified

    std::list <BBlock> blockChain;
    std::vector <std::pair<std::string, std::string>> trList;
    BBlock temporaryBlock;
    std::unordered_map <int, BBlock> blockStore;
    std::unordered_map <int, int> verifyCount;

    int CurrentGeneration = 0;
    static TypeId GetTypeId();
    virtual void StartApplication();
    virtual void StopApplication();
    virtual void OnInterest(std::shared_ptr<const ndn::Interest> interest);
    virtual void OnData(std::shared_ptr<const ndn::Data> contentObject);
    void SendInterest(std::shared_ptr<const ndn::Interest> interest);

    void NewTransaction(string trxData);
    void NewBlock();
    void ShowResults();

private:
    void BroadcastTransaction(string sender, string reciever);
    void BroadcastBlockToUsers(BBlock block);
    void BroadcastBlockToMiners(BBlock block);

    void verifyBlock(int hashVal);
    void addBlock(const BBlock& newBlock);
    void sendInitRequest(std::string initNode);
    std::string decode_data(std::shared_ptr<const ndn::Data> data);
};
}

#endif