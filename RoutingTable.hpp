/*! \file  RoutingTable.hpp
 *  \brief     Header file of RoutingTable module
 *  \details
 *  \author    Antti Siirilä, 501449
 *  \version   1.0
 *  \date      Tue Feb 19 09:27:44 2013
 */

/*!
 * \class RoutingTable
 * \brief RoutingTable module
 *  \details Routing Table contains BGP routes and offers methods
 * to manage, and access them
 */


#include "systemc"
#include "RoutingTable_If.hpp"
#include "BGPMessage.hpp"
#include "Configuration.hpp"
#include "BGPSession_If.hpp"
#include "Output_If.hpp"
#include "StringTools.hpp"

using namespace std;
using namespace sc_core;
using namespace sc_dt;


#ifndef _ROUTINGTABLE_H_
#define _ROUTINGTABLE_H_

#define AS_EMPTY "#"

struct struct_Route
{
    int id;
    string prefix;
    int mask;
    string ASes;
    int OutputPort;
    struct_Route * next;
};

class RoutingTable: public sc_module, public RoutingTable_If, public Output_If<BGPMessage>
{

public:


    /*! \brief System clock signal
     * \details The router's internal clock
     * \public
     */
    sc_in_clk port_Clk;


    /*! \brief Control port
     * \details Routing table can check through this port whether the
     * network interfaces are up or not.
     * \public
     */
    sc_port<BGPSession_If, 0, SC_ZERO_OR_MORE_BOUND> port_Session;

    /*! \brief Output port for BGP messages
     * \details The RoutingTable writes all the BGP messages to be send
     * to its neighbors into
     * this port. The port should be bind to the Data Plane's.
     * receiving FIFO
     * \public
     */
    sc_port<Output_If> port_Output;


    //    void before_end_of_elaboration()
    //{
    //}



    /*! \brief Elaborates the RoutingTable module
     * \details
     * \public
     */
    RoutingTable(sc_module_name p_ModuleName, ControlPlaneConfig * const p_RTConfig);



    /*! \brief Destructor of the RoutingTable module
     * \details Free's all the dynamically allocated memory
     * \public
     */
    ~RoutingTable();



    /*! \brief The main process of Control Plane module
     * \details \li Reads BGP messages from the m_ReceivingBuffer. \li
     * performs the route resolution process accoriding to BGP protocol.
     * \li Generates the required update messages. \li Keeps track on
     * different BGP sessions.
     * \public
     */
    void routingTableMain(void);

    /*! \brief
     * \details
     * \public
     */
    virtual int resolveRoute(string  p_IPAddress);



    /*! \brief Indicate the systemC producer that this module has a process.
     * \sa http://www.iro.umontreal.ca/~lablasso/docs/SystemC2.0.1/html/classproducer.html
     * \public
     */
    SC_HAS_PROCESS(RoutingTable);

    bool addRouteToRawTable(string p_msg, int p_outputPort);
    //void setMED(int p_routeId,)

    // Give preferred AS and some preference value to it.
    void setLocalPreference(int p_AS, int p_preferenceValue);

    // Remove some as from the list of preferred ASes
    void removeLocalPref(int p_AS);

    // Delete route from the RawRoutingTable. Parameters are router IDs.
    void deleteRoute(int p_router1, int p_router2);




    // Return MainRoutingTable
    string getRoutingTable();

    // Return RawRoutingTable
    string getRawRoutingTable();

    // Remove all the routes from raw routing table and update maintable as well after that
    void clearRoutingTables();

    virtual bool write(BGPMessage& p_BGPMsg);


    void killRoutingTable(void);

    void reviveRoutingTable(void);

    void setUp(bool p_Value);

    bool isRunning(void);







private:

    sc_mutex m_ReceivingBufferMutex;

    /*! \brief Receiving buffer
     * \details Data plain writes all the received BGP messages into
     * this fifo
     * \private
     */
    sc_fifo<BGPMessage> m_ReceivingBuffer;

    void addLocalRoute(void);

    // Advertise this route to peers

    void advertiseRoute(struct_Route * p_route, int p_Outputport);

//=======
//    void advertiseRoute(struct_Route * p_route, int p_outPutIf);
//>>>>>>> 4ee3ed4b07096339af037a9f7c39dbd08e896861

    // Construct new route from p_msg.
    bool createRoute(string p_msg,int p_outputPort, struct_Route * p_route);

    // Handle NOTIFICATION message type
    void handleNotification (BGPMessage NOTIFICATION_message);

    // Return struct_Route which prefix is p_prefix
    struct_Route * findRoute(string p_prefix);

    // Return how many bits are the same from p_route and p_IP. Used for deciding which route to use
    int matchLength(struct_Route * p_route, string p_IP);

    // Update MainRoutingTable. Iterate through the RawRoutingTable and pick the preferred routes from there to MainRoutingTable
    void updateRoutingTable();

    // Take two routes as a parameter and add the more preferred one to MainRoutingTable
    void addPreferredRoute(struct_Route p_route1, struct_Route p_route2);

    // Add p_route to MainRoutingTable
    void setRoute(struct_Route p_route);

    // Remove route from RawRoutingTable
    void removeFromRawTable(int p_routeId);

    // Remove route from MainRoutingTable
    void removeFromRoutingTable(int p_routeId);

    // Return true if p_route1 and p_route2 has the same prefix&mask combination
    bool sameRoutes(struct_Route p_route1, struct_Route p_route2);

    // Return the ASPathLength of p_route
    int ASpathLength(struct_Route p_route);

    // Convert p_route to string. Syntax: ID,Prefix,Mask,Routers,ASes (e.g. 5,100100200050,8,2-4-6-7,100-4212-231-22)
    string routeToString(struct_Route p_route);

    // Convert the hole RoutingTable to string. Start from p_route
    string routingTableToString(struct_Route * p_route);

    // Delete routes from RawRoutingTable with given output port
    void deleteRoutes(int p_outputPort);

    // Send withdraw-message to all peers
    void sendWithdraw(struct_Route p_route, int p_OutputPort);

    // Handle received withdraw message
    void handleWithdraw(string p_message);

    // Return the length of the table
    int tableLength();

    // Advertise the whole RawRoutingTable to one peer
    void advertiseRawRoutingTable(int p_outputPort);


    // Just for testing?
    void printRoutingTable();
    void printRawRoutingTable();
    void printOneRoute(struct_Route p_route);
    void fillRoutingTable();

    ControlPlaneConfig *m_RTConfig;

    /*! \brief BGP message
     * \details
     * \private
     */
    BGPMessage m_BGPMsg;

    // Pointer to the heads and ends of the tables
    struct_Route * m_headOfRawTable;
    struct_Route * m_endOfRawTable;
    struct_Route * m_iterator;

    struct_Route * m_headOfRoutingTable;
    struct_Route * m_endOfRoutingTable;

    // This is used to save the previous state of each session. 1 - UP , 0 - DOWN
    vector<int> m_sessions;



    // Preferred ASes and their preference values are stored in here
    // Syntax: [AS][VALUE][AS][VALUE][AS][VALUE]... so even number contain the as and odd numbers their values.
    vector<int> preferredASes;


    // //TODO Find out what parameters we need for this function.
    // /*! \brief Set new route to the Routing Table
    //  * \details
    //  * \public
    //  */
    // bool setRoute(void);

    // //TODO
    // /*! \brief Set new route to the Routing Table
    //  * \details
    //  * \public
    //  */
    // bool updateRoute(void);

    bool m_NewInputMsg;

    BGPMessage m_PreviousInput;

    BGPMessage m_UpdateOut;

    sc_time limit;

    StringTools m_Reporter;

    string m_AS;

    bool m_Up;

    sc_mutex m_UpMutex;

    BGPMessage m_Previous;


};



#endif /* _ROUTINGTABLE_H_ */
