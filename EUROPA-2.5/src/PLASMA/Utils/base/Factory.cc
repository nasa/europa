
#include "Factory.hh"
#include "Debug.hh"

namespace EUROPA 
{

Factory::Factory(const LabelStr& name)
    : m_id(this)
    , m_name(name)
{    
}

Factory::~Factory()
{    
    m_id.remove();
}

FactoryId& Factory::getId() 
{ 
    return m_id;         // TODO: log a message notifying of new registration

}

const LabelStr& Factory::getName() const
{
    return m_name;
}

FactoryMgr::FactoryMgr()
    : m_id(this)
{    
}

FactoryMgr::~FactoryMgr()
{
    purgeAll();
    m_id.remove();
}

FactoryMgrId& FactoryMgr::getId() 
{ 
    return m_id; 
}

void FactoryMgr::registerFactory(FactoryId& factory)
{
    std::map<edouble,FactoryId>::iterator it = m_factoryMap.find(factory->getName());
    if(it != m_factoryMap.end()) {
        delete ((Factory*)it->second);
        m_factoryMap.erase(it);
        std::cout << "FactoryMgr::registerFactory: Registered new factory for " << factory->getName().toString() << std::endl;
        // TODO: log INFO message notifying of new registration
        //debugMsg("FactoryMgr:registerFactory","Registered new factory for " << factory->getName().toString());
    }
    m_factoryMap.insert(std::make_pair(factory->getName(), factory));
}

void FactoryMgr::purgeAll()
{
    std::map<edouble,FactoryId>::iterator factories_iter = m_factoryMap.begin();
    while (factories_iter != m_factoryMap.end()) {
      Factory* factory = (Factory*)((factories_iter++)->second);
      debugMsg("FactoryMgr:purgeAll","Removing factory for " << factory->getName().toString());
      delete factory;
    }
    m_factoryMap.clear();
}

FactoryObjId& FactoryMgr::createInstance(const LabelStr& name, const FactoryArgs& args) 
{
  FactoryId& factory = getFactory(name);
  return factory->createInstance(args);
}

FactoryId& FactoryMgr::getFactory(const LabelStr& name)
{   
    std::map<edouble,FactoryId>::iterator it = m_factoryMap.find(name);
    checkError(it != m_factoryMap.end(), "No factory registered for '" << name.toString() << "'");    
    return it->second;
}

}
