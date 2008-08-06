
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
    return m_id; 
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
    std::map<double,FactoryId>::iterator it = m_factoryMap.find(factory->getName());
    if(it != m_factoryMap.end()) {
        delete ((Factory*)it->second);
        m_factoryMap.erase(it);
        // TODO: log a message notifying of new registration
    }
    m_factoryMap.insert(std::pair<double,FactoryId>(factory->getName(), factory));
}

void FactoryMgr::purgeAll()
{
    std::map<double,FactoryId>::iterator factories_iter = m_factoryMap.begin();
    while (factories_iter != m_factoryMap.end()) {
      Factory* factory = (Factory*)((factories_iter++)->second);
      debugMsg("FactoryMgr:purgeAll","Removing factory for " << factory->getName().toString());
      delete factory;
    }
    m_factoryMap.clear();
}

Factory* FactoryMgr::getFactory(const LabelStr& name) const
{   
    std::map<double,FactoryId>::const_iterator it = m_factoryMap.find(name);
    checkError(it != m_factoryMap.end(), "No factory registered for '" << name.toString() << "'");    
    return (Factory*)(it->second);
}

}
