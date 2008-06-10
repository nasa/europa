
#include "EuropaEngine.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleSolvers.hh"
#include "ModuleNddl.hh"
#ifndef NO_RESOURCES
#include "ModuleResource.hh"
#include "ModuleAnml.hh"
#endif

#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"

namespace EUROPA
{
    EuropaEngine::EuropaEngine()
    {
        createModules();
    }

    EuropaEngine::~EuropaEngine()
    {
    }

    void EuropaEngine::createModules()
    {
	    // TODO: make this data-driven
	    addModule((new ModuleConstraintEngine())->getId());
	    addModule((new ModuleConstraintLibrary())->getId());
	    addModule((new ModulePlanDatabase())->getId());
	    addModule((new ModuleRulesEngine())->getId());
	    addModule((new ModuleTemporalNetwork())->getId());
	    addModule((new ModuleSolvers())->getId());
	    addModule((new ModuleNddl())->getId());
#ifndef NO_RESOURCES
        addModule((new ModuleResource())->getId());
	    addModule((new ModuleAnml())->getId());
#endif
    }
    
    ConstraintEngineId& EuropaEngine::getConstraintEngine() { return (ConstraintEngineId&)((ConstraintEngine*)getComponent("ConstraintEngine"))->getId(); }
    PlanDatabaseId&     EuropaEngine::getPlanDatabase()     { return (PlanDatabaseId&)((PlanDatabase*)getComponent("PlanDatabase"))->getId(); }
    RulesEngineId&      EuropaEngine::getRulesEngine()      { return (RulesEngineId&)((RulesEngine*)getComponent("RulesEngine"))->getId(); }    
    
    const ConstraintEngine* EuropaEngine::getConstraintEnginePtr() const { return (const ConstraintEngine*)getComponent("ConstraintEngine"); }
    const PlanDatabase*     EuropaEngine::getPlanDatabasePtr()     const { return (const PlanDatabase*)getComponent("PlanDatabase"); }
    const RulesEngine*      EuropaEngine::getRulesEnginePtr()      const { return (const RulesEngine*)getComponent("RulesEngine"); }        
}

