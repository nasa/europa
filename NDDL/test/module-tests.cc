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
    ConstraintEngine ce; \
    Schema schema; \
    PlanDatabase db(ce.getId(), schema.getId()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce.getId()); \
      dbLId = (new DbLogger(std::cout, db.getId()))->getId(); \
    } \
    Object& object = *(new Object(db.getId(), LabelStr("AllObjects"), LabelStr("o1"))); \
    check_error(!object.getId().isNoId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN() \
    delete (DbLogger*) dbLId;

class NddlSchemaTest {
public:
  static bool test() {
    runTest(testObjectPredicateRelationships);
    return(true);
  }

private:

  static bool testObjectPredicateRelationships() {
    NddlSchema schema(LabelStr("TestSchema"));
    schema.addType(LabelStr("Resource"));
    schema.addObjectParent(LabelStr("Resource"), LabelStr("NddlResource"));
    schema.addPredicate(LabelStr("Resource.change"));
    schema.addType(LabelStr("Battery"));
    schema.addObjectParent(LabelStr("Battery"), LabelStr("Resource"));
    schema.addObjectPredicate(LabelStr("Resource"), LabelStr("Resource.change"));
    schema.addType(LabelStr("World"));
    schema.addPredicate(LabelStr("World.initialState"));
    schema.addObjectPredicate(LabelStr("World"), LabelStr("World.initialState"));
    schema.addPredicateParameter(LabelStr("Resource.change"), LabelStr("quantity"));

    check_error(schema.isPredicateDefined(LabelStr("Resource.change")));
    check_error(schema.isPredicateDefined(LabelStr("Battery.change")));
    check_error(schema.isPredicateDefined(LabelStr("World.initialState")));
    check_error(!schema.isPredicateDefined(LabelStr("NOCLASS.NOPREDICATE")));
    check_error(schema.isTypeDefined(LabelStr("Resource")));
    check_error(schema.isTypeDefined(LabelStr("World")));
    check_error(schema.isTypeDefined(LabelStr("Battery")));
    check_error(!schema.isTypeDefined(LabelStr("NOTYPE")));
    check_error(schema.canContain(LabelStr("Resource.change"), LabelStr("quantity")));
    check_error(schema.canContain(LabelStr("Battery.change"), LabelStr("quantity")));
    check_error(!schema.canContain(LabelStr("NddlResource.change"), LabelStr("quantity")));

    check_error(schema.canBeAssigned(LabelStr("World"), LabelStr("World.initialState")));
    check_error(schema.canBeAssigned(LabelStr("Resource"), LabelStr("Resource.change")));
    check_error(schema.canBeAssigned(LabelStr("Battery"), LabelStr("Resource.change")));
    check_error(!schema.canBeAssigned(LabelStr("World"), LabelStr("Resource.change")));

    check_error(!schema.isA(LabelStr("Resource"), LabelStr("Battery")));
    check_error(schema.isA(LabelStr("Battery"), LabelStr("Resource")));
    check_error(schema.isA(LabelStr("Battery"), LabelStr("Battery")));

    check_error(schema.hasParent(LabelStr("Battery")));
    check_error(schema.getParent(LabelStr("Battery")) == LabelStr("Resource"));

    return(true);
  }
};

class R_Predicate_0_0: public RuleInstance {
public:
  R_Predicate_0_0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb) { }

  void handleExecute() { }
};

DECLARE_AND_DEFINE_RULE(Predicate_0, Predicate);

class R_Predicate_1_0: public RuleInstance {
public:
  R_Predicate_1_0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb, const ConstrainedVariableId& guard)
    : RuleInstance(rule, token, planDb, guard){}
  void handleExecute(){}
};

DECLARE_AND_DEFINE_SINGLETON_GUARDED_RULE(Predicate_1, Predicate, object);

class R_Predicate_2_0: public RuleInstance {
public:
  R_Predicate_2_0(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb, const ConstrainedVariableId& guard, double value)
    : RuleInstance(rule, token, planDb, guard, value){}
  void handleExecute(){}
};

DECLARE_AND_DEFINE_VALUE_GUARDED_RULE(Predicate_2, Predicate, object, 10);

class NddlRuleIntergrationTest {
public:

  static bool test() {
    runTest(testBasicComponents);
    return(true);
  }

private:

  static bool testBasicComponents() {
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
    bool prop = ce.propagate();
    check_error(prop);
    DEFAULT_TEARDOWN();
    return(true);
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
  ConstraintLibrary::purgeAll();
}
