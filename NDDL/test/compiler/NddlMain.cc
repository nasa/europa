#include "Nddl.hh"
#include "NddlUtils.hh"
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

#include <list>
#include <vector>
#include <iostream>
#include <cassert>

namespace NDDL {

  NddlToken::NddlToken(const PlanDatabaseId& planDatabase, const LabelStr& predicateName)
    :Prototype::IntervalToken(planDatabase, 
			      predicateName,
			      true,
			      IntervalIntDomain(),
			      IntervalIntDomain(),
			      IntervalIntDomain(1, PLUS_INFINITY),
			      Prototype::Token::noObject(),
			      false){}

  NddlToken::NddlToken(const TokenId& master, const LabelStr& predicateName)
    :Prototype::IntervalToken(master, 
			      predicateName,
			      IntervalIntDomain(),
			      IntervalIntDomain(),
			      IntervalIntDomain(1, PLUS_INFINITY),
			      Prototype::Token::noObject(),
			      false){}
}

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
