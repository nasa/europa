#include "PlanDbModuleTests.hh"
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

#include "TestSupport.hh"
#include "Utils.hh"
#include "IntervalIntDomain.hh"
#include "StringDomain.hh"
#include "DefaultPropagator.hh"
#include "EqualityConstraintPropagator.hh"
#include "Constraint.hh"

#include <iostream>
#include <string>

namespace PLASMA {


  const LabelStr& DEFAULT_OBJECT_TYPE(){
    static const LabelStr sl_local("DEFAULT_OBJECT_TYPE");
    return sl_local;
  }

  const LabelStr& DEFAULT_PREDICATE(){
    static const LabelStr sl_local("DEFAULT_OBJECT_TYPE.DEFAULT_PREDICATE");
    return sl_local;
  }

  void initDbTestSchema(const SchemaId& schema) {
    schema->reset();
    // Set up object types and compositions for testing - builds a recursive structure
    schema->addObjectType(DEFAULT_OBJECT_TYPE());
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id0");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id1");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id2");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id3");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id4");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id5");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id6");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id7");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id8");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "id9");

    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o0");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o1");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o2");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o3");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o4");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o5");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o6");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o7");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o8");
    schema->addMember(DEFAULT_OBJECT_TYPE(), DEFAULT_OBJECT_TYPE(), "o9");

    // Set up primitive object type member variables for testing
    schema->addMember(DEFAULT_OBJECT_TYPE(), IntervalDomain::getDefaultTypeName(), "IntervalVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), IntervalIntDomain::getDefaultTypeName(), "IntervalIntVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), BoolDomain::getDefaultTypeName(), "BoolVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), LabelSet::getDefaultTypeName(), "LabelSetVar");
    schema->addMember(DEFAULT_OBJECT_TYPE(), EnumeratedDomain::getDefaultTypeName(), "EnumeratedVar");

    // Set up predicates for testing
    schema->addPredicate(DEFAULT_PREDICATE());
    schema->addMember(DEFAULT_PREDICATE(), IntervalDomain::getDefaultTypeName(), "IntervalParam");
    schema->addMember(DEFAULT_PREDICATE(), IntervalIntDomain::getDefaultTypeName(), "IntervalIntParam");
    schema->addMember(DEFAULT_PREDICATE(), BoolDomain::getDefaultTypeName(), "BoolParam");
    schema->addMember(DEFAULT_PREDICATE(), LabelSet::getDefaultTypeName(), "LabelSetParam");
    schema->addMember(DEFAULT_PREDICATE(), EnumeratedDomain::getDefaultTypeName(), "EnumeratedParam");
  }

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
      check_error(arguments.empty());
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
    IntervalTokenFactory(): ConcreteTokenFactory(DEFAULT_PREDICATE()){}
  private:
    TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name) const{
      TokenId token = (new IntervalToken(planDb, name, true))->getId();
      return token;
    }
    TokenId createInstance(const TokenId& master, const LabelStr& name) const{
      TokenId token = (new IntervalToken(master, name))->getId();
      return token;
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

    // Special designations for temporal relations
    REGISTER_CONSTRAINT(EqualConstraint, "concurrent", "Default");
    REGISTER_CONSTRAINT(LessThanEqualConstraint, "precedes", "Default");
    REGISTER_CONSTRAINT(AddEqualConstraint, "StartEndDurationRelation", "Default");
    
    // Support for Token implementations
    REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
    REGISTER_CONSTRAINT(ForceFailureConstraint, "ForceFailure", "Default");
    
    // Allocate default schema initially so tests don't fail because of ID's
    SCHEMA;
    initDbTestSchema(SCHEMA);

    // Have to register factories for testing.
    new StandardFooFactory();
    new SpecialFooFactory();
    new IntervalTokenFactory();
  }

  //class Foo;
  bool testBasicObjectAllocationImpl() {
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    Object o2(db.getId(), DEFAULT_OBJECT_TYPE(), "o2");

    ObjectId id0((new Object(o1.getId(), DEFAULT_OBJECT_TYPE(), "id0"))->getId());
    Object o3(o2.getId(), DEFAULT_OBJECT_TYPE(), "o3");
    assert(db.getObjects().size() == 4);
    assert(o1.getComponents().size() == 1);
    assert(o3.getParent() == o2.getId());
    delete (Object*) id0;
    assert(db.getObjects().size() == 3);
    assert(o1.getComponents().empty());

    ObjectId id1((new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "id1"))->getId());
    new Object(id1, DEFAULT_OBJECT_TYPE(), "id2");
    ObjectId id3((new Object(id1, DEFAULT_OBJECT_TYPE(), "id3"))->getId());
    assert(db.getObjects().size() == 6);
    assert(id3->getName().toString() == "id1.id3");

    // Test ancestor call
    ObjectId id4((new Object(id3, DEFAULT_OBJECT_TYPE(), "id4"))->getId());
    std::list<ObjectId> ancestors;
    id4->getAncestors(ancestors);
    assert(ancestors.front() == id3);
    assert(ancestors.back() == id1);

    // Force cascaded delete
    delete (Object*) id1;
    assert(db.getObjects().size() == 3);

    // Now allocate dynamically and allow the plan database to clean it up when it deallocates
    ObjectId id5 = ((new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "id5"))->getId());
    new Object(id5, DEFAULT_OBJECT_TYPE(), "id6");
    return(true);
  }

  bool testObjectDomainImpl() {
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    std::list<ObjectId> values;
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    Object o2(db.getId(), DEFAULT_OBJECT_TYPE(), "o2");
    assert(db.getObjects().size() == 2);
    values.push_back(o1.getId());
    values.push_back(o2.getId());
    ObjectDomain os1(values, DEFAULT_OBJECT_TYPE().c_str());
    assert(os1.isMember(o1.getId()));
    os1.remove(o1.getId());
    assert(!os1.isMember(o1.getId()));
    assert(os1.isSingleton());
    return true;
  }

  bool testObjectVariablesImpl() {
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1", true);
    assert(!o1.isComplete());
    o1.addVariable(IntervalIntDomain(), "IntervalIntVar");
    o1.addVariable(BoolDomain(), "BoolVar");
    o1.close();
    assert(o1.isComplete());
    assert(o1.getVariable("o1.BoolVar") != o1.getVariable("o1IntervalIntVar"));

    Object o2(db.getId(), DEFAULT_OBJECT_TYPE(), "o2", true);
    assert(!o2.isComplete());
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

    assert(db.getConstraintEngine()->propagate());
    assert(o1.getVariables()[0]->lastDomain() == o1.getVariables()[0]->lastDomain());

    // Delete one of the constraints to force automatic clean-up path and explciit clean-up
    delete (Constraint*) constraint;
    delete (Constraint*) subsetConstraint;

    return(true);
  }


  bool testObjectObjectTokenRelationImpl() {
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    // 1. Create 2 objects
    ObjectId object1 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "O1"))->getId();
    ObjectId object2 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "O2"))->getId();    
    db.close();

    assert(object1 != object2);
    assert(db.getObjects().size() == 2);
    // 2. Create 1 token.
    EventToken eventToken(db.getId(), DEFAULT_PREDICATE(), false, IntervalIntDomain(0, 10));

    // Confirm not added to the object
    assert(!eventToken.getObject()->getDerivedDomain().isSingleton());

    // 3. Activate token. (NO subgoals)
    eventToken.activate();

    // Confirm not added to the object
    assert(!eventToken.getObject()->getDerivedDomain().isSingleton());

    // 4. Specify tokens object variable to a ingletone

    eventToken.getObject()->specify(object1);

    // Confirm added to the object
    assert(eventToken.getObject()->getDerivedDomain().isSingleton());

    // 5. propagate
    db.getConstraintEngine()->propagate();

    // 6. reset object variables domain.
    eventToken.getObject()->reset();

    // Confirm it is no longer part of the object
    // Confirm not added to the object
    assert(!eventToken.getObject()->getDerivedDomain().isSingleton());

    return true;
  }

  bool testCommonAncestorConstraintImpl() {
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

      assert(ENGINE->propagate());
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

      assert(!ENGINE->propagate());
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

      assert(ENGINE->propagate());
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

      assert(ENGINE->propagate()); // All ok so far

      restrictions.specify(o2.getId());
      assert(ENGINE->propagate()); // Nothing happens yet.

      first.specify(o6.getId()); // Now we should propagate to failure
      assert(!ENGINE->propagate());
      first.reset();

      first.specify(o4.getId());
      assert(ENGINE->propagate());
    }    
    return true;
  }

  bool testHasAncestorConstraintImpl() {
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
      
      assert(ENGINE->propagate());
    }
    
    // negative test immediate ancestor
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o2.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assert(!ENGINE->propagate());
    }
    // Positive test higher up  ancestor
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o1.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assert(ENGINE->propagate());
    }
    // negative test higherup ancestor
    {
      Variable<ObjectDomain> first(ENGINE, ObjectDomain(o7.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      Variable<ObjectDomain> restrictions(ENGINE, ObjectDomain(o8.getId(), DEFAULT_OBJECT_TYPE().c_str()));
      HasAncestorConstraint constraint("hasAncestor", 
                                       "Default", 
                                       ENGINE, 
                                       makeScope(first.getId(), restrictions.getId()));
      
      assert(!ENGINE->propagate());
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
      
      assert(ENGINE->propagate());
      assert(first.getDerivedDomain().isSingleton());
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
      
      assert(ENGINE->propagate());
      assert(first.getDerivedDomain().getSize() == 2);
    }
    
    return true;
  }

  bool testMakeObjectVariableImpl() {
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    ConstrainedVariableId v0 = (new Variable<ObjectDomain>(ENGINE, ObjectDomain(DEFAULT_OBJECT_TYPE().c_str())))->getId();
    assert(!v0->isClosed());
    db.makeObjectVariableFromType(DEFAULT_OBJECT_TYPE(), v0);
    assert(!v0->isClosed());
    assert(ENGINE->propagate());

    // Now add an object and we should expect the constraint network to be consistent
    Object o1(db.getId(), DEFAULT_OBJECT_TYPE(), "o1");
    assert(ENGINE->propagate());
    assert(!db.isClosed(DEFAULT_OBJECT_TYPE().c_str()));
    assert(v0->lastDomain().isSingleton() && v0->lastDomain().getSingletonValue() == o1.getId());

    // Now delete the variable. This should remove the listener
    delete (ConstrainedVariable*) v0;

    return true;
  }

  bool testTokenObjectVariableImpl() {
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    // Now add an object and we should expect the constraint network to be consistent
    ObjectId o1 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "o1"))->getId();
    EventToken eventToken(db.getId(), DEFAULT_PREDICATE(), false, IntervalIntDomain(0, 10));

    eventToken.activate(); // Must be activate to eventually propagate the objectTokenRelation
    assert(ENGINE->propagate());

    // Make sure the object var of the token contains o1.
    assert(eventToken.getObject()->lastDomain().isMember(o1));

    // Since the object type has not been closed, the object variable will not propagate changes,
    // so the object token relation will not link up the Token and the object.
    assert(o1->getTokens().empty());

    // Deletion of the object should result in the domain of the token becoming empty. However,
    // that will not cause an inconsistency. Nor will it cuase propagation
    delete (Object*) o1;
    assert(ENGINE->constraintConsistent());
    assert(eventToken.getObject()->baseDomain().isEmpty());

    // Insertion of a new object should reecover the situation
    ObjectId o2 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "o2"))->getId();
    assert(ENGINE->constraintConsistent());
    assert(eventToken.getObject()->baseDomain().isSingleton());

    // Now specify it
    eventToken.getObject()->specify(o2);

    // Addition of a new object will update the base domain, but not the spec or derived.
    // Consequently, no further propagation is required
    ObjectId o3 = (new Object(db.getId(), DEFAULT_OBJECT_TYPE(), "o3"))->getId();
    assert(ENGINE->constraintConsistent());
    assert(!eventToken.getObject()->baseDomain().isSingleton());
    assert(eventToken.getObject()->lastDomain().isSingleton());

    // Now resetting the specified domain will revert the derived domain back completely
    eventToken.getObject()->reset();
    assert(ENGINE->constraintConsistent());
    assert(eventToken.getObject()->lastDomain().isMember(o2));
    assert(eventToken.getObject()->lastDomain().isMember(o3));

    // Finally, close the database for this type, and ensure propagation is triggered, and results in consistency
    db.close(DEFAULT_OBJECT_TYPE().c_str());
    assert(ENGINE->pending());
    assert(ENGINE->propagate());

    // Confirm the object-token relation has propagated
    return true;

  }

  bool testTokenWithNoObjectOnCreationImpl(){
    initDbTestSchema(SCHEMA);
    PlanDatabase db(ENGINE, SCHEMA);
    {
      // Leave this class of objects open. So we should be able to create a token and have things consistent
      EventToken eventToken(db.getId(), DEFAULT_PREDICATE(), false, IntervalIntDomain(0, 10));
      assert(ENGINE->propagate());

    // Now close the datbase for this class of objects, and ensure we are inconsistent
      db.close(DEFAULT_OBJECT_TYPE().c_str());
      assert(!ENGINE->propagate());
    }

    // Now the token has gone out of scope so we expect the system to be consistent again
    assert(ENGINE->propagate());
    return true;
  }

  bool testBasicTokenAllocationImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
    // Event Token
    EventToken eventToken(db, DEFAULT_PREDICATE(), true, IntervalIntDomain(0, 1000), Token::noObject(), false);
    assert(eventToken.getStart()->getDerivedDomain() == eventToken.getEnd()->getDerivedDomain());
    assert(eventToken.getDuration()->getDerivedDomain() == IntervalIntDomain(0, 0));
    eventToken.getStart()->specify(IntervalIntDomain(5, 10));
    assert(eventToken.getEnd()->getDerivedDomain() == IntervalIntDomain(5, 10));
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
    values.push_back(PLASMA::LabelStr("L1"));
    values.push_back(PLASMA::LabelStr("L4"));
    values.push_back(PLASMA::LabelStr("L2"));
    values.push_back(PLASMA::LabelStr("L5"));
    values.push_back(PLASMA::LabelStr("L3"));
    intervalToken.addParameter(LabelSet(values), "LabelSetParam");
    intervalToken.close();
    assert(intervalToken.getEnd()->getDerivedDomain().getLowerBound() == 2);
    intervalToken.getStart()->specify(IntervalIntDomain(5, 10));
    assert(intervalToken.getEnd()->getDerivedDomain() == IntervalIntDomain(7, 20));
    intervalToken.getEnd()->specify(IntervalIntDomain(9, 10));
    assert(intervalToken.getStart()->getDerivedDomain() == IntervalIntDomain(5, 8));
    assert(intervalToken.getDuration()->getDerivedDomain() == IntervalIntDomain(2, 5));

    // Create and delete a Token
    TokenId token = (new IntervalToken(db, 
                                       DEFAULT_PREDICATE(), 
                                       true, 
                                       IntervalIntDomain(0, 1000),
                                       IntervalIntDomain(0, 1000),
                                       IntervalIntDomain(2, 10),
                                       Token::noObject(), true))->getId();

    delete (Token*) token; // It is inComplete
    return true;
  }

  bool testBasicTokenCreationImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
    ObjectId timeline = (new Timeline(db, DEFAULT_OBJECT_TYPE(), "o2"))->getId();
    assert(!timeline.isNoId());
    db->close();                                                                          
  
    IntervalToken t1(db,                                                         
                     DEFAULT_PREDICATE(),                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));                                        
    return true;
  }

  bool testStateModelImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
    IntervalToken t0(db, 
                     DEFAULT_PREDICATE(), 
                     true, 
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(2, 10),
                     Token::noObject(), false);
  
    assert(t0.isIncomplete());
    t0.close();
    assert(t0.isInactive());
    t0.reject();
    assert(t0.isRejected());
    t0.cancel();
    assert(t0.isInactive());
    t0.activate();
    assert(t0.isActive());
    t0.cancel();
    assert(t0.isInactive());
  
    IntervalToken t1(db, 
                     DEFAULT_PREDICATE(), 
                     true, 
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(2, 10),
                     Token::noObject(), true);
  
    // Constraint the start variable of both tokens
    EqualConstraint c0("eq", "Default", ENGINE, makeScope(t0.getStart(), t1.getStart()));
  
    assert(t1.isInactive());
    t0.activate();
    t1.merge(t0.getId());
    assert(t1.isMerged());
    t1.cancel();
    assert(t1.isInactive());
    t1.merge(t0.getId());

    // Test that we can allocate a token, but if we constrain it with any external entity, then the state variable will be restricted
    // to exclude the possibility of rejecting the token.
    {


    }
    return true;
  }

  bool testMasterSlaveRelationshipImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
  
    TokenId t2 = (new IntervalToken(t0.getId(), 
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t3 = (new IntervalToken(t0.getId(), 
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t4 = (new IntervalToken(t0.getId(), 
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t5 = (new IntervalToken(t1, 
                                    DEFAULT_PREDICATE(), 
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
  
    TokenId t6 = (new EventToken(t0.getId(), 
                                 DEFAULT_PREDICATE(), 
                                 IntervalIntDomain(0, 1)))->getId();
  
    // These are mostly to avoid compiler warnings about unused variables.
    assert(t3 != t4);
    assert(t5 != t6);
  
    // Delete slave only
    delete (Token*) t2;
    // Should verify correct count of tokens remain. --wedgingt 2004 Feb 27
  
    // Delete master & slaves
    delete (Token*) t1;
    // Should verify correct count of tokens remain. --wedgingt 2004 Feb 27
  
    return true;
  }

  bool testBasicMergingImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
    // Create 2 mergeable tokens - predicates, types and base domaiuns match
    IntervalToken t0(db, 
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));
  
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
  
    IntervalToken t1(db,
                     DEFAULT_PREDICATE(), 
                     true,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));
  
    t1.getDuration()->specify(IntervalIntDomain(5, 7));
  
    // Activate & deactivate - ensure proper handling of rejectability variable
    assert(!t0.getState()->getDerivedDomain().isSingleton());
    t0.activate();
    assert(t0.getState()->getDerivedDomain().isSingleton());
    assert(t0.getState()->getDerivedDomain().getSingletonValue() == Token::ACTIVE);
    t0.cancel();
    assert(!t0.getState()->getDerivedDomain().isSingleton());
  
    // Now activate and merge
    t0.activate();
    t1.merge(t0.getId());
  
    // Make sure the necessary restrictions have been imposed due to merging i.e. restruction due to specified domain
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    assert(t1.isMerged());
  
    // Do a split and make sure the old values are reinstated.
    t1.cancel();
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
    assert(t1.isInactive());
  
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
  
    assert(!t0.getMergedTokens().empty());
  
    // Verify that the equality constraint has migrated and original has been deactivated.
    //TBW: when stacking instead of merging tokens, the next check is not true
    // assert(!equalityConstraint->isActive());
    assert(t0.getEnd()->getDerivedDomain().getLowerBound() == 8);
    assert(t0.getEnd()->getDerivedDomain() == t2.getEnd()->getDerivedDomain());
  
    // Undo the merge and check for initial conditions being established
    t1.cancel();
    assert(equalityConstraint->isActive());
  
    // Redo the merge
    t1.merge(t0.getId());
  
    // Confirm deletion of the constraint is handled correctly
    delete (Constraint*) equalityConstraint;
    assert(t0.getEnd()->getDerivedDomain() != t2.getEnd()->getDerivedDomain());
  
    // Confirm previous restriction due to specified domain, then reset and note the change
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    t1.getDuration()->reset();
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
  
  
    // Test subset path
    t1.cancel();
    Variable<IntervalIntDomain> superset(db->getConstraintEngine(), IntervalIntDomain(5, 6));

    ConstraintId subsetOfConstraint = ConstraintLibrary::createConstraint("SubsetOf",
                                                                          db->getConstraintEngine(),
                                                                          makeScope(t1.getDuration(), superset.getId()));
    t1.merge(t0.getId());
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 6);
    delete (Constraint*) subsetOfConstraint;
  
    return true;
  }

  bool testConstraintMigrationDuringMergeImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
    timeline1->constrain(t0.getId());
    timeline2->constrain(t2.getId());

    db->getConstraintEngine()->propagate();

    t1.merge(t0.getId());
    t3.merge(t2.getId());

    t3.cancel();
    t1.cancel();

    return true;
  }

  bool testNonChronGNATS2439Impl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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

    assert(ce->propagate());

    // after constraining t2 to come before t3, only t2 and t3 start and
    // end domains should've changed.

    assert(token0.getStart()->lastDomain().getLowerBound() == 0);
    assert(token0.getStart()->lastDomain().getUpperBound() == 10);
    assert(token0.getEnd()->lastDomain().getLowerBound() == 1);
    assert(token0.getEnd()->lastDomain().getUpperBound() == 200);

    assert(token1.getStart()->lastDomain().getLowerBound() == 0);
    assert(token1.getStart()->lastDomain().getUpperBound() == 10);
    assert(token1.getEnd()->lastDomain().getLowerBound() == 1);
    assert(token1.getEnd()->lastDomain().getUpperBound() == 200);

    assert(token2.getStart()->lastDomain().getLowerBound() == 0);
    assert(token2.getStart()->lastDomain().getUpperBound() == 9);
    assert(token2.getEnd()->lastDomain().getLowerBound() == 1);
    assert(token2.getEnd()->lastDomain().getUpperBound() == 10);

    assert(token3.getStart()->lastDomain().getLowerBound() == 1);
    assert(token3.getStart()->lastDomain().getUpperBound() == 10);
    assert(token3.getEnd()->lastDomain().getLowerBound() == 2);
    assert(token3.getEnd()->lastDomain().getUpperBound() == 200);

    token0.activate();
    token2.merge(token0.getId());
    assert(ce->propagate());
    token1.activate();
    token3.merge(token1.getId());
    assert(ce->propagate());

    // after merging t2->t0 and t3->t1, all parameters should be
    // singletons. Also, t0 should now be before t1 (inheriting the
    // relation between t2 and t3).


    assert(token0.getParameters()[0]->lastDomain().isSingleton());
    assert(token1.getParameters()[0]->lastDomain().isSingleton());
    assert(token2.getParameters()[0]->lastDomain().isSingleton());
    assert(token3.getParameters()[0]->lastDomain().isSingleton());

    assert(token0.getStart()->lastDomain().getLowerBound() == 0);
    assert(token0.getStart()->lastDomain().getUpperBound() == 9);
    assert(token0.getEnd()->lastDomain().getLowerBound() == 1);
    assert(token0.getEnd()->lastDomain().getUpperBound() == 10);

    assert(token1.getStart()->lastDomain().getLowerBound() == 1);
    assert(token1.getStart()->lastDomain().getUpperBound() == 10);
    assert(token1.getEnd()->lastDomain().getLowerBound() == 2);
    assert(token1.getEnd()->lastDomain().getUpperBound() == 200);

    token2.cancel();
    assert(ce->propagate());

    // after cancelling t2->t0, all parameters remain singleton except for
    // t0's since it no longer inherits the singleton domain from t2.
    // Furthermore, t0 should no longer be constrained to be before t1.
    // However, t1 should remain constrained to be before t2 since it still
    // inherits the before constraint between t2 and t3.

    assert(!token0.getParameters()[0]->lastDomain().isSingleton());
    assert(!token1.getParameters()[0]->lastDomain().isSingleton());
    assert(token2.getParameters()[0]->lastDomain().isSingleton());
    assert(token3.getParameters()[0]->lastDomain().isSingleton());

    assert(token0.getStart()->lastDomain().getLowerBound() == 0);
    assert(token0.getStart()->lastDomain().getUpperBound() == 10);
    assert(token0.getEnd()->lastDomain().getLowerBound() == 1);
    assert(token0.getEnd()->lastDomain().getUpperBound() == 200);

    assert(!token3.isMerged());

    return true;
  }

  bool testMergingPerformanceImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
    assert(sdom1.getLowerBound() == 0);
    assert(sdom1.getUpperBound() == 210);

    IntervalIntDomain edom1(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom1.getLowerBound() == 1);
    assert(edom1.getUpperBound() == 220);

    Id<TokenVariable<IntervalIntDomain> > pvar1(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom1(pvar1->getDerivedDomain());
    assert(pdom1.getLowerBound() == 500);
    assert(pdom1.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++) {
      tokens[i][0]->activate();
      timeline->constrain(tokens[i][0]);
    }

    IntervalIntDomain sdom2(tokens[0][0]->getStart()->getDerivedDomain());
    assert(sdom2.getLowerBound() == 0);
    assert(sdom2.getUpperBound() == 208);

    IntervalIntDomain edom2(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom2.getLowerBound() == 1);
    assert(edom2.getUpperBound() == 209);

    Id<TokenVariable<IntervalIntDomain> > pvar2(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom2(pvar2->getDerivedDomain());
    assert(pdom2.getLowerBound() == 500);
    assert(pdom2.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) { 
        tokens[i][j]->merge(tokens[i][0]);
        ce->propagate();
      }

    IntervalIntDomain sdom3(tokens[0][0]->getStart()->getDerivedDomain());
    assert(sdom3.getLowerBound() == 0);
    assert(sdom3.getUpperBound() == 208);

    IntervalIntDomain edom3(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom3.getLowerBound() == 1);
    assert(edom3.getUpperBound() == 209);

    Id<TokenVariable<IntervalIntDomain> > pvar3(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom3(pvar3->getDerivedDomain());
    assert(pdom3.getLowerBound() == 500+UNIFIED-1);
    assert(pdom3.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) {
        tokens[i][j]->cancel();
        ce->propagate();
      }

    IntervalIntDomain sdom4(tokens[0][0]->getStart()->getDerivedDomain());
    assert(sdom4.getLowerBound() == sdom2.getLowerBound());
    assert(sdom4.getUpperBound() == sdom2.getUpperBound());

    IntervalIntDomain edom4(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom4.getLowerBound() == edom2.getLowerBound());
    assert(edom4.getUpperBound() == edom2.getUpperBound());

    Id<TokenVariable<IntervalIntDomain> > pvar4(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom4(pvar4->getDerivedDomain());
    assert(pdom4.getLowerBound() == pdom2.getLowerBound());
    assert(pdom4.getUpperBound() == pdom2.getUpperBound());
    return true;
  }

  bool testTokenCompatibilityImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
    assert(res);
    db->getCompatibleTokens(t1.getId(), compatibleTokens);
    assert(compatibleTokens.size() == 1);
    assert(compatibleTokens[0] == t0.getId());

    compatibleTokens.clear();
    t0.cancel();
    res = ce->propagate();
    assert(res);
    db->getCompatibleTokens(t1.getId(), compatibleTokens);
    assert(compatibleTokens.empty()); // No match since no tokens are active

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
    assert(res);
    compatibleTokens.clear();
    db->getCompatibleTokens(t2.getId(), compatibleTokens);
    assert(compatibleTokens.empty()); // No match since parameter variable has no intersection


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
    assert(compatibleTokens.size() == 1); // Expect a single match
    return true;
  }

  bool testTokenFactoryImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
    TokenId master = TokenFactory::createInstance(db, DEFAULT_PREDICATE());
    master->activate();
    TokenId slave = TokenFactory::createInstance(master, DEFAULT_PREDICATE());
    assert(slave->getMaster() == master); 
    return true;
  }

  bool testCorrectSplit_Gnats2450impl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
    assert(ce->propagate());

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
    assert(ce->propagate());
    assert(tokenA.getStart()->lastDomain().getSingletonValue() == 5);

    // Now do the merges and test
    tokenB.merge(tokenA.getId());
    assert(ce->propagate());
    assert(tokenA.getStart()->lastDomain().getSingletonValue() == 5);

    tokenC.merge(tokenA.getId());
    assert(!ce->propagate()); // Should always fail

    // Now split it and test that the specified domain is unchanged
    tokenC.cancel();
    assert(ce->propagate()); // Should be OK now
    assert(tokenA.getStart()->lastDomain().getSingletonValue() == 5);

    return true;
  }

  bool testBasicInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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

    assert(!timeline.hasTokensToOrder());
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    std::vector<TokenId> tokens;
    timeline.getTokensToOrder(tokens);
    assert(tokens.size() == 3);
    assert(timeline.getTokenSequence().size() == 0);
    assert(timeline.hasTokensToOrder());

    unsigned int num_constraints = ce->getConstraints().size();

    timeline.constrain(tokenA.getId());
    num_constraints += 1; // Only object is constrained since sequence should be empty
    assert(ce->getConstraints().size() == num_constraints);

    timeline.constrain(tokenB.getId());
    num_constraints += 2; // Object variable and a single temporal constraint since placing at the end
    assert(ce->getConstraints().size() == num_constraints);

    timeline.constrain(tokenC.getId(), tokenA.getId());
    num_constraints += 2; // Object variable and a single temporal constraint since placing at the beginning
    assert(ce->getConstraints().size() == num_constraints);

    assert(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());
    assert(timeline.getTokenSequence().size() == 3);
    assert(!timeline.hasTokensToOrder());

    timeline.free(tokenA.getId());
    num_constraints -= 3; // Object variable and temporal constraints for placement w.r.t B and C.
    num_constraints += 1; // Should have added a new constraint to preserve temporal relationship between B and C which had been indirect
    assert(ce->getConstraints().size() == num_constraints);

    assert(timeline.getTokenSequence().size() == 2);
    tokens.clear();
    timeline.getTokensToOrder(tokens);
    assert(tokens.size() == 1);
    assert(timeline.hasTokensToOrder());

    // Now force it to be part of this timeline, even though it is not otherwise constrained
    tokenA.getObject()->specify(timeline.getId());
    tokens.clear();
    timeline.getTokensToOrder(tokens);
    assert(tokens.size() == 1); // Won't affect this quantity
    assert(tokens.front() == tokenA.getId());

    timeline.constrain(tokenA.getId());
    assert(!timeline.hasTokensToOrder());
    assert(timeline.getTokenSequence().size() == 3);
    timeline.free(tokenC.getId());

    assert(timeline.getTokenSequence().size() == 2);
    assert(timeline.hasTokensToOrder());


    return true;
  }

  bool testObjectTokenRelationImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
    assert(tokensToOrder.empty());

    // Specify the object variable of one - but still should return no tokens since they are all inactive
    tokenA.getObject()->specify(timeline.getId());
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.empty());

    // Now activate all of them
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 3);

    // Set remainders so they are singeltons and get all back
    tokenB.getObject()->specify(timeline.getId());
    tokenC.getObject()->specify(timeline.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 3);

    // Now incrementally constrain and show reduction in tokens to order
    timeline.constrain(tokenA.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 2);

    timeline.constrain(tokenB.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 1);

    timeline.constrain(tokenC.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.empty());


    // Test destruction call path
    Token* tokenD = new IntervalToken(db, 
                                      LabelStr(DEFAULT_PREDICATE()), 
                                      true,
                                      IntervalIntDomain(0, 10),
                                      IntervalIntDomain(0, 20),
                                      IntervalIntDomain(1, 1000));
    tokenD->activate();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 1);
    delete tokenD;
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.empty());
    return true;
  }

  bool testTokenOrderQueryImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
      assert(!token->getObject()->getBaseDomain().isSingleton());
      token->getObject()->specify(timeline->getId());
      token->activate();
    }

    assert(timeline->getTokens().size() == 0);
    ce->propagate();
    assert(timeline->getTokens().size() == (unsigned int) COUNT);

    int i = 0;
    std::vector<TokenId> tokensToOrder;
    timeline->getTokensToOrder(tokensToOrder);

    while(!tokensToOrder.empty()){
      assert(timeline->getTokenSequence().size() == (unsigned int) i);
      assert(tokensToOrder.size() == (unsigned int) (COUNT - i));
      std::vector<TokenId> choices;
      TokenId toConstrain = tokensToOrder.front();
      timeline->getOrderingChoices(toConstrain, choices);
      assert(!choices.empty());
      TokenId successor = choices.front();
      timeline->constrain(toConstrain, successor);
      bool res = ce->propagate();
      assert(res);
      tokensToOrder.clear();
      timeline->getTokensToOrder(tokensToOrder);
      i++;
      res = ce->propagate();
      assert(res);
    }

    const std::list<TokenId>& tokenSequence = timeline->getTokenSequence();
    assert(tokenSequence.front()->getStart()->getDerivedDomain().getSingletonValue() == 0);
    assert(tokenSequence.back()->getEnd()->getDerivedDomain().getSingletonValue() == COUNT*DURATION);

    // Now ensure the query can correctly indicate no options available
    TokenId token = (new IntervalToken(db, 
                                       LabelStr(DEFAULT_PREDICATE()),
                                       true,
                                       IntervalIntDomain(0, 0),
                                       IntervalIntDomain(),
                                       IntervalIntDomain(DURATION, DURATION)))->getId();
    token->getObject()->specify(timeline->getId());
    token->activate();
    std::vector<TokenId> choices;
    timeline->getOrderingChoices(token, choices);
    assert(choices.empty());

    return true;
  }

  bool testEventTokenInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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
    timeline.constrain(it1.getId(), TokenId::noId());

    // Insert at the end after a token
    EventToken et1(db, 
                   DEFAULT_PREDICATE(), 
                   true, 
                   IntervalIntDomain(0, 100), 
                   Token::noObject());

    et1.getObject()->specify(timeline.getId());
    et1.activate();
    timeline.constrain(et1.getId(), TokenId::noId());
    assert(it1.getEnd()->getDerivedDomain().getUpperBound() == 100);

    // Insert between a token and an event
    EventToken et2(db, 
                   DEFAULT_PREDICATE(), 
                   true, 
                   IntervalIntDomain(0, 100), 
                   Token::noObject());

    et2.getObject()->specify(timeline.getId());
    et2.activate();
    timeline.constrain(et2.getId(), et1.getId());
    assert(it1.getEnd()->getDerivedDomain().getUpperBound() == 100);

    // Insert before a token
    EventToken et3(db, 
                   DEFAULT_PREDICATE(), 
                   true, 
                   IntervalIntDomain(10, 100), 
                   Token::noObject());

    et3.getObject()->specify(timeline.getId());
    et3.activate();
    timeline.constrain(et3.getId(), it1.getId());
    assert(it1.getStart()->getDerivedDomain().getLowerBound() == 10);

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
    assert(res);
    return true;
  }

  bool testFullInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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

    assert(!timeline.hasTokensToOrder());
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();

    timeline.constrain(tokenA.getId()); // Put A on the end.
    timeline.constrain(tokenB.getId()); // Put B on the end.
    assert(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());

    // Now insert token C in the middle.
    timeline.constrain(tokenC.getId(), tokenB.getId());
    assert(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenC.getStart()->getDerivedDomain().getUpperBound());
    assert(tokenC.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());
    return true;
  }

  bool testNoChoicesThatFitImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
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

    timeline.constrain(tokenA.getId());
    timeline.constrain(tokenB.getId());
    bool res = ce->propagate();
    assert(res);

    std::vector<TokenId> choices;
    timeline.getOrderingChoices(tokenC.getId(), choices);
    assert(choices.empty());
    timeline.constrain(tokenC.getId(), tokenB.getId());
    res = ce->propagate();
    assert(!res);

    return true;
  }

  bool testBasicAllocationImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {

    DbClientId client = db->getClient();
    DbClientTransactionLog* txLog = new DbClientTransactionLog(client);

    FooId foo1 = client->createObject(DEFAULT_OBJECT_TYPE().c_str(), "foo1");
    assert(foo1.isValid());

    std::vector<ConstructorArgument> arguments;
    IntervalIntDomain arg0(10);
    LabelSet arg1(LabelSet::getDefaultTypeName(), "Label");
    arguments.push_back(ConstructorArgument(IntervalIntDomain::getDefaultTypeName(), &arg0)); 
    arguments.push_back(ConstructorArgument(LabelSet::getDefaultTypeName(), &arg1));
    FooId foo2 = client->createObject(DEFAULT_OBJECT_TYPE().c_str(), "foo2", arguments);
    assert(foo2.isValid());

    TokenId token = client->createToken(DEFAULT_PREDICATE().c_str());
    assert(token.isValid());

    // Constrain the token duration
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(token->getStart());
    scope.push_back(token->getDuration());
    client->createConstraint("eq", scope);

    delete txLog;

    return true;
  }

  bool testPathBasedRetrievalImpl(ConstraintEngineId &ce, PlanDatabaseId &db) {
    TokenId t0 = db->getClient()->createToken(DEFAULT_PREDICATE().c_str());
    t0->activate();

    TokenId t1 = db->getClient()->createToken(DEFAULT_PREDICATE().c_str());
    t1->activate();

    TokenId t0_0 = (new IntervalToken(t0, 
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_0->activate();

    TokenId t0_1 = (new IntervalToken(t0, 
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_1->activate();

    TokenId t0_2 = (new IntervalToken(t0, 
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_2->activate();

    TokenId t1_0 = (new IntervalToken(t1, 
                                      DEFAULT_PREDICATE(), 
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t1_0->activate();

    TokenId t0_1_0 = (new EventToken(t0_1, 
                                     DEFAULT_PREDICATE(), 
                                     IntervalIntDomain(0, 1)))->getId();
    t0_1_0->activate();

    TokenId t0_1_1 = (new EventToken(t0_1, 
                                     DEFAULT_PREDICATE(), 
                                     IntervalIntDomain(0, 1)))->getId();
    t0_1_1->activate();

    // Test paths
    std::vector<int> path;
    path.push_back(0); // Start with the index of the token key in the path


    // Base case with just the root
    assert(db->getClient()->getTokenByPath(path) == t0);
    assert(db->getClient()->getPathByToken(t0).size() == 1);

    // Now test a more convoluted path
    path.push_back(1);
    path.push_back(1);
    assert(db->getClient()->getTokenByPath(path) == t0_1_1);

    path.clear();
    path = db->getClient()->getPathByToken(t0_1_1);
    assert(path.size() == 3);
    assert(path[0] == 0);
    assert(path[1] == 1);
    assert(path[2] == 1);


    // Negative tests
    path.push_back(100);
    assert(db->getClient()->getTokenByPath(path) == TokenId::noId());
    path[0] = 99999;
    assert(db->getClient()->getTokenByPath(path) == TokenId::noId());
    return true;
  }

}
