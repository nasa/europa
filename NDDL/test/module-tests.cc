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
#include "PlanDatabaseTestSupport.hh"

using namespace Prototype;
using namespace NDDL;

class ObjectFilterConstraintTest {
public:
  static bool test() {
    runTest(testBasicAllocation);
    runTest(testPropagation);
    runTest(testFiltering);
    return true;
  }

private:
  static bool testBasicAllocation() {
    DEFAULT_SETUP(ce, db, schema, false);

    // Allocate an object with fields
    ObjectId object = makeObjectForTesting(db, LabelStr("objectName"), 1, LabelStr("A"), true, 2.1);
    Variable<ObjectDomain> filterVariable(ce, ObjectDomain(object));

    // Allocate a number of filter variables, one for each field
    Variable<LabelSet> v1(ce, makeLabelSetDomain());
    Variable<EnumeratedDomain> v3(ce, makeEnumeratedDomain());

    // Create the constraint with filter set up
    std::vector<ObjectFilterCondition*> filter;
    filter.push_back(new ConcreteObjectFilterCondition<LabelSet>(v1.getId(), 1, ObjectFilterConstraint::eq));
    filter.push_back(new ConcreteObjectFilterCondition<EnumeratedDomain>(v3.getId(), 3, ObjectFilterConstraint::eq));

    ObjectFilterConstraint c0(LabelStr("ObjectFilter"), 
			      LabelStr("Default"),
			      ce,
			      filterVariable.getId(),
			      ObjectFilterConstraint::CONSTRAIN,
			      filter);

    assert(ce->propagate());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testPropagation() {
    DEFAULT_SETUP(ce, db, schema, false);
    // Allocate a number of objects
    std::list<ObjectId> objects;
    objects.push_back(makeObjectForTesting(db, LabelStr("object0"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object1"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object2"), 1, LabelStr("B"), true, 3.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object3"), 1, LabelStr("B"), true, 4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object4"), 1, LabelStr("A"), false,4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object5"), 2, LabelStr("A"), true, 6.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object6"), 1, LabelStr("B"), false, 7.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object7"), 2, LabelStr("B"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object8"), 1, LabelStr("A"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object9"), 1, LabelStr("A"), true, 0.1));
    db->close();

    Variable<ObjectDomain> filterVariable(ce, ObjectDomain(objects));

    // Allocate a number of filter variables, one for each field    
    Variable<LabelSet> v1(ce, makeLabelSetDomain());
    Variable<EnumeratedDomain> v3(ce, makeEnumeratedDomain());

    // Create the constraint with filter set up
    std::vector<ObjectFilterCondition*> filter;
    filter.push_back(new ConcreteObjectFilterCondition<LabelSet>(v1.getId(), 1, ObjectFilterConstraint::eq));
    filter.push_back(new ConcreteObjectFilterCondition<EnumeratedDomain>(v3.getId(), 3, ObjectFilterConstraint::eq));

    ObjectFilterConstraint c0(LabelStr("ObjectFilter"), 
			      LabelStr("Default"),
			      ce,
			      filterVariable.getId(),
			      ObjectFilterConstraint::CONSTRAIN,
			      filter);

    assert(ce->propagate());

    // Confirm that the object variable has not yet been restricted
    assert(filterVariable.getDerivedDomain().getSize() == 10);

    // Confirm that the filter variables have been restricted.
    assert(v1.getDerivedDomain().getSize() == 2);
    assert(v1.getDerivedDomain().isMember(LabelStr("A")));
    assert(v1.getDerivedDomain().isMember(LabelStr("B")));
    assert(v3.getDerivedDomain().getSize() == 7);
    assert(!v3.getDerivedDomain().isMember(9.1));

    // Now select for one filter and refine objects
    v1.specify(LabelStr("A"));
    assert(filterVariable.getDerivedDomain().getSize() == 6);
    assert(v3.getDerivedDomain().getSize() == 5); // Also pruned, as objects are removed

    // Now select other and confirm again
    v3.specify(8.1);
    assert(filterVariable.getDerivedDomain().isSingleton());

    // Reset and confirm repropagation is correct. Non chronologically.
    v1.reset();
    assert(filterVariable.getDerivedDomain().getSize() == 2); // @ objects with 8.1

    // Propagate and confirm restrictions
    DEFAULT_TEARDOWN();
    return true;
  }


  static bool testFiltering() {
    DEFAULT_SETUP(ce, db, schema, false);
    // Allocate a number of objects
    std::list<ObjectId> objects;
    objects.push_back(makeObjectForTesting(db, LabelStr("object0"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object1"), 1, LabelStr("A"), true, 1.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object2"), 1, LabelStr("B"), true, 3.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object3"), 1, LabelStr("B"), true, 4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object4"), 1, LabelStr("A"), false,4.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object5"), 2, LabelStr("A"), true, 6.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object6"), 1, LabelStr("B"), false, 7.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object7"), 2, LabelStr("B"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object8"), 1, LabelStr("A"), true, 8.1));
    objects.push_back(makeObjectForTesting(db, LabelStr("object9"), 1, LabelStr("A"), true, 8.1));
    db->close();

    Variable<ObjectDomain> filterVariable(ce, ObjectDomain(objects));

    // Allocate a number of filter variables, one for each field    
    Variable<LabelSet> v1(ce, makeLabelSetDomain());
    Variable<EnumeratedDomain> v3(ce, makeEnumeratedDomain());

    // Create the constraint with filter set up
    std::vector<ObjectFilterCondition*> filter;
    filter.push_back(new ConcreteObjectFilterCondition<LabelSet>(v1.getId(), 1, ObjectFilterConstraint::eq));
    filter.push_back(new ConcreteObjectFilterCondition<EnumeratedDomain>(v3.getId(), 3, ObjectFilterConstraint::eq));

    ObjectFilterConstraint c0(LabelStr("ObjectFilter"), 
			      LabelStr("Default"),
			      ce,
			      filterVariable.getId(),
			      ObjectFilterConstraint::FILTER,
			      filter);

    v1.specify(LabelStr("A")); // Fixing A only will still leave the filter var empty
    v3.specify(8.1); // Fixing A only will still leave the filter var empty

    assert(ce->propagate());
    assert(c0.getFilteredObjects().getSize() == 2);

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief Helper method to allocate object with fields to support testing
   */
  static ObjectId makeObjectForTesting(const PlanDatabaseId& db, 
				       const LabelStr& name,
				       unsigned int arg0,
				       const LabelStr& arg1,
				       bool arg2,
				       float arg3){
    ObjectId object = (new Object(db, Schema::ALL_OBJECTS(), name, true))->getId();
    object->addVariable(IntervalIntDomain(arg0), LabelStr("FIELD_0"));
    object->addVariable(LabelSet(arg1), LabelStr("FIELD_1"));
    object->addVariable(BoolDomain(arg2), LabelStr("FIELD_2"));
    object->addVariable(IntervalDomain(arg3), LabelStr("FIELD_3"));
    object->close();
    return object;
  }

  static const LabelSet makeLabelSetDomain(){
    LabelSet lblSet;
    lblSet.insert(LabelStr("A"));
    lblSet.insert(LabelStr("B"));
    lblSet.insert(LabelStr("C"));
    lblSet.insert(LabelStr("D"));
    lblSet.insert(LabelStr("E"));
    lblSet.insert(LabelStr("F"));
    lblSet.insert(LabelStr("G"));
    lblSet.close();
    return lblSet;
  }

  static const EnumeratedDomain makeEnumeratedDomain(){
    EnumeratedDomain dom;
    dom.insert(0.1);
    dom.insert(1.1);
    dom.insert(2.1);
    dom.insert(3.1);
    dom.insert(4.1);
    dom.insert(5.1);
    dom.insert(6.1);
    dom.insert(7.1);
    dom.insert(8.1);
    dom.insert(9.1);
    dom.close();
    return dom;
  }

};

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

    IntervalToken t0(db, 
                     LabelStr("Predicate"),                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));
    t0.activate();
    bool prop = ce->propagate();
    assertTrue(prop);
    DEFAULT_TEARDOWN();
    return(true);
  }
};

int main() {
  // Special designations for temporal relations
  REGISTER_CONSTRAINT(EqualConstraint, "concurrent", "Default");
  REGISTER_CONSTRAINT(LessThanEqualConstraint, "precede", "Default");

  // Support for Token implementations
  REGISTER_CONSTRAINT(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");

  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");

  runTestSuite(ObjectFilterConstraintTest::test);
  runTestSuite(NddlSchemaTest::test);
  runTestSuite(NddlRuleIntergrationTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
}
