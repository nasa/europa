#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "DbLogger.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "DbClientTransactionLog.hh"

#include "DbClient.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "StringDomain.hh"

#include "TestSupport.hh"
#include "PlanDatabaseTestSupport.hh"
#include "PlanDatabaseWriter.hh"

#include <iostream>
#include <string>

class SchemaTest {
public:
  static bool test() {
    runTest(testEnumerations);
    runTest(testObjectTypeRelationships);
    runTest(testObjectPredicateRelationships);
    runTest(testPredicateParameterAccessors);
    return(true);
  }

private:

  static bool testEnumerations(){
    SCHEMA->reset();
    SCHEMA->addEnum(LabelStr("FooEnum"));
    SCHEMA->addValue(LabelStr("FooEnum"), LabelStr("FOO"));
    SCHEMA->addValue(LabelStr("FooEnum"), LabelStr("BAR"));
    SCHEMA->addValue(LabelStr("FooEnum"), LabelStr("BAZ"));
    SCHEMA->addEnum(LabelStr("BarEnum"));
    SCHEMA->addValue(LabelStr("BarEnum"), 0);
    SCHEMA->addValue(LabelStr("BarEnum"), 5);
    SCHEMA->addValue(LabelStr("BarEnum"), 10);

    assertTrue(SCHEMA->isEnum(LabelStr("FooEnum")));
    assertTrue(SCHEMA->isEnum(LabelStr("BarEnum")));
    assertFalse(SCHEMA->isEnum(LabelStr("BazEnum")));
    assertTrue(SCHEMA->isEnumValue(LabelStr("FooEnum"), LabelStr("FOO")));
    assertTrue(SCHEMA->isEnumValue(LabelStr("FooEnum"), LabelStr("BAZ")));
    assertTrue(SCHEMA->isEnumValue(LabelStr("BarEnum"), 5));
    assertFalse(SCHEMA->isEnumValue(LabelStr("BarEnum"), 6));

    std::list<LabelStr> allenums;
    SCHEMA->getEnumerations(allenums);
    assert(allenums.size() == 2);
    assert(allenums.back() == LabelStr("FooEnum"));
    assert(allenums.front() == LabelStr("BarEnum"));
    return true;
  }


  static bool testObjectTypeRelationships() {
    SCHEMA->reset();
    SCHEMA->addObjectType(LabelStr("Foo"));
    SCHEMA->addObjectType(LabelStr("Baz"));

    assertTrue(SCHEMA->isObjectType(LabelStr("Foo")));
    assertTrue(SCHEMA->isA(LabelStr("Foo"), LabelStr("Foo")));
    assertFalse(SCHEMA->isObjectType(LabelStr("Bar")));
    assertFalse(SCHEMA->isA(LabelStr("Foo"), LabelStr("Baz")));

    // Inheritance
    SCHEMA->addObjectType(LabelStr("Bar"), LabelStr("Foo"));
    assertTrue(SCHEMA->isObjectType(LabelStr("Bar")));
    assertTrue(SCHEMA->isA(LabelStr("Bar"), LabelStr("Foo")));
    assertFalse(SCHEMA->isA(LabelStr("Foo"), LabelStr("Bar")));

    // Composition
    SCHEMA->addMember(LabelStr("Foo"), LabelStr("float"), LabelStr("arg0"));
    SCHEMA->addMember(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg1"));
    SCHEMA->addMember(LabelStr("Foo"), LabelStr("Bar"), LabelStr("arg2"));

    assertTrue(SCHEMA->canContain(LabelStr("Foo"), LabelStr("float"), LabelStr("arg0")));
    assertTrue(SCHEMA->canContain(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg1")));
    assertTrue(SCHEMA->canContain(LabelStr("Foo"), LabelStr("Bar"), LabelStr("arg2")));
    assertTrue(SCHEMA->canContain(LabelStr("Foo"), LabelStr("Bar"), LabelStr("arg1"))); // isA(Bar,Foo)

    assertFalse(SCHEMA->canContain(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg2")));
    assertFalse(SCHEMA->canContain(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg3")));
    assertFalse(SCHEMA->canContain(LabelStr("Foo"), LabelStr("float"), LabelStr("arg1")));

    assertTrue(SCHEMA->canContain(LabelStr("Bar"), LabelStr("float"), LabelStr("arg0")));
    assertTrue(SCHEMA->canContain(LabelStr("Bar"), LabelStr("Foo"), LabelStr("arg1")));
    assertTrue(SCHEMA->canContain(LabelStr("Bar"), LabelStr("Bar"), LabelStr("arg1")));

    assert(SCHEMA->getAllObjectTypes().size() == 3);

    return true;
  }

  static bool testObjectPredicateRelationships() {
    SCHEMA->reset();
    SCHEMA->addObjectType(LabelStr("Resource"));
    SCHEMA->addObjectType(LabelStr("NddlResource"), LabelStr("Resource"));
    SCHEMA->addPredicate(LabelStr("Resource.change"));
    assertTrue(SCHEMA->isPredicate(LabelStr("Resource.change")));

    SCHEMA->addMember(LabelStr("Resource.change"), LabelStr("float"), LabelStr("quantity"));

    SCHEMA->addObjectType(LabelStr("Battery"), LabelStr("Resource"));
    assertTrue(SCHEMA->hasParent(LabelStr("Battery.change")));
    assertTrue(SCHEMA->getParent(LabelStr("Battery.change")) == LabelStr("Resource.change"));

    SCHEMA->addObjectType(LabelStr("World"));
    SCHEMA->addPredicate(LabelStr("World.initialState"));
 
    assertTrue(SCHEMA->isPredicate(LabelStr("Battery.change")));
    assertTrue(SCHEMA->isPredicate(LabelStr("World.initialState")));
    assertFalse(SCHEMA->isPredicate(LabelStr("World.NOPREDICATE")));
    assertTrue(SCHEMA->isObjectType(LabelStr("Resource")));
    assertTrue(SCHEMA->isObjectType(LabelStr("World")));
    assertTrue(SCHEMA->isObjectType(LabelStr("Battery")));
    assertFalse(SCHEMA->isObjectType(LabelStr("NOTYPE")));

    assertTrue(SCHEMA->canContain(LabelStr("Resource.change"), LabelStr("float"), LabelStr("quantity")));
    assertTrue(SCHEMA->canContain(LabelStr("Battery.change"), LabelStr("float"), LabelStr("quantity")));
    assertTrue(SCHEMA->canContain(LabelStr("NddlResource.change"), LabelStr("float"), LabelStr("quantity")));
    assertTrue(SCHEMA->hasMember(LabelStr("Resource.change"), LabelStr("quantity")));
    assertTrue(SCHEMA->hasMember(LabelStr("NddlResource.change"), LabelStr("quantity")));
    assertTrue(SCHEMA->hasMember(LabelStr("Battery.change"), LabelStr("quantity")));

    assertTrue(SCHEMA->canBeAssigned(LabelStr("World"), LabelStr("World.initialState")));
    assertTrue(SCHEMA->canBeAssigned(LabelStr("Resource"), LabelStr("Resource.change")));
    assertTrue(SCHEMA->canBeAssigned(LabelStr("Battery"), LabelStr("Resource.change")));
    assertFalse(SCHEMA->canBeAssigned(LabelStr("World"), LabelStr("Resource.change")));
    assertFalse(SCHEMA->canBeAssigned(LabelStr("Resource"), LabelStr("Battery.change")));

    assertFalse(SCHEMA->isA(LabelStr("Resource"), LabelStr("Battery")));
    assertTrue(SCHEMA->isA(LabelStr("Battery"), LabelStr("Resource")));
    assertTrue(SCHEMA->isA(LabelStr("Battery"), LabelStr("Battery")));
    assertTrue(SCHEMA->hasParent(LabelStr("Battery")));
    assertTrue(SCHEMA->getParent(LabelStr("Battery")) == LabelStr("Resource"));
    assertTrue(SCHEMA->getObjectType(LabelStr("World.initialState")) == LabelStr("World"));
    assertTrue(SCHEMA->getObjectType(LabelStr("Battery.change")) == LabelStr("Battery"));
    assertTrue(SCHEMA->getObjectType(LabelStr("Battery.change")) != LabelStr("Resource"));

    SCHEMA->addObjectType("Base");
    SCHEMA->addObjectType("Derived");
    SCHEMA->addPredicate("Derived.Predicate");
    SCHEMA->addMember("Derived.Predicate", "Battery", "battery");


    assert(SCHEMA->getParameterCount(LabelStr("Resource.change")) == 1);
    assert(SCHEMA->getParameterType(LabelStr("Resource.change"), 0) == LabelStr("float"));

    std::set<LabelStr> predicates;
    SCHEMA->getPredicates(LabelStr("Battery"), predicates);
    assert(predicates.size() == 1);
    predicates.clear();
    SCHEMA->getPredicates(LabelStr("Resource"), predicates);
    assert(predicates.size() == 1);

    SCHEMA->addObjectType("One");
    SCHEMA->addPredicate("One.Predicate1");
    SCHEMA->addPredicate("One.Predicate2");
    SCHEMA->addPredicate("One.Predicate3");
    SCHEMA->addPredicate("One.Predicate4");

    predicates.clear();
    SCHEMA->getPredicates(LabelStr("One"), predicates);
    assert(predicates.size() == 4);

    return(true);
  }

  static bool testPredicateParameterAccessors() {
    SCHEMA->reset();
    SCHEMA->addObjectType(LabelStr("Resource"));
    SCHEMA->addObjectType(LabelStr("NddlResource"), LabelStr("Resource"));
    SCHEMA->addPredicate(LabelStr("Resource.change"));
    SCHEMA->addObjectType(LabelStr("Battery"), LabelStr("Resource"));
    SCHEMA->addMember(LabelStr("Resource.change"), LabelStr("float"), LabelStr("quantity"));
    SCHEMA->addMember(LabelStr("Resource.change"), LabelStr("float"), LabelStr("quality"));
    assertTrue(SCHEMA->getIndexFromName(LabelStr("Resource.change"), LabelStr("quality")) == 1);
    assertTrue(SCHEMA->getNameFromIndex(LabelStr("Resource.change"), 0).getKey() == LabelStr("quantity").getKey());

    SCHEMA->addObjectType(LabelStr("Foo"));
    SCHEMA->addPredicate(LabelStr("Foo.Argle"));
    SCHEMA->addMember(LabelStr("Foo.Argle"), LabelStr("Bargle"), LabelStr("bargle"));
    SCHEMA->addMember(LabelStr("Foo.Argle"), LabelStr("Targle"), LabelStr("targle"));

    assertTrue(SCHEMA->getMemberType(LabelStr("Foo.Argle"), LabelStr("bargle")) == LabelStr("Bargle"));
    assertTrue(SCHEMA->getMemberType(LabelStr("Foo.Argle"), LabelStr("targle")) == LabelStr("Targle"));

    // Extend attributes on a derived class. Must declare predicate with derived type qualifier
    SCHEMA->addObjectType(LabelStr("Bar"), LabelStr("Foo"));
    SCHEMA->addPredicate(LabelStr("Bar.Argle"));
    assertTrue(SCHEMA->hasParent(LabelStr("Bar.Argle")));
    SCHEMA->addMember(LabelStr("Bar.Argle"), LabelStr("float"), LabelStr("huey"));
    assertTrue(SCHEMA->getMemberType(LabelStr("Bar.Argle"), LabelStr("huey")) == LabelStr("float"));

    SCHEMA->addObjectType(LabelStr("Baz"), LabelStr("Bar"));
    assertTrue(SCHEMA->getMemberType(LabelStr("Baz.Argle"), LabelStr("targle")) == LabelStr("Targle"));

    assert(SCHEMA->getParameterCount(LabelStr("Foo.Argle")) == 2);
    assert(SCHEMA->getParameterType(LabelStr("Foo.Argle"), 0) == LabelStr("Bargle"));
    assert(SCHEMA->getParameterType(LabelStr("Foo.Argle"), 1) == LabelStr("Targle"));

    return true;
  }
};

/**
 * Test class for testing client and factory
 */
class ForceFailureConstraint : public Constraint {
public:
  ForceFailureConstraint(const LabelStr& name,
			 const LabelStr& propagatorName,
			 const ConstraintEngineId& constraintEngine,
			 const std::vector<ConstrainedVariableId>& variables)
  : Constraint(name, propagatorName, constraintEngine, variables){}

  void handleExecute(){
    if(getCurrentDomain(m_variables[0]).isSingleton())
      getCurrentDomain(m_variables[0]).empty();
  }
};

class ObjectTest {
public:
  
  static bool test() {
    runTest(testBasicAllocation);
    runTest(testObjectDomain);
    runTest(testObjectVariables);
    runTest(testObjectTokenRelation);
    runTest(testCommonAncestorConstraint);
    runTest(testHasAncestorConstraint);
    runTest(testMakeObjectVariable);
    runTest(testTokenObjectVariable);
    runTest(testTokenWithNoObjectOnCreation);
    return(true);
  }
  
private:
  static bool testBasicAllocation() {
    return testBasicObjectAllocationImpl();
  }
  
  static bool testObjectDomain(){
    return testObjectDomainImpl();
  }
  
  static bool testObjectVariables(){
    return testObjectVariablesImpl();
  }
  
  
  static bool testObjectTokenRelation(){
    return testObjectObjectTokenRelationImpl();
  }
  
  static bool testCommonAncestorConstraint(){
    return testCommonAncestorConstraintImpl();
  }
  
  static bool testHasAncestorConstraint(){
    return testHasAncestorConstraintImpl();
  }
  /**
   * The most basic case for dynamic objects is that we can populate the variable correctly
   * and synchronize its values.
   */
  static bool testMakeObjectVariable(){
    return testMakeObjectVariableImpl();
  }
  
  /**
   * Have ate least one object in the system prior to creating a token. Then show how
   * removal triggers an inconsistency, and insertion of another object fixes it. Also
   * show that specifiying the object prevents propagation if we add another object, but
   * relaxing it will populate the object variable to include the new object.
   */
  static bool testTokenObjectVariable(){
    return testTokenObjectVariableImpl();
  }

  static bool testTokenWithNoObjectOnCreation(){
    return testTokenWithNoObjectOnCreationImpl();
  }
};

class TokenTest {
public:
  
  static bool test() {
    runTest(testBasicTokenAllocation);
    runTest(testBasicTokenCreation);
    runTest(testStateModel);
    runTest(testMasterSlaveRelationship);
    runTest(testBasicMerging);
    runTest(testConstraintMigrationDuringMerge);
    runTest(testNonChronGNATS2439);
    runTest(testMergingPerformance);
    runTest(testTokenCompatibility);
    runTest(testTokenFactory);
    runTest(testCorrectSplit_Gnats2450);
    return(true);
  }
  
private:
  
  static bool testBasicTokenAllocation() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testBasicTokenAllocationImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testBasicTokenCreation() {           
    bool retval = false;
    DEFAULT_SETUP(ce,db, false);
    retval = testBasicTokenCreationImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;                                                                         
  }                            

  static bool testStateModel(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testStateModelImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testMasterSlaveRelationship(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testMasterSlaveRelationshipImpl(ce, db);
    DEFAULT_TEARDOWN();
    // Remainder should be cleaned up automatically.
    return retval;
  }

  static bool testBasicMerging(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testBasicMergingImpl(ce, db);
    DEFAULT_TEARDOWN();
    // Deletion will now occur and test proper cleanup.
    return retval;
  }

  // This test has been fixed by line 56 in MergeMemento.cc.
  // If we invert the order of the splits at the end of this test, the code
  // will error out.

  static bool testConstraintMigrationDuringMerge() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testConstraintMigrationDuringMergeImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testNonChronGNATS2439() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testNonChronGNATS2439Impl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  // add backtracking and longer chain, also add a before constraint
  static bool testMergingPerformance(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testMergingPerformanceImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }    

  static bool testTokenCompatibility(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testTokenCompatibilityImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testTokenFactory(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testTokenFactoryImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  /**
   * @brief Tests that a split will not cause the specified domain of the merged token
   * to be relaxed to the base domain.
   */
  static bool testCorrectSplit_Gnats2450(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testCorrectSplit_Gnats2450impl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }
};

class TimelineTest {
public:
  static bool test(){
    runTest(testFullInsertion);
    runTest(testBasicInsertion);
    runTest(testObjectTokenRelation);
    runTest(testTokenOrderQuery);
    runTest(testEventTokenInsertion);
    runTest(testNoChoicesThatFit);
    return true;
  }

private:
  static bool testBasicInsertion(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testBasicInsertionImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testObjectTokenRelation(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testObjectTokenRelationImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testTokenOrderQuery(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testTokenOrderQueryImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testEventTokenInsertion(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testEventTokenInsertionImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testFullInsertion(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testFullInsertionImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testNoChoicesThatFit(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testNoChoicesThatFitImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }
};

class DbClientTest {
public:
  static bool test(){
    runTest(testFactoryMethods);
    runTest(testBasicAllocation);
    runTest(testPathBasedRetrieval);
    return true;
  }
private:
  static bool testFactoryMethods(){
    std::vector<ConstructorArgument> arguments;
    IntervalIntDomain arg0(10, 10, "int");
    LabelSet arg1(LabelStr("Label"), "string");
    arguments.push_back(ConstructorArgument(arg0.getTypeName(), &arg0)); 
    arguments.push_back(ConstructorArgument(arg1.getTypeName(), &arg1));
    LabelStr factoryName = ObjectFactory::makeFactoryName(LabelStr("Foo"), arguments);
    assert(factoryName == LabelStr("Foo:int:string"));
    return true;
  }

  static bool testBasicAllocation(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testBasicAllocationImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testPathBasedRetrieval(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testPathBasedRetrievalImpl(ce, db);
    DEFAULT_TEARDOWN();
    return retval;
  }
};



int main() {
  initDbModuleTests();
  for (int i=0;i<1;i++){
    runTestSuite(SchemaTest::test);
    runTestSuite(ObjectTest::test);
    runTestSuite(TokenTest::test);
    runTestSuite(TimelineTest::test);
    runTestSuite(DbClientTest::test);
    std::cout << "Finished" << std::endl;
  } 
  ConstraintLibrary::purgeAll();
}
