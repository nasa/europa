// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"

// Support for registered constraints
#include "ConstraintLibrary.hh"
#include "Constraints.hh"
#include "ObjectTokenRelation.hh"

// Support fro required plan database components
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "Schema.hh"
#include "ConstraintEngine.hh"

// Access for registered event loggers for instrumentation
#include "CeLogger.hh"
#include "DbLogger.hh"

// Utility for obtaining defualt constraint library registration
#include "TestSupport.hh"

void main(){
  // Initialize constraints
  initConstraintLibrary();
  
  REGISTER_NARY(EqualConstraint, "CoTemporal", "Default");
  REGISTER_NARY(EqualConstraint, "neq", "Default");
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "Before", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_NARY(EqualConstraint, "EqualConstraint", "EquivalenceClass");

  // Allocate the schema
  SchemaId schema = NDDL::schema();
  ConstraintEngine ce;
  PlanDatabase db(ce.getId(), schema);

  new DefaultPropagator(LabelStr("Default"), ce.getId());
  RulesEngine re(db.getId());

  if(loggingEnabled()){
    new DbLogger(std::cout, db.getId());
    new CeLogger(std::cout, ce.getId());
  }
  
  NDDL::initialize(db.getId());

  cout << "Finished" << endl;
}
