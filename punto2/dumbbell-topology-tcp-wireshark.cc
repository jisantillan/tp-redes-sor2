//Imports
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-dumbbell.h"
#include <fstream>
#include <stddef.h>                    
#include <iomanip>
#include "ns3/packet-sink.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <string>

//Namespace declaration
using namespace ns3;

static bool firstCwnd20 = true;
static bool firstSshThr20 = true;
static bool firstRtt20 = true;
static bool firstRto20 = true;

static Ptr<OutputStreamWrapper> cWndStream20;
static Ptr<OutputStreamWrapper> ssThreshStream20;
static Ptr<OutputStreamWrapper> rttStream20;
static Ptr<OutputStreamWrapper> rtoStream20;
static Ptr<OutputStreamWrapper> inFlightStream20;

static uint32_t cWndValue20;
static uint32_t ssThreshValue20;

static void
CongestionWindowTracer2_0 (uint32_t oldval, uint32_t newval)
{
  if (firstCwnd20)
    {
      *cWndStream20->GetStream () << "0.0 " << oldval << std::endl;
      firstCwnd20 = false;
    }
  *cWndStream20->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  cWndValue20 = newval;

  //SSThresh is set according to the CongestionWindow. When the CongestionWindow changes, so does the SSThresh, and viceversa
  if (!firstSshThr20)
    {
      *ssThreshStream20->GetStream () << Simulator::Now ().GetSeconds () << " " << ssThreshValue20 << std::endl;
    }
}

static void
SSThreshTracer2_0 (uint32_t oldval, uint32_t newval)
{
  if (firstSshThr20)
    {
      *ssThreshStream20->GetStream () << "0.0 " << oldval << std::endl;
      firstSshThr20 = false;
    }
  *ssThreshStream20->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
  ssThreshValue20 = newval;

  //Go to line 44
  if (!firstCwnd20)
    {
      *cWndStream20->GetStream () << Simulator::Now ().GetSeconds () << " " << cWndValue20 << std::endl;
    }
}

static void
RTTTracer2_0 (Time oldval, Time newval)
{
  if (firstRtt20)
    {
      *rttStream20->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRtt20 = false;
    }
  *rttStream20->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}

static void
RTOTracer2_0 (Time oldval, Time newval)
{
  if (firstRto20)
    {
      *rtoStream20->GetStream () << "0.0 " << oldval.GetSeconds () << std::endl;
      firstRto20 = false;
    }
  *rtoStream20->GetStream () << Simulator::Now ().GetSeconds () << " " << newval.GetSeconds () << std::endl;
}

static void
BytesInFlightTracer2_0 (uint32_t old, uint32_t inFlight)
{
  //NS_UNUSED (old);
  *inFlightStream20->GetStream () << Simulator::Now ().GetSeconds () << " " << inFlight << std::endl;
}

static void
TraceCongestionWindow2_0 (std::string cwnd_tr_file_name)
{
  AsciiTraceHelper ascii;
  cWndStream20 = ascii.CreateFileStream (cwnd_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer20));
}

static void
TraceSSThresh2_0 (std::string ssthresh_tr_file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream20 = ascii.CreateFileStream (ssthresh_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SsThreshTracer20));
}

static void
TraceRTT2_0 (std::string rtt_tr_file_name)
{
  AsciiTraceHelper ascii;
  rttStream20 = ascii.CreateFileStream (rtt_tr_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeCallback (&RttTracer20));
}

static void
TraceRTO2_0 (std::string rto_tr_file_name)
{
  AsciiTraceHelper ascii;
  rtoStream20 = ascii.CreateFileStream (rto_tr_file_name.c_str ());
  //We use NodeList/2 because internally we have 7 nodes in total (3 left nodes, 2 routers nodes and 3 right nodes). Node number 2 == client_nodes[0]
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTO", MakeCallback (&RtoTracer20));
}

static void
TraceBytesInFlight2_0 (std::string &in_flight_file_name)
{
  AsciiTraceHelper ascii;
  inFlightStream20 = ascii.CreateFileStream (in_flight_file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/BytesInFlight", MakeCallback (&InFlightTracer20));
}

//----------------------------------------------------------------------------------------------------

//Statement for doxygen documentation
uint32_t CLIENT_NODES_COUNT = 3;
uint32_t SERVER_NODES_COUNT = 3;
uint32_t port = 3000;

StringValue TCP_SOCKET ("ns3::TcpSocketFactory");

PointToPointHelper CreatePointToPointHelper(const std::string& queue, const std::string& dataRate, const std::string& delay, const std::string& maxSize) {
  //PointToPointHelper: Ayuda a la creación de redes punto a punto
  PointToPointHelper helper;
  //Tenemos que determinar el dataRate y el Delay para ver cual corresponde 
  helper.SetDeviceAttribute("DataRate", StringValue(dataRate));
  helper.SetChannelAttribute("Delay", StringValue(delay));
  /*Each point to point net device must have a queue to pass packets through. This method allows one to set the type
    of the queue that is automatically created when the device is created and attached to a node.*/
  helper.SetQueue(queue, "MaxSize", StringValue(maxSize));
  return helper;
}

PointToPointDumbbellHelper  SetupDumbbellTopology(uint32_t CLIENT_NODES_COUNT, uint32_t SERVER_NODES_COUNT, uint32_t port, PointToPointHelper &dumbbellHelper, PointToPointHelper &bottleneckHelper, InternetStackHelper &stack ) {

  //Create a PointToPointDumbbellHelper in order to easily create dumbbell topologies using p2p links
  /*Parameters
    nLeftLeaf number of left side leaf nodes in the dumbbell.
    leftHelper PointToPoint used to install the links between the left nodes and the left-most router.
    nRightLeaf PointToPointHelper used to install the links between the right leaf nodes and the right-most router.
    bottleneckHelper PointToPointHelper used to install the link between the inner-routers, usually known as the bottleneck link.
  */
  PointToPointDumbbellHelper dumbbell(CLIENT_NODES_COUNT, dumbbellHelper, SERVER_NODES_COUNT, dumbbellHelper, bottleneckHelper);

  dumbbell.InstallStack(stack);

  /*Ipv4AddressHelper: This class is a very simple IPv4 address generator. You can think of it as a simple local
    number incremeter. 
  */
  Ipv4AddressHelper leftIP("10.1.1.0", "255.255.255.0");
  Ipv4AddressHelper rightIP("10.2.1.0", "255.255.255.0");
  Ipv4AddressHelper routersIP("10.3.1.0", "255.255.255.0");
  
  /*AssignIpv4Addresses: 
    leftIP Ipv4AddressHelper to assign Ipv4 addresses to the interfaces on the left side of the dumbbell
    rightIP Ipv4AddressHelper to assign Ipv4 addresses to the interfaces on the right sied of the dumbell
    routerIP Ipv4AddressHelper to assign Ipv4 addresses tot he interfaces ont he bottleneck link
  */
  dumbbell.AssignIpv4Addresses(leftIP, rightIP, routersIP);

  /*It's facilitates the automatic setup of global routing for network simulations using IPv4. It automates the installation
  of routing tables on nodes within the simulation, allowing efficient and effective traffic management in complex 
  simulated enviroments.
  */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  return dumbbell;
}

void ConfigureApplicationDuration(ApplicationContainer &apps, Time startTime, Time stopTime) {
  //Arrange for all of the Applications in this container to Start() at the Time given as a parameter. 
  apps.Start(startTime);
  //Arrange for all of the Applications in this container to Stop() at the Time given as a parameter. 
  apps.Stop(stopTime);
}

void InstallApplications(uint32_t sourceNode, uint32_t destinationNode, uint32_t port, ApplicationContainer &clientApps, ApplicationContainer &serverApps, PointToPointDumbbellHelper &dumbbell, StringValue socket) {
  //A helper to make it easier to instantiate an OnOffApplication on a set o nodes. 
  OnOffHelper clientHelper(socket.Get(), Address());
  clientHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1000.0]"));
  clientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  //Final transport node with ipv4 and the port
  AddressValue remoteAddress(InetSocketAddress(dumbbell.GetRightIpv4Address(destinationNode), port));
  clientHelper.SetAttribute("Remote", remoteAddress);
  //We add to our APP cliente container, the new client with the origen node pointer
  clientApps.Add(clientHelper.Install(dumbbell.GetLeft(sourceNode)));
  //Creates the server/receptor app with final node and port.
  PacketSinkHelper server(socket.Get(), InetSocketAddress(dumbbell.GetRightIpv4Address(destinationNode), port));
  //Add to my app server conteiner the new server with the pointer and the origen node. (In this case= to the final node)
  serverApps.Add(server.Install(dumbbell.GetRight(destinationNode)));
}

int main (int argc, char *argv[]){

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
 
  uint32_t NODE_0 = 0;
  uint32_t NODE_1 = 1;
  uint32_t NODE_2 = 2;
  
  //Stack an InternetStackHelper which is used to install on every node in the dumbbell. 
  InternetStackHelper stack;
  
  PointToPointHelper dumbbellHelper = CreatePointToPointHelper("ns3::DropTailQueue", "100kb/s", "100ms", "10p");
  PointToPointHelper bottleneckHelper = CreatePointToPointHelper("ns3::DropTailQueue", "100kb/s", "100ms", "10p");   //PtPHelper used to install the link bw the inner-routers
  
  PointToPointDumbbellHelper dumbbell =  SetupDumbbellTopology(CLIENT_NODES_COUNT, SERVER_NODES_COUNT, port, dumbbellHelper, bottleneckHelper, stack);  

  //ONOFFAPPLICATION  nodo 0 --> nodo 1
  InstallApplications(NODE_0, NODE_1, port, clientApps, serverApps, dumbbell, TCP_SOCKET);
  //ONOFFAPPLICATION 2 nodo 1--> nodo 2
  InstallApplications(NODE_1, NODE_2, port, clientApps, serverApps, dumbbell, TCP_SOCKET);

  ConfigureApplicationDuration(clientApps, Seconds(0.0), Seconds(30.0));
  ConfigureApplicationDuration(serverApps, Seconds(0.0), Seconds(100.0));

  std::string prefix_file_name = "dumbbell-tp2";

  dumbbellHelper.EnablePcapAll ("dumbbell-tp2", false);

  std::ofstream ascii;
  Ptr<OutputStreamWrapper> ascii_wrap;
  ascii.open ((prefix_file_name + "-ascii").c_str ());
  ascii_wrap = new OutputStreamWrapper ((prefix_file_name + "-ascii").c_str (),
                                        std::ios::out);
  stack.EnableAsciiIpv4All (ascii_wrap);

  Simulator::Schedule (Seconds (0.001), &TraceCongestionWindow2_0, prefix_file_name + "-20-cwnd-test.data");
  Simulator::Schedule (Seconds (0.001), &TraceSSThresh2_0, prefix_file_name + "-20-ssth-test.data");
  Simulator::Schedule (Seconds (0.001), &TraceRTT2_0, prefix_file_name + "-20-rtt-test.data");
  Simulator::Schedule (Seconds (0.001), &TraceRTO2_0, prefix_file_name + "-20-rto-test.data");
  Simulator::Schedule (Seconds (0.001), &TraceBytesInFlight2_0, prefix_file_name + "-20-inflight-test.data");

  Simulator::Run ();
  Simulator::Destroy ();

}