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

using namespace Prototype;

namespace NDDL {

  void validate(const PlanDatabaseId& db){}
}

int main(){
  // Constraints with special names to allow mapping to temporal network propagator if necessary
  REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "before", "Default");

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

  // Set up the plan database assembly
  ConstraintEngine ce;
  PlanDatabase db(ce.getId(), schema);

  new DefaultPropagator(LabelStr("Default"), ce.getId());
  RulesEngine re(db.getId());

  if(loggingEnabled()){
    new DbLogger(std::cout, db.getId());
    new CeLogger(std::cout, ce.getId());
  }
  
  // Now kick in the initial state from generated function
  NDDL::initialize(db.getId());

  // Now invoke validation on the plan database
  NDDL::validate(db.getId());

  std::cout << "Finished" << std::endl;
}
