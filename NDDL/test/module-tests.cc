#include <iostream>
#include "NddlUtils.hh"
#include "TestSupport.hh"

// Support for default setup
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "Schema.hh"
#include "DefaultPropagator.hh"
#include "CeLogger.hh"
#include "DbLogger.hh"
#include "Object.hh"
#include "Constraints.hh"
#include "ObjectTokenRelation.hh"

using namespace Prototype;
using namespace NDDL;

#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce;\
    Schema schema;\
    PlanDatabase db(ce.getId(), schema.getId());\
    new DefaultPropagator(LabelStr("Default"), ce.getId());\
    if(loggingEnabled()){\
     new CeLogger(std::cout, ce.getId());\
     new DbLogger(std::cout, db.getId());\
    }\
    Object& object = *(new Object(db.getId(), LabelStr("AllObjects"), LabelStr("o1")));\
    if(autoClose) db.close();

class NddlSchemaTest {
public:
  static bool test() {
    runTest(testObjectPredicateRelationships);
    return true;
  }

private:

  static bool testObjectPredicateRelationships(){
    NddlSchema schema(LabelStr("TestSchema"));
    schema.addType(LabelStr("Resource"));
    schema.addObjectParent(LabelStr("Resource"), LabelStr("NddlResource"));
    schema.addPredicate(LabelStr("Resource.change"));
    schema.addType(LabelStr("Battery"));
    schema.addObjectParent(LabelStr("Battery"), LabelStr("NddlResource"));
    schema.addObjectParent(LabelStr("Battery"), LabelStr("Resource"));
    schema.addObjectPredicate(LabelStr("Resource"), LabelStr("Resource.change"));
    schema.addType(LabelStr("World"));
    schema.addPredicate(LabelStr("World.initialState"));
    schema.addObjectPredicate(LabelStr("World"), LabelStr("World.initialState"));

    assert(schema.isPredicateDefined(LabelStr("Resource.change")));
    assert(schema.isPredicateDefined(LabelStr("World.initialState")));
    assert(!schema.isPredicateDefined(LabelStr("NOPREDICATE")));
    assert(schema.isTypeDefined(LabelStr("Resource")));
    assert(schema.isTypeDefined(LabelStr("World")));
    assert(schema.isTypeDefined(LabelStr("Battery")));
    assert(!schema.isTypeDefined(LabelStr("NOTYPE")));

    assert(schema.canBeAssigned(LabelStr("World"), LabelStr("World.initialState")));
    assert(schema.canBeAssigned(LabelStr("Resource"), LabelStr("Resource.change")));
    assert(schema.canBeAssigned(LabelStr("Battery"), LabelStr("Resource.change")));

    assert(!schema.isA(LabelStr("Resource"), LabelStr("Battery")));
    assert(schema.isA(LabelStr("Battery"), LabelStr("Resource")));
    assert(schema.isA(LabelStr("Battery"), LabelStr("Battery")));

    return true;
  }
};


class R_Predicate_0_Root: public RuleInstance {
public:
  R_Predicate_0_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb){}
  void handleExecute(){}
};

DECLARE_AND_DEFINE_RULE(Predicate_0, Predicate);

class R_Predicate_1_Root: public RuleInstance {
public:
  R_Predicate_1_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb, const ConstrainedVariableId& guard)
    : RuleInstance(rule, token, planDb, guard){}
  void handleExecute(){}
};

DECLARE_AND_DEFINE_SINGLETON_GUARDED_RULE(Predicate_1, Predicate, object);

class R_Predicate_2_Root: public RuleInstance {
public:
  R_Predicate_2_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb, const ConstrainedVariableId& guard, double value)
    : RuleInstance(rule, token, planDb, guard, value){}
  void handleExecute(){}
};

DECLARE_AND_DEFINE_VALUE_GUARDED_RULE(Predicate_2, Predicate, object, 10);

class NddlRuleIntergrationTest{
public:
  static bool test(){
    runTest(testBasicComponents);
    return true;
  }
private:
  static bool testBasicComponents(){
    DEFAULT_SETUP(ce, db, schema, true);
    R_Predicate_0 rule0;
    R_Predicate_1 rule1;
    R_Predicate_2 rule2;

    IntervalToken t0(db.getId(),                                                         
                     LabelStr("Predicate"),                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));
    t0.activate();
    assert(ce.propagate());
    return true;
  }
};

int main() {
  // Special designations for temporal relations
  REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "precede", "Default");

  // Support for Token implementations
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");

  REGISTER_NARY(EqualConstraint, "eq", "Default");

  runTestSuite(NddlSchemaTest::test);
  runTestSuite(NddlRuleIntergrationTest::test);
  std::cout << "Finished" << std::endl;
}
