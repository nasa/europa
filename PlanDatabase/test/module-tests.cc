#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "Token.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "DbLogger.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "DbClientTransactionLog.hh"
#include "DbClientTransactionPlayer.hh"

#include "DbClient.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "StringDomain.hh"
#include "SymbolDomain.hh"

#include "TypeFactory.hh"
#include "EnumeratedTypeFactory.hh"

#include "TestSupport.hh"
#include "PlanDatabaseTestSupport.hh"
#include "PlanDatabaseWriter.hh"

#include <iostream>
#include <iomanip>
#include <string>

class SchemaTest {
public:
  static bool test() {
    runTest(testPrimitives);
    runTest(testEnumerations);
    runTest(testObjectTypeRelationships);
    runTest(testObjectPredicateRelationships);
    runTest(testPredicateParameterAccessors);
    return(true);
  }

private:

  static bool testPrimitives(){
    SCHEMA->reset();
    SCHEMA->addPrimitive("int");
    SCHEMA->addPrimitive("float");
    SCHEMA->addPrimitive("bool");
    SCHEMA->addPrimitive("string");
    assertTrue(SCHEMA->isPrimitive("int"));
    assertTrue(SCHEMA->isPrimitive("float"));
    assertTrue(SCHEMA->isPrimitive("bool"));
    assertTrue(SCHEMA->isPrimitive("string"));
    assertTrue(SCHEMA->isType("int"));
    assertFalse(SCHEMA->isPrimitive("strong"));
    return true;
  }

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
   * Have at least one object in the system prior to creating a token. Then show how
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

/**
 * @class DbTransPlayerTest
 * Test the DbClientTransactionPlayer class's interface and semantics in minor ways.
 */
class DbTransPlayerTest {

public:

  /** Run the tests. */
  static bool test() {
    runTest(testImpl);
    runTest(provokeErrors);
    return(true);
  }

  /*
   * For lists of arguments, by type and name (or type and string (even if "1") value).
   * @note This has a different use and purpose than a list or vector of ConstructorArgument.
   * That is for a type name and abstract domain; this is for a type name and a variable name.
   */
  typedef std::list<std::pair<std::string, std::string> > ArgList;
  typedef ArgList::const_iterator ArgIter;

  /** Run all tests within this class that do not try to provoke errors. */
  static bool testImpl() {
    DEFAULT_SETUP(ce, db, false);
    s_ce = ce;
    s_db = db;
    s_dbPlayer = new DbClientTransactionPlayer((s_db)->getClient());
    assertTrue(s_dbPlayer != 0);

    /* This does not use REGISTER_TYPE_FACTORY to avoid depending on anything under PLASMA/NDDL. */
    new EnumeratedTypeFactory("Locations", "Locations", LocationsBaseDomain());

    // new TestClass2Factory("TestClass2");
    REGISTER_OBJECT_FACTORY(TestClass2Factory, TestClass2);
    // new TestClass2Factory("TestClass2:STRING_ENUMERATION:INT_INTERVAL:REAL_INTERVAL:Locations");
    REGISTER_OBJECT_FACTORY(TestClass2Factory, TestClass2:STRING_ENUMERATION:INT_INTERVAL:REAL_INTERVAL:Locations);

    /* Token factory for predicate Sample */
    new TestClass2::Sample::Factory();

    /* Initialize state-domain-at-creation of mandatory and rejectable tokens.  Const after this. */
    s_mandatoryStateDom.insert(Token::ACTIVE);
    s_mandatoryStateDom.insert(Token::MERGED);
    s_mandatoryStateDom.close();
    s_rejectableStateDom.insert(Token::ACTIVE);
    s_rejectableStateDom.insert(Token::MERGED);
    s_rejectableStateDom.insert(Token::REJECTED);
    s_rejectableStateDom.close();

    /* Initialize the list of temporal relations.  Const after this. */
    //!!This list and their meanings should be documented (now GNATS 2697).
    //!!  Draft is now in CVS as wedgingt/Europa/TemporalRelations as part of GNATS 2697.
    //!!This list should also be available via a method, including count of bounds values allowed.
    //!!  Noted within GNATS 2572.
    s_tempRels.insert(LabelStr("after"));
    s_tempRels.insert(LabelStr("any"));
    s_tempRels.insert(LabelStr("before"));
    s_tempRels.insert(LabelStr("contained_by"));
    s_tempRels.insert(LabelStr("contains"));
    s_tempRels.insert(LabelStr("contains_end"));
    s_tempRels.insert(LabelStr("contains_start"));
    s_tempRels.insert(LabelStr("ends"));
    s_tempRels.insert(LabelStr("ends_after"));
    s_tempRels.insert(LabelStr("ends_after_start"));
    s_tempRels.insert(LabelStr("ends_before"));
    s_tempRels.insert(LabelStr("ends_during"));
    s_tempRels.insert(LabelStr("equal"));
    s_tempRels.insert(LabelStr("equals"));
    s_tempRels.insert(LabelStr("meets"));
    s_tempRels.insert(LabelStr("met_by"));
    s_tempRels.insert(LabelStr("paralleled_by"));
    s_tempRels.insert(LabelStr("parallels"));
    s_tempRels.insert(LabelStr("starts"));
    s_tempRels.insert(LabelStr("starts_after"));
    s_tempRels.insert(LabelStr("starts_before"));
    s_tempRels.insert(LabelStr("starts_before_end"));
    s_tempRels.insert(LabelStr("starts_during"));
    //!!parallels? precedes? succeeds?
    //!!each with explicit bounds?  The player does not appear to have a syntax for explicit bounds.

    //!!Delete this next line once the tests are debugged and print nothing when passing
    std::cout << '\n';

    testDefineEnumeration();
    testCreateVariable();
    testDefineClass();
    testCreateObject();
    testSpecifyVariable();
    testResetVariable();
    testCreateTokens();
    testInvokeConstraint();
    testConstrain();
    testFree();
    testActivate();
    testMerge();
    testReject();
    testCancel();

    TokenFactory::purgeAll();
    TypeFactory::purgeAll();
    delete s_dbPlayer;
    DEFAULT_TEARDOWN();
    return(true);
  }

  /** Run all tests within this class that do try to provoke errors. */
  static bool provokeErrors() {
    //!! None implemented yet
    return(true);
  }

  typedef SymbolDomain Locations;

  /**
   * Locations enumeration's base domain, as required by the schema.
   * @note Copied from System/test/basic-model-transaction.cc
   * as created from basic-model-transaction.nddl v1.3 with the NDDL compiler.
   */
  static const Locations& LocationsBaseDomain() {
    static Locations sl_enum("Locations");
    if (sl_enum.isOpen()) {
      sl_enum.insert(LabelStr("Hill"));
      sl_enum.insert(LabelStr("Rock"));
      sl_enum.insert(LabelStr("Lander"));
      sl_enum.close();
    }
    return(sl_enum);
  }

  class TestClass2;
  typedef Id<TestClass2> TestClass2Id;

  class TestClass2Domain : public EnumeratedDomain {
  public:
    TestClass2Domain()
      : EnumeratedDomain(false, "TestClass2") {
    }
    TestClass2Domain(TestClass2Id value)
      : EnumeratedDomain((double)value, false, "TestClass2") {
    }
    TestClass2Domain(TestClass2Id value, const char* typeName)
      : EnumeratedDomain((double)value, false, typeName) {
    }
  };

  class TestClass2 : public Timeline {
  public:
    TestClass2(const PlanDatabaseId& planDatabase, const LabelStr& name)
      : Timeline(planDatabase, "TestClass2", name, true) {
    }
    TestClass2(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
      : Timeline(planDatabase, type, name, true) {
    }
    TestClass2(const ObjectId& parent, const LabelStr& name)
      : Timeline(parent, "TestClass2", name, true) {
    }
    TestClass2(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
      : Timeline(parent, type, name, true) {
    }
    void handleDefaults(bool autoClose = false) {
      if (m_int1.isNoId())
        m_int1 = addVariable(IntervalIntDomain(), "int1");
      if (m_float2.isNoId())
        m_float2 = addVariable(IntervalDomain(), "float2");
      if (m_where.isNoId())
        m_where = addVariable(LocationsBaseDomain(), "where");
      if (autoClose || LocationsBaseDomain().isClosed())
        close();
    }
    void constructor() {
      handleDefaults();
    }
    //!! works: Id< Variable< IntervalIntDomain > > m_int1;
    ConstrainedVariableId m_int1;
    //!! works: Id< Variable< IntervalDomain > > m_float2;
    ConstrainedVariableId m_float2;
    //!! does not work, at least on Token (RedHat Linux 9.0, g++v3.3.3, java1.4.2_04):
    //!!  Id< Variable< Locations > > m_where;
    ConstrainedVariableId m_where;

    // Borrowed from System/test/backtr.{nddl,cc,hh,xml}
    class Sample : public IntervalToken {
    public:
      Sample(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false)
        : IntervalToken(planDb, name, rejectable, IntervalIntDomain(), IntervalIntDomain(),
                        IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false) {
        handleDefaults();
      }
      Sample(const TokenId& parent, const LabelStr& name, const LabelStr& relation)
        : IntervalToken(parent, relation, name, IntervalIntDomain(), IntervalIntDomain(),
                        IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false) {
        handleDefaults();
      }
      void handleDefaults() {
        if (m_x.isNoId())
          m_x = addParameter(IntervalDomain(), "m_x");
        if (m_y.isNoId())
          m_y = addParameter(IntervalDomain(), "m_y");
        if (m_closest.isNoId())
          m_closest = addParameter(LocationsBaseDomain(), "m_closest");
        close();
      }
      // Would do:
      // DECLARE_TOKEN_FACTORY(TestClass2::Sample, TestClass2.Sample);
      // ... but that is in NDDL/core/NddlUtils.hh, which this should not depend on, so:
      class Factory : public ConcreteTokenFactory {
      public:
        Factory()
          : ConcreteTokenFactory(LabelStr("TestClass2.Sample")) {
        }
      private:
        TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false) const {
          TokenId token = (new Sample(planDb, name, rejectable))->getId();
          return(token);
        }
        TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const {
          TokenId token = (new Sample(master, name, relation))->getId();
          return(token);
        }
      };
      ConstrainedVariableId m_x; // IntervalDomain
      ConstrainedVariableId m_y; // IntervalDomain
      ConstrainedVariableId m_closest; // Locations: SymbolDomain
    };
    typedef Id<Sample> SampleId;

  };

  class TestClass2Factory: public ConcreteObjectFactory {
  public:
    TestClass2Factory(const LabelStr& name)
      : ConcreteObjectFactory(name) {
    }
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType,
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      assertTrue(arguments.size() == 0 || arguments.size() == 4);
      if (arguments.size() == 4) {
        //!!I'm not sure why this first one is passed in; it appears to be the object's type info.
        //!!--wedgingt@email.arc.nasa.gov 2004 Nov 1
        assertTrue(arguments[0].first == LabelStr(StringDomain::getDefaultTypeName()));
        assertTrue(arguments[1].first == LabelStr(IntervalIntDomain::getDefaultTypeName()));
        assertTrue(arguments[2].first == LabelStr(IntervalDomain::getDefaultTypeName()));
        assertTrue(arguments[3].first == LabelStr("Locations"));
      }
      TestClass2Id instance = (new TestClass2(planDb, objectType, objectName))->getId();
      instance->handleDefaults();
      std::vector<ConstrainedVariableId> vars = instance->getVariables();
      for (unsigned int i = 1; i < arguments.size(); i++)
        vars[i - 1]->specify(*(arguments[i].second));
      std::cout << "TestClass2 objectId " << instance->getId() << ' ' << instance->getName().toString()
                << " has varIds " << vars[0] << ' ' << vars[1] << ' ' << vars[2] << '\n';
      return(instance);
    }
  };

  /**
   * Run a single test, reading the XML from the given string.
   * @param xml The XML to give to the player to test it.
   * @param file The source file that creates the XML string, for use in error messages.
   * @param line The source line that creates the XML string, for use in error messages.
   */
  static void testPlayingXML(const std::string& xml, const char *file, const int& line);

  /**
   * @def TEST_PLAYING_XML
   * Call testPlayingXML with __FILE__ and __LINE__.
   */
#define TEST_PLAYING_XML(xml) (testPlayingXML(xml, __FILE__, __LINE__))

  /** Test defining an enumeration. */
  static void testDefineEnumeration() {
    std::list<std::string> locs;
    locs.push_back(std::string("Hill"));
    locs.push_back(std::string("Rock"));
    locs.push_back(std::string("Lander"));

    /* Build it in the schema first. */
    //!!An easier way to do this would be nice.
    //!!  E.g., a member function that accepted the type name and the corresponding base domain.
    //!!  Per Tania, created a change request GNATS for such a method (GNATS 2698).
    s_db->getSchema()->addEnum("Locations");
    s_db->getSchema()->addValue("Locations", (double)LabelStr("Hill"));
    s_db->getSchema()->addValue("Locations", (double)LabelStr("Rock"));
    s_db->getSchema()->addValue("Locations", (double)LabelStr("Lander"));

    /* Create the XML string and play it. */
    TEST_PLAYING_XML(buildXMLEnumStr(std::string("Locations"), locs, __FILE__, __LINE__));
    // Nothing to verify, since the player cannot actually create enumerations.
  }

  /** Test creating variables. */
  static void testCreateVariable() {
    TEST_PLAYING_XML(buildXMLNameTypeStr("var", "g_int", IntervalIntDomain::getDefaultTypeName().toString(), __FILE__, __LINE__));
    TEST_PLAYING_XML(buildXMLNameTypeStr("var", "g_float", IntervalDomain::getDefaultTypeName().toString(), __FILE__, __LINE__));
    TEST_PLAYING_XML(buildXMLNameTypeStr("var", "g_location", "Locations", __FILE__, __LINE__));

    //!!Other types: symbols, objects, etc.
      
    ConstrainedVariableSet allVars = s_ce->getVariables();
    ConstrainedVariableSet::iterator varIter = allVars.begin();
    ConstrainedVariableId g_int2, g_float2, g_location2;
    for ( ; varIter != allVars.end(); varIter++)
      if ((*varIter)->getName() == LabelStr("g_int"))
        if (sg_int.isNoId())
          sg_int = *varIter;
        else
          g_int2 = *varIter;
      else
        if ((*varIter)->getName() == LabelStr("g_float"))
          if (sg_float.isNoId())
            sg_float = *varIter;
          else
            g_float2 = *varIter;
        else
          if ((*varIter)->getName() == LabelStr("g_location"))
            if (sg_location.isNoId())
              sg_location = *varIter;
            else
              g_location2 = *varIter;
    assertTrue(!sg_int.isNoId() && sg_int.isValid());
    assertTrue(sg_int->specifiedDomain() == IntervalIntDomain());
    assertTrue(!sg_float.isNoId() && sg_float.isValid());
    assertTrue(sg_float->specifiedDomain() == IntervalDomain());
    assertTrue(!sg_location.isNoId() && sg_location.isValid());
    assertTrue(sg_location->specifiedDomain() == LocationsBaseDomain());
    assertTrue(g_int2.isNoId());
    assertTrue(g_float2.isNoId());
    assertTrue(g_location2.isNoId());
  }

  /** Test defining classes. */
  static void testDefineClass() {
    s_db->getSchema()->addObjectType("TestClass1");
    assertTrue(s_db->getSchema()->isObjectType("TestClass1"));

    TEST_PLAYING_XML(buildXMLNameStr("class", "TestClass1", __FILE__, __LINE__));

    s_db->getSchema()->addObjectType("TestClass2");
    assertTrue(s_db->getSchema()->isObjectType("TestClass2"));
    s_db->getSchema()->addMember("TestClass2", IntervalIntDomain::getDefaultTypeName().toString(), "int1");
    s_db->getSchema()->addMember("TestClass2", IntervalDomain::getDefaultTypeName().toString(), "float2");
    s_db->getSchema()->addMember("TestClass2", "Locations", "where");
    s_db->getSchema()->addPredicate("TestClass2.Sample");
    s_db->getSchema()->addMember("TestClass2.Sample", IntervalDomain::getDefaultTypeName().toString(), "m_x");
    s_db->getSchema()->addMember("TestClass2.Sample", IntervalDomain::getDefaultTypeName().toString(), "m_y");
    s_db->getSchema()->addMember("TestClass2.Sample", "Locations", "m_closest");

    ArgList args;
    args.push_back(std::make_pair(IntervalIntDomain::getDefaultTypeName().toString(), std::string("int1")));
    args.push_back(std::make_pair(IntervalDomain::getDefaultTypeName().toString(), std::string("float2")));
    args.push_back(std::make_pair(std::string("Locations"), std::string("where")));
    //!!BoolDomain::getDefaultTypeName()
    TEST_PLAYING_XML(buildXMLCreateClassStr("TestClass2", args, __FILE__, __LINE__));
    // Nothing to verify, since the player cannot actually create classes.
  }

  /** Helper function to clean up after otherwise memory leaking calls to new. */
  inline static void cleanDomains(std::vector<AbstractDomain*>& doms) {
    for (unsigned i = 0; i < doms.size(); i++)
      delete doms[i];
    doms.clear();
  }

  /**
   * Test creating an object.
   * @note Side-effect: leaves two objects, "testObj2a" and "testObj2b", in plan db.
   */
  static void testCreateObject() {
    std::vector<AbstractDomain*> domains;
    domains.push_back(new IntervalIntDomain(1));
    domains.push_back(new IntervalDomain(1.414));
    domains.push_back(new Locations(LabelStr("Hill"), "Locations"));
    //!!Other types?  Has to match class's definition in the schema.
    TEST_PLAYING_XML(buildXMLCreateObjectStr("TestClass2", "testObj2a", domains));
    cleanDomains(domains);
    ObjectId obj2a = s_db->getObject("testObj2a");
    assertTrue(!obj2a.isNoId() && obj2a.isValid());
    assertTrue(obj2a->getType() == LabelStr("TestClass2"));
    assertTrue(obj2a->getName() == LabelStr("testObj2a"));
    std::vector<ConstrainedVariableId> obj2vars = obj2a->getVariables();
    assertTrue(obj2vars.size() == 3);
    for (int i = 0; i < 3; i++) {
      ConstrainedVariableId var = obj2vars[i];
      assertTrue(!var.isNoId() && var.isValid());
      assertTrue(var->isValid());
      std::set<ConstraintId> constraints;
      var->constraints(constraints);
      assertTrue(constraints.empty());
      assertTrue(var->getParent() == obj2a);
      assertTrue(var->derivedDomain() == var->specifiedDomain());
      assertTrue(i == var->getIndex());
      std::cout << "testObj2a var[" << i << ' ' << var->getName().toString() << "] has specDom " << var->specifiedDomain() << '\n';
      switch (i) {
      case 0:
        assertTrue(var->specifiedDomain() == IntervalIntDomain(1));
        break;
      case 1:
        assertTrue(var->specifiedDomain() == IntervalDomain(1.414));
        break;
      case 2:
        assertTrue(var->specifiedDomain() == SymbolDomain((double)LabelStr("Hill"), "Locations"));
        break;
      default:
        assertTrue(false, "erroneous variable index within obj2a");
      }
    }
    domains.push_back(new IntervalIntDomain(2, 14));
    domains.push_back(new IntervalDomain(1.414, 3.14159265358979));
    std::list<double> locs;
    locs.push_back(LabelStr("Hill"));
    locs.push_back(LabelStr("Rock"));
    domains.push_back(new Locations(locs, "Locations"));
    Locations toCompare(locs, "Locations");
    //!!Other types?  Has to match class's definition in the schema.
    TEST_PLAYING_XML(buildXMLCreateObjectStr("TestClass2", "testObj2b", domains));
    cleanDomains(domains);
    ObjectId obj2b = s_db->getObject("testObj2b");
    assertTrue(!obj2b.isNoId() && obj2b.isValid());
    assertTrue(obj2b->getType() == LabelStr("TestClass2"));
    assertTrue(obj2b->getName() == LabelStr("testObj2b"));
    obj2vars = obj2b->getVariables();
    assertTrue(obj2vars.size() == 3);
    for (int i = 0; i < 3; i++) {
      ConstrainedVariableId var = obj2vars[i];
      assertTrue(!var.isNoId() && var.isValid());
      assertTrue(var->isValid());
      std::set<ConstraintId> constraints;
      var->constraints(constraints);
      assertTrue(constraints.empty());
      assertTrue(var->getParent() == obj2b);
      assertTrue(var->derivedDomain() == var->specifiedDomain());
      assertTrue(i == var->getIndex());
      std::cout << "testObj2b var[" << i << ' ' << var->getName().toString() << "] has specDom " << var->specifiedDomain() << '\n';
      switch (i) {
      case 0:
        assertTrue(var->specifiedDomain() == IntervalIntDomain(2, 14));
        break;
      case 1:
        assertTrue(var->specifiedDomain() == IntervalDomain(1.414, 3.14159265358979));
        break;
      case 2:
        assertTrue(var->specifiedDomain() == toCompare);
        break;
      default:
        assertTrue(false, "erroneous variable index within obj2b");
      }
    }
    //!!ObjectSet objects = s_db->getObjects();
    //!!std::cout << "\n  PlanDB objects are:";
    //!!std::set<ObjectId, EntityComparator<ObjectId> >::const_iterator it = objects.begin();
    //!!for ( ; it != objects.end(); it++)
    //!!  std::cout << "\n    id " << *it << " name " << (*it)->getName().toString();
    //!!std::cout << std::endl;
    //!!Mix of singleton and non-singleton member vars in testObj2c?
    //!!Find each in PlanDB just after each is built
  }

  /** Test specifying variables. */
  static void testSpecifyVariable() {
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_int, IntervalIntDomain(-MAX_INT, PLUS_INFINITY)));
    assertTrue(sg_int->specifiedDomain() == IntervalIntDomain(-MAX_INT, PLUS_INFINITY));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_int, IntervalIntDomain(-1000, MAX_INT)));
    assertTrue(sg_int->specifiedDomain() == IntervalIntDomain(-1000, MAX_INT));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_int, IntervalIntDomain(-5)));
    assertTrue(sg_int->specifiedDomain() == IntervalIntDomain(-5));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_float, IntervalDomain(MINUS_INFINITY, MAX_INT)));
    assertTrue(sg_float->specifiedDomain() == IntervalDomain(MINUS_INFINITY, MAX_INT));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_float, IntervalDomain(MINUS_INFINITY, 3.14)));
    assertTrue(sg_float->specifiedDomain() == IntervalDomain(MINUS_INFINITY, 3.14));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_float, IntervalDomain(-MAX_INT, 3.14)));
    assertTrue(sg_float->specifiedDomain() == IntervalDomain(-MAX_INT, 3.14));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_float, IntervalDomain(-3.1415e6, 2.78)));
    assertTrue(sg_float->specifiedDomain() == IntervalDomain(-3.1415e6, 2.78));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_float, IntervalDomain(-10.0, 1.41)));
    assertTrue(sg_float->specifiedDomain() == IntervalDomain(-10.0, 1.41));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_float, IntervalDomain(-5.0)));
    assertTrue(sg_float->specifiedDomain() == IntervalDomain(-5.0));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_location, LocationsBaseDomain()));
    assertTrue(sg_location->specifiedDomain() == LocationsBaseDomain());
    std::list<double> locs;
    locs.push_back(LabelStr("Lander"));
    locs.push_back(LabelStr("Hill"));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_location, Locations(locs, "Locations")));
    assertTrue(sg_location->specifiedDomain() == Locations(locs, "Locations"));
    locs.pop_front();
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_location, Locations(locs, "Locations")));
    assertTrue(sg_location->specifiedDomain() == Locations(locs, "Locations"));

    //!!other global vars, object vars, object member vars, token parameter vars, token special vars, etc.
  }

  /** Test resetting variables. */
  static void testResetVariable() {
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_int));
    assertTrue(sg_int->specifiedDomain() == IntervalIntDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_float));
    assertTrue(sg_float->specifiedDomain() == IntervalDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_location));
    assertTrue(sg_location->specifiedDomain() == LocationsBaseDomain());
    ObjectId obj2b = s_db->getObject("testObj2b");
    assertTrue(!obj2b.isNoId() && obj2b.isValid());
    assertTrue(obj2b->getType() == LabelStr("TestClass2"));
    assertTrue(obj2b->getName() == LabelStr("testObj2b"));
    std::vector<ConstrainedVariableId> obj2vars = obj2b->getVariables();
    assertTrue(obj2vars.size() == 3);
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[0]));
    assertTrue(obj2vars[0]->specifiedDomain() == IntervalIntDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[1]));
    assertTrue(obj2vars[1]->specifiedDomain() == IntervalDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[2]));
    assertTrue(obj2vars[2]->specifiedDomain() == LocationsBaseDomain());

    //!!other variables, as for specify
  }

  /** Test invoking constraints, including "special cases" (as the player calls them). */
  static void testInvokeConstraint() {
    // First section: constraints between variables
    ObjectId obj2b = s_db->getObject("testObj2b");
    assertTrue(!obj2b.isNoId() && obj2b.isValid());
    assertTrue(obj2b->getType() == LabelStr("TestClass2"));
    assertTrue(obj2b->getName() == LabelStr("testObj2b"));
    std::vector<ConstrainedVariableId> obj2vars = obj2b->getVariables();
    assertTrue(obj2vars.size() == 3);

    // First constraint
    std::list<ConstrainedVariableId> vars;
    vars.push_back(sg_int);
    vars.push_back(obj2vars[0]);
    TEST_PLAYING_XML(buildXMLInvokeConstrainVarsStr("Equal", vars));
    std::set<ConstraintId> constraints;
    sg_int->constraints(constraints);
    assertTrue(constraints.size() == 1);
    ConstraintId constr = *(constraints.begin());
    assertTrue(constr->getName() == LabelStr("Equal"));
    assertTrue(constr->getScope().size() == 2);
    assertTrue(constr->isVariableOf(sg_int));
    assertTrue(constr->isVariableOf(obj2vars[0]));
    constraints.clear();
    obj2vars[0]->constraints(constraints);
    assertTrue(constraints.size() == 1);
    assertTrue(constr == *(constraints.begin()));

    // Second constraint
    vars.clear();
    vars.push_back(sg_int);
    vars.push_back(sg_float);
    TEST_PLAYING_XML(buildXMLInvokeConstrainVarsStr("LessThanEqual", vars));
    constraints.clear();
    sg_int->constraints(constraints);
    assertTrue(constraints.size() == 2);
    assertTrue(constraints.find(constr) != constraints.end());
    constraints.erase(constraints.find(constr));
    constr = *(constraints.begin());
    assertTrue(constr->getName() == LabelStr("LessThanEqual"));
    assertTrue(constr->getScope().size() == 2);
    assertTrue(constr->isVariableOf(sg_int));
    assertTrue(constr->isVariableOf(sg_float));
    constraints.clear();
    sg_float->constraints(constraints);
    assertTrue(constraints.size() == 1);
    assertTrue(constr == *(constraints.begin()));

    // Third constraint
    vars.clear();
    vars.push_back(sg_location);
    vars.push_back(obj2vars[2]);
    TEST_PLAYING_XML(buildXMLInvokeConstrainVarsStr("NotEqual", vars));
    constraints.clear();
    sg_location->constraints(constraints);
    assertTrue(constraints.size() == 1);
    constr = *(constraints.begin());
    assertTrue(constr->getName() == LabelStr("NotEqual"));
    assertTrue(constr->getScope().size() == 2);
    assertTrue(constr->isVariableOf(sg_location));
    assertTrue(constr->isVariableOf(obj2vars[2]));
    constraints.clear();
    obj2vars[2]->constraints(constraints);
    assertTrue(constraints.size() == 1);
    assertTrue(constr == *(constraints.begin()));

    // Specifying variables is one of the special cases.
    TEST_PLAYING_XML(buildXMLInvokeSpecifyVariableStr(sg_location, Locations(LabelStr("Hill"), "Locations")));
    assertTrue(sg_location->specifiedDomain() == Locations(LabelStr("Hill"), "Locations"));
    std::list<double> locs;
    locs.push_back(LabelStr("Hill"));
    locs.push_back(LabelStr("Rock"));
    //!!This is not supported yet:
    //!!TEST_PLAYING_XML(buildXMLInvokeSpecifyVariableStr(obj2vars[2], Locations(locs, "Locations")));
    //!!... so do it the other way, at least for now:
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(obj2vars[2], Locations(locs, "Locations")));
    assertTrue(obj2vars[2]->specifiedDomain() == Locations(locs, "Locations"));
    assertTrue(obj2vars[2]->derivedDomain() == Locations(LabelStr("Rock"), "Locations"));

    // Resetting variables via invoke is _not_ supported by the player, so do it the other way:
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_location));
    assertTrue(sg_location->specifiedDomain() == LocationsBaseDomain());
    assertTrue(obj2vars[2]->derivedDomain() == Locations(locs, "Locations"));
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[2]));
    assertTrue(obj2vars[2]->specifiedDomain() == LocationsBaseDomain());
    assertTrue(obj2vars[2]->derivedDomain() == LocationsBaseDomain());

    //!!Most special cases involve tokens: constrain, free, activate, merge, reject, cancel
    //!!  Of these, only activate and constrain are used in any of PLASMA/System/test/*.xml.
    //!!  So try each of those two once here to try to catch problems sooner but presume that
    //!!    the other variants of <invoke> are obsolete, at least for now.

    // Activate a token.  Will have to create one and identify it first.
    TokenSet oldTokens = s_db->getTokens();
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", true, "invokeActivateTestToken"));
    TokenSet newTokens = s_db->getTokens();
    assertTrue(oldTokens.size() + 1 == newTokens.size());
    TokenId tok = *(newTokens.begin());
    while (oldTokens.find(tok) != oldTokens.end()) {
      newTokens.erase(newTokens.begin());
      tok = *(newTokens.begin());
    }
    assertTrue(!tok->isActive() && tok->isInactive());
    TEST_PLAYING_XML(buildXMLInvokeActivateTokenStr("invokeActivateTestToken"));
    assertTrue(tok->isActive() && !tok->isInactive());
    // Now, destroy it so it doesn't affect later tests.
    delete (Token*) tok;
    tok = TokenId::noId();

    // Special case #1: close an class object domain
    assertTrue(!s_db->isClosed("TestClass2"));
    TEST_PLAYING_XML("<invoke name=\"close\" identifier=\"TestClass2\"/>");
    assertTrue(s_db->isClosed("TestClass2"));
    std::cout << __FILE__ << ':' << __LINE__ << ": TestClass2 object domain is "
              << ObjectDomain("TestClass2") << " (size " << ObjectDomain("TestClass2").getSize()
              << "); should be 2 members\n";

    //!!This is failing, despite the prior checks passing, because the domain is still open.
    //!!assertTrue(ObjectDomain("TestClass2").getSize() == 2);
    if (ObjectDomain("TestClass2").isOpen())
      std::cout << __FILE__ << ':' << __LINE__ << ": TestClass2 base domain is still open, despite plan database saying otherwise\n";
    //!!See if closing the entire database takes care of this as well:

    /* Closing the database is the last special case. */
    TEST_PLAYING_XML("<invoke name=\"close\"/>");

    //!!See just above
    std::cout << __FILE__ << ':' << __LINE__ << ": After closing db, TestClass2 object domain is "
              << ObjectDomain("TestClass2") << " (size " << ObjectDomain("TestClass2").getSize()
              << "); should be 2 members\n";
    //!!Still fails
    //!!assertTrue(ObjectDomain("TestClass2").getSize() == 2);
    if (ObjectDomain("TestClass2").isOpen())
      std::cout << __FILE__ << ':' << __LINE__ << ": TestClass2 base domain is still open, despite plan database saying otherwise\n";
  }

  /** Test creating tokens. */
  static void testCreateTokens() {
    testCreateGoalTokens();
    testCreateSubgoalTokens();
  }

  /**
   * Test that the given token matches the criteria.
   * @note If only used as condition in assertTrue() (or changed to
   * return void), this could itself use 'assertTrue(cond)' rather
   * than 'if (!cond) return(false);', making which condition failed
   * obvious.
   */
  static bool checkToken(const TokenId& token, const LabelStr& name, const LabelStr& predName,
                         const TokenId& master, const StateDomain& stateDom) {
    if (token.isNoId() || !token.isValid())
      return(false);
    if (token->getName() != name)
      return(false);
    if (token->getPredicateName() != predName)
      return(false);
    StateVarId state = token->getState();
    return(stateDom == state->getDerivedDomain());
  }

  /** Test creating goal (master or orphan) tokens. */
  static void testCreateGoalTokens() {
    /* Create a mandatory token. */
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", true, "sample1"));
    /* Verify it. */
    TokenSet tokens = s_db->getTokens();
    assertTrue(tokens.size() == 1);
    TokenId token = *(tokens.begin());
    assertTrue(checkToken(token, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                          TokenId::noId(), s_mandatoryStateDom));

    /* Create a rejectable token. */
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", false, "sample2"));
    /* Find and verify it. */
    tokens = s_db->getTokens();
    assertTrue(tokens.size() == 2);
    TokenId token2 = *(tokens.begin());
    if (token2 == token) {
      tokens.erase(tokens.begin());
      token2 = *(tokens.begin());
    }
    assertTrue(checkToken(token2, LabelStr("TestClass2.Sample"), 
			  LabelStr("TestClass2.Sample"),
			  TokenId::noId(), s_rejectableStateDom));

    //!!other predicates?
  }

  /**
   * Test creating subgoal (slave or child) tokens.
   */
  static void testCreateSubgoalTokens() {
    TokenSet oldTokens = s_db->getTokens();
    /* Create a new token to use to test creating constraints between tokens. */
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", true, "sample3"));
    TokenSet currentTokens = s_db->getTokens();
    assertTrue(oldTokens.size() + 1 == currentTokens.size());
    TokenId goal = *(currentTokens.begin());
    while (oldTokens.find(goal) != oldTokens.end()) {
      currentTokens.erase(currentTokens.begin());
      goal = *(currentTokens.begin());
    }
    assertTrue(checkToken(goal, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                          TokenId::noId(), s_mandatoryStateDom));
    oldTokens.insert(goal);
    /* Create a subgoal for each temporal relation. */
    //!!should do at least two for each temporal relation between master and slave: with and without explicit temporal bounds
    std::set<LabelStr>::const_iterator which = s_tempRels.begin();
    for ( ; which != s_tempRels.end(); which++) {
      std::string subgoalName = "subgoal1" + which->toString();
      /* Create a new subgoal token. */
      TEST_PLAYING_XML(buildXMLCreateSubgoalStr("sample1", "TestClass2.Sample", subgoalName, *which));
      /* Find it. */
      currentTokens = s_db->getTokens();
      assertTrue(oldTokens.size() + 1 == currentTokens.size());
      TokenId subgoal = *(currentTokens.begin());
      while (oldTokens.find(subgoal) != oldTokens.end()) {
        currentTokens.erase(currentTokens.begin());
        subgoal = *(currentTokens.begin());
      }
      /* Check it. */
      //!!Should use the master token's Id rather than TokenId::noId() here, but the player doesn't behave that way.
      //!!Is that a bug in the player or not?
      //!!  May mean that this is an inappropriate overloading of the '<goal>' XML tag per Tania and I (17 Nov 2004)
      assertTrue(checkToken(subgoal, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                            TokenId::noId(), s_mandatoryStateDom));
      assertTrue(verifyTokenRelation(goal, subgoal, *which));
      /* Update list of old tokens. */
      oldTokens.insert(subgoal);
    }
  }

  /**
   * Test constraining tokens to an object and ordering tokens on an object.
   */
  static void testConstrain() {
    TokenId constrainedToken = createToken("constrainedSample", true);
    std::cout << __FILE__ << ':' << __LINE__ << ": constrainedToken is " << constrainedToken << '\n';
    std::cout << __FILE__ << ':' << __LINE__ << ": constrainedSample's derived object domain is " << constrainedToken->getObject()->derivedDomain() << '\n';
    /* Activate it. */
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "constrainedSample", ""));
    /* Verify that it is not attached to an object. */
    std::cout << __FILE__ << ':' << __LINE__ << ": constrainedSample's derived object domain is " << constrainedToken->getObject()->derivedDomain() << '\n';
    assertTrue(!constrainedToken->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
    /* Get an existing object.  See testCreateObject(). */
    ObjectId obj2b = s_db->getObject("testObj2b");
    assertTrue(!obj2b.isNoId() && obj2b.isValid());
    assertTrue(obj2b->getType() == LabelStr("TestClass2"));
    assertTrue(obj2b->getName() == LabelStr("testObj2b"));
    /* Create the constraint. */
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2b", "constrainedSample", ""));
    /* Verify its intended effect. */
    std::cout << __FILE__ << ':' << __LINE__ << ": constrainedSample's derived object domain is " << constrainedToken->getObject()->derivedDomain() << '\n';
    assertTrue(constrainedToken->getObject()->derivedDomain().isSingleton(), "player did not constrain token to one object");
    ObjectDomain objDom2b(obj2b, "TestClass2");;
    assertTrue(constrainedToken->getObject()->derivedDomain() == objDom2b, "player did not constrain token to expected object");
    /* Leave it in plan db for testFree(). */

    /* Again, but also constrain it with the prior token. */
    TokenId constrained2 = createToken("constrainedSample2", true);
    std::cout << __FILE__ << ':' << __LINE__ << ": constrained2 is " << constrained2 << '\n';
    std::cout << __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain() << '\n';
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "constrainedSample2", ""));
    std::cout << __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain() << '\n';
    assertTrue(!constrained2->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2b", "constrainedSample2", "constrainedSample"));
    std::cout << __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain() << '\n';
    assertTrue(constrained2->getObject()->derivedDomain().isSingleton(), "player did not constrain token to one object");
    assertTrue(constrained2->getObject()->derivedDomain() == objDom2b, "player did not constrain token to expected object");
    assertTrue(verifyTokenRelation(constrainedToken, constrained2, "before")); //!! "precedes" ?
    /* Leave them in plan db for testFree(). */

    /* Create two rejectable tokens and do the same tests, but with testObj2a. */
    TokenId rejectable = createToken("rejectableConstrainedSample", false);
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable is " << rejectable << '\n';
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain() << '\n';
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "rejectableConstrainedSample", ""));
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain() << '\n';
    assertTrue(!rejectable->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2a", "rejectableConstrainedSample", ""));
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain() << '\n';
    assertTrue(rejectable->getObject()->derivedDomain().isSingleton(), "player did not constrain token to one object");
    ObjectId obj2a = s_db->getObject("testObj2a");
    assertTrue(!obj2a.isNoId() && obj2a.isValid());
    assertTrue(obj2a->getType() == LabelStr("TestClass2"));
    assertTrue(obj2a->getName() == LabelStr("testObj2a"));
    ObjectDomain objDom2a(obj2a, "TestClass2");;
    assertTrue(rejectable->getObject()->derivedDomain() == objDom2a, "player did not constrain token to expected object");
    TokenId rejectable2 = createToken("rejectable2", false);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "rejectable2", ""));
    assertTrue(!rejectable2->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2a", "rejectable2", "rejectableConstrainedSample"));
    assertTrue(rejectable2->getObject()->derivedDomain().isSingleton(), "player did not constrain token to one object");
    assertTrue(rejectable2->getObject()->derivedDomain() == objDom2a, "player did not constrain token to expected object");
    assertTrue(verifyTokenRelation(rejectable, rejectable2, "before")); //!! "precedes" ?
    /* Leave them in plan db for testFree(). */
  }

  /** Test freeing tokens. */
  static void testFree() {
    ObjectId obj2a = s_db->getObject("testObj2a");
    assertTrue(!obj2a.isNoId() && obj2a.isValid());
    assertTrue(obj2a->getType() == LabelStr("TestClass2"));
    assertTrue(obj2a->getName() == LabelStr("testObj2a"));
    ObjectDomain objDom2a(obj2a, "TestClass2");
    ObjectId obj2b = s_db->getObject("testObj2b");
    assertTrue(!obj2b.isNoId() && obj2b.isValid());
    assertTrue(obj2b->getType() == LabelStr("TestClass2"));
    assertTrue(obj2b->getName() == LabelStr("testObj2b"));
    ObjectDomain objDom2b(obj2b, "TestClass2");
    TokenSet tokens = obj2b->getTokens();
    std::cout << __FILE__ << ':' << __LINE__ << ": there are " << tokens.size() << " tokens on testObj2b; should be 2.\n";
    /*!!For debugging:
    TokenSet tokens2 = tokens;
    for (int i = 1; !tokens2.empty(); i++) {
      std::cout << __FILE__ << ':' << __LINE__ << ": testFree(): on obj2b, token " << i << " is " << *(tokens2.begin()) << '\n';
      tokens2.erase(tokens2.begin());
    }
    !!*/
    assertTrue(tokens.size() == 2);
    TokenId one = *(tokens.begin());
    assertTrue(!one.isNoId() && one.isValid());
    assertTrue(one->getObject()->derivedDomain().isSingleton());
    assertTrue(one->getObject()->derivedDomain() == objDom2b);
    TokenId two = *(tokens.rbegin());
    assertTrue(!two.isNoId() && two.isValid() && one != two);
    assertTrue(two->getObject()->derivedDomain().isSingleton());
    assertTrue(two->getObject()->derivedDomain() == objDom2b);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample", ""));
    assertTrue(!one->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!assertTrue(one->getObject()->derivedDomain() == ObjectDomain("TestClass2"));

    assertTrue(two->getObject()->derivedDomain().isSingleton());
    assertTrue(two->getObject()->derivedDomain() == objDom2b);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample2", ""));
    assertTrue(!one->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!assertTrue(one->getObject()->derivedDomain() == ObjectDomain("TestClass2"));

    assertTrue(!two->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!assertTrue(two->getObject()->derivedDomain() == ObjectDomain("TestClass2"));

    tokens = obj2a->getTokens();
    /*!!For debugging:
    TokenSet tokens3 = tokens;
    for (int i = 1; !tokens3.empty(); i++) {
      std::cout << __FILE__ << ':' << __LINE__ << ": testFree(): on obj2a, token " << i << " is " << *(tokens3.begin()) << '\n';
      tokens3.erase(tokens3.begin());
    }
    !!*/
    // This is correct because Object::getTokens() returns tokens that _could_ go on the object,
    //   not just the tokens that _are_ on the object.
    assertTrue(tokens.size() == 4);
    TokenId three, four;
    for ( ; !tokens.empty(); tokens.erase(tokens.begin())) {
      four = *(tokens.begin());
      if (four == one || four == two)
        four = TokenId::noId();
      else
        if (three.isNoId()) {
          three = four;
          four = TokenId::noId();
        }
    }
    assertTrue(!three.isNoId() && three.isValid() && one != three);
    assertTrue(!four.isNoId() && four.isValid() && one != four);
    assertTrue(two != three && two != four && three != four);
    assertTrue(three->getObject()->derivedDomain().isSingleton());
    assertTrue(three->getObject()->derivedDomain() == objDom2a);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2a", "rejectableConstrainedSample", ""));
    assertTrue(!three->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!assertTrue(three->getObject()->derivedDomain() == ObjectDomain("TestClass2"));
  }

  /** Test activating a token. */
  static void testActivate() {
    s_activatedToken = createToken("activateSample", true);
    std::cout << __FILE__ << ':' << __LINE__ << ": s_activatedToken is " << s_activatedToken << '\n';
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "activateSample", ""));
    assertTrue(s_activatedToken->isActive(), "token not activated by player");
    /* Leave activated for testMerge(). */
  }

  /** Test merging tokens. */
  static void testMerge() {
    s_mergedToken = createToken("mergeSample", true);
    std::cout << __FILE__ << ':' << __LINE__ << ": s_mergedToken is " << s_mergedToken << '\n';
    TEST_PLAYING_XML(buildXMLObjTokTokStr("merge", "", "mergeSample", "activateSample"));
    assertTrue(s_mergedToken->isMerged(), "token not merged by player");
    /* Leave merged for testCancel(). */
  }

  /** Test rejecting a token. */
  static void testReject() {
    s_rejectedToken = createToken("rejectSample", false);
    std::cout << __FILE__ << ':' << __LINE__ << ": s_rejectedToken is " << s_rejectedToken << '\n';
    TEST_PLAYING_XML(buildXMLObjTokTokStr("reject", "", "rejectSample", ""));
    assertTrue(s_rejectedToken->isRejected(), "token not rejected by player");
    /* Leave rejected for testCancel(). */
  }

  /** Test cancelling tokens. */
  static void testCancel() {
    TEST_PLAYING_XML(buildXMLObjTokTokStr("cancel", "", "rejectSample", ""));
    assertTrue(!s_rejectedToken->isRejected(), "token not unrejected by player");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("cancel", "", "mergeSample", ""));
    assertTrue(!s_mergedToken->isMerged(), "token not unmerged by player");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("cancel", "", "activateSample", ""));
    assertTrue(!s_activatedToken->isActive(), "token not unactivated by player");
  }

  static TokenId createToken(const LabelStr& name, bool mandatory) {
    /* Get the list of tokens so the new one can be identified. */
    TokenSet oldTokens = s_db->getTokens();
    /* Create the token using the player so its name will be recorded there. */
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", mandatory, name));
    /* Find it. */
    TokenSet currentTokens = s_db->getTokens();
    assertTrue(oldTokens.size() + 1 == currentTokens.size());
    TokenId tok = *(currentTokens.begin());
    while (oldTokens.find(tok) != oldTokens.end()) {
      currentTokens.erase(currentTokens.begin());
      tok = *(currentTokens.begin());
    }
    /* Check it. */
    assertTrue(checkToken(tok, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                          TokenId::noId(), mandatory ? s_mandatoryStateDom : s_rejectableStateDom));
    return(tok);
  }

  /**
   * @def ADD_TR_DESC
   * Add the arguments to the appropriate lists.
   * @note Helper macro for getConstraintsFromRelations; should not be used otherwise.
   */
#define ADD_TR_DESC(one, two, low, high) { \
    firsts.push_back(one); \
    seconds.push_back(two); \
    intervals.push_back(new IntervalIntDomain(low, high)); \
}

  /**
   * Helper function: map relation names to the constraints it implies on the two tokens given.
   * @note I think that this should be part of the core API, not part of the test code.
   * --wedgingt@email.arc.nasa.gov 2004 Dec 13
   */
  static void getConstraintsFromRelations(const TokenId& master, const TokenId& slave, const LabelStr& relation,
                                          std::list<ConstrainedVariableId>& firsts,
                                          std::list<ConstrainedVariableId>& seconds,
                                          std::list<AbstractDomain*>& intervals) {
    assertTrue(s_tempRels.find(relation) != s_tempRels.end(), "unknown temporal relation name given");
    if (relation == LabelStr("after")) {
      ADD_TR_DESC(slave->getEnd(), master->getStart(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("any")) {
      // Not an actual constraint, so it would be incorrect to require one.
      return;
    }
    if (relation == LabelStr("before")) {
      ADD_TR_DESC(master->getEnd(), slave->getStart(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contained_by")) {
      ADD_TR_DESC(slave->getStart(), master->getStart(), 0, PLUS_INFINITY);
      ADD_TR_DESC(master->getEnd(), slave->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contains")) {
      ADD_TR_DESC(master->getStart(), slave->getStart(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->getEnd(), master->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contains_end")) {
      ADD_TR_DESC(master->getStart(), slave->getEnd(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->getEnd(), master->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contains_start")) {
      ADD_TR_DESC(master->getStart(), slave->getStart(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->getEnd(), master->getStart(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends")) {
      ADD_TR_DESC(master->getEnd(), slave->getEnd(), 0, 0);
      return;
    }
    if (relation == LabelStr("ends_after")) {
      ADD_TR_DESC(slave->getEnd(), master->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends_after_start")) {
      ADD_TR_DESC(slave->getStart(), master->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends_before")) {
      ADD_TR_DESC(master->getEnd(), slave->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends_during")) {
      ADD_TR_DESC(slave->getStart(), master->getEnd(), 0, PLUS_INFINITY);
      ADD_TR_DESC(master->getEnd(), slave->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("equal") || relation == LabelStr("equals")) {
      ADD_TR_DESC(master->getStart(), slave->getStart(), 0, 0);
      ADD_TR_DESC(master->getEnd(), slave->getEnd(), 0, 0);
      return;
    }
    if (relation == LabelStr("meets")) {
      ADD_TR_DESC(master->getEnd(), slave->getStart(), 0, 0);
      return;
    }
    if (relation == LabelStr("met_by")) {
      ADD_TR_DESC(slave->getEnd(), master->getStart(), 0, 0);
      return;
    }
    if (relation == LabelStr("paralleled_by")) {
      ADD_TR_DESC(slave->getStart(), master->getStart(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->getEnd(), master->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("parallels")) {
      ADD_TR_DESC(master->getStart(), slave->getStart(), 0, PLUS_INFINITY);
      ADD_TR_DESC(master->getEnd(), slave->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("starts")) {
      ADD_TR_DESC(master->getStart(), slave->getStart(), 0, 0);
      return;
    }
    if (relation == LabelStr("starts_after")) {
      ADD_TR_DESC(slave->getStart(), master->getStart(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("starts_before")) {
      ADD_TR_DESC(master->getStart(), slave->getStart(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("starts_before_end")) {
      ADD_TR_DESC(master->getStart(), slave->getEnd(), 0, PLUS_INFINITY);
      return;
    }
    assertTrue(relation == LabelStr("starts_during"),
                "when a new temporal relation name was added, s_tempRels was updated but getConstraintsFromRelation() was not");
    ADD_TR_DESC(slave->getStart(), master->getStart(), 0, PLUS_INFINITY);
    ADD_TR_DESC(master->getStart(), slave->getEnd(), 0, PLUS_INFINITY);
    return;
  }

  static bool verifyTokenRelation(const TokenId& master, const TokenId& slave, const LabelStr& relation) {
    std::list<ConstrainedVariableId> firstVars;
    std::list<ConstrainedVariableId> secondVars;
    std::list<AbstractDomain*> intervals;
    // Get the appropriate list of timepoint pairs and bounds from the relation name.
    getConstraintsFromRelations(master, slave, relation, firstVars, secondVars, intervals);
    assertTrue(firstVars.size() == secondVars.size());
    assertTrue(firstVars.size() == intervals.size());
    while (!firstVars.empty()) {
      ConstrainedVariableId one = *(firstVars.begin());
      ConstrainedVariableId two = *(secondVars.begin());
      AbstractDomain *dom = *(intervals.begin());
      firstVars.erase(firstVars.begin());
      secondVars.erase(secondVars.begin());
      intervals.erase(intervals.begin());
      std::set<ConstraintId> oneConstraints, twoConstraints;
      one->constraints(oneConstraints);
      assertTrue(!oneConstraints.empty());
      two->constraints(twoConstraints);
      // Look for a constraint in both lists.
      for ( ; !twoConstraints.empty(); twoConstraints.erase(twoConstraints.begin())) {
        if (oneConstraints.find(*(twoConstraints.begin())) == oneConstraints.end())
          continue;
        // Got one.  Does it have other variables?
        ConstraintId both = *(twoConstraints.begin());
        if (both->getScope().size() > 2)
          continue; // Yes: can't be the one we're looking for.
        assertTrue(both->getScope().size() == 2);
        //!!How to get the bound from the constraint to compare with *dom ?
      }
      delete dom;
    }
    return(true);
  }

  /** Create an XML string for the tag with the name attribute of the given name. */
  static std::string buildXMLNameStr(const std::string& tag, const std::string& name,
                                     const char *file, const int& line);

  /** Create an XML string to declare the enumeration. */
  static std::string buildXMLEnumStr(const std::string& name, const std::list<std::string>& entries,
                                     const char *file, const int& line);

  /** Create an XML string for the tag with the name and type given. */
  static std::string buildXMLNameTypeStr(const std::string& tag, const std::string& name, const std::string& type,
                                         const char *file, const int& line);

  /** Create an XML string that creates a (complex) model class. */
  static std::string buildXMLCreateClassStr(const std::string& className, const ArgList& args,
                                            const char *file, const int& line);

  /** Create an XML string that creates a model object. */
  static std::string buildXMLCreateObjectStr(const std::string& className, const std::string& objName,
                                             const std::vector<AbstractDomain*>& args);

  /** Create an XML string that specifies a variable's domain. */
  static std::string buildXMLSpecifyVariableStr(const ConstrainedVariableId& var, const AbstractDomain& dom);

  /**
   * Create an XML string that resets a variable's specified domain.
   */
  static std::string buildXMLResetVariableStr(const ConstrainedVariableId& var);

  /**
   * Create an XML string that creates a constraint between the listed variables.
   */
  static std::string buildXMLInvokeConstrainVarsStr(const std::string& name,
                                                    const std::list<ConstrainedVariableId>& vars);

  /**
   * Create an XML string that specifies the variable's domain via '<invoke>'.
   */
  static std::string buildXMLInvokeSpecifyVariableStr(const ConstrainedVariableId& var, const AbstractDomain& dom);

  /**
   * Create an XML string that creates a goal token.
   */
  static std::string buildXMLCreateGoalStr(const LabelStr& type, bool required, const LabelStr& name);

  /**
   * Create an XML string that creates a subgoal token.
   */
  static std::string buildXMLCreateSubgoalStr(const LabelStr& master, const LabelStr& type,
                                              const LabelStr& name, const LabelStr& relation);

  /**
   * Create an XML <invoke> string that activates the named token.
   * @note Should accept __FILE__ and __LINE__ as additional arguments.
   */
  static std::string buildXMLInvokeActivateTokenStr(const LabelStr& token);

  /**
   * Create an XML string for an optional object, a token, and an optional token.
   */
  static std::string buildXMLObjTokTokStr(const LabelStr& tag, const LabelStr& obj, const LabelStr& tok, const LabelStr& tok2);

  /** Create an XML string denoting/naming/identifying the variable. */
  static std::string buildXMLVariableStr(const ConstrainedVariableId& var);

  /** Create an XML string describing the domain. */
  static std::string buildXMLDomainStr(const AbstractDomain& dom);

  /** Saves the constraint engine to avoid creating one for each test. */
  static ConstraintEngineId s_ce;

  /** Saves the plan database to avoid creating one for each test. */
  static PlanDatabaseId s_db;

  /** Pointer to transaction player to be tested. */
  static DbClientTransactionPlayer *s_dbPlayer;

  /**
   * A global-to-the-schema integer variable.
   * @note Created in testCreateVariable() and used in others to test specifying, resetting, etc.
   */
  static ConstrainedVariableId sg_int;

  /**
   * A global-to-the-schema floating point variable.
   * @note Created in testCreateVariable() and used in others to test specifying, resetting, etc.
   */
  static ConstrainedVariableId sg_float;

  /**
   * A global-to-the-schema Locations variable.
   * @note Created in testCreateVariable() and used in others to test specifying, resetting, etc.
   */
  static ConstrainedVariableId sg_location;

  /**
   * A token for testing activation and cancellation via the player.
   */
  static TokenId s_activatedToken;

  /**
   * A token for testing merging and cancellation via the player.
   */
  static TokenId s_mergedToken;

  /**
   * A token for testing rejection and cancellation via the player.
   */
  static TokenId s_rejectedToken;

  /**
   * The at-creation state domain of mandatory tokens.
   */
  static StateDomain s_mandatoryStateDom;

  /**
   * The at-creation state domain of rejectable tokens.
   */
  static StateDomain s_rejectableStateDom;

  /**
   * The list of names of temporal relations.
   * @note This is const after initialization in testImpl().
   * @note Is parallels missing?
   * @note This list or something based on it should be part of the interface of the schema or some similar (C++) class.
   * @see testImpl
   */
  static std::set<LabelStr> s_tempRels;
};

ConstraintEngineId DbTransPlayerTest::s_ce;
PlanDatabaseId DbTransPlayerTest::s_db;
DbClientTransactionPlayer * DbTransPlayerTest::s_dbPlayer;
ConstrainedVariableId DbTransPlayerTest::sg_int;
ConstrainedVariableId DbTransPlayerTest::sg_float;
ConstrainedVariableId DbTransPlayerTest::sg_location;
TokenId DbTransPlayerTest::s_activatedToken;
TokenId DbTransPlayerTest::s_mergedToken;
TokenId DbTransPlayerTest::s_rejectedToken;
StateDomain DbTransPlayerTest::s_mandatoryStateDom;
StateDomain DbTransPlayerTest::s_rejectableStateDom;
std::set<LabelStr> DbTransPlayerTest::s_tempRels;

/** Run a single test, reading the XML from the given string. */
void DbTransPlayerTest::testPlayingXML(const std::string& xml, const char *file, const int& line) {
  assertTrue(s_dbPlayer != 0);
  std::istringstream iss(xml);
  std::cout << file << ':' << line << ": testPlayingXML() about to play '" << xml << "'\n";
  s_dbPlayer->play(iss);
}

std::string DbTransPlayerTest::buildXMLNameStr(const std::string& tag, const std::string& name,
                                               const char *file, const int& line) {
  std::string str("<");
  str += tag;
  str += " line=\"";
  std::ostringstream oss;
  oss << line;
  str += oss.str();
  str += "\" column=\"1\" filename=\"";
  str += file;
  str += "\" name=\"";
  str += name;
  str += "\"> </";
  str += tag;
  str += ">";
  return(str);
}

std::string DbTransPlayerTest::buildXMLEnumStr(const std::string& name, const std::list<std::string>& entries,
                                               const char *file, const int& line) {
  std::string str("<enum line=\"");
  std::ostringstream oss;
  oss << line;
  str += oss.str();
  str += "\" column=\"1\" filename=\"";
  str += file;
  str += "\" name=\"";
  str += name;
  str += "\"> <set>";
  std::list<std::string>::const_iterator it = entries.begin();
  assertTrue(it != entries.end());
  for ( ; it != entries.end(); it++) {
    str += " <symbol value=\"";
    str += *it;
    str += "\" type=\"";
    str += name;
    str += "\"/>";
  }
  str += " </set> </enum>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLNameTypeStr(const std::string& tag, const std::string& name,
                                                   const std::string& type,
                                                   const char *file, const int& line) {
  std::string str("<");
  str += tag;
  str += " line=\"";
  std::ostringstream oss;
  oss << line;
  str += oss.str();
  str += "\" column=\"1\" filename=\"";
  str += file;
  str += "\" name=\"";
  str += name;
  str += "\" type=\"";
  str += type;
  str += "\"/>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLCreateClassStr(const std::string& className, const ArgList& args,
                                                      const char *file, const int& line) {
std::string str("<class line=\"");
  std::ostringstream oss;
  oss << line;
  str += oss.str();
  str += "\" column=\"1\" filename=\"";
  str += file;
  str += "\" name=\"";
  str += className;
  str += "\">";
  ArgIter it = args.begin();
  assertTrue(it != args.end());
  int l_line = line - args.size(); /* "Guess" that args was create line by line in same file. */
  for ( ; it != args.end(); it++) {
    str += " <var line=\"";
    std::ostringstream oss2;
    oss2 << l_line++;
    str += oss2.str();
    str += "\" column=\"1\" name=\"m_";
    str += it->second;
    str += "\" type=\"";
    str += it->first;
    str += "\"/>";
  }
assertTrue(line == l_line);
  str += " <constructor line=\"";
  str += oss.str();
  str += "\" column=\"1\">";
  for (it = args.begin(); it != args.end(); it++) {
    str += " <arg name=\"";
    str += it->second;
    str += "\" type=\"";
    str += it->first;
    str += "\"/>";
  }
  l_line -= args.size();
  for (it = args.begin(); it != args.end(); it++) {
    str += " <assign line=\"";
    std::ostringstream oss3;
    oss3 << l_line++;
    str += oss3.str();
    str += "\" column=\"1\" name=\"m_";
    str += it->second;
    str += "\"> <id name=\"";
    str += it->second;
    str += "\" type=\"";
    str += it->first;
    str += "\"/> </assign>";
  }
  str += "</constructor> </class>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLCreateObjectStr(const std::string& className, const std::string& objName,
                                                       const std::vector<AbstractDomain*>& domains) {
  assertTrue(!domains.empty());
  std::string str("<new type=\"");
  str += className;
  str += "\" name=\"";
  str += objName;
  str += "\"> <";
  str += StringDomain::getDefaultTypeName().toString();
  str += " value=\"";
  str += objName;
  str += "\"/> ";
  for (unsigned int i = 0; i < domains.size(); i++, str += " ")
    str += buildXMLDomainStr(*(domains[i]));
  str += "</new>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLSpecifyVariableStr(const ConstrainedVariableId& var, const AbstractDomain& dom) {
  std::string str("<specify>");
  str += buildXMLVariableStr(var);
  str += " ";
  str += buildXMLDomainStr(dom);
  str += " </specify>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLResetVariableStr(const ConstrainedVariableId& var) {
  std::string str("<reset>");
  str += buildXMLVariableStr(var);
  str += " </reset>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLInvokeConstrainVarsStr(const std::string& name,
                                                              const std::list<ConstrainedVariableId>& vars) {
  std::string str("<invoke name=\"");
  str += name;
  str += "\">";
  std::list<ConstrainedVariableId>::const_iterator it = vars.begin();
  for ( ; it != vars.end(); it++)
    str += buildXMLVariableStr(*it);
  str += " </invoke>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLInvokeSpecifyVariableStr(const ConstrainedVariableId& var,
                                                                const AbstractDomain& dom) {
  std::string str("<invoke name=\"specify\" identifier=\""); 
  //!!Would like to re-use buildXMLVariableStr() here, but this wants a different syntax(!)
  if (var->getParent().isNoId())
    str += var->getName().toString();
  else {
    if (TokenId::convertable(var->getParent())) {
      //!!For token variables, the player's name for the token is needed: identifier="tokenName.varName"
      //!!  To implement this, a map of the ids to the names will have to be kept as they are created.
      assertTrue(false, "sorry: specifying variables of tokens in tests of <invoke> is presently unsupported");
    }
    assertTrue(ObjectId::convertable(var->getParent()), "var's parent is neither token nor object");
    //!!I don't understand the details in DbClientTransactionPlayer.cc:parseVariable() well enough to figure this out yet
    //!!But here's a guess:
    str += var->getParent()->getName();
    str += ".";
    str += var->getName();
  }
  str += "\"> ";
  str += buildXMLDomainStr(dom);
  str += " </invoke>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLCreateGoalStr(const LabelStr& type, bool mandatory, const LabelStr& name) {
  std::string str("<goal mandatory=\"");
  if (mandatory)
    str += "true";
  else
    str += "false";
  str += "\"> <predicateinstance type=\"";
  str += type.toString();
  str += "\" name=\"";
  str += name.toString();
  str += "\"/> </goal>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLCreateSubgoalStr(const LabelStr& master, const LabelStr& type,
                                                        const LabelStr& name, const LabelStr& relation) {
  std::string str("<goal mandatory=\"true\"> <predicateinstance type=\"");
  str += type.toString();
  str += "\" name=\"";
  str += name.toString();
  str += "\"/> </goal>";
  /* Done specifying the subgoal token; add its relation to its master token. */
  str += " <goal origin=\"";
  str += master.toString();
  str += "\" relation=\"";
  str += relation.toString();
  str += "\" target=\"";
  str += name.toString();
  str += "\"/>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLInvokeActivateTokenStr(const LabelStr& token) {
  std::string str("<invoke name=\"activate\" identifier=\"");
  str += token.toString();
  str += "\"/>";
  return(str);
}

//!!constrain token(s) to object and to each other if two tokens:
//!!1: <constrain><object name="rover" /><token path="0" /></constrain>
//!!2: <constrain><object name="rover.mutex" /><token path="1.1.2" /><token path="1.2" /></constrain>
//!!3: <invoke line="21" column="3" name="constrain" identifier="f2">
//!!        <id name="t1" type="Foo.bar"/>
//!!    </invoke>
//!!4:  <invoke line="23" column="3" name="constrain" identifier="f2">
//!!        <id name="t2" type="Foo.baz"/>
//!!        <id name="t1" type="Foo.bar"/>
//!!    </invoke>

std::string DbTransPlayerTest::buildXMLObjTokTokStr(const LabelStr& tag, const LabelStr& obj, const LabelStr& tok, const LabelStr& tok2) {
  std::string str("<");
  str += tag.toString();
  str += "> ";
  if (obj != LabelStr("")) {
    str += "<object name=\"";
    str += obj.toString();
    str += "\"/> ";
  }
  str += "<token name=\"";
  str += tok.toString();
  str += "\"/> ";
  if (tok2 != LabelStr("")) {
    str += "<token name=\"";
    str += tok2.toString();
    str += "\"/> ";
  }
  str += "</";
  str += tag.toString();
  str += ">";
  return(str);
}

std::string DbTransPlayerTest::buildXMLVariableStr(const ConstrainedVariableId& var) {
  std::string str(" <");
  if (var->getParent().isNoId()) {
    str += "id name =\"";
    str += var->getName().toString();
  } else {
    str += "variable index=\"";
    std::ostringstream oss;
    oss << var->getIndex() << "\" ";
    if (ObjectId::convertable(var->getParent()))
      oss << "object=\"" << var->getParent()->getName().toString();
    else {
      assertTrue(TokenId::convertable(var->getParent()), "unknown or unsupported (C++) type of parent of variable");
      oss << "token=\"";
      TokenId token = var->getParent();
      if (token->getMaster().isNoId())
        oss << token->getKey();
      else {
        TokenId rootToken = token->getMaster();
        while (!rootToken->getMaster().isNoId())
          rootToken = rootToken->getMaster();
        oss << s_db->getClient()->getPathAsString(rootToken);
      }
    }
    str += oss.str();
  }
  str += "\"/>";
  return(str);
}

std::string DbTransPlayerTest::buildXMLDomainStr(const AbstractDomain& dom) {
  std::string str("<");
  if (dom.isSingleton() && dom.isNumeric()) {
    str += dom.getTypeName().toString();
    str += " value=\"";
    std::ostringstream oss;
    oss << dom.getSingletonValue();
    str += oss.str();
    str += "\"/>";
    return(str);
  }
  if (dom.isInterval()) {
    str += "interval type=\"";
    str += dom.getTypeName().toString();
    str += "\" min=\"";
    std::ostringstream oss2;
    std::fixed(oss2);
    oss2 << dom.getLowerBound() << "\" max=\"" << dom.getUpperBound();
    str += oss2.str();
    str += "\"/>";
    return(str);
  }
  assertTrue(dom.isEnumerated(), "domain is not singleton, interval, nor enumerated");
  str += "set> ";
  std::list<double> vals;
  for (dom.getValues(vals); !vals.empty(); vals.pop_front()) {
    str += "<";
    if (dom.getType() == AbstractDomain::SYMBOL_ENUMERATION)
      str += "symbol";
    else
      str += dom.getTypeName().toString();
    str += " value=\"";
    if (dom.getType() == AbstractDomain::STRING_ENUMERATION ||
        dom.getType() == AbstractDomain::SYMBOL_ENUMERATION)
      str += LabelStr(*(vals.begin())).toString();
    else {
      assertTrue(dom.getType() == AbstractDomain::REAL_ENUMERATION, "sorry: only string, symbol, and real enumerations are supported");
      std::ostringstream oss4;
      oss4 << *(vals.begin());
      str += oss4.str();
    }
    str += "\" type=\"";
    str += dom.getTypeName().toString();
    str += "\"/> ";
  }
  str += "</set>";
  return(str);
}

/* Done with class DbTransPlayerTest, so drop this macro. */
#undef TEST_PLAYING_XML

int main() {
  initDbModuleTests();
  for (int i=0;i<1;i++){
    runTestSuite(SchemaTest::test);
    runTestSuite(ObjectTest::test);
    runTestSuite(TokenTest::test);
    runTestSuite(TimelineTest::test);
    runTestSuite(DbClientTest::test);
    runTestSuite(DbTransPlayerTest::test);
    std::cout << "Finished" << std::endl;
  } 
  ConstraintLibrary::purgeAll();
}
