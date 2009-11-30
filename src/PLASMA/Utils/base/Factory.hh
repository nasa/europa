#ifndef FACTORY_HH_
#define FACTORY_HH_

#include <map>
#include "Engine.hh"
#include "LabelStr.hh"

namespace EUROPA {

class FactoryObj;
typedef Id<FactoryObj> FactoryObjId;

class Factory;
typedef Id<Factory> FactoryId;

class FactoryMgr;
typedef Id<FactoryMgr> FactoryMgrId;


class FactoryArgs
{
public:
    virtual ~FactoryArgs() {}
};

class FactoryObj
{
public:
    virtual ~FactoryObj() {}
};

class Factory
{
public:
    Factory(const LabelStr& name);
    virtual ~Factory();
    
    FactoryId& getId(); 
    const LabelStr& getName() const;

    virtual FactoryObjId& createInstance(const FactoryArgs& args) = 0;
    
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
        
    virtual FactoryObjId& createInstance(const LabelStr& name, const FactoryArgs& args);
    
protected:
    FactoryMgrId m_id;     
    std::map<edouble,FactoryId> m_factoryMap;   
    
    FactoryId& getFactory(const LabelStr& name);    
};

}


#endif /*FACTORY_HH_*/
