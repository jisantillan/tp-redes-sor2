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


int main (int argc, char *argv[]){
    uint32_t cantNodosServidor = 3;
    uint32_t cantNodosCliente = 3;

    uint32_t port= 0;
    
    //PointToPointHelper: Ayuda a la creación de redes punto a punto
    PointToPointHelper  firstDumbbell;

    //Tenemos que determinar el dataRate y el Delay para ver cual corresponde 
    firstDumbbell.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    firstDumbbell.SetChannelAttribute("Delay",StringValue("2ms"));
    /*Each point to point net device must have a queue to pass packets through. This method allows one to set the type
    of the queue that is automatically created when the device is created and attached to a node.*/
    firstDumbbell.SetQueue("ns3::DropTailQueue","MaxSize", StringValue("10p"));

    //PtPHelper used to install the link bw the inner-routers
    PointToPointHelper bottleneck;
    bottleneck.SetDeviceAttribute("DataRate",StringValue("5Mbps"));
    bottleneck.SetChannelAttribute("Delay",StringValue("2ms"));
    bottleneck.SetQueue("ns3::DropTailQueue","MaxSize",StringValue("10p"));

    //Create a PointToPointDumbbellHelper in order to easily create dumbbell topologies using p2p links
    /*Parameters
    nLeftLeaf number of left side leaf nodes in the dumbbell.
    leftHelper PointToPoint used to install the links between the left nodes and the left-most router.
    nRightLeaf PointToPointHelper used to install the links between the right leaf nodes and the right-most router.
    bottleneckHelper PointToPointHelper used to install the link between the inner-routers, usually known as the bottleneck link.
    */
    PointToPointDumbbellHelper dumbbell (cantNodosCliente, firstDumbbell, cantNodosServidor, firstDumbbell, bottleneck);
    //Create a new InternetStackHelper which uses a mix of static routing and flobal routing by default. IPv4
    InternetStackHelper stack;
    //Stack an InternetStackHelper which is used to install on every node in the dumbbell. 
    dumbbell.InstallStack(stack);
    
    /*Ipv4AddressHelper: This class is a very simple IPv4 address generator. You can think of it as a simple local
    number incremeter. 
   */
    Ipv4AddressHelper leftIP("10.1.1.0","255.255.255.0"); 
    Ipv4AddressHelper rightIP("10.2.1.0","255.255.255.0");
    Ipv4AddressHelper routersIP("10.3.1.0","255.255.255.0");
 
    /*AssignIpv4Addresses: 
    leftIP Ipv4AddressHelper to assign Ipv4 addresses to the interfaces on the left side of the dumbbell
    rightIP Ipv4AddressHelper to assign Ipv4 addresses to the interfaces on the right sied of the dumbell
    routerIP Ipv4AddressHelper to assign Ipv4 addresses tot he interfaces ont he bottleneck link
    */
    dumbbell.AssignIpv4Addresses(leftIP,rightIP,routersIP);
    
    /*It's facilitates the automatic setup of global routing for network simulations using IPv4. It automates the installation
    of routing tables on nodes within the simulation, allowing efficient and effective traffic management in complex 
    simulated enviroments.
    */
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    ApplicationContainer appCliente;
    ApplicationContainer appServidor;

    uint32_t nodo0=0;
    uint32_t nodo1=0;
    uint32_t nodo2=0;

    //ONOFFAPPLICATION  nodo 0 --> nodo 1
    //A helper to make it easier to instantiate an OnOffApplication on a set o nodes. 
    OnOffHelper clienteHelper("ns3::TcpSocketFactory",Address());
    clienteHelper.SetAttribute("OnTime",StringValue("ns3::ConstantRandomVariable[Constant=1000.0]"));
    clienteHelper.SetAttribute("OffTime",StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    //Final transport node with ipv4 and the port
    AddressValue remoteAddress(InetSocketAddress(dumbbell.GetRightIpv4Address(nodo1),port));
    clienteHelper.SetAttribute("Remote",remoteAddress);

    //We add to our APP cliente container, the new client with the origen node pointer
    appCliente.Add(clienteHelper.Install(dumbbell.GetLeft(nodo0)));
    
    //Creates the server/receptor app with final node and port.
    PacketSinkHelper servidor("ns3::TcpSocketFactory",InetSocketAddress(dumbbell.GetRightIpv4Address(nodo1),port));

    //Add to my app server conteiner the new server with the pointer and the origen node. (In this case= to the final node)
    appServidor.Add(servidor.Install(dumbbell.GetRight(nodo1)));

    
    //ONOFFAPPLICATION 2 nodo 1--> nodo 2
    //A helper to make it easier to instantiate an OnOffApplication on a set o nodes. 
    OnOffHelper clienteHelper2("ns3::TcpSocketFactory",Address());
    clienteHelper2.SetAttribute("OnTime",StringValue("ns3::ConstantRandomVariable[Constant=1000.0]"));
    clienteHelper2.SetAttribute("OffTime",StringValue("ns3::ConstantRandomVariable[Constant=0]"));

    //Final transport node with ipv4 and the port
    AddressValue remoteAddress(InetSocketAddress(dumbbell.GetRightIpv4Address(nodo2),port));
    clienteHelper2.SetAttribute("Remote",remoteAddress);

    //We add to our APP cliente container, the new client with the origen node pointer
    appCliente.Add(clienteHelper2.Install(dumbbell.GetLeft(nodo1)));
    
    //Creates the server/receptor app with final node and port.
    PacketSinkHelper servidor("ns3::TcpSocketFactory",InetSocketAddress(dumbbell.GetRightIpv4Address(nodo2),port));

    //Add to my app server conteiner the new server with the pointer and the origen node. (In this case= to the final node)
    appServidor.Add(servidor.Install(dumbbell.GetRight(nodo2)));

}