#ifndef _H_PSResources
#define _H_PSResources

#include "PSEngineImpl.hh"
#include "SAVH_ResourceDefs.hh"

namespace EUROPA {

  class PSResource;
  
  class PSEngineWithResources : public PSEngineImpl 
  {
    public:
        PSList<PSResource*> getResourcesByType(const std::string& objectType);
        PSResource* getResourceByKey(PSEntityKey id);
        
    protected:
		virtual void registerObjectWrappers();	  
  };
}

#endif
