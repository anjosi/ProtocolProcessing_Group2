/*! \file Simulation.cpp 
 *  \brief     Implementation of Simulation module.
 *  \details   
 *  \author    Antti Siirilä, 501449
 *  \version   1.0
 *  \date      28.1.2013
 */


#include "Simulation.hpp"


Simulation::Simulation(sc_module_name p_ModuleName):sc_module(p_ModuleName)
{


  

    ///Constructor briefly: 


    m_BGPSessionParam.m_HoldDownTime = 180;
    m_BGPSessionParam.m_KeepaliveFraction = 3;


    /// \li Allocate Router pointer array
    m_Router = new Router*[ROUTER_COUNT];

    /// \li Set the base name for the router modules
    m_Name = "Router_";



    /// \li Initiate the Router modules as m_Router
    for(int i = 0; i < ROUTER_COUNT; i++)
        {
            cout << "Building " << appendName(m_Name, i) << endl;
            /// \li Generate the routers
            m_Router[i] = new Router(appendName(m_Name, i), INTERFACE_COUNT, m_BGPSessionParam);
            cout << appendName(m_Name, i) << " built." << endl;
        }
  

    ///Build the network TO-DO: clean up the binding process -> make a connection method in the router

    ///connect the 0-interfaces of router 0 and router 1
    m_Router[0]->port_ForwardingInterface[0]->bind(*(m_Router[1]->export_ReceivingInterface[0]));

    m_Router[1]->port_ForwardingInterface[0]->bind(*(m_Router[0]->export_ReceivingInterface[0]));

    ///set the those interfaces up
    m_Router[0]->interfaceUp(0);
    m_Router[1]->interfaceUp(0);


 
    ///build a ring if there is more than two routers in the simulation
    if(ROUTER_COUNT > 2)
        {
            cout << "More than two routers." << endl;

            ///connect each router to the next one
            for(int i = 1; i < ROUTER_COUNT-1; i++)
                {

                    m_Router[i]->port_ForwardingInterface[1]->bind(*(m_Router[i+1]->export_ReceivingInterface[0]));
                    m_Router[i+1]->port_ForwardingInterface[0]->bind(*(m_Router[i]->export_ReceivingInterface[1]));
                    m_Router[i]->interfaceUp(1);
                    m_Router[i+1]->interfaceUp(0);

                }

            ///close the ring by connecting the last router to the first
            m_Router[ROUTER_COUNT-1]->port_ForwardingInterface[1]->bind(*(m_Router[0]->export_ReceivingInterface[1]));
            m_Router[0]->port_ForwardingInterface[1]->bind(*(m_Router[ROUTER_COUNT-1]->export_ReceivingInterface[1]));

            m_Router[0]->interfaceUp(1);
            m_Router[ROUTER_COUNT-1]->interfaceUp(1);

        }

    SC_THREAD(socketMain);
    sensitive << port_Clk.pos();

}

Simulation::~Simulation()
{

  /// \li Free all the memory
  for(int i = 0; i < ROUTER_COUNT; i++)
    delete m_Router[i];

  delete m_Router;
}

const char* Simulation::appendName(string p_Name, int p)
{
  stringstream ss;
  ss << p;
  p_Name += ss.str();
  return p_Name.c_str();
}

void Simulation::Die(char *mess) { perror(mess); exit(1); }


void Simulation::HandleClient(int sock) {
            char buffer[BUFFSIZE];
            int received = -1;
            /* Receive message */
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
              Die("Failed to receive initial bytes from client");
            }
            /* Send bytes and check for more incoming data in loop */
            while (received > 0) {
              /* Send back received data */
              if (send(sock, buffer, received, 0) != received) {
                Die("Failed to send bytes to client");
              }
              /* Check for more data */
              if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                Die("Failed to receive additional bytes from client");
              }
            }
            close(sock);
          }


void Simulation::socketMain(void)
{
    while(true)
        {
            wait();
          cout << name() << " starts at " << sc_time_stamp();

           int serversock, clientsock;
            struct sockaddr_in echoserver, echoclient;
            /* Create the TCP socket */


            if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
              Die("Failed to create socket");
            }
            /* Construct the server sockaddr_in structure */
            memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
            echoserver.sin_family = AF_INET;                  /* Internet/IP */
            echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
          cout << name() << " binds the socket at " << sc_time_stamp();

            echoserver.sin_port = htons(3333);       /* server port */
          /* Bind the server socket */
          if (bind(serversock, (struct sockaddr *) &echoserver,
                                       sizeof(echoserver)) < 0) {
            Die("Failed to bind the server socket");
          }
          cout << name() << " starts to listen the socket at " << sc_time_stamp();
          /* Listen on the server socket */
          if (listen(serversock, MAXPENDING) < 0) {
            Die("Failed to listen on server socket");
          }

            /* Run until cancelled */
            
              unsigned int clientlen = sizeof(echoclient);
              /* Wait for client connection */
              if ((clientsock =
                   accept(serversock, (struct sockaddr *) &echoclient,
                          &clientlen)) < 0) {
                Die("Failed to accept client connection");
              }
              fprintf(stdout, "Client connected: %s\n",
                              inet_ntoa(echoclient.sin_addr));
              HandleClient(clientsock);
            
          }
 


        
}
