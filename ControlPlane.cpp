<<<<<<< HEAD
/*! \file ControlPlane.cpp
 *  \brief     Implementation of ControlPlane.
 *  \details
 *  \author    Antti Siiril�, 501449
 *  \version   1.0
 *  \date      Tue Feb 12 12:32:09 2013
 */


#include "ControlPlane.hpp"
#include "ReportGlobals.hpp"

ControlPlane::ControlPlane(sc_module_name p_ModName, int p_Sessions, BGPSessionParameters p_BGPParameters):sc_module(p_ModName), m_Name("BGP_Session")
{

  //make the inner bindings
    export_ToControlPlane(m_ReceivingBuffer); //export the receiving
                                             //buffer's input
                                             //interface for the data plane


    //set the session count
    m_SessionCount = p_Sessions;

    //initiate the BGPSession pointer arrays
    m_BGPSessions = new BGPSession*[m_SessionCount];

    //inititate the sessions
    for (int i = 0; i < m_SessionCount; ++i)
        {
            //create a session
            m_BGPSessions[i] = new BGPSession(m_Name.getNextName(), p_BGPParameters);
        }

    SC_THREAD(controlPlaneMain);
    sensitive << port_Clk.pos();
}

ControlPlane::~ControlPlane()
{

    for (int i = 0; i < m_SessionCount; ++i)
        delete m_BGPSessions[i];
    delete m_BGPSessions;
   delete l_Temp;
}


void ControlPlane::controlPlaneMain(void)
{
    StringTools *l_Temp = new StringTools(name());
    SC_REPORT_INFO(g_ReportID,l_Temp->newReportString("starting"));




  //The main thread of the control plane starts
    while(true)
    {
        wait();


        //Check if there's messages in the input buffer
      if(m_ReceivingBuffer.num_available() == 0)     // IIRO testi, if lause vaihdettu ">" --> "=="
          {

              //m_ReceivingBuffer.read(m_BGPMsg);    IIRO kommentoin pois koska lukeminen ei futaa viel
              m_BGPMsg.m_Type = UPDATE;

              // IIRO testailuu - message structure: prefix;mask;ASes
              // TODO from where is OutputPort coming? vs. create Route class and pass them? NOT bcoz BGPmessages differ so much
              m_BGPMsg.m_Message = "100.200.85;8;1-2-3-4-5";


              //check whether the session is valid
              if (m_BGPSessions[m_BGPMsg.m_OutboundInterface]->isThisSession(m_BGPMsg.m_BGPIdentifier))
                  {
                      // determine which type of message this is
                      switch(m_BGPMsg.m_Type)
                      {
                          case KEEPALIVE:
                              // KEEPALIVE received, reset KeepAlive timer for this session
                              // TODO is OutboudInterface correct way to identify the session's index?
                              m_BGPSessions[m_BGPMsg.m_OutboundInterface]->resetHoldDown();
                              break;
                          case UPDATE:
                              // Just forward to routingtable?
                              port_ToRoutingTable->write(m_BGPMsg);

                              break;
                          case NOTIFICATION:
                              // Forward to routingtable, ...
                              break;
                          case OPEN:
                              // Session is already open, what to do if OPEN-message is received?
                              break;
                      }

                  }

              //if the session was not valid but this is an OPEN message
              else if (m_BGPMsg.m_Type == OPEN)
                  {
                      //start new session for the session index
                      //corresponding the interface index to which the
                      //peer is connected
                      m_BGPSessions[m_BGPMsg.m_OutboundInterface]->setPeerIdentifier(m_BGPMsg.m_BGPIdentifier);

                      //start the session
                      m_BGPSessions[m_BGPMsg.m_OutboundInterface]->sessionStart();

                  }
              //Ohterwise
              else
                  {
                      //drop

                  }
          }

              //To send a message to data plane
              port_ToDataPlane->write(m_BGPMsg);

              m_BGPMsg.m_OutboundInterface = 5;
              //port_ToRoutingTable->write(m_BGPMsg);

              SC_REPORT_INFO(g_DebugID, l_Temp->newReportString("wrote to RT"));
               //Handle the message here





    }





 
}

=======
/*! \file ControlPlane.cpp
 *  \brief     Implementation of ControlPlane.
 *  \details
 *  \author    Antti Siiril�, 501449
 *  \version   1.0
 *  \date      Tue Feb 12 12:32:09 2013
 */


#include "ControlPlane.hpp"
#include "ReportGlobals.hpp"


ControlPlane::ControlPlane(sc_module_name p_ModName, ControlPlaneConfig * const p_BGPConfig):sc_module(p_ModName),m_BGPConfig(p_BGPConfig), m_Name("BGP_Session")
{

  //make the inner bindings
    export_ToControlPlane(m_ReceivingBuffer); //export the receiving
                                             //buffer's input
                                             //interface for the data plane



    //initiate the BGPSession pointer arrays
    m_BGPSessions = new BGPSession*[m_BGPConfig->getNumberOfInterfaces()];
    //allocate reference array for receiving exports
    export_Session = new sc_export<BGPSession_If>*[m_BGPConfig->getNumberOfInterfaces()];

    //inititate the sessions
    for (int i = 0; i < m_BGPConfig->getNumberOfInterfaces()-1; i++)
        {
            //create a session
            m_BGPSessions[i] = new BGPSession(m_Name.getNextName(), m_BGPConfig);
            //Export the session interface for RT
            export_Session[i] = new sc_export<BGPSession_If>;
            export_Session[i]->bind(*m_BGPSessions[i]);
        }

    SC_THREAD(controlPlaneMain);
    sensitive << port_Clk.pos();
}

ControlPlane::~ControlPlane()
{

    for (int i = 0; i < m_BGPConfig->getNumberOfInterfaces()-1; i++)
        delete m_BGPSessions[i];
    delete m_BGPSessions;
}


void ControlPlane::controlPlaneMain(void)
{
    m_Name.setBaseName(name());

    StringTools *l_Temp = new StringTools(name());
    SC_REPORT_INFO(g_ReportID,l_Temp->newReportString("starting"));

    int count = 0;
    ///build and send the OPEN messages to all local interfaces except
    ///the last one. The last local interface is the AS interface.
    for (int i = 0; i < m_BGPConfig->getNumberOfInterfaces()-1; i++)
        {
            m_BGPMsg.m_BGPIdentifier = m_BGPConfig->getIPAsString();
            m_BGPMsg.m_AS = m_BGPConfig->getASNumber();
            m_BGPMsg.m_Type = OPEN;
            m_BGPMsg.m_OutboundInterface = i;
            port_ToDataPlane->write(m_BGPMsg);
        }


  //The main thread of the control plane starts
    while(true)
        {    
        wait();
        // cout << "MED is now: " << m_BGPConfig->getMED() << endl;

        //Check if there's messages in the input buffer
      if(m_ReceivingBuffer.num_available() > 0)     //Antti: laitto
                                                    //takas >, kun
                                                    //simu hetti
                                                    //segmentation faultia19.3.2013. IIRO testi, if lause vaihdettu ">" --> "=="
          {

              m_ReceivingBuffer.read(m_BGPMsg);  


              // IIRO testailuu - message structure: prefix;mask;ASes
              // TODO from where is OutputPort coming? vs. create Route class and pass them? NOT bcoz BGPmessages differ so much
              m_BGPMsg.m_Message = "100.200.85;8;1-2-3-4-5";


              //check whether the session is valid
              if (m_BGPSessions[m_BGPMsg.m_OutboundInterface]->isSessionValid())
                  {

                      // determine which type of message this is
                      switch(m_BGPMsg.m_Type)
                          {
                          case KEEPALIVE:
                              // KEEPALIVE received, reset KeepAlive timer for this session
                              // TODO is OutboudInterface correct way to identify the session's index?
                              SC_REPORT_INFO(g_DebugCPID,m_Name.newReportString("KEEPALIVE received"));
                              m_BGPSessions[m_BGPMsg.m_OutboundInterface]->resetHoldDown();
                              m_BGPMsg.m_Type = -1;
                              break;
                          case UPDATE:
                              // Just forward to routingtable?
                              // port_ToRoutingTable->write(m_BGPMsg);

                              break;
                          case NOTIFICATION:
                              // Forward to routingtable, ...
                              break;
                          case OPEN:
                              // Session is already open, what to do
                              // if OPEN-message is received? Antti: The session is closed and
                              // the message is dropped. 
                              break;
                          default:
                              SC_REPORT_INFO(g_DebugCPID,m_Name.newReportString("unknown received"));
                              break;
                          }

                  }

              //if the session was not valid but this is an OPEN message
              else if (m_BGPMsg.m_Type == OPEN)
                  {
                      //start new session for the session index
                      //corresponding the interface index to which the
                      //peer is connected
                      m_BGPSessions[m_BGPMsg.m_OutboundInterface]->setPeerIdentifier(m_BGPMsg.m_BGPIdentifier);
                      m_BGPSessions[m_BGPMsg.m_OutboundInterface]->setPeeringInterface(m_BGPMsg.m_OutboundInterface);
                      m_BGPSessions[m_BGPMsg.m_OutboundInterface]->setPeerAS(m_BGPMsg.m_AS);
                      //start the session
                      m_BGPSessions[m_BGPMsg.m_OutboundInterface]->sessionStart();

                  }
              //Ohterwise
              else
                  {
                      //drop

                  }
          }

              //To send a message to data plane
              // write(m_BGPMsg);


              port_ToRoutingTable->write(m_BGPMsg);

              if(!(count%20))
                  {
                      l_Temp->newReportString("wrote to RT: ");
                      SC_REPORT_INFO(g_DebugID, l_Temp->appendReportString(m_BGPMsg.m_OutboundInterface));

                  }               //Handle the message here
              count++;
    }

    delete l_Temp;
}

bool ControlPlane::write(BGPMessage p_BGPMsg)
{

    //enter to the critical region
    mutex_Write.lock();
    //reset the corresponding keepalive timer
    m_BGPSessions[p_BGPMsg.m_OutboundInterface]->resetKeepalive();
    //write message to the DataPlane
    port_ToDataPlane->write(p_BGPMsg);
    //exit from the critical region
    mutex_Write.unlock();
    return true;

}
>>>>>>> 50d3166c9cf35b2d5452ae8437284bff21129d6a
