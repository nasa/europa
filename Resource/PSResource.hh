#ifndef _H_PSResource
#define _H_PSResource

#include "PSUtils.hh"
#include "PSPlanDatabase.hh"

namespace EUROPA {

  typedef int TimePoint;

  class PSResourceProfile;
  
  class PSResource : public virtual PSObject
  {
    public:
      PSResource(const EntityId& id) : PSObject(id) {}	
  	  virtual ~PSResource() {}
  	  
      virtual PSResourceProfile* getLimits() = 0;
      virtual PSResourceProfile* getLevels() = 0;        	  
    
      virtual PSList<PSEntityKey> getOrderingChoices(TimePoint t) = 0;    
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
