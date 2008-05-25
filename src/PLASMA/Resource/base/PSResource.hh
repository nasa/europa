#ifndef _H_PSResource
#define _H_PSResource

#include "PSPlanDatabase.hh"
#include "SAVH_ResourceDefs.hh"

namespace EUROPA {

  typedef int TimePoint;

  class PSResourceProfile;
  
  class PSResource : public virtual PSObject
  {
    public:
      PSResource() {}	
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
      
      virtual const PSList<TimePoint>& getTimes();
      virtual double getLowerBound(TimePoint time);
      virtual double getUpperBound(TimePoint time);
    protected:
       friend class SAVH::Resource;
       PSResourceProfile(const double lb, const double ub);
       PSResourceProfile(const SAVH::ProfileId& profile);
     private:
       bool m_isConst;
       double m_lb, m_ub;
       PSList<TimePoint> m_times;
       SAVH::ProfileId m_profile;
  };
}

#endif
