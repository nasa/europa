#ifndef _H_EngineBase
#define _H_EngineBase

#include "Engine.hh"
#include "Module.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"

namespace EUROPA {

  class EuropaEngineBase : public EngineBase 
  {
    public:  
       	EuropaEngineBase();          
    	
        virtual ConstraintEngineId& getConstraintEngine();
        virtual PlanDatabaseId&     getPlanDatabase();
        virtual RulesEngineId&      getRulesEngine();
        
        virtual const ConstraintEngine* getConstraintEnginePtr() const;
        virtual const PlanDatabase*     getPlanDatabasePtr() const;
        virtual const RulesEngine*      getRulesEnginePtr() const;
        
    protected: 
    	virtual ~EuropaEngineBase();    	
    	void createModules();    	    			
  };
}

#endif
