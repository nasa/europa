#include "Nddl.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "Schema.hh"
#include "ObjectTokenRelation.hh"
#include "ObjectFilter.hh"
#include "DbLogger.hh"

#include "../ConstraintEngine/ConstraintEngine.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "../ConstraintEngine/BoolDomain.hh"
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/Constraint.hh"
#include "../ConstraintEngine/Constraints.hh"
#include "../ConstraintEngine/TestSupport.hh"
#include "../ConstraintEngine/CeLogger.hh"

#include "Macros.hh"

#include <list>
#include <vector>
#include <iostream>
#include <cassert>

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
