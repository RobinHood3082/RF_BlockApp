#ifndef IPBCAPP_HPP
#define IPBCAPP_HPP

#include "BBlock.hpp"
#include "ns3/application.h"
#include "ns3/socket.h"

using namespace std;

namespace ns3 {
class ipBlockchainApp : public Application
{
public:
  ipBlockchainApp ();

  BBlock originBlock;
  AppMode appMode;

  std::string nameOfNode;
  int64_t cssz = 20;
  std::vector<std::string> minerList = {"miner-1",  "miner-2",  "miner-3",  "miner-4",
                                        "miner-5",  "miner-6",  "miner-7",  "miner-8",
                                        "miner-9",  "miner-10", "miner-11", "miner-12",
                                        "miner-13", "miner-14", "miner-15", "miner-16"};

  std::vector<std::string> userList = {
      "user-144", "user-145", "user-146", "user-226", "user-157", "user-197", "user-249",
      "user-250", "user-153", "user-155", "user-177", "user-175", "user-198", "user-199",
      "user-184", "user-286", "user-189", "user-193", "user-289", "user-252", "user-291",
      "user-220", "user-215", "user-206", "user-294", "user-221", "user-314", "user-305",
      "user-253", "user-254", "user-307", "user-231", "user-302", "user-309", "user-232",
      "user-287", "user-230", "user-233", "user-310", "user-238", "user-246", "user-248",
      "user-319", "user-278", "user-284", "user-281", "user-280", "user-258", "user-259",
      "user-265", "user-283", "user-299", "user-300", "user-308", "user-313", "user-311"};

  static TypeId GetTypeId ();
  virtual TypeId GetInstanceTypeId () const;

  void OnPacket (Ptr<Socket> socket);
  void SendPacket (Ptr<Packet> packet, Ipv4Address destination, uint16_t port);

  // candidates: Donald Clinton Russel Andrew Smith

  // suffix list for interest packet
  /// /vote : new vote
  /// /<miner_number>/<block number>/block_ready : to let everyone know that new block is generated
  /// /<miner_number>/<block number>/block_ready_ack : acknowledgement
  /// /<miner_number>/<block number>/verify : block verified

  Ptr<Socket> m_recvSocket, m_sendSocket;
  uint16_t m_port;
  std::map<std::string, std::string> m_responseString;
  std::string m_thisAddress;

  std::list<BBlock> blockChain;
  std::vector<std::pair<std::string, std::string>> trList;
  BBlock temporaryBlock;
  std::unordered_map<int, BBlock> blockStore;
  std::unordered_map<int, int> verifyCount;
  std::unordered_map<int, bool> addedToBC;

  virtual void StartApplication ();
  virtual void StopApplication ();

  void SetupRecieveSocket (Ptr<Socket> socket, uint16_t port);

  void NewTransaction (string trxData);
  void NewBlock ();
  void ShowResults ();
  void sendInitRequest (std::string initNode);

  // private:
  void BroadcastTransaction (string sender, string reciever);
  void BroadcastBlockToUsers (BBlock block);
  void BroadcastBlockToMiners (BBlock block);

  void verifyBlock (int hashVal);
  void addBlock (const BBlock &newBlock);
  std::string decode_data (std::shared_ptr<const ndn::Data> data);
  void printNodeName (std::string prompt);
};
} // namespace ns3

#endif