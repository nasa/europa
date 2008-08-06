#ifndef FACTORY_HH_
#define FACTORY_HH_

#include <map>
#include "Engine.hh"
#include "LabelStr.hh"

namespace EUROPA {

class Factory;
typedef Id<Factory> FactoryId;

class FactoryMgr;
typedef Id<FactoryMgr> FactoryMgrId;

// TODO: template <class FactoryType>
class Factory
{
public:
    Factory(const LabelStr& name);
    virtual ~Factory();
    
    FactoryId& getId(); 
    const LabelStr& getName() const;
    
protected:
    FactoryId m_id;
    LabelStr m_name;
};


class FactoryMgr : public EngineComponent
{    
public:
    FactoryMgr();
    virtual ~FactoryMgr();

    FactoryMgrId& getId();
  
    void registerFactory(FactoryId& factory);
    void purgeAll();
        
    // TODO: make createInstance method generic        
    
protected:
    FactoryMgrId m_id;     
    std::map<double,FactoryId> m_factoryMap;   
    
    Factory* getFactory(const LabelStr& name) const;    
};

}


#endif /*FACTORY_HH_*/
