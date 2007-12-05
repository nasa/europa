#ifndef _H_PSResourceImpl
#define _H_PSResourceImpl

#include "PSResource.hh"
#include "PSPlanDatabaseImpl.hh"
#include "SAVH_ResourceDefs.hh"

namespace EUROPA {

  class PSResourceImpl : public PSResource, PSObjectImpl
  {
  public:
    PSResourceImpl(const SAVH::ResourceId& res);
    PSResourceProfile* getLimits();
    PSResourceProfile* getLevels();        	  
    
    PSList<PSEntityKey> getOrderingChoices(TimePoint t);
    
  protected:
    SAVH::ResourceId m_res;
  };   

  class PSResourceProfileImpl : public PSResourceProfile
  {
  public:
    const PSList<TimePoint>& getTimes();
    double getLowerBound(TimePoint time);
    double getUpperBound(TimePoint time);
  protected:
    friend class PSResourceImpl;
    PSResourceProfileImpl(const double lb, const double ub);
    PSResourceProfileImpl(const SAVH::ProfileId& profile);
  private:
    bool m_isConst;
    double m_lb, m_ub;
    PSList<TimePoint> m_times;
    SAVH::ProfileId m_profile;
  };  
}

#endif
