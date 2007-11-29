
#include "PSResources.hh"
#include "PSResourceImpl.hh"
#include "LabelStr.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "PSResourceImpl.hh"
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
      return new PSResourceImpl(SAVH::ResourceId(obj));
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
    return new PSResourceImpl(entity);
  }  
}
