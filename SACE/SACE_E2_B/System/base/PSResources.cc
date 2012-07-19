#include "PSResources.hh"
#include "FlawHandler.hh"
#include "SAVH_Resource.hh"
#include "SAVH_Profile.hh"
#include "SAVH_ProfilePropagator.hh"
#include "ResourcePropagator.hh"
// TODO: registration for these needs to happen somewhere else
#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_IncrementalFlowProfile.hh" 
#include "ResourceThreatDecisionPoint.hh"

namespace EUROPA {

  PSEngineWithResources::PSEngineWithResources() : PSEngine() {
    REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);
    REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile);
    REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreatDecisionPoint);
  }

  void PSEngineWithResources::start() {
    PSEngine::start();

    new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_constraintEngine);
  }

  void PSEngineWithResources::initDatabase() {
    PSEngine::initDatabase();
    new ResourcePropagator(LabelStr("Resource"), m_constraintEngine, m_planDatabase);
  }

  PSResource::PSResource(const SAVH::ResourceId& res) : PSObject(res), m_res(res) {}

  PSResourceProfile* PSResource::getLimits() {
    return new PSResourceProfile(m_res->getLowerLimit(), m_res->getUpperLimit());
  }

  PSResourceProfile* PSResource::getLevels() {
    return new PSResourceProfile(m_res->getProfile());
  }

  PSResourceProfile::PSResourceProfile(const double lb, const double ub)
    : m_isConst(true), m_lb(lb), m_ub(ub) {
    TimePoint inst = (TimePoint) MINUS_INFINITY;
    m_times.push_back(inst);
  }

  PSResourceProfile::PSResourceProfile(const SAVH::ProfileId& profile)
    : m_isConst(false), m_profile(profile) {
    SAVH::ProfileIterator it(m_profile);
    while(!it.done()) {
      TimePoint inst = (TimePoint) it.getTime();
      m_times.push_back(inst);
      it.next();
    }
  }

  double PSResourceProfile::getLowerBound(TimePoint time) {
    if(m_isConst)
      return m_lb;

    IntervalDomain dom;
    m_profile->getLevel((int) time, dom);
    return dom.getLowerBound();
  }

  double PSResourceProfile::getUpperBound(TimePoint time) {
    if(m_isConst)
      return m_ub;
    IntervalDomain dom;
    m_profile->getLevel((int) time, dom);
    return dom.getUpperBound();
  }

  const PSList<TimePoint>& PSResourceProfile::getTimes() {return m_times;}

  class ResourceWrapperGenerator : public ObjectWrapperGenerator {
  public:
    PSObject* wrap(const ObjectId& obj) {
      checkRuntimeError(SAVH::ResourceId::convertable(obj),
			"Object " << obj->toString() << " is not a resource.");
      return new PSResource(SAVH::ResourceId(obj));
    }
  };
    
  class PSResourceLocalStatic {
  public:
    PSResourceLocalStatic() {
      PSEngine::addObjectWrapperGenerator("Reservoir", new ResourceWrapperGenerator());
      PSEngine::addObjectWrapperGenerator("Reusable", new ResourceWrapperGenerator());
      PSEngine::addObjectWrapperGenerator("Unary", new ResourceWrapperGenerator());
    }
  };

  namespace PSResources {
    PSResourceLocalStatic s_localStatic;
  }
}
