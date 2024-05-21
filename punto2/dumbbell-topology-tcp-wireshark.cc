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
#include "ns3/netanim-module.h"

#include <string>

//Namespace declaration
using namespace ns3;

static Ptr<OutputStreamWrapper> congestionWindowStream2;
static bool fCongestionWindow2 = true;
static Ptr<OutputStreamWrapper> ssThreshStream2;
static bool fSSThresh2 = true;
static Ptr<OutputStreamWrapper> rttStream2;
static bool fRTT2 = true;
static Ptr<OutputStreamWrapper> rtoStream2;
static bool fRTO2 = true;
static Ptr<OutputStreamWrapper> bytesInFlightStream2;

static Ptr<OutputStreamWrapper> congestionWindowStream3;
static bool fCongestionWindow3 = true;
static Ptr<OutputStreamWrapper> ssThreshStream3;
static bool fSSThresh3 = true;
static Ptr<OutputStreamWrapper> rttStream3;
static bool fRTT3 = true;
static Ptr<OutputStreamWrapper> rtoStream3;
static bool fRTO3 = true;
static Ptr<OutputStreamWrapper> bytesInFlightStream3;

static uint32_t congestionWindowValue2;
static uint32_t ssThreshValue2;

static uint32_t congestionWindowValue3;
static uint32_t ssThreshValue3;

std::string animFile = "my-animation.xml" ;  // Name of file for animation output

static void
saveTCPData(Ptr<OutputStreamWrapper> stream, float time, uint32_t value)
{
  *stream->GetStream () << time << " " << value << std::endl;
}

//Tracers para recopilar informacion de los cambios en los atributos de la conexión TCP del nodo 2 interfaz 0 (el primer nodo de la rama izquierda de nuestra red)
static void
CongestionWindowTracer2_0 (uint32_t oldval, uint32_t newval)
{
  //Como empezamos luego de la simulacion (0.1 segundos luego) primero registramos en el segundo 0 manualmente. Esto tambien nos sirve a la hora de plotear los graficos de los datos recopilados
  if (fCongestionWindow2)
    {
      saveTCPData(congestionWindowStream2, 0.0, oldval);
      fCongestionWindow2 = false;
    }
  else
    {
      saveTCPData(congestionWindowStream2, Simulator::Now ().GetSeconds (), newval);
    }
  congestionWindowValue2 = newval;
  //SSThresh is set according to the CongestionWindow. When the CongestionWindow changes, so does the SSThresh, and viceversa
  if (!fSSThresh2)
    {
      saveTCPData(ssThreshStream2, Simulator::Now ().GetSeconds (), ssThreshValue2);
    }
}

static void
SSThreshTracer2_0 (uint32_t oldval, uint32_t newval)
{
  if (fSSThresh2)
    {
      saveTCPData(ssThreshStream2, 0.0, oldval);
      fSSThresh2 = false;
    }
  else
    {
      saveTCPData(ssThreshStream2, Simulator::Now ().GetSeconds (), newval);
    }
  ssThreshValue2 = newval;

  //Go to line 60
  if (!fCongestionWindow2)
    {
      saveTCPData(ssThreshStream2, Simulator::Now ().GetSeconds (), congestionWindowValue2);
    }
}

static void
RTTTracer2_0 (Time oldval, Time newval)
{
  if (fRTT2)
    {
      saveTCPData(rttStream2, 0.0, oldval.GetSeconds ());
      fRTT2 = false;
    }
  else
    {
      saveTCPData(rttStream2, Simulator::Now ().GetSeconds (), newval.GetSeconds ());
    }
}

static void
RTOTracer2_0 (Time oldval, Time newval)
{
  if (fRTO2)
    {
      saveTCPData(rtoStream2, 0.0, oldval.GetSeconds ());
      fRTO2 = false;
    }
  else
    {
      saveTCPData(rtoStream2, Simulator::Now ().GetSeconds (), newval.GetSeconds ());
    }
}

static void
BytesInFlightTracer2_0 (uint32_t unused, uint32_t bytesInFlight)
{
  NS_UNUSED (unused);
  saveTCPData(bytesInFlightStream2, Simulator::Now ().GetSeconds (), bytesInFlight);
}

static void
TraceCongestionWindow2_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  congestionWindowStream2 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CongestionWindowTracer2_0));
}

static void
TraceSSThresh2_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream2 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SSThreshTracer2_0));
}

static void
TraceRTT2_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  rttStream2 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeCallback (&RTTTracer2_0));
}

static void
TraceRTO2_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  rtoStream2 = ascii.CreateFileStream (file_name.c_str ());
  //We use NodeList/2 because internally we have 7 nodes in total (3 left nodes, 2 routers nodes and 3 right nodes). Node number 2 == client_nodes[0]
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/RTO", MakeCallback (&RTOTracer2_0));
}

static void
TraceBytesInFlight2_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  bytesInFlightStream2 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/2/$ns3::TcpL4Protocol/SocketList/0/BytesInFlight", MakeCallback (&BytesInFlightTracer2_0));
}

//Tracers para recopilar informacion de los cambios en los atributos de la conexión TCP del nodo 3 interfaz 0 (el segundo nodo de la rama izquierda de nuestra red)
static void
CongestionWindowTracer3_0 (uint32_t oldval, uint32_t newval)
{
  //Como empezamos luego de la simulacion (0.1 segundos luego) primero registramos en el segundo 0 manualmente. Esto tambien nos sirve a la hora de plotear los graficos de los datos recopilados
  if (fCongestionWindow3)
    {
      saveTCPData(congestionWindowStream3, 0.0, oldval);
      fCongestionWindow3 = false;
    }
  else
    {
      saveTCPData(congestionWindowStream3, Simulator::Now ().GetSeconds (), newval);
    }
  congestionWindowValue3 = newval;
  //SSThresh is set according to the CongestionWindow. When the CongestionWindow changes, so does the SSThresh, and viceversa
  if (!fSSThresh3)
    {
      saveTCPData(ssThreshStream3, Simulator::Now ().GetSeconds (), ssThreshValue3);
    }
}

static void
SSThreshTracer3_0 (uint32_t oldval, uint32_t newval)
{
  if (fSSThresh3)
    {
      saveTCPData(ssThreshStream3, 0.0, oldval);
      fSSThresh3 = false;
    }
  else
    {
      saveTCPData(ssThreshStream3, Simulator::Now ().GetSeconds (), newval);
    }
  ssThreshValue3 = newval;

  //Go to line 60
  if (!fCongestionWindow3)
    {
      saveTCPData(ssThreshStream3, Simulator::Now ().GetSeconds (), congestionWindowValue3);
    }
}

static void
RTTTracer3_0 (Time oldval, Time newval)
{
  if (fRTT3)
    {
      saveTCPData(rttStream3, 0.0, oldval.GetSeconds ());
      fRTT3 = false;
    }
  else
    {
      saveTCPData(rttStream3, Simulator::Now ().GetSeconds (), newval.GetSeconds ());
    }
}

static void
RTOTracer3_0 (Time oldval, Time newval)
{
  if (fRTO3)
    {
      saveTCPData(rtoStream3, 0.0, oldval.GetSeconds ());
      fRTO3 = false;
    }
  else
    {
      saveTCPData(rtoStream3, Simulator::Now ().GetSeconds (), newval.GetSeconds ());
    }
}

static void
BytesInFlightTracer3_0 (uint32_t unused, uint32_t bytesInFlight)
{
  NS_UNUSED (unused);
  saveTCPData(bytesInFlightStream3, Simulator::Now ().GetSeconds (), bytesInFlight);
}

static void
TraceCongestionWindow3_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  congestionWindowStream3 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CongestionWindowTracer3_0));
}

static void
TraceSSThresh3_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  ssThreshStream3 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/0/SlowStartThreshold", MakeCallback (&SSThreshTracer3_0));
}

static void
TraceRTT3_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  rttStream3 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/0/RTT", MakeCallback (&RTTTracer3_0));
}

static void
TraceRTO3_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  rtoStream3 = ascii.CreateFileStream (file_name.c_str ());
  //We use NodeList/3 because internally we have 7 nodes in total (3 left nodes, 3 routers nodes and 3 right nodes). Node number 3 == client_nodes[1]
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/0/RTO", MakeCallback (&RTOTracer3_0));
}

static void
TraceBytesInFlight3_0 (std::string file_name)
{
  AsciiTraceHelper ascii;
  bytesInFlightStream3 = ascii.CreateFileStream (file_name.c_str ());
  Config::ConnectWithoutContext ("/NodeList/3/$ns3::TcpL4Protocol/SocketList/0/BytesInFlight", MakeCallback (&BytesInFlightTracer3_0));
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
  
  Config::SetDefault("ns3::TcpL4Protocol::SocketType",TypeIdValue(TcpNewReno::GetTypeId()));
  PointToPointHelper dumbbellHelper = CreatePointToPointHelper("ns3::DropTailQueue", "100KBps", "100ms", "10p");
  PointToPointHelper bottleneckHelper = CreatePointToPointHelper("ns3::DropTailQueue", "50KBps", "50ms", "10p");   //PtPHelper used to install the link bw the inner-routers
  
  PointToPointDumbbellHelper dumbbell =  SetupDumbbellTopology(CLIENT_NODES_COUNT, SERVER_NODES_COUNT, port, dumbbellHelper, bottleneckHelper, stack);  

  //ONOFFAPPLICATION  nodo 0 --> nodo 1
  InstallApplications(NODE_0, NODE_1, port, clientApps, serverApps, dumbbell, TCP_SOCKET);
  //ONOFFAPPLICATION 2 nodo 1--> nodo 2
  InstallApplications(NODE_1, NODE_2, port, clientApps, serverApps, dumbbell, TCP_SOCKET);

  ConfigureApplicationDuration(clientApps, Seconds(0.0), Seconds(40.0));
  ConfigureApplicationDuration(serverApps, Seconds(0.0), Seconds(100.0));

  std::string prefix_file_name = "tp_redes_";

  dumbbellHelper.EnablePcapAll ("tp_redes_", false);

  std::ofstream ascii;
  Ptr<OutputStreamWrapper> ascii_wrap;
  ascii.open ((prefix_file_name + "-ascii").c_str ());
  ascii_wrap = new OutputStreamWrapper ((prefix_file_name + "-ascii").c_str (),
                                        std::ios::out);
  stack.EnableAsciiIpv4All (ascii_wrap);

  Simulator::Schedule (Seconds (0.001), &TraceCongestionWindow2_0, prefix_file_name + "congestion_window2_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceSSThresh2_0, prefix_file_name + "ssth2_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceRTT2_0, prefix_file_name + "rtt2_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceRTO2_0, prefix_file_name + "rto2_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceBytesInFlight2_0, prefix_file_name + "bytes_inflight2_0.data");

  Simulator::Schedule (Seconds (0.001), &TraceCongestionWindow3_0, prefix_file_name + "congestion_window3_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceSSThresh3_0, prefix_file_name + "ssth3_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceRTT3_0, prefix_file_name + "rtt3_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceRTO3_0, prefix_file_name + "rto3_0.data");
  Simulator::Schedule (Seconds (0.001), &TraceBytesInFlight3_0, prefix_file_name + "bytes_inflight3_0.data");

  dumbbell.BoundingBox (1, 1, 100, 100);

  // Create the animation object and configure for specified output
  AnimationInterface anim (animFile);
  anim.EnablePacketMetadata (); // Optional

  Simulator::Run ();
  Simulator::Destroy ();

}