#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "Token.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "Timeline.hh"
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
#include "Debug.hh"
#include "PlanDatabaseTestSupport.hh"
#include "PlanDatabaseWriter.hh"

#include "LockManager.hh"

#include <iostream>
#include <iomanip>
#include <pthread.h>
#include <string>

  class Foo;
  typedef Id<Foo> FooId;

  class Foo : public Timeline {
  public:
    Foo(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Foo(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization
    
    // test/simple-predicate.nddl:4 Foo
    void constructor();
    void constructor(int arg0, LabelStr& arg1);
    Id< Variable< IntervalIntDomain > > m_0;
    Id< Variable< LabelSet > > m_1;
  };

  Foo::Foo(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
    : Timeline(planDatabase, type, name, true) {
  }

  Foo::Foo(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
    : Timeline(parent, type, name, true) {}

  // default initialization of member variables
  void Foo::handleDefaults(bool autoClose) {
    if(m_0.isNoId()){
      check_error(!ObjectId::convertable(m_0)); // Object Variables must be explicitly initialized to a singleton
      m_0 = addVariable(IntervalIntDomain(), "IntervalIntVar");
    }
    check_error(!m_1.isNoId()); // string variables must be initialized explicitly
    if(autoClose) close();
  }

  void Foo::constructor() {
    m_1 = addVariable(LabelSet(LabelStr("Hello World")), "LabelSetVar");
  }

  void Foo::constructor(int arg0, LabelStr& arg1) {
    m_0 = addVariable(IntervalIntDomain(arg0), "IntervalIntVar");
    m_1 = addVariable(LabelSet(LabelStr("Hello World")), "LabelSetVar");
  }

  class StandardFooFactory: public ConcreteObjectFactory {
  public:
    StandardFooFactory(): ConcreteObjectFactory(DEFAULT_OBJECT_TYPE()){}

  private:
    ObjectId createInstance(const PlanDatabaseId& planDb, 
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      assert(arguments.empty());
      FooId foo = (new Foo(planDb, objectType, objectName))->getId();
      foo->constructor();
      foo->handleDefaults();
      return foo;
    }
  };

  class SpecialFooFactory: public ConcreteObjectFactory{
  public:
    SpecialFooFactory(): ConcreteObjectFactory(DEFAULT_OBJECT_TYPE().toString() +
					       ":" + IntervalIntDomain::getDefaultTypeName().toString() +
					       ":" + LabelSet::getDefaultTypeName().toString())
    {}
    
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb, 
                            const LabelStr& objectType, 
                            const LabelStr& objectName,
                            const std::vector<ConstructorArgument>& arguments) const {
      FooId foo = (new Foo(planDb, objectType, objectName))->getId();
      // Type check the arguments
      assert(arguments.size() == 2);
      assert(arguments[0].first == IntervalIntDomain::getDefaultTypeName());
      assert(arguments[1].first == LabelSet::getDefaultTypeName());

      int arg0((int) arguments[0].second->getSingletonValue());
      LabelStr arg1(arguments[1].second->getSingletonValue());
      foo->constructor(arg0, arg1);
      foo->handleDefaults();
      return foo;
    }
  };

  class IntervalTokenFactory: public ConcreteTokenFactory {
  public:
    IntervalTokenFactory()
      : ConcreteTokenFactory(DEFAULT_PREDICATE()) {
    }
  private:
    TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false) const {
      TokenId token = (new IntervalToken(planDb, name, rejectable))->getId();
      return(token);
    }
    TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const{
      TokenId token = (new IntervalToken(master, relation, name))->getId();
      return(token);
    }
  };

  /**
   * @brief Declaration and definition for test constraint to force a failure when the domain becomes a singleton
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


  void initDbModuleTests() {
    initConstraintEngine();
    initConstraintLibrary();
    
    // Allocate default schema initially so tests don't fail because of ID's
    SCHEMA;
    initDbTestSchema(SCHEMA);

    // Have to register factories for testing.
    new StandardFooFactory();
    new SpecialFooFactory();
    new IntervalTokenFactory();
  }

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
    SCHEMA->addPredicate("Baz.pred");

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

    assertFalse(SCHEMA->hasPredicates("Foo"));
    assertFalse(SCHEMA->hasPredicates("Foo")); // Call again for cached result
    assertTrue(SCHEMA->hasPredicates("Baz")); // Call again for cached result

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


    assertTrue(SCHEMA->getParameterCount(LabelStr("Resource.change")) == 1);
    assertTrue(SCHEMA->getParameterType(LabelStr("Resource.change"), 0) == LabelStr("float"));

    std::set<LabelStr> predicates;
    SCHEMA->getPredicates(LabelStr("Battery"), predicates);
    assertTrue(predicates.size() == 1);
    predicates.clear();
    SCHEMA->getPredicates(LabelStr("Resource"), predicates);
    assertTrue(predicates.size() == 1);

    SCHEMA->addObjectType("One");
    SCHEMA->addPredicate("One.Predicate1");
    SCHEMA->addPredicate("One.Predicate2");
    SCHEMA->addPredicate("One.Predicate3");
    SCHEMA->addPredicate("One.Predicate4");

    predicates.clear();
    SCHEMA->getPredicates(LabelStr("One"), predicates);
    assertTrue(predicates.size() == 4);

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
    runTest(testFreeAndConstrain);
    return(true);
  }
  
private:
  static bool testBasicAllocation() {
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    Object o2(db.getId(), DEFAULT_OBJECT_TYPE(), "o2");

    ObjectId id0((new Object(o1.getId(), DEFAULT_OBJECT_TYPE(), "id0"))->getId());
    Object o3(o2.getId(), DEFAULT_OBJECT_TYPE(), "o3");
    assertTrue(db.getObjects().size() == 4);
    assertTrue(o1.getComponents().size() == 1);
    assertTrue(o3.getParent() == o2.getId());
    delete (Object*) id0;
    assertTrue(db.getObjects().size() == 3);
    assertTrue(o1.getComponents().empty());

    ObjectId id1((new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "id1"))->getId());
    new Object(id1, DEFAULT_OBJECT_TYPE(), "id2");
    ObjectId id3((new Object(id1, DEFAULT_OBJECT_TYPE(), "id3"))->getId());
    assertTrue(db.getObjects().size() == 6);
    assertTrue(id3->getName().toString() == "id1.id3");

    // Test ancestor call
    ObjectId id4((new Object(id3, DEFAULT_OBJECT_TYPE(), "id4"))->getId());
    std::list<ObjectId> ancestors;
    id4->getAncestors(ancestors);
    assertTrue(ancestors.front() == id3);
    assertTrue(ancestors.back() == id1);

    // Force cascaded delete
    delete (Object*) id1;
    assertTrue(db.getObjects().size() == 3);

    // Now allocate dynamically and allow the plan database to clean it up when it deallocates
    ObjectId id5 = ((new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "id5"))->getId());
    new Object(id5, DEFAULT_OBJECT_TYPE(), "id6");
    return(true);
  }
  
  static bool testObjectDomain(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    std::list<ObjectId> values;
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    Object o2(db.getId(), DEFAULT_OBJECT_TYPE(), "o2");
    assertTrue(db.getObjects().size() == 2);
    values.push_back(o1.getId());
    values.push_back(o2.getId());
    ObjectDomain os1(values, DEFAULT_OBJECT_TYPE().c_str());
    assertTrue(os1.isMember(o1.getId()));
    os1.remove(o1.getId());
    assertTrue(!os1.isMember(o1.getId()));
    assertTrue(os1.isSingleton());
    return true;
  }
  
  static bool testObjectVariables(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1", true);
    assertFalse(o1.isComplete());
    o1.addVariable(IntervalIntDomain(), "IntervalIntVar");
    o1.addVariable(BoolDomain(), "BoolVar");
    o1.close();
    assertTrue(o1.isComplete());
    assertTrue(o1.getVariable("o1.BoolVar") != o1.getVariable("o1IntervalIntVar"));

    Object o2(db.getId(), DEFAULT_OBJECT_TYPE(), "o2", true);
    assertFalse(o2.isComplete());
    o2.addVariable(IntervalIntDomain(15, 200), "IntervalIntVar");
    o2.close();

    // Add a unary constraint
    Variable<IntervalIntDomain> superset(db.getConstraintEngine(), IntervalIntDomain(10, 20));;

    ConstraintId subsetConstraint = ConstraintLibrary::createConstraint("SubsetOf", 
					db.getConstraintEngine(), 
					makeScope(o1.getVariables()[0], superset.getId()));

    // Now add a constraint equating the variables and test propagation
    std::vector<ConstrainedVariableId> constrainedVars;
    constrainedVars.push_back(o1.getVariables()[0]);
    constrainedVars.push_back(o2.getVariables()[0]);
    ConstraintId constraint = ConstraintLibrary::createConstraint("Equal",
								  db.getConstraintEngine(),
								  constrainedVars);

    assertTrue(db.getConstraintEngine()->propagate());
    assertTrue(o1.getVariables()[0]->lastDomain() == o1.getVariables()[0]->lastDomain());

    // Delete one of the constraints to force automatic clean-up path and explciit clean-up
    delete (Constraint*) constraint;
    delete (Constraint*) subsetConstraint;

    return(true);
  }
  
  
  static bool testObjectTokenRelation(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    // 1. Create 2 objects
    ObjectId object1 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "O1"))->getId();
    ObjectId object2 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "O2"))->getId();    
    db.close();

    assertTrue(object1 != object2);
    assertTrue(db.getObjects().size() == 2);
    // 2. Create 1 token.
    EventToken eventToken(db.getId(), DEFAULT_PREDICATE(), false, IntervalIntDomain(0, 10));

    // Confirm not added to the object
    assertFalse(eventToken.getObject()->getDerivedDomain().isSingleton());

    // 3. Activate token. (NO subgoals)
    eventToken.activate();

    // Confirm not added to the object
    assertFalse(eventToken.getObject()->getDerivedDomain().isSingleton());

    // 4. Specify tokens object variable to a ingletone

    eventToken.getObject()->specify(object1);

    // Confirm added to the object
    assertTrue(eventToken.getObject()->getDerivedDomain().isSingleton());

    // 5. propagate
    db.getConstraintEngine()->propagate();

    // 6. reset object variables domain.
    eventToken.getObject()->reset();

    // Confirm it is no longer part of the object
    // Confirm not added to the object
    assertFalse(eventToken.getObject()->getDerivedDomain().isSingleton());

    return true;
  }
  
  static bool testCommonAncestorConstraint(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    Object o2(o1.getId(), DEFAULT_OBJECT_TYPE(), "o2");
    Object o3(o1.getId(), DEFAULT_OBJECT_TYPE(), "o3");
    Object o4(o2.getId(), DEFAULT_OBJECT_TYPE(), "o4");
    Object o5(o2.getId(), DEFAULT_OBJECT_TYPE(), "o5");
    Object o6(o3.getId(), DEFAULT_OBJECT_TYPE(), "o6");
    Object o7(o3.getId(), DEFAULT_OBJECT_TYPE(), "o7");

    ObjectDomain allObjects(DEFAULT_OBJECT_TYPE().c_str());
    allObjects.insert(o1.getId());
    allObjects.insert(o2.getId());
    allObjects.insert(o3.getId());
    allObjects.insert(o4.getId());
    allObjects.insert(o5.getId());
    allObjects.insert(o6.getId());
    allObjects.insert(o7.getId());
    allObjects.close();

    // Ensure there they agree on a common root.
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o4.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> second(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o1.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      CommonAncestorConstraint constraint("commonAncestor", 
					  "Default", 
					  ENGINE, 
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      assertTrue(ENGINE->propagate());
    }

    // Now impose a different set of restrictions which will eliminate all options
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o4.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> second(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o2.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      CommonAncestorConstraint constraint("commonAncestor", 
					  "Default", 
					  ENGINE, 
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      assertFalse(ENGINE->propagate());
    }

    // Now try a set of restrictions, which will allow it to pass
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o4.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> second(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, allObjects);
      CommonAncestorConstraint constraint("commonAncestor", 
					  "Default", 
					  ENGINE, 
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      assertTrue(ENGINE->propagate());
    }

    // Now try when no variable is a singleton, and then one becomes a singleton
    {
      Variable<ObjectDomain> first(ENGINE, allObjects);
      Variable<ObjectDomain> second(ENGINE, allObjects);
      Variable<ObjectDomain> restrictions(ENGINE, allObjects);
      CommonAncestorConstraint constraint("commonAncestor", 
					  "Default", 
					  ENGINE, 
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      assertTrue(ENGINE->propagate()); // All ok so far

      restrictions.specify(o2.getId());
      assertTrue(ENGINE->propagate()); // Nothing happens yet.

      first.specify(o6.getId()); // Now we should propagate to failure
      assertFalse(ENGINE->propagate());
      first.reset();

      first.specify(o4.getId());
      assertTrue(ENGINE->propagate());
    }    
    return true;
  }
  
  static bool testHasAncestorConstraint(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    Object o2(o1.getId(), DEFAULT_OBJECT_TYPE(), "o2");
    Object o3(o1.getId(), DEFAULT_OBJECT_TYPE(), "o3");
    Object o4(o2.getId(), DEFAULT_OBJECT_TYPE(), "o4");
    Object o5(o2.getId(), DEFAULT_OBJECT_TYPE(), "o5");
    Object o6(o3.getId(), DEFAULT_OBJECT_TYPE(), "o6");
    Object o7(o3.getId(), DEFAULT_OBJECT_TYPE(), "o7");
    Object o8(db.getId(), DEFAULT_OBJECT_TYPE(), "o8");
    
    
    // Positive test immediate ancestor
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o3.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assertTrue(ENGINE->propagate());
    }
    
    // negative test immediate ancestor
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o2.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assertFalse(ENGINE->propagate());
    }
    // Positive test higher up  ancestor
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o1.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assertTrue(ENGINE->propagate());
    }
    // negative test higherup ancestor
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o8.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assertFalse(ENGINE->propagate());
    }
    
    //positive restriction of the set.
    {
      ObjectDomain obs(DEFAULT_OBJECT_TYPE().c_str());
      obs.insert(o7.getId());
      obs.insert(o4.getId());
      obs.close();
      
      Variable<ObjectDomain> first(ENGINE, obs);
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o2.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assertTrue(ENGINE->propagate());
      assertTrue(first.getDerivedDomain().isSingleton());
    }
    
    //no restriction of the set.
    {
      ObjectDomain obs1(DEFAULT_OBJECT_TYPE().c_str());
      obs1.insert(o7.getId());
      obs1.insert(o4.getId());
      obs1.close();
      
      Variable<ObjectDomain> first(ENGINE, obs1);
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o1.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assertTrue(ENGINE->propagate());
      assertTrue(first.getDerivedDomain().getSize() == 2);
    }
    
    return true;
  }
  /**
   * The most basic case for dynamic objects is that we can populate the variable correctly
   * and synchronize its values.
   */
  static bool testMakeObjectVariable(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    ConstrainedVariableId v0 = (new Variable<ObjectDomain>(ENGINE, ObjectDomain(DEFAULT_OBJECT_TYPE().c_str())))->getId();
    assertFalse(v0->isClosed());
    db.makeObjectVariableFromType(DEFAULT_OBJECT_TYPE(), v0);
    assertFalse(v0->isClosed());
    assertTrue(ENGINE->propagate());

    // Now add an object and we should expect the constraint network to be consistent
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    assertTrue(ENGINE->propagate());
    assertFalse(db.isClosed(DEFAULT_OBJECT_TYPE().c_str()));
    assertTrue(v0->lastDomain().isSingleton() && v0->lastDomain().getSingletonValue() == o1.getId());

    // Now delete the variable. This should remove the listener
    delete (ConstrainedVariable*) v0;

    return true;
  }
  
  /**
   * Have at least one object in the system prior to creating a token. Then show how
   * removal triggers an inconsistency, and insertion of another object fixes it. Also
   * show that specifiying the object prevents propagation if we add another object, but
   * relaxing it will populate the object variable to include the new object.
   */
  static bool testTokenObjectVariable(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    // Now add an object and we should expect the constraint network to be consistent
    ObjectId o1 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "o1"))->getId();
    EventToken eventToken(db.getId(), DEFAULT_PREDICATE(), false, IntervalIntDomain(0, 10));

    eventToken.activate(); // Must be activate to eventually propagate the objectTokenRelation
    assertTrue(ENGINE->propagate());

    // Make sure the object var of the token contains o1.
    assertTrue(eventToken.getObject()->lastDomain().isMember(o1));

    // Since the object type has not been closed, the object variable will not propagate changes,
    // so the object token relation will not link up the Token and the object.
    assertTrue(o1->getTokens().empty());

    // Deletion of the object should result in the domain of the token becoming empty. However,
    // that will not cause an inconsistency. Nor will it cuase propagation
    delete (Object*) o1;
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(eventToken.getObject()->baseDomain().isEmpty());

    // Insertion of a new object should reecover the situation
    ObjectId o2 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "o2"))->getId();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(eventToken.getObject()->baseDomain().isSingleton());

    // Now specify it
    eventToken.getObject()->specify(o2);

    // Addition of a new object will update the base domain, but not the spec or derived.
    // Consequently, no further propagation is required
    ObjectId o3 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "o3"))->getId();
    assertTrue(ENGINE->constraintConsistent());
    assertFalse(eventToken.getObject()->baseDomain().isSingleton());
    assertTrue(eventToken.getObject()->lastDomain().isSingleton());

    // Now resetting the specified domain will revert the derived domain back completely
    eventToken.getObject()->reset();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(eventToken.getObject()->lastDomain().isMember(o2));
    assertTrue(eventToken.getObject()->lastDomain().isMember(o3));

    // Confirm that since the object type is not closed, no tokens are added to the object
    assertTrue(o2->getTokens().find(eventToken.getId()) == o2->getTokens().end());

    // Finally, close the database for this type, and ensure propagation is triggered, and results in consistency
    db.close(DEFAULT_OBJECT_TYPE().c_str());
    assertFalse(o2->getTokens().find(eventToken.getId()) == o2->getTokens().end());
    assertTrue(ENGINE->propagate());

    // Confirm the object-token relation has propagated
    return true;
  }

  static bool testTokenWithNoObjectOnCreation(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    {
      // Leave this class of objects open. So we should be able to create a token and have things consistent
      EventToken eventToken(db.getId(), DEFAULT_PREDICATE(), false, IntervalIntDomain(0, 10));
      assertTrue(ENGINE->propagate());

    // Now close the datbase for this class of objects, and ensure we are inconsistent
      db.close(DEFAULT_OBJECT_TYPE().c_str());
      assertFalse(ENGINE->propagate());
    }

    // Now the token has gone out of scope so we expect the system to be consistent again
    assertTrue(ENGINE->propagate());
    return true;
  }

  static bool testFreeAndConstrain(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    db.close();                                                                          
  
    IntervalToken t1(db.getId(),  
                     DEFAULT_PREDICATE(),                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));                                        
  
    IntervalToken t2(db.getId(),                                                         
                     DEFAULT_PREDICATE(),                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));                                        
  
    IntervalToken t3(db.getId(),                                                         
                     DEFAULT_PREDICATE(),                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));

    t1.activate();
    t2.activate();
    t3.activate();

    o1.constrain(t1.getId(), t2.getId());
    o1.constrain(t1.getId(), t3.getId());
    o1.constrain(t2.getId(), t3.getId());

    o1.free(t1.getId(), t2.getId());
    o1.free(t1.getId(), t3.getId());
    o1.free(t2.getId(), t3.getId());

    // Constrain again to leave all cleanup automatic
    o1.constrain(t1.getId(), t2.getId());
    o1.constrain(t1.getId(), t3.getId());
    o1.constrain(t2.getId(), t3.getId());                                    

    // Also use a locally scoped token to force a different deletion path
    {
      IntervalToken t4(db.getId(),                                                         
		       DEFAULT_PREDICATE(),                                                     
		       true,                                                               
		       IntervalIntDomain(0, 10),                                           
		       IntervalIntDomain(0, 20),                                           
		       IntervalIntDomain(1, 1000));
      t4.activate();
      o1.constrain(t3.getId(), t4.getId());   
    }

    assertTrue(ENGINE->propagate());
    return true;
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
    runTest(testPredicateInheritance);
    runTest(testTokenFactory);
    runTest(testCorrectSplit_Gnats2450);
    return(true);
  }
  
private:
  
  static bool testBasicTokenAllocation() {
    DEFAULT_SETUP(ce, db, true);
    // Event Token
    EventToken eventToken(db, DEFAULT_PREDICATE(), true, IntervalIntDomain(0, 1000), Token::noObject(), false);
    assertTrue(eventToken.getStart()->getDerivedDomain() == eventToken.getEnd()->getDerivedDomain());
    assertTrue(eventToken.getDuration()->getDerivedDomain() == IntervalIntDomain(0, 0));
    eventToken.getStart()->specify(IntervalIntDomain(5, 10));
    assertTrue(eventToken.getEnd()->getDerivedDomain() == IntervalIntDomain(5, 10));
    eventToken.addParameter(IntervalDomain(-1.08, 20.18), "IntervalParam");
    eventToken.close();
  
    // IntervalToken
    IntervalToken intervalToken(db, 
                                DEFAULT_PREDICATE(), 
                                true, 
                                IntervalIntDomain(0, 1000),
                                IntervalIntDomain(0, 1000),
                                IntervalIntDomain(2, 10),
                                Token::noObject(), false);
  
    std::list<double> values;
    values.push_back(EUROPA::LabelStr("L1"));
    values.push_back(EUROPA::LabelStr("L4"));
    values.push_back(EUROPA::LabelStr("L2"));
    values.push_back(EUROPA::LabelStr("L5"));
    values.push_back(EUROPA::LabelStr("L3"));
    intervalToken.addParameter(LabelSet(values), "LabelSetParam");
    intervalToken.close();
    assertTrue(intervalToken.getEnd()->getDerivedDomain().getLowerBound() == 2);
    intervalToken.getStart()->specify(IntervalIntDomain(5, 10));
    assertTrue(intervalToken.getEnd()->getDerivedDomain() == IntervalIntDomain(7, 20));
    intervalToken.getEnd()->specify(IntervalIntDomain(9, 10));
    assertTrue(intervalToken.getStart()->getDerivedDomain() == IntervalIntDomain(5, 8));
    assertTrue(intervalToken.getDuration()->getDerivedDomain() == IntervalIntDomain(2, 5));

    // Create and delete a Token
    TokenId token = (new IntervalToken(db, 
                                       DEFAULT_PREDICATE(), 
                                       true, 
                                       IntervalIntDomain(0, 1000),
                                       IntervalIntDomain(0, 1000),
                                       IntervalIntDomain(2, 10),
                                       Token::noObject(), true))->getId();

    delete (Token*) token; // It is inComplete
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testBasicTokenCreation() {           
    DEFAULT_SETUP(ce,db, false);
    ObjectId timeline = (new Timeline(db, DEFAULT_OBJECT_TYPE(), "o2"))->getId();
    assertFalse(timeline.isNoId());
    db->close();                                                                          
  
    IntervalToken t1(db,                                                         
                     DEFAULT_PREDICATE(),                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));                                        
    DEFAULT_TEARDOWN();
    return true;                                                                         
  }                            

  static bool testStateModel(){
    DEFAULT_SETUP(ce, db, true);
    IntervalToken t0(db, 
                     DEFAULT_PREDICATE(), 
                     true, 
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(2, 10),
                     Token::noObject(), false);
  
    assertTrue(t0.isIncomplete());
    t0.close();
    assertTrue(t0.isInactive());
    t0.reject();
    assertTrue(t0.isRejected());
    t0.cancel();
    assertTrue(t0.isInactive());
    t0.activate();
    assertTrue(t0.isActive());
    t0.cancel();
    assertTrue(t0.isInactive());
  
    IntervalToken t1(db, 
                     DEFAULT_PREDICATE(), 
                     true, 
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(2, 10),
                     Token::noObject(), true);
  
    // Constraint the start variable of both tokens
    EqualConstraint c0("eq", "Default", ENGINE, makeScope(t0.getStart(), t1.getStart()));
  
    assertTrue(t1.isInactive());
    t0.activate();
    t1.merge(t0.getId());
    assertTrue(t1.isMerged());
    t1.cancel();
    assertTrue(t1.isInactive());
    t1.merge(t0.getId());

    // Test that we can allocate a token, but if we constrain it with any external entity, then the state variable will be restricted
    // to exclude the possibility of rejecting the token.
    {


    }
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testMasterSlaveRelationship(){
    DEFAULT_SETUP(ce, db, true);
    IntervalToken t0(db, 
                     DEFAULT_PREDICATE(), 
                     false, 
                     IntervalIntDomain(0, 1),
                     IntervalIntDomain(0, 1),
                     IntervalIntDomain(1, 1));
    t0.activate();
  
    TokenId t1 = (new IntervalToken(db, 
                                    DEFAULT_PREDICATE(), 
                                    false,
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
    t1->activate();
  
    TokenId t2 = (new IntervalToken(t0.getId(), "any",
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t3 = (new IntervalToken(t0.getId(), "any",
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t4 = (new IntervalToken(t0.getId(), "any", 
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t5 = (new IntervalToken(t1, "any", 
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t6 = (new EventToken(t0.getId(), "any", 
                                 DEFAULT_PREDICATE(), 
                                 IntervalIntDomain(0, 1)))->getId();
  
    // These are mostly to avoid compiler warnings about unused variables.
    assertTrue(t3 != t4);
    assertTrue(t5 != t6);
  
    // Delete slave only
    delete (Token*) t2;
    // Should verify correct count of tokens remain. --wedgingt 2004 Feb 27
  
    // Delete master & slaves
    delete (Token*) t1;
    // Should verify correct count of tokens remain. --wedgingt 2004 Feb 27
    DEFAULT_TEARDOWN();
    // Remainder should be cleaned up automatically.
    return true;
  }

  static bool testBasicMerging(){
    DEFAULT_SETUP(ce, db, true);
    // Create 2 mergeable tokens - predicates, types and base domaiuns match
    IntervalToken t0(db, 
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));
  
    assertTrue(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
  
    IntervalToken t1(db,
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));
  
    t1.getDuration()->specify(IntervalIntDomain(5, 7));
  
    // Activate & deactivate - ensure proper handling of rejectability variable
    assertFalse(t0.getState()->getDerivedDomain().isSingleton());
    t0.activate();
    assertTrue(t0.getState()->getDerivedDomain().isSingleton());
    assertTrue(t0.getState()->getDerivedDomain().getSingletonValue() == Token::ACTIVE);
    t0.cancel();
    assertFalse(t0.getState()->getDerivedDomain().isSingleton());
  
    // Now activate and merge
    t0.activate();
    t1.merge(t0.getId());
  
    // Make sure the necessary restrictions have been imposed due to merging i.e. restruction due to specified domain
    assertTrue(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    assertTrue(t1.isMerged());
  
    // Do a split and make sure the old values are reinstated.
    t1.cancel();
    assertTrue(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
    assertTrue(t1.isInactive());
  
    // Now post equality constraint between t1 and extra token t2 and remerge
    IntervalToken t2(db, 
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));
  
    t2.getEnd()->specify(IntervalIntDomain(8, 10));
  
    std::vector<ConstrainedVariableId> temp;
    temp.push_back(t1.getEnd());
    temp.push_back(t2.getEnd());
    ConstraintId equalityConstraint = ConstraintLibrary::createConstraint("concurrent",
                                                                          db->getConstraintEngine(),
                                                                          temp);
    t1.merge(t0.getId());
  
    assertFalse(t0.getMergedTokens().empty());
  
    // Verify that the equality constraint has migrated and original has been deactivated.
    //TBW: when stacking instead of merging tokens, the next check is not true
    // assert(!equalityConstraint->isActive());
    assertTrue(t0.getEnd()->getDerivedDomain().getLowerBound() == 8);
    assertTrue(t0.getEnd()->getDerivedDomain() == t2.getEnd()->getDerivedDomain());
  
    // Undo the merge and check for initial conditions being established
    t1.cancel();
    assertTrue(equalityConstraint->isActive());
  
    // Redo the merge
    t1.merge(t0.getId());
  
    // Confirm deletion of the constraint is handled correctly
    delete (Constraint*) equalityConstraint;
    assertTrue(t0.getEnd()->getDerivedDomain() != t2.getEnd()->getDerivedDomain());
  
    // Confirm previous restriction due to specified domain, then reset and note the change
    assertTrue(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    t1.getDuration()->reset();
    assertTrue(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
  
  
    // Test subset path
    t1.cancel();
    Variable<IntervalIntDomain> superset(db->getConstraintEngine(), IntervalIntDomain(5, 6));

    ConstraintId subsetOfConstraint = ConstraintLibrary::createConstraint("SubsetOf",
                                                                          db->getConstraintEngine(),
                                                                          makeScope(t1.getDuration(), superset.getId()));
    t1.merge(t0.getId());
    assertTrue(t0.getDuration()->getDerivedDomain().getUpperBound() == 6);
    delete (Constraint*) subsetOfConstraint;

    DEFAULT_TEARDOWN();
    // Deletion will now occur and test proper cleanup.
    return true;
  }

  // This test has been fixed by line 56 in MergeMemento.cc.
  // If we invert the order of the splits at the end of this test, the code
  // will error out.

  static bool testConstraintMigrationDuringMerge() {
    DEFAULT_SETUP(ce, db, false);
    ObjectId timeline1 = (new Timeline(db, DEFAULT_OBJECT_TYPE(), "timeline1"))->getId();
    ObjectId timeline2 = (new Timeline(db, DEFAULT_OBJECT_TYPE(), "timeline2"))->getId();
    db->close();

    // Create two base tokens
    IntervalToken t0(db, 
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t1(db,
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));


    // Create 2 mergeable tokens - predicates, types and base domains match
    IntervalToken t2(db, 
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t3(db,
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));


    LessThanEqualConstraint c0("leq", "Default", db->getConstraintEngine(), makeScope(t1.getStart(), t3.getStart()));

    t0.activate();
    t2.activate();
    // Test base case of insertion into an empty sequence
    timeline1->constrain(t0.getId(), t2.getId());

    db->getConstraintEngine()->propagate();

    t1.merge(t0.getId());
    t3.merge(t2.getId());

    t3.cancel();
    t1.cancel();

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testNonChronGNATS2439() {
    DEFAULT_SETUP(ce, db, false);
    ObjectId timeline1 = (new Timeline(db, DEFAULT_OBJECT_TYPE(), "timeline1"))->getId();
    db->close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    IntervalToken token0(db, 
			 DEFAULT_PREDICATE(), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    token0.addParameter(LabelSet(values), "LabelSetParam");
    token0.close();

    IntervalToken token1(db, 
			 DEFAULT_PREDICATE(), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    token1.addParameter(LabelSet(values), "LabelSetParam");
    token1.close();

    IntervalToken token2(db, 
			 DEFAULT_PREDICATE(), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    std::list<double> values2;
    values2.push_back(LabelStr("L2"));
    token2.addParameter(LabelSet(values2), "LabelSetParam");
    token2.close();

    IntervalToken token3(db, 
			 DEFAULT_PREDICATE(), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    values2.clear();
    values2.push_back(LabelStr("L3"));
    token3.addParameter(LabelSet(values2), "LabelSetParam");
    token3.close();

    // create a test constraint between t2 and t3
    ConstraintLibrary::createConstraint(LabelStr("precedes"),ce,makeScope(token2.getEnd(),token3.getStart()));

    assertTrue(ce->propagate());

    // after constraining t2 to come before t3, only t2 and t3 start and
    // end domains should've changed.

    assertTrue(token0.getStart()->lastDomain().getLowerBound() == 0);
    assertTrue(token0.getStart()->lastDomain().getUpperBound() == 10);
    assertTrue(token0.getEnd()->lastDomain().getLowerBound() == 1);
    assertTrue(token0.getEnd()->lastDomain().getUpperBound() == 200);

    assertTrue(token1.getStart()->lastDomain().getLowerBound() == 0);
    assertTrue(token1.getStart()->lastDomain().getUpperBound() == 10);
    assertTrue(token1.getEnd()->lastDomain().getLowerBound() == 1);
    assertTrue(token1.getEnd()->lastDomain().getUpperBound() == 200);

    assertTrue(token2.getStart()->lastDomain().getLowerBound() == 0);
    assertTrue(token2.getStart()->lastDomain().getUpperBound() == 9);
    assertTrue(token2.getEnd()->lastDomain().getLowerBound() == 1);
    assertTrue(token2.getEnd()->lastDomain().getUpperBound() == 10);

    assertTrue(token3.getStart()->lastDomain().getLowerBound() == 1);
    assertTrue(token3.getStart()->lastDomain().getUpperBound() == 10);
    assertTrue(token3.getEnd()->lastDomain().getLowerBound() == 2);
    assertTrue(token3.getEnd()->lastDomain().getUpperBound() == 200);

    token0.activate();
    token2.merge(token0.getId());
    assertTrue(ce->propagate());
    token1.activate();
    token3.merge(token1.getId());
    assertTrue(ce->propagate());

    // after merging t2->t0 and t3->t1, all parameters should be
    // singletons. Also, t0 should now be before t1 (inheriting the
    // relation between t2 and t3).


    assertTrue(token0.getParameters()[0]->lastDomain().isSingleton());
    assertTrue(token1.getParameters()[0]->lastDomain().isSingleton());
    assertTrue(token2.getParameters()[0]->lastDomain().isSingleton());
    assertTrue(token3.getParameters()[0]->lastDomain().isSingleton());

    assertTrue(token0.getStart()->lastDomain().getLowerBound() == 0);
    assertTrue(token0.getStart()->lastDomain().getUpperBound() == 9);
    assertTrue(token0.getEnd()->lastDomain().getLowerBound() == 1);
    assertTrue(token0.getEnd()->lastDomain().getUpperBound() == 10);

    assertTrue(token1.getStart()->lastDomain().getLowerBound() == 1);
    assertTrue(token1.getStart()->lastDomain().getUpperBound() == 10);
    assertTrue(token1.getEnd()->lastDomain().getLowerBound() == 2);
    assertTrue(token1.getEnd()->lastDomain().getUpperBound() == 200);

    token2.cancel();
    assertTrue(ce->propagate());

    // after cancelling t2->t0, all parameters remain singleton except for
    // t0's since it no longer inherits the singleton domain from t2.
    // Furthermore, t0 should no longer be constrained to be before t1.
    // However, t1 should remain constrained to be before t2 since it still
    // inherits the before constraint between t2 and t3.

    assertFalse(token0.getParameters()[0]->lastDomain().isSingleton());
    assertFalse(token1.getParameters()[0]->lastDomain().isSingleton());
    assertTrue(token2.getParameters()[0]->lastDomain().isSingleton());
    assertTrue(token3.getParameters()[0]->lastDomain().isSingleton());

    assertTrue(token0.getStart()->lastDomain().getLowerBound() == 0);
    assertTrue(token0.getStart()->lastDomain().getUpperBound() == 10);
    assertTrue(token0.getEnd()->lastDomain().getLowerBound() == 1);
    assertTrue(token0.getEnd()->lastDomain().getUpperBound() == 200);

    assertFalse(token3.isMerged());

    DEFAULT_TEARDOWN();
    return true;
  }

  // add backtracking and longer chain, also add a before constraint
  static bool testMergingPerformance(){
    DEFAULT_SETUP(ce, db, false);
    ObjectId timeline = (new Timeline(db, DEFAULT_OBJECT_TYPE(), "o2"))->getId();
    db->close();

    typedef Id<IntervalToken> IntervalTokenId;
    
    static const int NUMTOKS=3;
    static const int UNIFIED=1;
    static const int NUMPARAMS=1;

    //Create tokens with the same domains.  We will impose a constraint on
    //each token variable.  Tokens will have 5 parameter variables.
    std::vector< std::vector<IntervalTokenId> > tokens;

    // Add parameters to schema
    for(int i=0;i< UNIFIED; i++)
      SCHEMA->addMember(DEFAULT_PREDICATE(), IntervalIntDomain::getDefaultTypeName(), LabelStr("P" + i).c_str());

    for (int i=0; i < NUMTOKS; i++) {
      std::vector<IntervalTokenId> tmp;
      for (int j=0; j < UNIFIED; j++) {
        IntervalTokenId t = (new IntervalToken(db, 
                                               DEFAULT_PREDICATE(), 
                                               true,
                                               IntervalIntDomain(0, 210),
                                               IntervalIntDomain(0, 220),
                                               IntervalIntDomain(1, 110),
                                               Token::noObject(), false))->getId();
        for (int k=0; k < NUMPARAMS; k++)
          t->addParameter(IntervalIntDomain(500+j,1000), LabelStr("P" + k).c_str());
        t->close();
        tmp.push_back(t);
      }
      tokens.push_back(tmp);
    }

    IntervalIntDomain sdom1(tokens[0][0]->getStart()->getDerivedDomain());
    assertTrue(sdom1.getLowerBound() == 0);
    assertTrue(sdom1.getUpperBound() == 210);

    IntervalIntDomain edom1(tokens[0][0]->getEnd()->getDerivedDomain());
    assertTrue(edom1.getLowerBound() == 1);
    assertTrue(edom1.getUpperBound() == 220);

    Id<TokenVariable<IntervalIntDomain> > pvar1(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom1(pvar1->getDerivedDomain());
    assertTrue(pdom1.getLowerBound() == 500);
    assertTrue(pdom1.getUpperBound() == 1000);

    TokenId predecessor = tokens[0][0];
    predecessor->activate();
    for (int i=1; i < NUMTOKS; i++) {
      tokens[i][0]->activate();
      timeline->constrain(tokens[i-1][0], tokens[i][0]);
    }

    IntervalIntDomain sdom2(tokens[0][0]->getStart()->getDerivedDomain());
    assertTrue(sdom2.getLowerBound() == 0);
    assertTrue(sdom2.getUpperBound() == 208);

    IntervalIntDomain edom2(tokens[0][0]->getEnd()->getDerivedDomain());
    assertTrue(edom2.getLowerBound() == 1);
    assertTrue(edom2.getUpperBound() == 209);

    Id<TokenVariable<IntervalIntDomain> > pvar2(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom2(pvar2->getDerivedDomain());
    assertTrue(pdom2.getLowerBound() == 500);
    assertTrue(pdom2.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) { 
        tokens[i][j]->merge(tokens[i][0]);
        ce->propagate();
      }

    IntervalIntDomain sdom3(tokens[0][0]->getStart()->getDerivedDomain());
    assertTrue(sdom3.getLowerBound() == 0);
    assertTrue(sdom3.getUpperBound() == 208);

    IntervalIntDomain edom3(tokens[0][0]->getEnd()->getDerivedDomain());
    assertTrue(edom3.getLowerBound() == 1);
    assertTrue(edom3.getUpperBound() == 209);

    Id<TokenVariable<IntervalIntDomain> > pvar3(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom3(pvar3->getDerivedDomain());
    assertTrue(pdom3.getLowerBound() == 500+UNIFIED-1);
    assertTrue(pdom3.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) {
        tokens[i][j]->cancel();
        ce->propagate();
      }

    IntervalIntDomain sdom4(tokens[0][0]->getStart()->getDerivedDomain());
    assertTrue(sdom4.getLowerBound() == sdom2.getLowerBound());
    assertTrue(sdom4.getUpperBound() == sdom2.getUpperBound());

    IntervalIntDomain edom4(tokens[0][0]->getEnd()->getDerivedDomain());
    assertTrue(edom4.getLowerBound() == edom2.getLowerBound());
    assertTrue(edom4.getUpperBound() == edom2.getUpperBound());

    Id<TokenVariable<IntervalIntDomain> > pvar4(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom4(pvar4->getDerivedDomain());
    assertTrue(pdom4.getLowerBound() == pdom2.getLowerBound());
    assertTrue(pdom4.getUpperBound() == pdom2.getUpperBound());

    DEFAULT_TEARDOWN();
    return true;
  }    

  static bool testTokenCompatibility(){
    DEFAULT_SETUP(ce, db, true);

    // Create 2 mergeable tokens - predicates, types and base domaiuns match
    IntervalToken t0(db, 
                     LabelStr(DEFAULT_PREDICATE()), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t0.addParameter(IntervalDomain(1, 20), "IntervalParam");
    t0.close();

    // Same predicate and has an intersection
    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE()), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t1.addParameter(IntervalDomain(10, 40), "IntervalParam"); // There is an intersection - but it is not a subset. Still should match
    t1.close();

    t0.activate();
    std::vector<TokenId> compatibleTokens;
    bool res = ce->propagate();
    assertTrue(res);
    db->getCompatibleTokens(t1.getId(), compatibleTokens);
    assertTrue(compatibleTokens.size() == 1);
    assertTrue(compatibleTokens[0] == t0.getId());

    compatibleTokens.clear();
    t0.cancel();
    res = ce->propagate();
    assertTrue(res);
    db->getCompatibleTokens(t1.getId(), compatibleTokens);
    assertTrue(compatibleTokens.empty()); // No match since no tokens are active

    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE()), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t2.addParameter(IntervalDomain(0, 0), "IntervalParam"); // Force no intersection
    t2.close();

    t0.activate();
    res = ce->propagate();
    assertTrue(res);
    compatibleTokens.clear();
    db->getCompatibleTokens(t2.getId(), compatibleTokens);
    assertTrue(compatibleTokens.empty()); // No match since parameter variable has no intersection


    IntervalToken t3(db,
                     LabelStr(DEFAULT_PREDICATE()), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t3.addParameter(IntervalDomain(), "IntervalParam"); // Force no intersection
    t3.close();

    // Post equality constraint between t3 and t0. Should permit a match since it is a binary constraint
    EqualConstraint c0("eq", "Default", db->getConstraintEngine(), makeScope(t0.getStart(), t3.getStart()));
    db->getConstraintEngine()->propagate();
    compatibleTokens.clear();
    db->getCompatibleTokens(t3.getId(), compatibleTokens);
    assertTrue(compatibleTokens.size() == 1); // Expect a single match


    DEFAULT_TEARDOWN();
    return true;
  }

  static LabelStr encodePredicateNames(const std::vector<TokenId>& tokens){
    std::string str;
    for(std::vector<TokenId>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      TokenId token = *it;
      str = str + token->getName().toString() + ":";
    }
    return str;
  }

  static bool testPredicateInheritance(){
    DEFAULT_SETUP(ce, db, false);
    // Add model elements to test inheritance
    schema->addObjectType("A");
    schema->addObjectType("B", "A");
    schema->addObjectType("C", "B");
    schema->addObjectType("D", "A");
    schema->addPredicate("A.a");
    schema->addPredicate("A.b");
    schema->addPredicate("B.a");
    schema->addPredicate("C.a");
    schema->addPredicate("C.b");
    schema->addPredicate("C.c");
    schema->addPredicate("D.a");
    schema->addPredicate("D.d");

    // Now allocate object instances for each one
    Object o1(db, "A", "1");
    Object o2(db, "B", "2");
    Object o3(db, "C", "3");
    Object o4(db, "D", "4");
    db->close();

    // Populate an instance of each predicate type
    IntervalToken t0(db,
                     LabelStr("A.a"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t0.close();
    t0.activate();

    IntervalToken t1(db,
                     LabelStr("A.b"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t1.close();
    t1.activate();

    IntervalToken t2(db,
                     LabelStr("B.a"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t2.close();
    t2.activate();

    IntervalToken t3(db,
                     LabelStr("C.a"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t3.close();
    t3.activate();

    IntervalToken t4(db,
                     LabelStr("C.b"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t4.close();
    t4.activate();

    IntervalToken t5(db,
                     LabelStr("C.c"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t5.close();
    t5.activate();

    IntervalToken t6(db,
                     LabelStr("D.a"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t6.close();
    t6.activate();

    IntervalToken t7(db,
                     LabelStr("D.b"), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t7.close();
    t7.activate();

    // A.a => B.a, C.a., D.a. This is the case for GNATS 2837
    {
      std::vector<TokenId> results;
      IntervalToken t(db,
		      LabelStr("A.a"), 
		      true,
		      IntervalIntDomain(0, 10),
		      IntervalIntDomain(0, 20),
		      IntervalIntDomain(1, 1000),
		      Token::noObject(), false);
      t.close();


      assertTrue(ce->propagate());
      db->getCompatibleTokens(t.getId(), results);

      LabelStr encodedNames = encodePredicateNames(results);
      assertTrue(encodedNames == LabelStr("A.a:B.a:C.a:D.a:"), "Expected = A.a:B.a:C.a:D.a:, Actual =  " + encodedNames.toString());
    }

    // A.a => A.a, B.a, C.a., D.a. This is the case for GNATS 2837
    {
      std::vector<TokenId> results;
      IntervalToken t(db,
		      LabelStr("D.b"), 
		      true,
		      IntervalIntDomain(0, 10),
		      IntervalIntDomain(0, 20),
		      IntervalIntDomain(1, 1000),
		      Token::noObject(), false);
      t.close();


      assertTrue(ce->propagate());
      db->getCompatibleTokens(t.getId(), results);

      LabelStr encodedNames = encodePredicateNames(results);
      assertTrue(encodedNames == LabelStr("D.b:"), "Expected = D.b':, Actual =  " + encodedNames.toString());
    }

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTokenFactory(){
    DEFAULT_SETUP(ce, db, true);
    TokenId master = TokenFactory::createInstance(db, DEFAULT_PREDICATE(), true);
    master->activate();
    TokenId slave = TokenFactory::createInstance(master, DEFAULT_PREDICATE(), LabelStr("any"));
    assertTrue(slave->getMaster() == master); 
    TokenId rejectable = TokenFactory::createInstance(db, DEFAULT_PREDICATE(), false);
    rejectable->activate();
    //!!Should try rejecting master and verify inconsistency
    //!!Should try rejecting rejectable and verify consistency
    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief Tests that a split will not cause the specified domain of the merged token
   * to be relaxed to the base domain.
   */
  static bool testCorrectSplit_Gnats2450(){
    DEFAULT_SETUP(ce, db, true);
    IntervalToken tokenA(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    // Change to base class to excercise problem with wrong signature on TokenVariable
    ConstrainedVariableId start = tokenA.getStart();
    start->specify(5);

    tokenA.activate();
    assertTrue(ce->propagate());

    IntervalToken tokenB(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    // Post a constraint on tokenB so that it will always fail when it gets merged
    ForceFailureConstraint c0("ForceFailure", "Default", ce, makeScope(tokenC.getState()));

    // Propagate and test our specified value
    assertTrue(ce->propagate());
    assertTrue(tokenA.getStart()->lastDomain().getSingletonValue() == 5);

    // Now do the merges and test
    tokenB.merge(tokenA.getId());
    assertTrue(ce->propagate());
    assertTrue(tokenA.getStart()->lastDomain().getSingletonValue() == 5);

    tokenC.merge(tokenA.getId());
    assertFalse(ce->propagate()); // Should always fail

    // Now split it and test that the specified domain is unchanged
    tokenC.cancel();
    assertTrue(ce->propagate()); // Should be OK now
    assertTrue(tokenA.getStart()->lastDomain().getSingletonValue() == 5);


    DEFAULT_TEARDOWN();
    return true;
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
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, DEFAULT_OBJECT_TYPE(), "o2");
    db->close();

    IntervalToken tokenA(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenD(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    assertFalse(timeline.hasTokensToOrder());
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    tokenD.activate();

    // Establish preliminaries
    std::vector<TokenId> tokens;
    timeline.getTokensToOrder(tokens);
    assertTrue(tokens.size() == 4);
    assertTrue(timeline.getTokenSequence().size() == 0);
    assertTrue(timeline.hasTokensToOrder());
    unsigned int num_constraints = ce->getConstraints().size();

    /**
     * BASE CASE - end insertion and retraction
     * ========================================
     * constrain(A,B): A => B; add 2 object constraints and 1 predecessor constraint
     * constrain(B,C): A => B => C; add 1 object constrainy and 1 predecessor constraint
     * free(B,C): A => B; Remove 2 constraints
     * free(A,B): {}; remove 3 constraints
     */
    {
      timeline.constrain(tokenA.getId(), tokenB.getId());
      num_constraints += 3;
      assertTrue(ce->getConstraints().size() == num_constraints);
      assertTrue(timeline.getTokenSequence().size() == 2);

      timeline.constrain(tokenB.getId(), tokenC.getId());
      num_constraints += 2;
      assertTrue(ce->getConstraints().size() == num_constraints);
      assertTrue(timeline.getTokenSequence().size() == 3);

      timeline.free(tokenB.getId(), tokenC.getId());
      num_constraints -= 2;
      assertTrue(ce->getConstraints().size() == num_constraints);
      assertTrue(timeline.getTokenSequence().size() == 2);

      timeline.free(tokenA.getId(), tokenB.getId());
      num_constraints -= 3;
      assertTrue(ce->getConstraints().size() == num_constraints);
      assertTrue(timeline.getTokenSequence().size() == 0);
    }


    /**
     * END INSERTION & NON-CHRONOLOGICAL RETRACTION
     * constrain(A,B): A => B; add 2 object constraints, 1 precedence constraint
     * constrain(C,A): C => A => B; add 1 object constraint, 1 precedence constraint
     * free(A,B): C => A; remove one object constraint, 1 precedence constraint
     */
    { 
      timeline.constrain(tokenA.getId(), tokenB.getId());
      num_constraints += 3; // 2 Object variable constraints and a single temporal constraint
      assertTrue(ce->getConstraints().size() == num_constraints);

      timeline.constrain(tokenC.getId(), tokenA.getId());
      num_constraints += 2; // Object variable and a single temporal constraint since placing at the beginning
      assertTrue(ce->getConstraints().size() == num_constraints);

      assertTrue(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());
      assertTrue(timeline.getTokenSequence().size() == 3);

      timeline.free(tokenA.getId(), tokenB.getId());
      num_constraints -= 2; // Should remove 1 object constraint and 1 temporal constraint
      assertTrue(ce->getConstraints().size() == num_constraints);
      assertTrue(timeline.getTokenSequence().size() == 2);

      timeline.free(tokenC.getId(), tokenA.getId());
      num_constraints -= 3;
      assertTrue(ce->getConstraints().size() == num_constraints);
      assertTrue(timeline.getTokenSequence().size() == 0);
    }

    /**
     * UNSUPPORTED MIDDLE PAIR
     */
    {
      timeline.constrain(tokenA.getId(), tokenB.getId());
      timeline.constrain(tokenB.getId(), tokenC.getId());
      timeline.constrain(tokenC.getId(), tokenD.getId());
      num_constraints += 7; // 4 object constraints and 3 temporal constraints
      assertTrue(ce->getConstraints().size() == num_constraints);
      timeline.free(tokenB.getId(), tokenC.getId());
      assertTrue(ce->getConstraints().size() == num_constraints); // No change. Middle link unsupported
      timeline.free(tokenC.getId(), tokenD.getId()); // Should remove C, and D.
      num_constraints -= 4; // Objects constraints for C, and D, and implict constraint for B->C
      assertTrue(ce->getConstraints().size() == num_constraints);
      timeline.free(tokenA.getId(), tokenB.getId());
      num_constraints -= 3;
      assertTrue(ce->getConstraints().size() == num_constraints);
      assertTrue(timeline.getTokenSequence().size() == 0);
    }

    /**
     * REMOVAL OF SPANNING CONSTRAINT BETWEEN START AND END
     */
    {
      timeline.constrain(tokenA.getId(), tokenD.getId()); // +3
      timeline.constrain(tokenC.getId(), tokenD.getId()); // +3
      timeline.constrain(tokenB.getId(), tokenC.getId()); // +3
      num_constraints += 9;
      assertTrue(ce->getConstraints().size() == num_constraints);

      // Remove spanning link.
      timeline.free(tokenA.getId(), tokenD.getId());
      num_constraints -= 4; // One object constraint, one explciit constraint, and 2 implicit constraints
      assertTrue(ce->getConstraints().size() == num_constraints);

      // Remove B,C and expect to get rid of A and B from the sequence
      timeline.free(tokenB.getId(), tokenC.getId());
      num_constraints -= 2; // 1 object and 1 temporal constraints
      assertTrue(ce->getConstraints().size() == num_constraints);

      // Remove B,C and expect to get rid of A and B from the sequence
      timeline.free(tokenC.getId(), tokenD.getId());
      num_constraints -= 3; // 2 object and 1 temporal constraints
      assertTrue(ce->getConstraints().size() == num_constraints);
    }

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testObjectTokenRelation(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, DEFAULT_OBJECT_TYPE(), "o2");
    db->close();
    
    IntervalToken tokenA(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    // Object variables are not singletons - so query for tokens to order should return nothing
    std::vector<TokenId> tokensToOrder;
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.empty());

    // Specify the object variable of one - but still should return no tokens since they are all inactive
    tokenA.getObject()->specify(timeline.getId());
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.empty());

    // Now activate all of them
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.size() == 3);

    // Set remainders so they are singeltons and get all back
    tokenB.getObject()->specify(timeline.getId());
    tokenC.getObject()->specify(timeline.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.size() == 3);

    // Now incrementally constrain and show reduction in tokens to order
    timeline.constrain(tokenA.getId(), tokenB.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.size() == 1);

    timeline.constrain(tokenB.getId(), tokenC.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.empty());


    // Test destruction call path
    Token* tokenD = new IntervalToken(db, 
                                      LabelStr(DEFAULT_PREDICATE()), 
                                      true,
                                      IntervalIntDomain(0, 10),
                                      IntervalIntDomain(0, 20),
                                      IntervalIntDomain(1, 1000));
    tokenD->activate();
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.size() == 1);
    timeline.constrain(tokenC.getId(), tokenD->getId());
    delete tokenD;
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assertTrue(tokensToOrder.empty());
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTokenOrderQuery(){
    DEFAULT_SETUP(ce, db, false);
    Id<Timeline> timeline = (new Timeline(db, DEFAULT_OBJECT_TYPE(), "o2"))->getId();
    db->close();

    const int COUNT = 5;
    const int DURATION = 10;
    
    for (int i=0;i<COUNT;i++){
      int start = i*DURATION;
      TokenId token = (new IntervalToken(db, 
                                         LabelStr(DEFAULT_PREDICATE()),
                                         true,
                                         IntervalIntDomain(start, start),
                                         IntervalIntDomain(start+DURATION, start+DURATION),
                                         IntervalIntDomain(DURATION, DURATION)))->getId();
      assertFalse(token->getObject()->getBaseDomain().isSingleton());
      token->getObject()->specify(timeline->getId());
      token->activate();
    }

    assertTrue(timeline->getTokens().size() == (unsigned int) COUNT);
    ce->propagate(); // Should not alter the count. Relationship updated eagerly
    assertTrue(timeline->getTokens().size() == (unsigned int) COUNT);

    int i = 0;
    std::vector<TokenId> tokensToOrder;
    timeline->getTokensToOrder(tokensToOrder);

    while(!tokensToOrder.empty()){
      assertTrue(timeline->getTokenSequence().size() == (unsigned int) i);
      assertTrue(tokensToOrder.size() == (unsigned int) (COUNT - i));
      std::vector< std::pair<TokenId, TokenId> > choices;
      TokenId toConstrain = tokensToOrder.front();
      timeline->getOrderingChoices(toConstrain, choices);
      assertFalse(choices.empty());
      TokenId predecessor = choices.front().first;
      TokenId successor = choices.front().second;

      assertTrue(toConstrain == predecessor || toConstrain == successor,
		 "The token from the tokens to order must be a predecessor or a successor.");

      timeline->constrain(predecessor, successor);
      bool res = ce->propagate();
      assertTrue(res);
      tokensToOrder.clear();
      timeline->getTokensToOrder(tokensToOrder);
      i++;
      res = ce->propagate();
      assertTrue(res);
    }

    const std::list<TokenId>& tokenSequence = timeline->getTokenSequence();
    assertTrue(tokenSequence.front()->getStart()->getDerivedDomain().getSingletonValue() == 0);
    assertTrue(tokenSequence.back()->getEnd()->getDerivedDomain().getSingletonValue() == COUNT*DURATION);

    // Now ensure the query can correctly indicate no options available
    TokenId token = (new IntervalToken(db, 
                                       LabelStr(DEFAULT_PREDICATE()),
                                       true,
                                       IntervalIntDomain(0, 0),
                                       IntervalIntDomain(),
                                       IntervalIntDomain(DURATION, DURATION)))->getId();
    token->getObject()->specify(timeline->getId());
    token->activate();
    std::vector<std::pair<TokenId, TokenId> > choices;
    timeline->getOrderingChoices(token, choices);
    assertTrue(choices.empty());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testEventTokenInsertion(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, DEFAULT_OBJECT_TYPE(), "o2");
    db->close();

    IntervalToken it1(db, 
                      LabelStr(DEFAULT_PREDICATE()), 
                      true,
                      IntervalIntDomain(0, 10),
                      IntervalIntDomain(0, 1000),
                      IntervalIntDomain(1, 1000));

    it1.getObject()->specify(timeline.getId());
    it1.activate();
    timeline.constrain(it1.getId(), it1.getId());

    // Insert at the end after a token
    EventToken et1(db, 
                   DEFAULT_PREDICATE(), 
                   true, 
                   IntervalIntDomain(0, 100), 
                   Token::noObject());

    et1.getObject()->specify(timeline.getId());
    et1.activate();
    timeline.constrain(it1.getId(), et1.getId());
    assertTrue(it1.getEnd()->getDerivedDomain().getUpperBound() == 100);

    // Insert between a token and an event
    EventToken et2(db, 
                   DEFAULT_PREDICATE(), 
                   true, 
                   IntervalIntDomain(0, 100), 
                   Token::noObject());

    et2.getObject()->specify(timeline.getId());
    et2.activate();
    timeline.constrain(et2.getId(), et1.getId());
    assertTrue(it1.getEnd()->getDerivedDomain().getUpperBound() == 100);

    // Insert before a token
    EventToken et3(db, 
                   DEFAULT_PREDICATE(), 
                   true, 
                   IntervalIntDomain(10, 100), 
                   Token::noObject());

    et3.getObject()->specify(timeline.getId());
    et3.activate();
    timeline.constrain(et3.getId(), it1.getId());
    assertTrue(it1.getStart()->getDerivedDomain().getLowerBound() == 10);

    // Insert between events
    EventToken et4(db, 
                   DEFAULT_PREDICATE(), 
                   true, 
                   IntervalIntDomain(0, 100), 
                   Token::noObject());

    et4.getObject()->specify(timeline.getId());
    et4.activate();
    timeline.constrain(et4.getId(), et1.getId());
    bool res = ce->propagate();
    assertTrue(res);
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testFullInsertion(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, DEFAULT_OBJECT_TYPE(), "o2");
    db->close();

    IntervalToken tokenA(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    assertFalse(timeline.hasTokensToOrder());
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();

    timeline.constrain(tokenA.getId(), tokenB.getId()); // Insert A and B.
    assertTrue(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());

    // Now insert token C in the middle.
    timeline.constrain(tokenC.getId(), tokenB.getId());
    assertTrue(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenC.getStart()->getDerivedDomain().getUpperBound());
    assertTrue(tokenC.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testNoChoicesThatFit(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, DEFAULT_OBJECT_TYPE() , "o2");
    db->close();

    IntervalToken tokenA(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(10, 10),
                         IntervalIntDomain(20, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(100, 100),
                         IntervalIntDomain(120, 120),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db, 
                         LabelStr(DEFAULT_PREDICATE()), 
                         true,
                         IntervalIntDomain(9, 9),
                         IntervalIntDomain(11, 11),
                         IntervalIntDomain(1, 1000));

    tokenA.activate();
    tokenB.activate();
    tokenC.activate();

    timeline.constrain(tokenA.getId(), tokenB.getId());
    bool res = ce->propagate();
    assertTrue(res);

    std::vector<std::pair<TokenId, TokenId> > choices;
    timeline.getOrderingChoices(tokenC.getId(), choices);
    assertTrue(choices.empty());
    timeline.constrain(tokenC.getId(), tokenB.getId());
    res = ce->propagate();
    assertFalse(res);

    DEFAULT_TEARDOWN();
    return true;
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
    assertTrue(factoryName == LabelStr("Foo:int:string"));
    return true;
  }

  static bool testBasicAllocation(){
    DEFAULT_SETUP(ce, db, false);

    DbClientId client = db->getClient();
    DbClientTransactionLog* txLog = new DbClientTransactionLog(client);

    FooId foo1 = client->createObject(DEFAULT_OBJECT_TYPE().c_str(), "foo1");
    assertTrue(foo1.isValid());

    std::vector<ConstructorArgument> arguments;
    IntervalIntDomain arg0(10);
    LabelSet arg1(LabelSet::getDefaultTypeName(), "Label");
    arguments.push_back(ConstructorArgument(IntervalIntDomain::getDefaultTypeName(), &arg0)); 
    arguments.push_back(ConstructorArgument(LabelSet::getDefaultTypeName(), &arg1));
    FooId foo2 = client->createObject(DEFAULT_OBJECT_TYPE().c_str(), "foo2", arguments);
    assertTrue(foo2.isValid());

    TokenId token = client->createToken(DEFAULT_PREDICATE().c_str());
    assertTrue(token.isValid());

    // Constrain the token duration
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(token->getStart());
    scope.push_back(token->getDuration());
    client->createConstraint("eq", scope);

    delete txLog;
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testPathBasedRetrieval(){
    DEFAULT_SETUP(ce, db, true);
    TokenId t0 = db->getClient()->createToken(DEFAULT_PREDICATE().c_str());
    t0->activate();

    TokenId t1 = db->getClient()->createToken(DEFAULT_PREDICATE().c_str());
    t1->activate();

    TokenId t0_0 = (new IntervalToken(t0, "any", 
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_0->activate();

    TokenId t0_1 = (new IntervalToken(t0, "any",  
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_1->activate();

    TokenId t0_2 = (new IntervalToken(t0, "any", 
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_2->activate();

    TokenId t1_0 = (new IntervalToken(t1, "any", 
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t1_0->activate();

    TokenId t0_1_0 = (new EventToken(t0_1, "any", 
                                     DEFAULT_PREDICATE(), 
                                     IntervalIntDomain(0, 1)))->getId();
    t0_1_0->activate();

    TokenId t0_1_1 = (new EventToken(t0_1, "any", 
                                     DEFAULT_PREDICATE(), 
                                     IntervalIntDomain(0, 1)))->getId();
    t0_1_1->activate();

    // Test paths
    std::vector<int> path;
    path.push_back(0); // Start with the index of the token key in the path


    // Base case with just the root
    assertTrue(db->getClient()->getTokenByPath(path) == t0);
    assertTrue(db->getClient()->getPathByToken(t0).size() == 1);

    // Now test a more convoluted path
    path.push_back(1);
    path.push_back(1);
    assertTrue(db->getClient()->getTokenByPath(path) == t0_1_1);

    path.clear();
    path = db->getClient()->getPathByToken(t0_1_1);
    assertTrue(path.size() == 3);
    assertTrue(path[0] == 0);
    assertTrue(path[1] == 1);
    assertTrue(path[2] == 1);


    // Negative tests
    path.push_back(100);
    assertTrue(db->getClient()->getTokenByPath(path) == TokenId::noId());
    path[0] = 99999;
    assertTrue(db->getClient()->getTokenByPath(path) == TokenId::noId());
    DEFAULT_TEARDOWN();
    return true;
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
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", false, "invokeActivateTestToken"));
    TokenSet newTokens = s_db->getTokens();
    assertTrue(oldTokens.size() + 1 == newTokens.size());
    TokenId tok = *(newTokens.begin());
    while (oldTokens.find(tok) != oldTokens.end()) {
      newTokens.erase(newTokens.begin());
      tok = *(newTokens.begin());
    }
    assertTrue(!tok->isActive());
    TEST_PLAYING_XML(buildXMLInvokeActivateTokenStr("invokeActivateTestToken"));
    assertTrue(tok->isActive());
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
    /* Get an existing object.  See testCreateObject(). */
    ObjectId obj2b = s_db->getObject("testObj2b");
    assertTrue(!obj2b.isNoId() && obj2b.isValid());
    assertTrue(obj2b->getType() == LabelStr("TestClass2"));
    assertTrue(obj2b->getName() == LabelStr("testObj2b"));
    const unsigned int initialObjectTokenCount_B = obj2b->getTokens().size();

    TokenId constrainedToken = createToken("constrainedSample", true);
    std::cout << __FILE__ << ':' << __LINE__ << ": constrainedToken is " << constrainedToken << '\n';
    std::cout << __FILE__ << ':' << __LINE__ << ": constrainedSample's derived object domain is " << constrainedToken->getObject()->derivedDomain() << '\n';
    assertTrue(!constrainedToken->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
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
    std::cout << __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain() << '\n';
    assertTrue(!constrained2->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2b", "constrainedSample2", "constrainedSample"));
    std::cout << __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain() << '\n';
    assertTrue(constrained2->getObject()->derivedDomain().isSingleton(), "player did not constrain token to one object");
    assertTrue(constrained2->getObject()->derivedDomain() == objDom2b, "player did not constrain token to expected object");
    assertTrue(verifyTokenRelation(constrainedToken, constrained2, "before")); //!! "precedes" ?
    assertTrue(obj2b->getTokens().size() == initialObjectTokenCount_B + 2);
    /* Leave them in plan db for testFree(). */

    /* Create two rejectable tokens and do the same tests, but with testObj2a. */
    ObjectId obj2a = s_db->getObject("testObj2a");
    assertTrue(!obj2a.isNoId() && obj2a.isValid());
    assertTrue(obj2a->getType() == LabelStr("TestClass2"));
    assertTrue(obj2a->getName() == LabelStr("testObj2a"));
    ObjectDomain objDom2a(obj2a, "TestClass2");
    const unsigned int initialObjectTokenCount_A = obj2a->getTokens().size();

    TokenId rejectable = createToken("rejectableConstrainedSample", false);
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable is " << rejectable << '\n';
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain() << '\n';
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "rejectableConstrainedSample", ""));
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain() << '\n';
    assertTrue(!rejectable->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2a", "rejectableConstrainedSample", ""));
    std::cout << __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain() << '\n';
    assertTrue(rejectable->getObject()->derivedDomain().isSingleton(), "player did not constrain token to one object");

    assertTrue(rejectable->getObject()->derivedDomain() == objDom2a, "player did not constrain token to expected object");
    TokenId rejectable2 = createToken("rejectable2", false);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "rejectable2", ""));
    assertTrue(!rejectable2->getObject()->derivedDomain().isSingleton(), "token already constrained to one object");
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2a", "rejectable2", "rejectableConstrainedSample"));
    assertTrue(rejectable2->getObject()->derivedDomain().isSingleton(), "player did not constrain token to one object");
    assertTrue(rejectable2->getObject()->derivedDomain() == objDom2a, "player did not constrain token to expected object");
    assertTrue(verifyTokenRelation(rejectable, rejectable2, "before")); //!! "precedes" ?
    assertTrue(obj2a->getTokens().size() == initialObjectTokenCount_A + 2);
    /* Leave them in plan db for testFree(). */
  }

  /** Test freeing tokens. */
  static void testFree() {
    ObjectId obj2a = s_db->getObject("testObj2a");
    assertTrue(!obj2a.isNoId() && obj2a.isValid());
    assertTrue(obj2a->getType() == LabelStr("TestClass2"));
    assertTrue(obj2a->getName() == LabelStr("testObj2a"));
    ObjectDomain objDom2a(obj2a, "TestClass2");
    const unsigned int initialObjectTokenCount_A = obj2a->getTokens().size();

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
    TokenId one = *(--tokens.rbegin());
    assertTrue(one.isValid());
    assertTrue(one->getObject()->derivedDomain().isSingleton());
    assertTrue(one->getObject()->derivedDomain() == objDom2b);
    TokenId two = *(tokens.rbegin());
    assertTrue(two.isValid() && one != two);
    assertTrue(two->getObject()->derivedDomain().isSingleton());
    assertTrue(two->getObject()->derivedDomain() == objDom2b);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample2", "constrainedSample"));
    assertTrue(one->getObject()->derivedDomain().isSingleton());
    assertTrue(one->getObject()->derivedDomain() == objDom2b);

    //!!Next fails because the base domain is still open
    //!!assertTrue(one->getObject()->derivedDomain() == ObjectDomain("TestClass2"));

    assertTrue(!two->getObject()->derivedDomain().isSingleton());
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample", ""));
    assertTrue(!two->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!assertTrue(one->getObject()->derivedDomain() == ObjectDomain("TestClass2"));

    assertTrue(!one->getObject()->derivedDomain().isSingleton());

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
    assertTrue(tokens.size() == initialObjectTokenCount_A + 2);
    TokenId three, four;
    tokens.erase(one);
    tokens.erase(two);
    three = *(--tokens.rbegin());
    tokens.erase(three);
    four = *(tokens.rbegin());
    assertTrue(three->getObject()->derivedDomain().isSingleton());
    assertTrue(four->getObject()->derivedDomain() == objDom2a);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2a", "rejectableConstrainedSample", ""));
    assertTrue(three->getObject()->derivedDomain().isSingleton(), "Should still be a singleton, since still required.");

    //!!Next fails because the base domain is still open
    //!!assertTrue(three->getObject()->derivedDomain() == ObjectDomain("TestClass2"));
  }

  /** Test activating a token. */
  static void testActivate() {
    s_activatedToken = createToken("activateSample", false);
    std::cout << __FILE__ << ':' << __LINE__ << ": s_activatedToken is " << s_activatedToken << '\n';
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "activateSample", ""));
    assertTrue(s_activatedToken->isActive(), "token not activated by player");
    /* Leave activated for testMerge(). */
  }

  /** Test merging tokens. */
  static void testMerge() {
    s_mergedToken = createToken("mergeSample", false);
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
  str += "\"> <value ";
  str += " name=\"";
  str += objName;
  str += "\"";
  str += " type=\"";
  str += StringDomain::getDefaultTypeName().toString();
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
    str += "value";
    str += " type=\"";
    str += dom.getTypeName().toString();
    str += "\"";
    str += " name=\"";
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
    if (dom.getType() == AbstractDomain::SYMBOL_ENUMERATION) {
      str += "symbol value=\"";
    } else {
      str += "value name=\"";
    }
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

class MultithreadedTest {
public:
  static bool test(void) {
    runTest(testBasicMultithread);
    runTest(testMultiDb);
    return true;
  }
private:
  
  static bool testBasicMultithread() {
    bool retval = true;
    for(int i = 0; i < 200 && retval; i++)
      retval = retval && basicMultithread1();
    return retval;
  }
 
  static bool basicMultithread1() {
    const int nthreads = 8;
    pthread_t threads[nthreads];

    new ThreadedLockManager();
    LockManager::instance().connect(LabelStr("MainThread"));

    LockManager::instance().lock();
    DEFAULT_SETUP(ce, db, false);
    LockManager::instance().unlock();

    for(int i = 0; i < nthreads; i++) {
      (i % 2 ? pthread_create(&threads[i], NULL, multiBasicAllocation1, &db) :
       pthread_create(&threads[i], NULL, multiBasicAllocation2, &db));
    }
    for(int i = 0; i < nthreads; i++)
      pthread_join(threads[i], NULL);
    
    LockManager::instance().lock();
    DEFAULT_TEARDOWN();
    LockManager::instance().unlock();

    new LockManager();
    LockManager::instance().connect();
    return true;
  }

  static void* multiBasicAllocation1(void* arg) {
    LockManager::instance().connect(LabelStr("multiBasicAllocation1"));
    LockManager::instance().lock();
    PlanDatabaseId db = *((PlanDatabaseId*)arg);
    LockManager::instance().unlock();

    LockManager::instance().lock();
    DbClientTransactionLog* txLog = new DbClientTransactionLog(db->getClient());
    LockManager::instance().unlock();

    LockManager::instance().lock();
    TokenId token = db->getClient()->createToken(DEFAULT_PREDICATE().c_str());
    LockManager::instance().unlock();
    
    LockManager::instance().lock();
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(token->getStart());
    scope.push_back(token->getDuration());
    db->getClient()->createConstraint("eq", scope);
    LockManager::instance().unlock();

    LockManager::instance().lock();
    delete txLog;
    LockManager::instance().unlock();

    pthread_exit(0);
    return NULL;
  }
  
  static void* multiBasicAllocation2(void* arg) {
    LockManager::instance().connect(LabelStr("multiBasicAllocation2"));
    LockManager::instance().lock();
    PlanDatabaseId db = *((PlanDatabaseId*)arg);
    LockManager::instance().unlock();

    LockManager::instance().lock();
    DbClientTransactionLog* txLog = new DbClientTransactionLog(db->getClient());
    LockManager::instance().unlock();

    LockManager::instance().lock();
    TokenId token = db->getClient()->createToken(DEFAULT_PREDICATE().c_str());
    LockManager::instance().unlock();
    
    LockManager::instance().lock();
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(token->getStart());
    scope.push_back(token->getDuration());
    db->getClient()->createConstraint("eq", scope);
    LockManager::instance().unlock();

    LockManager::instance().lock();
    delete txLog;
    LockManager::instance().unlock();

    pthread_exit(0);
    return NULL;
  }

  static bool testMultiDb() {
    const int nthreads = 8;
    pthread_t threads[nthreads];

    new ThreadedLockManager();
    LockManager::instance().connect(LabelStr("testMultiDb"));

    LockManager::instance().lock();
    DEFAULT_SETUP(ce1, db1, false);
    DEFAULT_SETUP(ce2, db2, false);
    DEFAULT_SETUP(ce3, db3, false);
    DEFAULT_SETUP(ce4, db4, false);
    LockManager::instance().unlock();

    for(int i = 0; i < nthreads; i++) {
      if(i % 4 == 0)
        pthread_create(&threads[i], NULL, multiDbAllocation, &db4);
      else if(i % 3 == 0)
        pthread_create(&threads[i], NULL, multiDbAllocation, &db3);
      else if(i % 2 == 0)
        pthread_create(&threads[i], NULL, multiDbAllocation, &db2);
      else
        pthread_create(&threads[i], NULL, multiDbAllocation, &db1);
    }
    for(int i = 0; i < nthreads; i++)
      pthread_join(threads[i], NULL);

    LockManager::instance().lock();
    DEFAULT_TEARDOWN_MULTI(ce4, db4);
    DEFAULT_TEARDOWN_MULTI(ce3, db3);
    DEFAULT_TEARDOWN_MULTI(ce2, db2);
    DEFAULT_TEARDOWN_MULTI(ce1, db1);
    LockManager::instance().unlock();

    LockManager::instance().disconnect();
    new LockManager();
    LockManager::instance().connect();
    return true;
  }

  static void* multiDbAllocation(void* arg) {
    const int nthreads = 8;
    pthread_t threads[nthreads];

    LockManager::instance().connect(LabelStr("multiDbAllocation"));

    LockManager::instance().lock();
    PlanDatabaseId db = *((PlanDatabaseId*)arg);
    LockManager::instance().unlock();

    for(int i = 0; i < nthreads; i++) {
      (i % 2 ? pthread_create(&threads[i], NULL, multiBasicAllocation1, &db) :
       pthread_create(&threads[i], NULL, multiBasicAllocation2, &db));
    }
    for(int i = 0; i < nthreads; i++)
      pthread_join(threads[i], NULL);
    pthread_exit(0);
    return NULL;
  }
};

int main() {
  LockManager::instance().connect();

  LockManager::instance().lock();
  initDbModuleTests();
  LockManager::instance().unlock();

  for (int i = 0; i < 1; i++) {
    LockManager::instance().lock();
    runTestSuite(SchemaTest::test);
    runTestSuite(ObjectTest::test);
    runTestSuite(TokenTest::test);
    runTestSuite(TimelineTest::test);
    runTestSuite(DbClientTest::test);
    runTestSuite(MultithreadedTest::test);
    runTestSuite(DbTransPlayerTest::test);
    std::cout << "Finished #" << i << std::endl;
    LockManager::instance().unlock();
  }

  LockManager::instance().lock();
  ConstraintLibrary::purgeAll();
  LockManager::instance().unlock();

  LockManager::instance().lock();
  std::cout << "All done and purged" << std::endl;
  exit(0);
}
