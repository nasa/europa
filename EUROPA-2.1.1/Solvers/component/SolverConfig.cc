#include "SolverConfig.hh"

namespace EUROPA {
  namespace SOLVERS {

  bool& alreadyInitialized()
  {
	    static bool sl_alreadyInitialized = false;
      return sl_alreadyInitialized;	
  }
  
  void SolverConfig::init()
  {
  	if (alreadyInitialized()) 
  		return;
  	
      BASE_REGISTRATIONS;
      RESOURCE_REGISTRATIONS;

      alreadyInitialized() = true;
  }

  void SolverConfig::uninit()
  {
  	if (!alreadyInitialized()) 
  		return;

  	// TODO: cleanup
      alreadyInitialized() = false;
  }
  
 }
}
