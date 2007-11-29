
#include "PSResources.hh"
#include "SAVH_Resource.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Transaction.hh"

namespace EUROPA 
{
  class ResourceWrapperGenerator : public ObjectWrapperGenerator 
  {
  public:
    PSObject* wrap(const ObjectId& obj) {
      checkRuntimeError(SAVH::ResourceId::convertable(obj),
			"Object " << obj->toString() << " is not a resource.");
      return new PSResource(SAVH::ResourceId(obj));
    }
  };
    
  void PSEngineWithResources::registerObjectWrappers()
  {
	  PSEngineImpl::registerObjectWrappers();
      addObjectWrapperGenerator("Reservoir", new ResourceWrapperGenerator());
      addObjectWrapperGenerator("Reusable", new ResourceWrapperGenerator());
      addObjectWrapperGenerator("Unary", new ResourceWrapperGenerator());
  }
  
  
  PSList<PSResource*> PSEngineWithResources::getResourcesByType(const std::string& objectType) {
    check_runtime_error(m_planDatabase.isValid());
    
    PSList<PSResource*> retval;
    
    const ObjectSet& objects = m_planDatabase->getObjects();
    for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
      ObjectId object = *it;
      if(Schema::instance()->isA(object->getType(), objectType.c_str()))
	    retval.push_back(dynamic_cast<PSResource*>(getObjectWrapperGenerator(object->getType())->wrap(object)));
    }
    
    return retval;
  }
  
  PSResource* PSEngineWithResources::getResourceByKey(PSEntityKey id) {
    check_runtime_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSResource(entity);
  }
  
  PSResource::PSResource(const SAVH::ResourceId& res) : PSObjectImpl(res), m_res(res) {}

  PSResourceProfile* PSResource::getLimits() {
    return new PSResourceProfile(m_res->getLowerLimit(), m_res->getUpperLimit());
  }

  PSResourceProfile* PSResource::getLevels() {
    return new PSResourceProfile(m_res->getProfile());
  }
  
  PSList<PSEntityKey> PSResource::getOrderingChoices(TimePoint t)
  {
	  PSList<PSEntityKey> retval;
	  
	  SAVH::InstantId instant;
	  
	  SAVH::ProfileIterator it(m_res->getProfile());
	  while(!it.done()) {
	      TimePoint inst = (TimePoint) it.getTime();
	      if (inst == t) {
	          instant = it.getInstant();
	          break;
	      }
	      it.next();
	  }
	  
	  if (instant.isNoId()) {
		  // TODO: log error
		  return retval;
	  }
	  
	  std::vector<std::pair<SAVH::TransactionId, SAVH::TransactionId> > results;
	  m_res->getOrderingChoices(instant,results);
	  for (unsigned int i = 0;i<results.size(); i++) {
	      SAVH::TransactionId predecessor = results[i].first;
	      SAVH::TransactionId successor = results[i].second;	
	      retval.push_back(predecessor->time()->getParent()->getKey());
	      retval.push_back(successor->time()->getParent()->getKey());
	  }
	  
	  return retval;
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
}
