#ifndef _H_PSResources
#define _H_PSResources

#include "PSEngineImpl.hh"
#include "SAVH_ResourceDefs.hh"

namespace EUROPA {

  class PSResource;
  
  class PSEngineWithResources : public PSEngineImpl 
  {
    public:
        PSEngineWithResources();
    
        // TODO: remove this void initDatabase();    
        PSList<PSResource*> getResourcesByType(const std::string& objectType);
        PSResource* getResourceByKey(PSEntityKey id);
        
    protected:
		virtual void createModules();
		virtual void initializeModules();
		virtual void allocateComponents();
		virtual void registerObjectWrappers();	  
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


  class PSResource : public PSObjectImpl
  {
  public:
    PSResourceProfile* getLimits();
    PSResourceProfile* getLevels();        	  
    
    PSList<PSEntityKey> getOrderingChoices(TimePoint t);
    
  protected:
    friend class PSEngineWithResources;
    friend class ResourceWrapperGenerator;
    PSResource(const SAVH::ResourceId& res);
  private:
    SAVH::ResourceId m_res;
  };   
    
}

#endif
