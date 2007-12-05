#ifndef _H_PSResource
#define _H_PSResource

#include "Engine.hh"
#include "PSUtils.hh"
#include "PSPlanDatabase.hh"

namespace EUROPA {

  typedef int TimePoint;

  class PSResource;
  class PSResourceProfile;
  
  class PSResourceManager : public EngineComponent
  {
    public:
      virtual ~PSResourceManager() {}

      virtual PSResource* getResourceByKey(PSEntityKey id) = 0;	              
      virtual PSList<PSResource*> getResourcesByType(const std::string& objectType) = 0;
  };
  
  class PSResource : public virtual PSObject
  {
    public:
      PSResource(const EntityId& id) : PSObject(id) {}	
  	  virtual ~PSResource() {}
  	  
      virtual PSResourceProfile* getLimits() = 0;
      virtual PSResourceProfile* getLevels() = 0;        	  
    
      virtual PSList<PSEntityKey> getOrderingChoices(TimePoint t) = 0;    
      
      static PSResource* asPSResource(PSObject* obj);
  };   
    
  class PSResourceProfile
  {
    public:
      virtual ~PSResourceProfile() {}
      
      virtual const PSList<TimePoint>& getTimes() = 0;
      virtual double getLowerBound(TimePoint time) = 0;
      virtual double getUpperBound(TimePoint time) = 0;
  };
}

#endif
