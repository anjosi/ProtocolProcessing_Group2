/*! \file  Configuration.hpp
 *  \brief    Defines the Connection, RouterConfig, and
 *  SimulationConfig classes
 *  \details   
 *  \author    Antti Siirilä, 501449
 *  \version   1.0
 *  \date      Wed Mar 20 15:30:11 2013
 */





//#include "BGPSessionParameters.hpp"


//using namespace std;

#ifndef _CONFIGURATION_H_
#define _CONFIGURATION_H_


/*!
 * \class Connection
 * \brief Holds the connection parameters for local interface
 *  \details   
 */
class Connection
{
public:

    /*! \brief The id of the neighbor interface to where this router connects
     * \public
     */
    int m_NeighborInterfaceId;

    /*! \brief The id of the neighbor router on which the neighbor
     * interface is located
     * \details
     * \public
     */
    int m_NeighborRouterId;    

};



/*!
 * \class RouterConfig
 * \brief Holds the parameters for a router including interface
 * connection parameters
 *  \details   
 */
class RouterConfig
{

public:

    inline RouterConfig(int p_NumberOfInterfaces):m_NumberOfInterfaces(p_NumberOfInterfaces)
    {
        m_NeighborConnections = new Connection*[p_NumberOfInterfaces];
    };

    inline virtual ~RouterConfig()
    {
        for (int i = 0; i < m_NumberOfInterfaces; ++i)
            delete m_NeighborConnections[i];
        delete m_NeighborConnections;
    };

    inline void addConnectionConfig( int p_LocalInterfaceId, int p_NeighborRouterId, int p_NeighborInterfaceId)
    {
        m_NeighborConnections[p_LocalInterfaceId] = new Connection;
        m_NeighborConnections[p_LocalInterfaceId]->m_NeighborRouterId = p_NeighborRouterId;
        m_NeighborConnections[p_LocalInterfaceId]->m_NeighborInterfaceId = p_NeighborInterfaceId;


    };    
    
    /*! \brief Number of network interfaces that this router should allocate
     * \details
     * \public
     */
    int m_NumberOfInterfaces;
    
    /*! \brief Linked list that holds the connection information for
     * each interface of the router
     * \details
     * 
     * \public
     */
    Connection **m_NeighborConnections;

    /*! \brief The prefix of the AS connecting this router
     * \details 
     * \public
     */
    sc_int<32> m_Prefix;

    /*! \brief The mask defined by prefix /-notation
     * \details 
     * \public
     */
    sc_int<32> m_PrefixMask;

    /*! \brief The AS number of this router
     * \details 
     * \public
     */
    int m_ASNumber;

    /*! \brief BGP MED variable
     * \details 
     * \public
     */
    int m_MED;

    /*! \brief BGP Local Preference variable
     * \details 
     * \public
     */
    int m_LocalPref;

    /*! \brief BGP session parameter for this router
     * \public
     */
    BGPSessionParameters m_BGPSessionConfig;


};

/*! \Class SimulationConfig
 *  \brief Holds all the simulation parameters received from GUI
 *  \details Used to build up the simulatin environment
 *
 */
class SimulationConfig
{

public:

    inline SimulationConfig(){};    

    inline SimulationConfig(int p_NumberOfRouters):m_NumberOfRouters(p_NumberOfRouters)
    {
        m_RouterConfiguration = new RouterConfig*[p_NumberOfRouters];

    };

    inline virtual ~SimulationConfig()
    {
        for (int i = 0; i < m_NumberOfRouters; ++i)
            {
                delete m_RouterConfiguration[i];
            }
        delete m_RouterConfiguration;
    };

    inline void addRouterConfig(int p_RouterId, int p_NumberOfInterfaces)
    {
        m_RouterConfiguration[p_RouterId] = new RouterConfig(p_NumberOfInterfaces);

    };
    
    inline void addConnectionConfig(int p_LocalRouterId, int p_LocalInterfaceId, int p_NeighborInterfaceId, int p_NeighborRouterId)
    {

        m_RouterConfiguration[p_LocalRouterId]->addConnectionConfig(p_LocalInterfaceId, p_NeighborInterfaceId, p_NeighborRouterId);
    };

    inline void addBGPSessionParameters(int p_LocalRouterId, int p_KeepaliveTime, int p_HoldDownTimeFactor)
    {

        m_RouterConfiguration[p_LocalRouterId]->m_BGPSessionConfig.m_KeepaliveTime = p_KeepaliveTime;
        m_RouterConfiguration[p_LocalRouterId]->m_BGPSessionConfig.m_HoldDownTimeFactor = p_HoldDownTimeFactor;
      
    };


    /*! \brief Number of routers that this simulation should allocate
     * \details
     * \public
     */
    int m_NumberOfRouters;

    /*! \property RouterConfig** m_RouterConfiguration
     * \brief Holds the router configurations for each router in this simulation
     * \details
     * \public
     */
    RouterConfig **m_RouterConfiguration;


};



#endif /* _CONFIGURATION_H_ */
