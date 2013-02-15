/*! 
 *  \file main.cpp
 *  \brief    The sc_main
 *  \details Start point of the simulation.
 *  \author    Antti Siirilä, 501449
 *  \version   1.0
 *  \date      28.1.2013
 */


#include "Simulation.hpp"



//!Defines the file name for the VCD output.
//#define VCD_FILE_NAME "anjosi_ex3_vcd"


using namespace std;
using namespace sc_core;
using namespace sc_dt;

#define SIMULATION_DURATION 200

/*!
 * \brief sc_main
 * \details Initiates the Simulation module, which builds up the Router modules and starts the simulation.
 */
int sc_main(int argc, char * argv [])
{




    /*!
     * \property   const sc_time *clk_Periods
     * \brief 
     * \details 
     * \private
     */
    const sc_time m_ClkPeriod(10, SC_NS);


    /*!
     * \property sc_clock *clk_Router
     * \brief Pointer to sc_clock
     * \details  
     * \private
     */
    sc_clock m_Clk("CLK", m_ClkPeriod);


  ///initiate the simulation
  Simulation test("Test");
  test.port_Clk(m_Clk);
  cout << "Simulation starts for " << SIMULATION_DURATION << " s" << endl; 
  ///run the simulation	
  sc_start(SIMULATION_DURATION, SC_SEC);

return 0;
}//end of main
