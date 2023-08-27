#ifndef NDNBCAPP_HPP
#define NDNBCAPP_HPP

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
        "miner-1",
        "miner-2",
        "miner-3",
        "miner-4",
        "miner-5",
        "miner-6",
        "miner-7",
        "miner-8",
        "miner-9",
        "miner-10",
        "miner-11",
        "miner-12",
        "miner-13",
        "miner-14",
        "miner-15",
        "miner-16"
    };

    std::vector <std::string> userList = {
        "user-144",
        "user-145",
        "user-146",
        "user-226",
        "user-157",
        "user-197",
        "user-249",
        "user-250",
        "user-153",
        "user-155",
        "user-177",
        "user-175",
        "user-198",
        "user-199",
        "user-184",
        "user-286",
        "user-189",
        "user-193",
        "user-289",
        "user-252",
        "user-291",
        "user-220",
        "user-215",
        "user-206",
        "user-294",
        "user-221",
        "user-314",
        "user-305",
        "user-253",
        "user-254",
        "user-307",
        "user-231",
        "user-302",
        "user-309",
        "user-232",
        "user-287",
        "user-230",
        "user-233",
        "user-310",
        "user-238",
        "user-246",
        "user-248",
        "user-319",
        "user-278",
        "user-284",
        "user-281",
        "user-280",
        "user-258",
        "user-259",
        "user-265",
        "user-283",
        "user-299",
        "user-300",
        "user-308",
        "user-313",
        "user-311"
    };


    // candidates: Donald Clinton Russel Andrew Smith

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
    std::unordered_map <int, bool> addedToBC;

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
    void sendInitRequest(std::string initNode);

// private:
    void BroadcastTransaction(string sender, string reciever);
    void BroadcastBlockToUsers(BBlock block);
    void BroadcastBlockToMiners(BBlock block);

    void verifyBlock(int hashVal);
    void addBlock(const BBlock& newBlock);
    std::string decode_data(std::shared_ptr<const ndn::Data> data);
    void printNodeName(std::string prompt);
};
}

#endif