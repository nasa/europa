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
    assertTrue(!object.getId().isNoId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN() \
    delete (DbLogger*) dbLId;

class NddlSchemaTest {
public:
  static bool test() {
    runTest(testObjectPredicateRelationships);
    runTest(testPredicateParameterAccessors);
    runTest(testTypeQueries);
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

    assertTrue(schema.isPredicateDefined(LabelStr("Resource.change")));
    assertTrue(schema.isPredicateDefined(LabelStr("Battery.change")));
    assertTrue(schema.isPredicateDefined(LabelStr("World.initialState")));
    assertTrue(!schema.isPredicateDefined(LabelStr("NOCLASS.NOPREDICATE")));
    assertTrue(schema.isTypeDefined(LabelStr("Resource")));
    assertTrue(schema.isTypeDefined(LabelStr("World")));
    assertTrue(schema.isTypeDefined(LabelStr("Battery")));
    assertTrue(!schema.isTypeDefined(LabelStr("NOTYPE")));
    assertTrue(schema.canContain(LabelStr("Resource.change"), LabelStr("quantity")));
    assertTrue(schema.canContain(LabelStr("Battery.change"), LabelStr("quantity")));
    assertTrue(!schema.canContain(LabelStr("NddlResource.change"), LabelStr("quantity")));

    assertTrue(schema.canBeAssigned(LabelStr("World"), LabelStr("World.initialState")));
    assertTrue(schema.canBeAssigned(LabelStr("Resource"), LabelStr("Resource.change")));
    assertTrue(schema.canBeAssigned(LabelStr("Battery"), LabelStr("Resource.change")));
    assertTrue(!schema.canBeAssigned(LabelStr("World"), LabelStr("Resource.change")));

    assertTrue(!schema.isA(LabelStr("Resource"), LabelStr("Battery")));
    assertTrue(schema.isA(LabelStr("Battery"), LabelStr("Resource")));
    assertTrue(schema.isA(LabelStr("Battery"), LabelStr("Battery")));
    assertTrue(schema.hasParent(LabelStr("Battery")));
    assertTrue(schema.getParent(LabelStr("Battery")) == LabelStr("Resource"));
    assertTrue(schema.getObjectType(LabelStr("World.initialState")) == LabelStr("World"));
    assertTrue(schema.getObjectType(LabelStr("Battery.change")) == LabelStr("Battery"));
    assertTrue(schema.getObjectType(LabelStr("Battery.change")) != LabelStr("Resource"));
    return(true);
  }

  static bool testPredicateParameterAccessors() {
    NddlSchema schema(LabelStr("TestSchema"));
    schema.addType(LabelStr("Resource"));
    schema.addObjectParent(LabelStr("Resource"), LabelStr("NddlResource"));
    schema.addPredicate(LabelStr("Resource.change"));
    schema.addType(LabelStr("Battery"));
    schema.addObjectParent(LabelStr("Battery"), LabelStr("Resource"));
    schema.addObjectPredicate(LabelStr("Resource"), LabelStr("Resource.change"));
    schema.addPredicateParameter(LabelStr("Resource.change"), LabelStr("quantity"));
    schema.addPredicateParameter(LabelStr("Resource.change"), LabelStr("quality"));
    assertTrue(schema.getIndexFromName(LabelStr("Resource.change"), LabelStr("quality")) == 1);
    assertTrue(schema.getNameFromIndex(LabelStr("Resource.change"), 0).getKey() == LabelStr("quantity").getKey());
    return true;
  }

  static bool testTypeQueries() {
    NddlSchema schema(LabelStr("TestSchema"));
    schema.addEnum(LabelStr("FooEnum"));
    schema.addEnumMember(LabelStr("FooEnum"), LabelStr("FOO"));
    schema.addEnumMember(LabelStr("FooEnum"), LabelStr("BAR"));
    schema.addEnumMember(LabelStr("FooEnum"), LabelStr("BAZ"));
    schema.addEnum(LabelStr("BarEnum"));
    schema.addEnumMember(LabelStr("BarEnum"), LabelStr("QUUX"));
    schema.addEnumMember(LabelStr("BarEnum"), LabelStr("QUUUX"));
    assertTrue(schema.isEnum(LabelStr("FOO")));
    assertTrue(schema.isEnum(LabelStr("QUUUX")));
    assertTrue(!schema.isEnum(LabelStr("ARG")));

    schema.addType(LabelStr("Foo"));
    assertTrue(schema.isClass(LabelStr("Foo")));
    assertTrue(!schema.isClass(LabelStr("Bar")));
    return true;
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
    assertTrue(prop);
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
