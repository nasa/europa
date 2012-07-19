#ifndef _H_PSResources
#define _H_PSResources

#include "PSEngine.hh"
#include "SAVH_ResourceDefs.hh"

namespace EUROPA {

  class PSEngineWithResources : public PSEngine {
  public:
    PSEngineWithResources();
    void start();
    void initDatabase();
  };

  class PSResourceProfile
  {
  public:
    const PSList<TimePoint>& getTimes();
    double getLowerBound(TimePoint time);
    double getUpperBound(TimePoint time);
  protected:
    friend class PSResource;
    PSResourceProfile(const double lb, const double ub);
    PSResourceProfile(const SAVH::ProfileId& profile);
  private:
    bool m_isConst;
    double m_lb, m_ub;
    PSList<TimePoint> m_times;
    SAVH::ProfileId m_profile;
  };


  class PSResource : public PSObject
  {
  public:
    PSResourceProfile* getLimits();
    PSResourceProfile* getLevels();        	  
  protected:
    friend class PSEngine;
    friend class PSEngineWithResources;
    friend class ResourceWrapperGenerator;
    PSResource(const SAVH::ResourceId& res);
  private:
    SAVH::ResourceId m_res;
  };   
    
}

#endif
