#include "ndnBlockchainApp.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/ndnSIM/ndn-cxx/lp/tags.hpp"
#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

#include <random>

#include "ns3/string.h"
#include "ns3/integer.h"

using namespace std;

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("ndnBlockchainApp");
NS_OBJECT_ENSURE_REGISTERED (ndnBlockchainApp);

std::random_device dev;
std::mt19937 rng (dev ());
std::uniform_int_distribution<std::mt19937::result_type>
    dist (0, std::numeric_limits<uint32_t>::max ());

TypeId
ndnBlockchainApp::GetTypeId ()
{
  auto checker = MakeIntegerChecker<int64_t> ();
  static TypeId tid =
      TypeId ("ndnBlockchainApp")
          .SetParent<ndn::App> ()
          .AddAttribute ("NodeName", "Name of the Node", StringValue ("/"),
                         MakeStringAccessor (&ndnBlockchainApp::nameOfNode), MakeStringChecker ())
          .AddAttribute ("cssz", "CS Size", IntegerValue ((int64_t) 20),
                         MakeIntegerAccessor (&ndnBlockchainApp::cssz), checker)
          .AddConstructor<ndnBlockchainApp> ();

  return tid;
}

void
ndnBlockchainApp::StartApplication ()
{
  originBlock.sz = 0;
  blockChain.push_back (originBlock);
  temporaryBlock.hashLast = originBlock.hashThis;

  // NS_LOG_INFO("ndnBlockchainApp start: " << nameOfNode);
  ndn::App::StartApplication ();
  std::string route = "/ndn.blockchain";
  ndn::FibHelper::AddRoute (GetNode (), route, m_face, 0);

  if (nameOfNode[0] == 'm')
    appMode = MINER;
  else
    appMode = USER;
}

void
ndnBlockchainApp::StopApplication ()
{
  ndn::App::StopApplication ();
}

void
ndnBlockchainApp::SendInterest (std::shared_ptr<const ndn::Interest> interest)
{
  m_transmittedInterests (interest, this, m_face);
  m_appLink->onReceiveInterest (*interest);
}

std::string
ndnBlockchainApp::decode_data (std::shared_ptr<const ns3::ndn::Data> data)
{
  auto &contentBlock = data->getContent ();
  std::stringstream ss;
  std::string content, temp;

  ss << contentBlock;
  ss >> temp;

  int status = 0;
  for (int i = 0, sz = temp.size (); i < sz; i++)
    {
      if (temp[i] == '[')
        {
          status = SIZE_BEGIN;
        }
      else if (temp[i] == ']')
        {
          status = SIZE_END;
        }
      else if (temp[i] == '=')
        {
          status = CONTENT_BEGIN;
        }
      else
        {
          if (status != CONTENT_BEGIN)
            continue;
          std::string hexStr;
          hexStr += temp[i];
          i++;
          hexStr += temp[i];

          char c = stoi (hexStr, 0, 16);
          content += c;
        }
    }

  return content;
}

void
ndnBlockchainApp::verifyBlock (int hashVal)
{ // DONE
  verifyCount[hashVal]++;
  BBlock newBlock = blockStore[hashVal];
  if (verifyCount[hashVal] > minerList.size () / 2)
    {
      if (addedToBC[hashVal])
        return;
      this->addBlock (newBlock);
      this->BroadcastBlockToUsers (newBlock);
      return;
    }
}

void
ndnBlockchainApp::sendInitRequest (std::string initNode)
{
  std::string prefix = "/ndn.blockchain/" + initNode;
  prefix += "/init";

  // NS_LOG_INFO("Sending init request from " << nameOfNode);

  auto newinterest = std::make_shared<ndn::Interest> (prefix);
  newinterest->setNonce (dist (rng));
  newinterest->setInterestLifetime (ndn::time::seconds (40000));
  this->SendInterest (newinterest);
}

void
ndnBlockchainApp::addBlock (const BBlock &newBlock)
{
  if (addedToBC[newBlock.hashThis])
    return;

  NS_LOG_DEBUG ("Block " << blockChain.size () << " added to " << nameOfNode << "\'s blockchain");
  blockChain.push_back (newBlock);
  trList.clear ();
  //   cout << "Creator: " << newBlock.creator << "\nPreserved Size: " << newBlock.sz
  //        << "\nActual Size: " << newBlock.trDest.size () << "\nHashLast: " << newBlock.hashLast
  //        << "\nHashThis: " << newBlock.hashThis << "\nProof-of-Work: " << newBlock.pow << endl;

  addedToBC[newBlock.hashThis] = true;
}

void
ndnBlockchainApp::ShowResults ()
{
  std::string output;
  std::map<std::string, int> voteCount;

  for (auto &b : blockChain)
    {
      if (b.hashThis == 0)
        continue; // if originBlock, skip
      for (int i = 0; i < b.sz; i++)
        {
          voteCount[b.trDest[i]]++;
        }
    }

  output += "Results on " + nameOfNode;
  output += "\n";

  output += "<candidate>\t<vote_count>\n";
  for (auto [s, c] : voteCount)
    {
      output += s + "\t";
      output += std::to_string (c) + "\n";
    }

  output += "---\n";
  NS_LOG_INFO (output);
}

void
ndnBlockchainApp::NewTransaction (string trxData)
{
  istringstream is (trxData);
  string user, rcpnt;

  is >> user;
  is >> rcpnt;

  this->BroadcastTransaction (user, rcpnt);

  NS_LOG_DEBUG ("Transaction created by " << nameOfNode);
}

void
ndnBlockchainApp::BroadcastTransaction (string sender, string reciever)
{
  for (auto &miner : minerList)
    {
      string prefix = "/ndn.blockchain/" + miner + "/" + reciever;
      prefix += "/" + nameOfNode;
      prefix += "/vote";

      auto interest = std::make_shared<ndn::Interest> (prefix);
      interest->setNonce (dist (rng));
      interest->setInterestLifetime (ndn::time::seconds (40000));
      this->SendInterest (interest);
    }
}

void
ndnBlockchainApp::BroadcastBlockToUsers (BBlock newBlock)
{
  for (auto rcpnt : userList)
    {
      if (rcpnt == nameOfNode)
        continue;

      std::string tmpPrefix = "/ndn.blockchain/";
      tmpPrefix += rcpnt + "/";
      tmpPrefix += newBlock.creator + "/";
      tmpPrefix += std::to_string (blockChain.size () - 1) + "/";
      tmpPrefix += "block_ready";

      auto interest = std::make_shared<ndn::Interest> (tmpPrefix);
      interest->setNonce (dist (rng));
      interest->setInterestLifetime (ndn::time::seconds (40000));
      this->SendInterest (interest);
    }
}

void
ndnBlockchainApp::BroadcastBlockToMiners (BBlock newBlock)
{
  for (auto rcpnt : minerList)
    {
      if (rcpnt == nameOfNode)
        continue;

      std::string tmpPrefix = "/ndn.blockchain/";
      tmpPrefix += rcpnt + "/";
      tmpPrefix += newBlock.creator + "/";
      tmpPrefix += std::to_string (blockChain.size () - 1) + "/";
      tmpPrefix += "block_ready";

      auto interest = std::make_shared<ndn::Interest> (tmpPrefix);
      interest->setNonce (dist (rng));
      interest->setInterestLifetime (ndn::time::seconds (40000));
      this->SendInterest (interest);
    }
}

void
ndnBlockchainApp::OnInterest (std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest (interest);

  std::string interestName = "";
  std::stringstream ss;
  ss << interest->getName ();
  ss >> interestName;

  // NS_LOG_INFO("Recieved Interest: " << interestName);

  for (char &c : interestName)
    if (c == '/')
      c = ' ';
  std::istringstream is (interestName);
  std::string word;
  std::vector<std::string> vec;

  while (is >> word)
    vec.push_back (word);

  if (vec.back () == "init")
    {
      std::string candidates = "# Donald Clinton Russel Andrew Smith #";

      auto data = std::make_shared<ndn::Data> (interest->getName ());
      data->setFreshnessPeriod (ndn::time::milliseconds (400000));
      data->setContent (std::make_shared<::ndn::Buffer> (candidates.begin (), candidates.end ()));
      ndn::StackHelper::getKeyChain ().sign (*data);

      m_transmittedDatas (data, this, m_face);
      m_appLink->onReceiveData (*data);

      return;
    }
  else if (vec.back () == "verify")
    {
      int hashVal = std::stoi (vec[2]);
      this->verifyBlock (hashVal);
      return;
    }

  if (vec.back () == "vote")
    {
      std::string voteRecipient = vec[2], byUser = vec[3];

      trList.push_back (std::make_pair (byUser, voteRecipient));
    }

  else if (vec.back () == "block_ready_ack")
    {
      BBlock &last = blockChain.back ();
      std::string blockContent;

      blockContent += last.creator;
      blockContent += " " + std::to_string (last.trSource.size ());
      blockContent += " " + std::to_string (last.hashLast);
      blockContent += " " + std::to_string (last.hashThis);
      blockContent += " " + std::to_string (last.pow);

      for (int i = 0; i < last.trSource.size (); i++)
        {
          blockContent += " " + last.trSource[i];
          blockContent += " " + last.trDest[i];
        }

      auto data = std::make_shared<ndn::Data> (interest->getName ());
      data->setFreshnessPeriod (ndn::time::milliseconds (400000));
      data->setContent (
          std::make_shared<::ndn::Buffer> (blockContent.begin (), blockContent.end ()));
      ndn::StackHelper::getKeyChain ().sign (*data);

      //            NS_LOG_DEBUG("Sending Data for " << data->getName() << " from " << nameOfNode);
      m_transmittedDatas (data, this, m_face);
      m_appLink->onReceiveData (*data);
    }

  else if (vec.back () == "block_ready")
    {
      std::string prefix = "/ndn.blockchain/";
      prefix += vec[2] + "/";
      prefix += vec[3] + "/";
      prefix += "block_ready_ack";

      auto newinterest = std::make_shared<ndn::Interest> (prefix);
      newinterest->setNonce (dist (rng));
      newinterest->setInterestLifetime (ndn::time::seconds (40000));
      this->SendInterest (newinterest);
    }
}

void
ndnBlockchainApp::OnData (std::shared_ptr<const ndn::Data> data)
{
  App::OnData (data);

  NS_LOG_FUNCTION (this << data);

  std::string decodedData = this->decode_data (data), word;
  std::string dataName;
  std::stringstream tmpStream;
  tmpStream << data->getName ();
  tmpStream >> dataName;

  if (dataName.back () == 't')
    {
      // NS_LOG_INFO("Recieved candidates list on <" << nameOfNode << ">: " << decodedData);
      return;
    }

  std::istringstream is (decodedData);
  BBlock newBlock;

  is >> word;
  std::string blockSource = word;
  newBlock.creator = blockSource;

  is >> word; // sz
  newBlock.sz = std::stoi (word);

  is >> word;
  newBlock.hashLast = std::stoi (word);

  is >> word;
  newBlock.hashThis = std::stoi (word);

  is >> word;
  newBlock.pow = std::stoi (word);

  for (int i = 0; i < newBlock.sz; i++)
    {
      is >> word;
      newBlock.trSource.push_back (word);

      is >> word;
      newBlock.trDest.push_back (word);
    }

  if (addedToBC[newBlock.hashThis])
    return;

  blockStore[newBlock.hashThis] = newBlock;

  if (appMode == USER)
    {
      addBlock (newBlock);
      return;
    }

  this->verifyBlock (newBlock.hashThis);

  // broadcast verification
  for (auto rcpnt : minerList)
    {
      if (rcpnt == nameOfNode)
        continue;

      std::string prefix = "/ndn.blockchain/" + rcpnt;
      prefix += "/" + std::to_string (newBlock.hashThis);
      prefix += "/" + nameOfNode;
      prefix += "/verify";

      auto newinterest = std::make_shared<ndn::Interest> (prefix);
      newinterest->setNonce (dist (rng));
      newinterest->setInterestLifetime (ndn::time::seconds (40000));
      this->SendInterest (newinterest);
    }
}

void
ndnBlockchainApp::NewBlock ()
{
  temporaryBlock.creator = nameOfNode;
  temporaryBlock.hashLast = blockChain.back ().hashThis;
  temporaryBlock.hashThis = temporaryBlock.hashLast + 2;
  temporaryBlock.pow = 5 * blockChain.size ();
  temporaryBlock.sz = trList.size ();

  NS_LOG_DEBUG (nameOfNode << " generated Proof-of-work for block " << blockChain.size ());

  temporaryBlock.trSource.clear ();
  temporaryBlock.trDest.clear ();
  for (auto [u, v] : trList)
    {
      temporaryBlock.trSource.push_back (u);
      temporaryBlock.trDest.push_back (v);
    }

  NS_LOG_DEBUG ("Creator: " << temporaryBlock.creator << "\nPreserved Size: " << temporaryBlock.sz
                            << "\nActual Size: " << temporaryBlock.trDest.size () << "\nHashLast: "
                            << temporaryBlock.hashLast << "\nHashThis: " << temporaryBlock.hashThis
                            << "\nProof-of-Work: " << temporaryBlock.pow);

  addBlock (temporaryBlock);

  this->BroadcastBlockToMiners (temporaryBlock);
}

void
ndnBlockchainApp::printNodeName (std::string prompt)
{
  std::cout << prompt << " " << nameOfNode << std::endl;
}
} // namespace ns3