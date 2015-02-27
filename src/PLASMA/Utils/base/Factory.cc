
#include "Factory.hh"
#include "Debug.hh"

namespace EUROPA 
{

Factory::Factory(const std::string& name)
    : m_id(this)
    , m_name(name)
{    
}

Factory::~Factory()
{    
    m_id.remove();
}

FactoryId Factory::getId() 
{ 
    return m_id;         // TODO: log a message notifying of new registration

}

const std::string& Factory::getName() const
{
    return m_name;
}

FactoryMgr::FactoryMgr()
    : m_id(this), m_factoryMap()
{    
}

FactoryMgr::~FactoryMgr()
{
    purgeAll();
    m_id.remove();
}

FactoryMgrId FactoryMgr::getId() 
{ 
    return m_id; 
}

void FactoryMgr::registerFactory(FactoryId factory) {
  std::map<std::string,FactoryId>::iterator it = m_factoryMap.find(factory->getName());
  if(it != m_factoryMap.end()) {
    delete (static_cast<Factory*>(it->second));
    m_factoryMap.erase(it);
    debugMsg("FactoryMgr:registerFactory","Registered new factory for " << factory->getName());
  }
  m_factoryMap.insert(std::make_pair(factory->getName(), factory));
}

void FactoryMgr::purgeAll()
{
  std::map<std::string,FactoryId>::iterator factories_iter = m_factoryMap.begin();
    while (factories_iter != m_factoryMap.end()) {
      Factory* factory = static_cast<Factory*>((factories_iter++)->second);
      debugMsg("FactoryMgr:purgeAll","Removing factory for " << factory->getName());
      delete factory;
    }
    m_factoryMap.clear();
}

FactoryObjId FactoryMgr::createInstance(const std::string& name, const FactoryArgs& args) 
{
  FactoryId factory = getFactory(name);
  return factory->createInstance(args);
}

FactoryId FactoryMgr::getFactory(const std::string& name)
{   
  std::map<std::string,FactoryId>::iterator it = m_factoryMap.find(name);
  checkError(it != m_factoryMap.end(),
             "No factory registered for '" << name << "'");    
  return it->second;
}

}
