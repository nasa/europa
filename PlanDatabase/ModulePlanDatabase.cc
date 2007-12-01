#include "ModulePlanDatabase.hh"
#include "Schema.hh"
#include "ConstraintLibrary.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "ObjectTokenRelation.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"

namespace EUROPA {

  static bool & planDatabaseInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModulePlanDatabase::ModulePlanDatabase()
      : Module("PlanDatabase")
  {
	  
  }

  ModulePlanDatabase::~ModulePlanDatabase()
  {	  
  }  
  
  void ModulePlanDatabase::initialize()
  {
      if(planDatabaseInitialized())
    	  return;

      REGISTER_SYSTEM_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
  	  REGISTER_SYSTEM_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");
  	  REGISTER_SYSTEM_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
      
	  planDatabaseInitialized() = true;
  }  

  void ModulePlanDatabase::uninitialize()
  {
      Schema::instance()->reset();
	  ObjectFactory::purgeAll();
	  TokenFactory::purgeAll();	  
	  
	  planDatabaseInitialized() = false;
  }  
  
  void ModulePlanDatabase::initialize(EngineId engine)
  {
  }
  
  void ModulePlanDatabase::uninitialize(EngineId engine)
  {	  
  }  
}
