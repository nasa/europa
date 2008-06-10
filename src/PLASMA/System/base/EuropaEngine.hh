#ifndef _H_EuropaEngine
#define _H_EuropaEngine

#include "Engine.hh"
#include "Module.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"

namespace EUROPA {

  class EuropaEngine : public EngineBase 
  {
    public:  
       	EuropaEngine();          
    	
        virtual ConstraintEngineId& getConstraintEngine();
        virtual PlanDatabaseId&     getPlanDatabase();
        virtual RulesEngineId&      getRulesEngine();
        
        virtual const ConstraintEngine* getConstraintEnginePtr() const;
        virtual const PlanDatabase*     getPlanDatabasePtr() const;
        virtual const RulesEngine*      getRulesEnginePtr() const;
        
    protected: 
    	virtual ~EuropaEngine();    	
    	void createModules();    	    			
  };
}

#endif
