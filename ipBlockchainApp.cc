#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ipBlockchainApp.hpp"
#include "globalNodeContainer.hpp"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ipBlockchainApp");
NS_OBJECT_ENSURE_REGISTERED (ipBlockchainApp);

TypeId
ipBlockchainApp::GetTypeId ()
{
  static TypeId tid =
      TypeId ("ns3::ipBlockchainApp")
          .AddConstructor<ipBlockchainApp> ()
          .SetParent<Application> ()
          .AddAttribute ("NodeName", "Name of the Node", StringValue ("/"),
                         MakeStringAccessor (&ipBlockchainApp::nameOfNode), MakeStringChecker ())
          .AddAttribute ("m_thisAddress", "IPv4 address of itself", ObjectVectorValue (),
                         MakeStringAccessor (&ipBlockchainApp::m_thisAddress),
                         MakeStringChecker ());

  return tid;
}

void
ipBlockchainApp::SetNodeContainer (NodeContainer nodes)
{
  allNodes = nodes;
}

TypeId
ipBlockchainApp::GetInstanceTypeId () const
{
  return ipBlockchainApp::GetTypeId ();
}

ipBlockchainApp::ipBlockchainApp ()
{
  // Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();
  // Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);
  m_port = 80;
}

void
ipBlockchainApp::SetupRecieveSocket (Ptr<Socket> socket, uint16_t port)
{
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
  if (socket->Bind (local) == -1)
    {
      NS_FATAL_ERROR ("Failed to bind socket");
    }
}

void
ipBlockchainApp::StartApplication ()
{
  cout << nameOfNode << " started application" << endl;

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  m_recvSocket = Socket::CreateSocket (GetNode (), tid);

  SetupRecieveSocket (m_recvSocket, m_port);

  m_recvSocket->SetRecvCallback (MakeCallback (&ipBlockchainApp::OnPacket, this));
  m_recvSocket->SetAllowBroadcast (true);

  m_sendSocket = Socket::CreateSocket (GetNode (), tid);
  m_sendSocket->SetAllowBroadcast (true);
}

void
ipBlockchainApp::StopApplication ()
{
}

void
ipBlockchainApp::OnPacket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_DEBUG (m_thisAddress << ": Recieved a packet!");
  cout << m_thisAddress << " recieved a packet!" << endl;
  Ptr<Packet> packet;
  Address from, localAddress;

  // if (packet = socket->RecvFrom (from))
  //   {
  //     // std::string recvdStr = packet->ToString();
  //     Ipv4Header ipHeader;
  //     packet->RemoveHeader (ipHeader);

  //     uint8_t buffer[packet->GetSize ()];
  //     int len = packet->CopyData (buffer, packet->GetSize ());
  //     std::string recvdStr (buffer, buffer + packet->GetSize () - 1);

  //     NS_LOG_INFO ("Received a Packet: \"" << recvdStr << "\" at time " << Now ().GetSeconds ()
  //                                          << " on " << m_thisAddress);

  //     if (recvdStr == "")
  //       return;

  //     std::string sendStr = m_responseString[recvdStr];

  //     Ptr<Packet> sendPacket = Create<Packet> ((uint8_t *) sendStr.c_str (), sendStr.size () + 1);

  //     SendPacket (sendPacket, ipHeader.GetSource (), 80);

  //     // PppHeader PppHeader;
  //     // copy->RemoveHeader(PppHeader);

  //     // // Ipv4Address dest =
  //     // std::cout << "Recieved from ";
  //     // ipHeader.GetSource().Print(std::cout);
  //     // std::cout << std::endl;
  //     // ipHeader.GetDestination().Print(std::cout);
  //     // std::cout << std::endl;

  //     // NS_LOG_DEBUG(sendStr);
  //     // if (recvdStr == "hello1") {
  //     //     Ipv4Address dest("10.1.1.1");
  //     //     SendPacket(sendPacket, dest, 80);
  //     // }
  //   }
}

void
ipBlockchainApp::SendPacket (Ptr<Packet> packet, Ipv4Address destination, uint16_t port)
{
  cout << m_thisAddress << ": Sending data to " << destination << " on port " << port << endl;

  uint8_t buffer[packet->GetSize ()];
  int len = packet->CopyData (buffer, packet->GetSize ());
  std::string recvdStr (buffer, buffer + packet->GetSize () - 1);

  NS_LOG_INFO ("Sending " << recvdStr);

  NS_LOG_FUNCTION (this << packet << destination << port);
  m_sendSocket->Connect (InetSocketAddress (destination, port));
  m_sendSocket->Send (packet);
}

void
ipBlockchainApp::NewTransaction (string trxData)
{
  istringstream is (trxData);
  string user, rcpnt;

  is >> user;
  is >> rcpnt;

  this->BroadcastTransaction (user, rcpnt);

  cout << "Transaction created by " << nameOfNode << endl;
}

void
ipBlockchainApp::BroadcastTransaction (string sender, string reciever)
{
  string trxData = "/udp.blockchain/" + reciever;
  trxData += "/" + nameOfNode;
  trxData += "/vote";

  cout << "Broadcasting transaction: " << trxData << endl;

  Ptr<Packet> sendPacket = Create<Packet> ((uint8_t *) trxData.c_str (), trxData.size () + 1);

  NS_LOG_DEBUG ("Size is: " << allNodesContainer.size ());

  // for (auto it = allNodes.begin (); it != allNodes.end (); it++)
  //   {
  //     Ptr<Node> node = *it;
  //     Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  //     Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1, 0);
  //     Ipv4Address ipAddr = iaddr.GetLocal ();

  //     NS_LOG_DEBUG (ipAddr);

  //     // if (Names::FindName (node)[0] == 'r')
  //     //   continue;

  //     // if (Names::FindName (node) == nameOfNode)
  //     //   continue;

  //     this->SendPacket (sendPacket, ipAddr, 80);
  //   }
}