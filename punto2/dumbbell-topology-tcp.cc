//Imports
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-dumbbell.h"

#include <string>

//Namespace declaration
using namespace ns3;

//Statement for doxygen documentation
uint32_t CLIENT_NODES_COUNT = 3;
uint32_t SERVER_NODES_COUNT = 3;
uint32_t port = 0;

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
}