// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"

// Support for registered constraints
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/Constraints.hh"
#include "../PlanDatabase/ObjectTokenRelation.hh"

// Support fro required plan database components
#include "../PlanDatabase/PlanDatabase.hh"
#include "../PlanDatabase/RulesEngine.hh"
#include "../PlanDatabase/Schema.hh"
#include "../ConstraintEngine/ConstraintEngine.hh"

// Access for registered event loggers for instrumentation
#include "../ConstraintEngine/CeLogger.hh"
#include "../PlanDatabase/DbLogger.hh"

// Utility for obtaining defualt constraint library registration
#include "../ConstraintEngine/TestSupport.hh"

int main(){
  // Constraints with special names to allow mapping to temporal network propagator if necessary
  REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "precede", "Default");

  // Constraints used in Token implementations
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");


  // Mappings from library to NDDL language
  REGISTER_NARY(EqualConstraint, "eq", "Default");
  REGISTER_NARY(NotEqualConstraint, "neq", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "leq", "Default");

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

  std::cout << "Finished" << std::endl;
}
