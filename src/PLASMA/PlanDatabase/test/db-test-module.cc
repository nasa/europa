#include "db-test-module.hh"

#include "Utils.hh"
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
#include "Domains.hh"

#include "Debug.hh"
#include "PlanDatabaseWriter.hh"

#include "Constraints.hh"
#include "Engine.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>

const char* DEFAULT_OBJECT_TYPE = "TestObject";
const char* DEFAULT_PREDICATE = "TestObject.DEFAULT_PREDICATE";

#define GET_DEFAULT_OBJECT_TYPE(ce) ce->getCESchema()->getDataType(DEFAULT_OBJECT_TYPE)
#define GET_DATA_TYPE(pdb,dt) pdb->getSchema()->getCESchema()->getDataType(dt)

  class DBFoo;
  typedef Id<DBFoo> DBFooId;

  class DBFoo : public Timeline {
  public:
    DBFoo(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    DBFoo(const ObjectId& parent, const LabelStr& type, const LabelStr& name);
    void handleDefaults(bool autoClose = true); // default variable initialization

    // test/simple-predicate.nddl:4 DBFoo
    void constructor();
    void constructor(int arg0, LabelStr& arg1);
    ConstrainedVariableId m_0;
    ConstrainedVariableId m_1;
  };

  DBFoo::DBFoo(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
    : Timeline(planDatabase, type, name, true) {
  }

  DBFoo::DBFoo(const ObjectId& parent, const LabelStr& type, const LabelStr& name)
    : Timeline(parent, type, name, true) {}

  // default initialization of member variables
  void DBFoo::handleDefaults(bool autoClose) {
    if(m_0.isNoId()){
      check_error(!ObjectId::convertable(m_0)); // Object Variables must be explicitly initialized to a singleton
      m_0 = addVariable(IntervalIntDomain(), "IntervalIntVar");
    }
    check_error(!m_1.isNoId()); // string variables must be initialized explicitly
    if(autoClose) close();
  }

  void DBFoo::constructor() {
    m_1 = addVariable(LabelSet(LabelStr("Hello World")), "LabelSetVar");
  }

  void DBFoo::constructor(int arg0, LabelStr& arg1) {
    m_0 = addVariable(IntervalIntDomain(arg0), "IntervalIntVar");
    m_1 = addVariable(LabelSet(LabelStr("Hello World")), "LabelSetVar");
  }

  class StandardDBFooFactory: public ObjectFactory {
  public:
    StandardDBFooFactory(): ObjectFactory(LabelStr(DEFAULT_OBJECT_TYPE)){}

  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType,
                            const LabelStr& objectName,
                            const std::vector<const AbstractDomain*>& arguments) const {
      CPPUNIT_ASSERT(arguments.empty());
      DBFooId foo = (new DBFoo(planDb, objectType, objectName))->getId();
      foo->constructor();
      foo->handleDefaults();
      return foo;
    }
  };

  class SpecialDBFooFactory: public ObjectFactory{
  public:
    SpecialDBFooFactory(): ObjectFactory(LabelStr(DEFAULT_OBJECT_TYPE).toString() +
                           ":" + IntDT::NAME() +
                           ":" + StringDT::NAME())
    {}

  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType,
                            const LabelStr& objectName,
                            const std::vector<const AbstractDomain*>& arguments) const {
      DBFooId foo = (new DBFoo(planDb, objectType, objectName))->getId();
      // Type check the arguments
      CPPUNIT_ASSERT(arguments.size() == 2);
      CPPUNIT_ASSERT(arguments[0]->getTypeName().toString() == IntDT::NAME());
      CPPUNIT_ASSERT(arguments[1]->getTypeName().toString() == StringDT::NAME());

      int arg0((int) arguments[0]->getSingletonValue());
      LabelStr arg1(arguments[1]->getSingletonValue());
      foo->constructor(arg0, arg1);
      foo->handleDefaults();
      return foo;
    }
  };

  class IntervalTokenFactory: public TokenFactory {
  public:
    IntervalTokenFactory()
      : TokenFactory(LabelStr(DEFAULT_PREDICATE)) {
        addArg(FloatDT::NAME(), "IntervalParam");
        addArg(IntDT::NAME(), "IntervalIntParam");
        addArg(BoolDT::NAME(), "BoolParam");
        addArg(StringDT::NAME(), "LabelSetParam");
        addArg(FloatDT::NAME(), "EnumeratedParam");
    }
  private:
    TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false, bool isFact = false) const {
      TokenId token = (new IntervalToken(planDb, name, rejectable, isFact))->getId();
      return(token);
    }
    TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const{
      TokenId token = (new IntervalToken(master, relation, name))->getId();
      return(token);
    }
  };

void initDbTestSchema(const SchemaId& schema) {
  // Set up object types and compositions for testing - builds a recursive structure
  ObjectType* objType = new ObjectType(DEFAULT_OBJECT_TYPE,Schema::rootObject().c_str());

  objType->addMember(DEFAULT_OBJECT_TYPE, "id0");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id1");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id2");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id3");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id4");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id5");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id6");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id7");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id8");
  objType->addMember(DEFAULT_OBJECT_TYPE, "id9");

  objType->addMember(DEFAULT_OBJECT_TYPE, "o0");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o1");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o2");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o3");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o4");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o5");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o6");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o7");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o8");
  objType->addMember(DEFAULT_OBJECT_TYPE, "o9");

  // Set up primitive object type member variables for testing
  objType->addMember(FloatDT::NAME().c_str(), "IntervalVar");
  objType->addMember(IntDT::NAME().c_str(), "IntervalIntVar");
  objType->addMember(BoolDT::NAME().c_str(), "BoolVar");
  objType->addMember(StringDT::NAME().c_str(), "LabelSetVar");
  objType->addMember(FloatDT::NAME().c_str(), "EnumeratedVar");

  // Set up predicates for testing
  objType->addTokenFactory((new IntervalTokenFactory())->getId());

  // Set up constructors for testing
  objType->addObjectFactory((new StandardDBFooFactory())->getId());
  objType->addObjectFactory((new SpecialDBFooFactory())->getId());

  schema->registerObjectType(objType->getId());

  SymbolDomain locationsBaseDomain;
  locationsBaseDomain.insert(LabelStr("Hill"));
  locationsBaseDomain.insert(LabelStr("Rock"));
  locationsBaseDomain.insert(LabelStr("Lander"));
  locationsBaseDomain.close();

  schema->registerEnum("Locations",locationsBaseDomain);
}

class PDBTestEngine  : public EngineBase
{
  public:
    PDBTestEngine();
    virtual ~PDBTestEngine();

    const ConstraintEngineId& getConstraintEngine() const;
    const SchemaId& getSchema() const;
    const PlanDatabaseId& getPlanDatabase() const;

  protected:
    void createModules();
};

PDBTestEngine::PDBTestEngine()
{
    createModules();
    doStart();
    const SchemaId& schema = ((Schema*)getComponent("Schema"))->getId();
    initDbTestSchema(schema);

    // Tokens require temporal distance constraints
    CESchema* ces = (CESchema*)getComponent("CESchema");
    REGISTER_SYSTEM_CONSTRAINT(ces,EqualConstraint, "concurrent", "Default");
    REGISTER_SYSTEM_CONSTRAINT(ces,LessThanEqualConstraint, "precedes", "Default");
    REGISTER_SYSTEM_CONSTRAINT(ces,AddEqualConstraint, "temporaldistance", "Default");
    REGISTER_SYSTEM_CONSTRAINT(ces,AddEqualConstraint, "temporalDistance", "Default");
}

PDBTestEngine::~PDBTestEngine()
{
    doShutdown();
}

const ConstraintEngineId& PDBTestEngine::getConstraintEngine() const
{
    return ((ConstraintEngine*)getComponent("ConstraintEngine"))->getId();
}

const SchemaId& PDBTestEngine::getSchema() const
{
    return ((Schema*)getComponent("Schema"))->getId();
}

const PlanDatabaseId& PDBTestEngine::getPlanDatabase() const
{
    return ((PlanDatabase*)getComponent("PlanDatabase"))->getId();
}

void PDBTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
}

ConstraintEngineId ce;
SchemaId schema;
PlanDatabaseId db;

#define DEFAULT_SETUP(ce, db, autoClose) \
    PDBTestEngine testEngine; \
    ce = testEngine.getConstraintEngine(); \
    schema = testEngine.getSchema(); \
    db = testEngine.getPlanDatabase(); \
    if (autoClose) \
      db->close();\
    {

#define DEFAULT_TEARDOWN() \
    } \

#define DEFAULT_TEARDOWN_MULTI(ce, db) \
    }\

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


class SchemaTest {
public:
  static bool test() {
    EUROPA_runTest(testPrimitives);
    EUROPA_runTest(testEnumerations);
    EUROPA_runTest(testObjectTypeRelationships);
    EUROPA_runTest(testObjectPredicateRelationships);
    EUROPA_runTest(testPredicateParameterAccessors);
    return(true);
  }

private:

  static bool testPrimitives(){
      DEFAULT_SETUP(ce, db, true);

    schema->reset();
    CPPUNIT_ASSERT(schema->isPrimitive("int"));
    CPPUNIT_ASSERT(schema->isPrimitive("float"));
    CPPUNIT_ASSERT(schema->isPrimitive("bool"));
    CPPUNIT_ASSERT(schema->isPrimitive("string"));
    CPPUNIT_ASSERT(schema->isType("int"));
    CPPUNIT_ASSERT(!schema->isPrimitive("strong"));
    DEFAULT_TEARDOWN();

    return true;
  }

  static bool testEnumerations(){
      DEFAULT_SETUP(ce, db, true);
    schema->reset();
    schema->addEnum(LabelStr("FooEnum"));
    schema->addValue(LabelStr("FooEnum"), LabelStr("FOO"));
    schema->addValue(LabelStr("FooEnum"), LabelStr("BAR"));
    schema->addValue(LabelStr("FooEnum"), LabelStr("BAZ"));
    schema->addEnum(LabelStr("BarEnum"));
    schema->addValue(LabelStr("BarEnum"), 0);
    schema->addValue(LabelStr("BarEnum"), 5);
    schema->addValue(LabelStr("BarEnum"), 10);

    CPPUNIT_ASSERT(schema->isEnum(LabelStr("FooEnum")));
    CPPUNIT_ASSERT(schema->isEnum(LabelStr("BarEnum")));
    CPPUNIT_ASSERT(!schema->isEnum(LabelStr("BazEnum")));
    CPPUNIT_ASSERT(schema->isEnumValue(LabelStr("FooEnum"), LabelStr("FOO")));
    CPPUNIT_ASSERT(schema->isEnumValue(LabelStr("FooEnum"), LabelStr("BAZ")));
    CPPUNIT_ASSERT(schema->isEnumValue(LabelStr("BarEnum"), 5));
    CPPUNIT_ASSERT(!schema->isEnumValue(LabelStr("BarEnum"), 6));

    std::list<LabelStr> allenums;
    schema->getEnumerations(allenums);
    CPPUNIT_ASSERT(allenums.size() == 2);
    CPPUNIT_ASSERT(allenums.back() == LabelStr("BarEnum"));
    CPPUNIT_ASSERT(allenums.front() == LabelStr("FooEnum"));

    // test getEnumValues.
    LabelStr enumDomainName = "TestEnum";
    std::set<double> testEnumDomain;
    testEnumDomain.insert( 1 );
    testEnumDomain.insert( 2 );
    testEnumDomain.insert( 3 );

    schema->addEnum( enumDomainName );
    std::set<double>::iterator i;
    for ( i = testEnumDomain.begin(); i != testEnumDomain.end(); ++i ) {
      schema->addValue( enumDomainName, *i );
    }

    std::set<double> enumDomainReturned;
    enumDomainReturned = schema->getEnumValues( enumDomainName );
    CPPUNIT_ASSERT( enumDomainReturned == testEnumDomain );

    DEFAULT_TEARDOWN();
    return true;
  }


  static bool testObjectTypeRelationships() {
      DEFAULT_SETUP(ce, db, true);

      unsigned int initOTcnt = schema->getAllObjectTypes().size();

    schema->addObjectType(LabelStr("Foo"));
    schema->addObjectType(LabelStr("Baz"));
    schema->addPredicate("Baz.pred");

    CPPUNIT_ASSERT(schema->isObjectType(LabelStr("Foo")));
    CPPUNIT_ASSERT(schema->isA(LabelStr("Foo"), LabelStr("Foo")));
    CPPUNIT_ASSERT(!schema->isObjectType(LabelStr("Bar")));
    CPPUNIT_ASSERT(!schema->isA(LabelStr("Foo"), LabelStr("Baz")));

    // Inheritance
    schema->addObjectType(LabelStr("Bar"), LabelStr("Foo"));
    CPPUNIT_ASSERT(schema->isObjectType(LabelStr("Bar")));
    CPPUNIT_ASSERT(schema->isA(LabelStr("Bar"), LabelStr("Foo")));
    CPPUNIT_ASSERT(!schema->isA(LabelStr("Foo"), LabelStr("Bar")));
    CPPUNIT_ASSERT(schema->getAllObjectTypes(LabelStr("Bar")).size() == 3);

    // Composition
    schema->addMember(LabelStr("Foo"), LabelStr("float"), LabelStr("arg0"));
    schema->addMember(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg1"));
    schema->addMember(LabelStr("Foo"), LabelStr("Bar"), LabelStr("arg2"));

    CPPUNIT_ASSERT(schema->canContain(LabelStr("Foo"), LabelStr("float"), LabelStr("arg0")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg1")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Foo"), LabelStr("Bar"), LabelStr("arg2")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Foo"), LabelStr("Bar"), LabelStr("arg1"))); // isA(Bar,Foo)

    CPPUNIT_ASSERT(!schema->canContain(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg2")));
    CPPUNIT_ASSERT(!schema->canContain(LabelStr("Foo"), LabelStr("Foo"), LabelStr("arg3")));
    CPPUNIT_ASSERT(!schema->canContain(LabelStr("Foo"), LabelStr("float"), LabelStr("arg1")));

    CPPUNIT_ASSERT(schema->canContain(LabelStr("Bar"), LabelStr("float"), LabelStr("arg0")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Bar"), LabelStr("Foo"), LabelStr("arg1")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Bar"), LabelStr("Bar"), LabelStr("arg1")));

    CPPUNIT_ASSERT(schema->getAllObjectTypes().size() == (initOTcnt+3));

    CPPUNIT_ASSERT(!schema->hasPredicates("Foo"));
    CPPUNIT_ASSERT(!schema->hasPredicates("Foo")); // Call again for cached result
    CPPUNIT_ASSERT(schema->hasPredicates("Baz")); // Call again for cached result

    DEFAULT_TEARDOWN();

    return true;
  }

  static bool testObjectPredicateRelationships() {
      DEFAULT_SETUP(ce, db, true);

    schema->addObjectType(LabelStr("Reservoir"));
    schema->addObjectType(LabelStr("NddlReservoir"), LabelStr("Reservoir"));
    schema->addPredicate(LabelStr("Reservoir.consume"));
    schema->addPredicate(LabelStr("Reservoir.produce"));
    CPPUNIT_ASSERT(schema->isPredicate(LabelStr("Reservoir.produce")));
    CPPUNIT_ASSERT(schema->isPredicate(LabelStr("Reservoir.consume")));

    schema->addMember(LabelStr("Reservoir.produce"), LabelStr("float"), LabelStr("quantity"));
    schema->addMember(LabelStr("Reservoir.consume"), LabelStr("float"), LabelStr("quantity"));

    schema->addObjectType(LabelStr("Battery"), LabelStr("Reservoir"));
    CPPUNIT_ASSERT(schema->hasParent(LabelStr("Battery.produce")));
    CPPUNIT_ASSERT(schema->hasParent(LabelStr("Battery.consume")));
    CPPUNIT_ASSERT(schema->getParent(LabelStr("Battery.consume")) == LabelStr("Reservoir.consume"));
    CPPUNIT_ASSERT(schema->getParent(LabelStr("Battery.produce")) == LabelStr("Reservoir.produce"));

    schema->addObjectType(LabelStr("World"));
    schema->addPredicate(LabelStr("World.initialState"));
    CPPUNIT_ASSERT(schema->isPredicate(LabelStr("Battery.produce")));
    CPPUNIT_ASSERT(schema->isPredicate(LabelStr("Battery.consume")));

    CPPUNIT_ASSERT(schema->isPredicate(LabelStr("World.initialState")));
    CPPUNIT_ASSERT(!schema->isPredicate(LabelStr("World.NOPREDICATE")));
    CPPUNIT_ASSERT(schema->isObjectType(LabelStr("Reservoir")));
    CPPUNIT_ASSERT(schema->isObjectType(LabelStr("World")));
    CPPUNIT_ASSERT(schema->isObjectType(LabelStr("Battery")));
    CPPUNIT_ASSERT(!schema->isObjectType(LabelStr("NOTYPE")));

    CPPUNIT_ASSERT(schema->canContain(LabelStr("Reservoir.consume"), LabelStr("float"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Reservoir.produce"), LabelStr("float"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Battery.consume"), LabelStr("float"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("Battery.produce"), LabelStr("float"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("NddlReservoir.consume"), LabelStr("float"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->canContain(LabelStr("NddlReservoir.produce"), LabelStr("float"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->hasMember(LabelStr("Reservoir.consume"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->hasMember(LabelStr("Reservoir.produce"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->hasMember(LabelStr("NddlReservoir.consume"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->hasMember(LabelStr("NddlReservoir.produce"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->hasMember(LabelStr("Battery.consume"), LabelStr("quantity")));
    CPPUNIT_ASSERT(schema->hasMember(LabelStr("Battery.produce"), LabelStr("quantity")));

    CPPUNIT_ASSERT(schema->canBeAssigned(LabelStr("World"), LabelStr("World.initialState")));

    CPPUNIT_ASSERT(schema->canBeAssigned(LabelStr("Reservoir"), LabelStr("Reservoir.consume")));
    CPPUNIT_ASSERT(schema->canBeAssigned(LabelStr("Reservoir"), LabelStr("Reservoir.produce")));
    CPPUNIT_ASSERT(schema->canBeAssigned(LabelStr("Battery"), LabelStr("Reservoir.consume")));
    CPPUNIT_ASSERT(schema->canBeAssigned(LabelStr("Battery"), LabelStr("Reservoir.produce")));
    CPPUNIT_ASSERT(!schema->canBeAssigned(LabelStr("World"), LabelStr("Reservoir.consume")));
    CPPUNIT_ASSERT(!schema->canBeAssigned(LabelStr("World"), LabelStr("Reservoir.produce")));
    CPPUNIT_ASSERT(!schema->canBeAssigned(LabelStr("Reservoir"), LabelStr("Battery.consume")));
    CPPUNIT_ASSERT(!schema->canBeAssigned(LabelStr("Reservoir"), LabelStr("Battery.produce")));

    CPPUNIT_ASSERT(!schema->isA(LabelStr("Reservoir"), LabelStr("Battery")));
    CPPUNIT_ASSERT(schema->isA(LabelStr("Battery"), LabelStr("Reservoir")));
    CPPUNIT_ASSERT(schema->isA(LabelStr("Battery"), LabelStr("Battery")));
    CPPUNIT_ASSERT(schema->hasParent(LabelStr("Battery")));
    CPPUNIT_ASSERT(schema->getParent(LabelStr("Battery")) == LabelStr("Reservoir"));
    CPPUNIT_ASSERT(schema->getObjectTypeForPredicate(LabelStr("World.initialState")) == LabelStr("World"));
    CPPUNIT_ASSERT(schema->getObjectTypeForPredicate(LabelStr("Battery.consume")) == LabelStr("Battery"));
    CPPUNIT_ASSERT(schema->getObjectTypeForPredicate(LabelStr("Battery.produce")) == LabelStr("Battery"));
    CPPUNIT_ASSERT(schema->getObjectTypeForPredicate(LabelStr("Battery.consume")) != LabelStr("Reservoir"));
    CPPUNIT_ASSERT(schema->getObjectTypeForPredicate(LabelStr("Battery.produce")) != LabelStr("Reservoir"));

    schema->addObjectType("Base");
    schema->addObjectType("Derived");
    schema->addPredicate("Derived.Predicate");
    schema->addMember("Derived.Predicate", "Battery", "battery");


    CPPUNIT_ASSERT(schema->getParameterCount(LabelStr("Reservoir.produce")) == 1);
    CPPUNIT_ASSERT(schema->getParameterCount(LabelStr("Reservoir.consume")) == 1);
    CPPUNIT_ASSERT(schema->getParameterType(LabelStr("Reservoir.consume"), 0) == LabelStr("float"));
    CPPUNIT_ASSERT(schema->getParameterType(LabelStr("Reservoir.produce"), 0) == LabelStr("float"));

    std::set<LabelStr> predicates;
    schema->getPredicates(LabelStr("Battery"), predicates);
    CPPUNIT_ASSERT(predicates.size() == 2);
    predicates.clear();
    schema->getPredicates(LabelStr("Reservoir"), predicates);
    CPPUNIT_ASSERT(predicates.size() == 2);

    schema->addObjectType("One");
    schema->addPredicate("One.Predicate1");
    schema->addPredicate("One.Predicate2");
    schema->addPredicate("One.Predicate3");
    schema->addPredicate("One.Predicate4");

    predicates.clear();
    schema->getPredicates(LabelStr("One"), predicates);
    CPPUNIT_ASSERT(predicates.size() == 4);

    DEFAULT_TEARDOWN();

    return(true);
  }

  static bool testPredicateParameterAccessors() {
      DEFAULT_SETUP(ce, db, true);
    schema->addObjectType(LabelStr("Reservoir"));
    schema->addObjectType(LabelStr("NddlReservoir"), LabelStr("Reservoir"));
    schema->addPredicate(LabelStr("Reservoir.consume"));
    schema->addPredicate(LabelStr("Reservoir.produce"));
    schema->addObjectType(LabelStr("Battery"), LabelStr("Reservoir"));
    schema->addMember(LabelStr("Reservoir.consume"), LabelStr("float"), LabelStr("quantity"));
    schema->addMember(LabelStr("Reservoir.produce"), LabelStr("float"), LabelStr("quantity"));
    schema->addMember(LabelStr("Reservoir.consume"), LabelStr("float"), LabelStr("quality"));
    schema->addMember(LabelStr("Reservoir.produce"), LabelStr("float"), LabelStr("quality"));
    CPPUNIT_ASSERT(schema->getIndexFromName(LabelStr("Reservoir.consume"), LabelStr("quality")) == 1);
    CPPUNIT_ASSERT(schema->getIndexFromName(LabelStr("Reservoir.produce"), LabelStr("quality")) == 1);
    CPPUNIT_ASSERT(schema->getNameFromIndex(LabelStr("Reservoir.consume"), 0).getKey() == LabelStr("quantity").getKey());
    CPPUNIT_ASSERT(schema->getNameFromIndex(LabelStr("Reservoir.produce"), 0).getKey() == LabelStr("quantity").getKey());

    schema->addObjectType(LabelStr("Foo"));
    schema->addPredicate(LabelStr("Foo.Argle"));
    schema->addPrimitive("Bargle");
    schema->addMember(LabelStr("Foo.Argle"), LabelStr("Bargle"), LabelStr("bargle"));
    schema->addPrimitive("Targle");
    schema->addMember(LabelStr("Foo.Argle"), LabelStr("Targle"), LabelStr("targle"));

    CPPUNIT_ASSERT(schema->getMemberType(LabelStr("Foo.Argle"), LabelStr("bargle")) == LabelStr("Bargle"));
    CPPUNIT_ASSERT(schema->getMemberType(LabelStr("Foo.Argle"), LabelStr("targle")) == LabelStr("Targle"));

    // Extend attributes on a derived class. Must declare predicate with derived type qualifier
    schema->addObjectType(LabelStr("Bar"), LabelStr("Foo"));
    schema->addPredicate(LabelStr("Bar.Argle"));
    CPPUNIT_ASSERT(schema->hasParent(LabelStr("Bar.Argle")));
    schema->addMember(LabelStr("Bar.Argle"), LabelStr("float"), LabelStr("huey"));
    CPPUNIT_ASSERT(schema->getMemberType(LabelStr("Bar.Argle"), LabelStr("huey")) == LabelStr("float"));

    schema->addObjectType(LabelStr("Baz"), LabelStr("Bar"));
    CPPUNIT_ASSERT(schema->getMemberType(LabelStr("Baz.Argle"), LabelStr("targle")) == LabelStr("Targle"));

    CPPUNIT_ASSERT(schema->getParameterCount(LabelStr("Foo.Argle")) == 2);
    CPPUNIT_ASSERT(schema->getParameterType(LabelStr("Foo.Argle"), 0) == LabelStr("Bargle"));
    CPPUNIT_ASSERT(schema->getParameterType(LabelStr("Foo.Argle"), 1) == LabelStr("Targle"));

    DEFAULT_TEARDOWN();

    return true;
  }
};

class ObjectTest {
public:

  static bool test() {
    EUROPA_runTest(testBasicAllocation);
    EUROPA_runTest(testObjectDomain);
    EUROPA_runTest(testObjectVariables);
    EUROPA_runTest(testObjectTokenRelation);
    EUROPA_runTest(testCommonAncestorConstraint);
    EUROPA_runTest(testHasAncestorConstraint);
    EUROPA_runTest(testMakeObjectVariable);
    EUROPA_runTest(testInterleavedDynamicObjetAndVariableCreation);
    EUROPA_runTest(testTokenObjectVariable);
    EUROPA_runTest(testFreeAndConstrain);
    return(true);
  }

private:
  static bool testBasicAllocation() {
      DEFAULT_SETUP(ce, db, false);
    Object o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "o1");
    Object o2(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2");

    Object* objectPtr = &o1;
    CPPUNIT_ASSERT(objectPtr->getId() == o1.getId());

    ObjectId id0((new Object(o1.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "id0"))->getId());
    Object o3(o2.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o3");
    CPPUNIT_ASSERT(db->getObjects().size() == 4);
    CPPUNIT_ASSERT(o1.getComponents().size() == 1);
    CPPUNIT_ASSERT(o3.getParent() == o2.getId());
    delete (Object*) id0;
    CPPUNIT_ASSERT(db->getObjects().size() == 3);
    CPPUNIT_ASSERT(o1.getComponents().empty());

    ObjectId id1((new Object(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "id1"))->getId());
    new Object(id1, LabelStr(DEFAULT_OBJECT_TYPE), "id2");
    ObjectId id3((new Object(id1, LabelStr(DEFAULT_OBJECT_TYPE), "id3"))->getId());
    CPPUNIT_ASSERT(db->getObjects().size() == 6);
    CPPUNIT_ASSERT(id3->getName().toString() == "id1.id3");

    // Test ancestor call
    ObjectId id4((new Object(id3, LabelStr(DEFAULT_OBJECT_TYPE), "id4"))->getId());
    std::list<ObjectId> ancestors;
    id4->getAncestors(ancestors);
    CPPUNIT_ASSERT(ancestors.front() == id3);
    CPPUNIT_ASSERT(ancestors.back() == id1);

    // Force cascaded delete
    delete (Object*) id1;
    CPPUNIT_ASSERT(db->getObjects().size() == 3);

    // Now allocate dynamically and allow the plan database to clean it up when it deallocates
    ObjectId id5 = ((new Object(db, LabelStr(DEFAULT_OBJECT_TYPE), "id5"))->getId());
    new Object(id5, LabelStr(DEFAULT_OBJECT_TYPE), "id6");

    DEFAULT_TEARDOWN();

    return(true);
  }

  static bool testObjectDomain(){
      DEFAULT_SETUP(ce, db, false);
    std::list<ObjectId> values;
    Object o1(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o1");
    Object o2(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    CPPUNIT_ASSERT(db->getObjects().size() == 2);
    values.push_back(o1.getId());
    values.push_back(o2.getId());
    ObjectDomain os1(GET_DEFAULT_OBJECT_TYPE(ce),values);
    CPPUNIT_ASSERT(os1.isMember(o1.getId()));
    os1.remove(o1.getId());
    CPPUNIT_ASSERT(!os1.isMember(o1.getId()));
    CPPUNIT_ASSERT(os1.isSingleton());

    {
      std::stringstream str;
      str << o2.getKey();
      double value(0);
      CPPUNIT_ASSERT(os1.convertToMemberValue(str.str(), value));
      CPPUNIT_ASSERT(value == o2.getId());
    }

    {
      std::stringstream str;
      str << o1.getKey();
      double value(0);
      CPPUNIT_ASSERT(!os1.convertToMemberValue(str.str(), value));
      CPPUNIT_ASSERT(value == 0);
    }

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testObjectVariables(){
    DEFAULT_SETUP(ce, db, false);

    Object o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "o11", true);
    CPPUNIT_ASSERT(!o1.isComplete());
    o1.addVariable(IntervalIntDomain(), "IntervalIntVar");
    o1.addVariable(BoolDomain(), "BoolVar");
    o1.close();
    CPPUNIT_ASSERT(o1.isComplete());
    CPPUNIT_ASSERT(o1.getVariable("o11.BoolVar") != o1.getVariable("o1IntervalIntVar"));

    Object o2(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2", true);
    CPPUNIT_ASSERT(!o2.isComplete());
    o2.addVariable(IntervalIntDomain(15, 200), "IntervalIntVar");
    o2.close();

    // Add a unary constraint
    Variable<IntervalIntDomain> superset(db->getConstraintEngine(), IntervalIntDomain(10, 20));;

    ConstraintId subsetConstraint = db->getConstraintEngine()->createConstraint("SubsetOf",
					makeScope(o1.getVariables()[0], superset.getId()));

    // Now add a constraint equating the variables and test propagation
    std::vector<ConstrainedVariableId> constrainedVars;
    constrainedVars.push_back(o1.getVariables()[0]);
    constrainedVars.push_back(o2.getVariables()[0]);
    ConstraintId constraint = db->getConstraintEngine()->createConstraint("Equal",
								  constrainedVars);

    CPPUNIT_ASSERT(db->getConstraintEngine()->propagate());
    CPPUNIT_ASSERT(o1.getVariables()[0]->lastDomain() == o1.getVariables()[0]->lastDomain());

    // Delete one of the constraints to force automatic clean-up path and explciit clean-up
    delete (Constraint*) constraint;
    delete (Constraint*) subsetConstraint;

    DEFAULT_TEARDOWN();

    return(true);
  }


  static bool testObjectTokenRelation(){
      DEFAULT_SETUP(ce, db, false);
    // 1. Create 2 objects
    ObjectId object1 = (new Object(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "O1"))->getId();
    ObjectId object2 = (new Object(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "O2"))->getId();
    db->close();

    CPPUNIT_ASSERT(object1 != object2);
    CPPUNIT_ASSERT(db->getObjects().size() == 2);
    // 2. Create 1 token.
    EventToken eventToken(db->getId(), LabelStr(DEFAULT_PREDICATE), false, false, IntervalIntDomain(0, 10));

    // Confirm not added to the object
    CPPUNIT_ASSERT(!eventToken.getObject()->getDerivedDomain().isSingleton());

    // 3. Activate token. (NO subgoals)
    eventToken.activate();

    // Confirm not added to the object
    CPPUNIT_ASSERT(!eventToken.getObject()->getDerivedDomain().isSingleton());

    // 4. Specify tokens object variable to a ingletone

    eventToken.getObject()->specify(object1);

    // Confirm added to the object
    CPPUNIT_ASSERT(eventToken.getObject()->getDerivedDomain().isSingleton());

    // 5. propagate
    db->getConstraintEngine()->propagate();

    // 6. reset object variables domain.
    eventToken.getObject()->reset();

    // Confirm it is no longer part of the object
    // Confirm not added to the object
    CPPUNIT_ASSERT(!eventToken.getObject()->getDerivedDomain().isSingleton());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCommonAncestorConstraint(){
      DEFAULT_SETUP(ce, db, false);
    Object o1(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o1");
    Object o2(o1.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    Object o3(o1.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o3");
    Object o4(o2.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o4");
    Object o5(o2.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o5");
    Object o6(o3.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o6");
    Object o7(o3.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o7");

    ObjectDomain allObjects(GET_DEFAULT_OBJECT_TYPE(ce));
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
      Variable<ObjectDomain> first(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o4.getId()));
      Variable<ObjectDomain> second(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o7.getId()));
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o1.getId()));
      CommonAncestorConstraint constraint("commonAncestor",
					  "Default",
					  ce,
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(ce->propagate());
    }

    // Now impose a different set of restrictions which will eliminate all options
    {
      Variable<ObjectDomain> first(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o4.getId()));
      Variable<ObjectDomain> second(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o7.getId()));
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o2.getId()));
      CommonAncestorConstraint constraint("commonAncestor",
					  "Default",
					  ce,
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(!ce->propagate());
    }

    // Now try a set of restrictions, which will allow it to pass
    {
      Variable<ObjectDomain> first(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o4.getId()));
      Variable<ObjectDomain> second(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o7.getId()));
      Variable<ObjectDomain> restrictions(ce, allObjects);
      CommonAncestorConstraint constraint("commonAncestor",
					  "Default",
					  ce,
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(ce->propagate());
    }

    // Now try when no variable is a singleton, and then one becomes a singleton
    {
      Variable<ObjectDomain> first(ce, allObjects);
      Variable<ObjectDomain> second(ce, allObjects);
      Variable<ObjectDomain> restrictions(ce, allObjects);
      CommonAncestorConstraint constraint("commonAncestor",
					  "Default",
					  ce,
					  makeScope(first.getId(), second.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(ce->propagate()); // All ok so far

      restrictions.specify(o2.getId());
      CPPUNIT_ASSERT(ce->propagate()); // Nothing happens yet.

      first.specify(o6.getId()); // Now we should propagate to failure
      CPPUNIT_ASSERT(!ce->propagate());
      first.reset();

      first.specify(o4.getId());
      CPPUNIT_ASSERT(ce->propagate());
    }
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testHasAncestorConstraint(){
      DEFAULT_SETUP(ce, db, false);
    Object o1(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o1");
    Object o2(o1.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    Object o3(o1.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o3");
    Object o4(o2.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o4");
    Object o5(o2.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o5");
    Object o6(o3.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o6");
    Object o7(o3.getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o7");
    Object o8(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o8");


    // Positive test immediate ancestor
    {
      Variable<ObjectDomain> first(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o7.getId()));
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o3.getId()));
      HasAncestorConstraint constraint("hasAncestor",
                                       "Default",
                                       ce,
                                       makeScope(first.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(ce->propagate());
    }

    // negative test immediate ancestor
    {
      Variable<ObjectDomain> first(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o7.getId()));
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o2.getId()));
      HasAncestorConstraint constraint("hasAncestor",
                                       "Default",
                                       ce,
                                       makeScope(first.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(!ce->propagate());
    }
    // Positive test higher up  ancestor
    {
      Variable<ObjectDomain> first(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o7.getId()));
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o1.getId()));
      HasAncestorConstraint constraint("hasAncestor",
                                       "Default",
                                       ce,
                                       makeScope(first.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(ce->propagate());
    }
    // negative test higherup ancestor
    {
      Variable<ObjectDomain> first(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o7.getId()));
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o8.getId()));
      HasAncestorConstraint constraint("hasAncestor",
                                       "Default",
                                       ce,
                                       makeScope(first.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(!ce->propagate());
    }

    //positive restriction of the set.
    {
      ObjectDomain obs(GET_DEFAULT_OBJECT_TYPE(ce));
      obs.insert(o7.getId());
      obs.insert(o4.getId());
      obs.close();

      Variable<ObjectDomain> first(ce, obs);
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o2.getId()));
      HasAncestorConstraint constraint("hasAncestor",
                                       "Default",
                                       ce,
                                       makeScope(first.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(ce->propagate());
      CPPUNIT_ASSERT(first.getDerivedDomain().isSingleton());
    }

    //no restriction of the set.
    {
      ObjectDomain obs1(GET_DEFAULT_OBJECT_TYPE(ce));
      obs1.insert(o7.getId());
      obs1.insert(o4.getId());
      obs1.close();

      Variable<ObjectDomain> first(ce, obs1);
      Variable<ObjectDomain> restrictions(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce),o1.getId()));
      HasAncestorConstraint constraint("hasAncestor",
                                       "Default",
                                       ce,
                                       makeScope(first.getId(), restrictions.getId()));

      CPPUNIT_ASSERT(ce->propagate());
      CPPUNIT_ASSERT(first.getDerivedDomain().getSize() == 2);
    }

    DEFAULT_TEARDOWN();
    return true;
  }
  /**
   * The most basic case for dynamic objects is that we can populate the variable correctly
   * and synchronize its values.
   */
  static bool testMakeObjectVariable(){
      DEFAULT_SETUP(ce, db, false);
    ConstrainedVariableId v0 = (new Variable<ObjectDomain>(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce))))->getId();
    CPPUNIT_ASSERT(!v0->isClosed());
    db->makeObjectVariableFromType(LabelStr(DEFAULT_OBJECT_TYPE), v0);
    CPPUNIT_ASSERT(!v0->isClosed());
    CPPUNIT_ASSERT(ce->propagate());

    // Now add an object and we should expect the constraint network to be consistent
    Object o1(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o1");
    CPPUNIT_ASSERT(ce->propagate());
    CPPUNIT_ASSERT(!db->isClosed(LabelStr(DEFAULT_OBJECT_TYPE).c_str()));
    CPPUNIT_ASSERT(v0->lastDomain().isSingleton() && v0->lastDomain().getSingletonValue() == o1.getId());

    // Now delete the variable. This should remove the listener
    delete (ConstrainedVariable*) v0;

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Ensure that we can allocate variables, interleaved with Object creation,and get correct results
   */
  static bool testInterleavedDynamicObjetAndVariableCreation(){
      DEFAULT_SETUP(ce, db, false);

    // Now add an object and we should expect the constraint network to be consistent
    Object o1(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o1");
    CPPUNIT_ASSERT(ce->propagate());

    ConstrainedVariableId v0 = (new Variable<ObjectDomain>(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce))))->getId();
    CPPUNIT_ASSERT(!v0->isClosed());
    db->makeObjectVariableFromType(LabelStr(DEFAULT_OBJECT_TYPE), v0);
    CPPUNIT_ASSERT(!v0->isClosed());
    CPPUNIT_ASSERT(ce->propagate());
    CPPUNIT_ASSERT(!db->isClosed(LabelStr(DEFAULT_OBJECT_TYPE).c_str()));
    CPPUNIT_ASSERT(v0->lastDomain().isSingleton() && v0->lastDomain().getSingletonValue() == o1.getId());

    // Now create another object and verify it is part of the initial domain of the next variable
    Object o2(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    CPPUNIT_ASSERT(ce->propagate());

    // Confirm the first variable has the value
    CPPUNIT_ASSERT(!v0->lastDomain().isSingleton());

    // Allocate another variable and confirm the domains are equal
    ConstrainedVariableId v1 = (new Variable<ObjectDomain>(ce, ObjectDomain(GET_DEFAULT_OBJECT_TYPE(ce))))->getId();
    CPPUNIT_ASSERT(!v1->isClosed());
    db->makeObjectVariableFromType(LabelStr(DEFAULT_OBJECT_TYPE), v1);
    CPPUNIT_ASSERT(!v1->isClosed());
    CPPUNIT_ASSERT_MESSAGE(v1->lastDomain().toString(),
               v0->lastDomain() == v1->lastDomain() &&
	       v1->lastDomain().isMember(o1.getId())  &&
	       v1->lastDomain().isMember(o2.getId()));

    // Now delete the variables.
    delete (ConstrainedVariable*) v0;
    delete (ConstrainedVariable*) v1;

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Have at least one object in the system prior to creating a token. Show that we can successfully allocate
   * a token. Show that the object variable is closed.
   */
  static bool testTokenObjectVariable(){
      DEFAULT_SETUP(ce, db, false);

    CPPUNIT_ASSERT(ce->propagate());
    // Now add an object and we should expect the constraint network to be consistent next time we add the token.
    ObjectId o1 = (new Object(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o1"))->getId();
    EventToken eventToken(db->getId(), LabelStr(DEFAULT_PREDICATE), false, false, IntervalIntDomain(0, 10));

    eventToken.activate(); // Must be activate to eventually propagate the objectTokenRelation
    CPPUNIT_ASSERT(ce->propagate());

    // Make sure the object var of the token contains o1.
    CPPUNIT_ASSERT(eventToken.getObject()->lastDomain().isMember(o1));

    // Since the object type should be closed automatically, the object variable will propagate changes,
    // so the object token relation will link up the Token and the object.
    CPPUNIT_ASSERT(!o1->tokens().empty());

    // Insertion of a new object should not affect the given event token
    ObjectId o2 = (new Object(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
    CPPUNIT_ASSERT(ce->constraintConsistent());
    CPPUNIT_ASSERT(!eventToken.getObject()->baseDomain().isMember(o2));

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testFreeAndConstrain(){
      DEFAULT_SETUP(ce,db,false);
      Object o1(db->getId(), LabelStr(DEFAULT_OBJECT_TYPE), "o1");

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t3(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
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
      IntervalToken t4(db,
		       LabelStr(DEFAULT_PREDICATE),
		       true,
		       false,
		       IntervalIntDomain(0, 10),
		       IntervalIntDomain(0, 20),
		       IntervalIntDomain(1, 1000));
      t4.activate();
      o1.constrain(t3.getId(), t4.getId());
    }

    CPPUNIT_ASSERT(ce->propagate());

    DEFAULT_TEARDOWN();

    return true;
  }
};

class TokenTest {
public:

  static bool test() {
    EUROPA_runTest(testKeepingCommittedTokensInActiveSet);
    EUROPA_runTest(testBasicTokenAllocation);
    EUROPA_runTest(testBasicTokenCreation);
    EUROPA_runTest(testStateModel);
    EUROPA_runTest(testMasterSlaveRelationship);
    EUROPA_runTest(testTermination);
    EUROPA_runTest(testBasicMerging);
    EUROPA_runTest(testMergingWithEmptyDomains);
    EUROPA_runTest(testConstraintMigrationDuringMerge);
    EUROPA_runTest(testConstraintAdditionAfterMerging);
    EUROPA_runTest(testNonChronGNATS2439);
    EUROPA_runTest(testMergingPerformance);
    EUROPA_runTest(testTokenCompatibility);
    EUROPA_runTest(testPredicateInheritance);
    EUROPA_runTest(testTokenFactory);
    EUROPA_runTest(testCorrectSplit_Gnats2450);
    EUROPA_runTest(testOpenMerge);
    EUROPA_runTest(testGNATS_3086);
    EUROPA_runTest(testCompatCacheReset);
    EUROPA_runTest(testAssignemnt);
    EUROPA_runTest(testDeleteMasterAndPreserveSlave);
    EUROPA_runTest(testPreserveMergeWithNonChronSplit);
    EUROPA_runTest(testGNATS_3163);
    EUROPA_runTest(testGNATS_3193);
    return(true);
  }

private:

  static bool testBasicTokenAllocation() {
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();
    // Event Token
    EventToken eventToken(db, LabelStr(DEFAULT_PREDICATE), true, false, IntervalIntDomain(0, 1000), Token::noObject(), false);
    CPPUNIT_ASSERT(eventToken.start()->getDerivedDomain() == eventToken.end()->getDerivedDomain());
    CPPUNIT_ASSERT(eventToken.duration()->getDerivedDomain() == IntervalIntDomain(0, 0));
    eventToken.start()->restrictBaseDomain(IntervalIntDomain(5, 10));
    CPPUNIT_ASSERT(eventToken.end()->getDerivedDomain() == IntervalIntDomain(5, 10));
    eventToken.addParameter(IntervalDomain(-1.08, 20.18), "IntervalParam");
    eventToken.close();

    // IntervalToken
    IntervalToken intervalToken(db,
                                LabelStr(DEFAULT_PREDICATE),
                                true,
                                false,
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
    CPPUNIT_ASSERT(intervalToken.end()->getDerivedDomain().getLowerBound() == 2);
    intervalToken.start()->restrictBaseDomain(IntervalIntDomain(5, 10));
    CPPUNIT_ASSERT(intervalToken.end()->getDerivedDomain() == IntervalIntDomain(7, 20));
    intervalToken.end()->restrictBaseDomain(IntervalIntDomain(9, 10));
    CPPUNIT_ASSERT(intervalToken.start()->getDerivedDomain() == IntervalIntDomain(5, 8));
    CPPUNIT_ASSERT(intervalToken.duration()->getDerivedDomain() == IntervalIntDomain(2, 5));

    // Create and delete a Token
    TokenId token = (new IntervalToken(db,
                                       LabelStr(DEFAULT_PREDICATE),
                                       true,
                                       false,
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
    ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
    CPPUNIT_ASSERT(!timeline.isNoId());
    db->close();

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testStateModel(){
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(2, 10),
                     Token::noObject(), false);

    CPPUNIT_ASSERT(t0.isIncomplete());
    t0.close();
    CPPUNIT_ASSERT(t0.isInactive());
    t0.reject();
    CPPUNIT_ASSERT(t0.isRejected());
    t0.cancel();
    CPPUNIT_ASSERT(t0.isInactive());
    t0.activate();
    CPPUNIT_ASSERT(t0.isActive());
    t0.cancel();
    CPPUNIT_ASSERT(t0.isInactive());

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(0, 1000),
                     IntervalIntDomain(2, 10),
                     Token::noObject(), true);

    // Constraint the start variable of both tokens
    EqualConstraint c0("eq", "Default", ce, makeScope(t0.start(), t1.start()));

    CPPUNIT_ASSERT(t1.isInactive());
    t0.activate();
    t1.doMerge(t0.getId());
    CPPUNIT_ASSERT(t1.isMerged());
    t1.cancel();
    CPPUNIT_ASSERT(t1.isInactive());
    t1.doMerge(t0.getId());

    // Test that we can allocate a token, but if we constrain it with any external entity,
    // then the state variable will be restricted
    // to exclude the possibility of rejecting the token.
    {


    }
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testMasterSlaveRelationship(){
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     false,
                     false,
                     IntervalIntDomain(0, 1),
                     IntervalIntDomain(0, 1),
                     IntervalIntDomain(1, 1));
    t0.activate();

    TokenId t1 = (new IntervalToken(db,
                                    LabelStr(DEFAULT_PREDICATE),
                                    false,
                                    false,
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();
    t1->activate();

    TokenId t2 = (new IntervalToken(t0.getId(), "any",
                                    LabelStr(DEFAULT_PREDICATE),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();

    TokenId t3 = (new IntervalToken(t0.getId(), "any",
                                    LabelStr(DEFAULT_PREDICATE),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();

    TokenId t4 = (new IntervalToken(t0.getId(), "any",
                                    LabelStr(DEFAULT_PREDICATE),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();

    TokenId t5 = (new IntervalToken(t1, "any",
                                    LabelStr(DEFAULT_PREDICATE),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(0, 1),
                                    IntervalIntDomain(1, 1)))->getId();

    TokenId t6 = (new EventToken(t0.getId(), "any",
                                 LabelStr(DEFAULT_PREDICATE),
                                 IntervalIntDomain(0, 1)))->getId();

    // These are mostly to avoid compiler warnings about unused variables.
    CPPUNIT_ASSERT(t3 != t4);
    CPPUNIT_ASSERT(t5 != t6);

    // Delete slave only - master must be committed to allow this
    t0.commit();
    delete (Token*) t2;
    CPPUNIT_ASSERT(t0.slaves().size() == 3);

    // Should verify correct count of tokens remain. --wedgingt 2004 Feb 27

    // Delete master & slaves
    delete (Token*) t1;
    // Should verify correct count of tokens remain. --wedgingt 2004 Feb 27
    DEFAULT_TEARDOWN();
    // Remainder should be cleaned up automatically.
    return true;
  }

  /**
   * Test that the terminatin behavior correctly permits deallocation of tokens
   * without causing prpagation.
   */
  static bool testTermination(){
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();

    {
      IntervalToken t0(db,
		       LabelStr(DEFAULT_PREDICATE),
		       true,
		       false,
		       IntervalIntDomain(0, 10),
		       IntervalIntDomain(0, 20),
		       IntervalIntDomain(1, 1000));

      IntervalToken t1(db,
		       LabelStr(DEFAULT_PREDICATE),
		       true,
		       false,
		       IntervalIntDomain(0, 10),
		       IntervalIntDomain(0, 20),
		       IntervalIntDomain(1, 1000));

      CPPUNIT_ASSERT(ce->propagate());
    }
    // The delation of the above tokens should imply we have something to propagate
    CPPUNIT_ASSERT(ce->pending());

    // Now try again, but make sure that if we terminate them, the deletion causes no problems
    {
      IntervalToken t0(db,
		       LabelStr(DEFAULT_PREDICATE),
		       true,
		       false,
		       IntervalIntDomain(0, 0),
		       IntervalIntDomain(),
		       IntervalIntDomain(1, 1));

      IntervalToken t1(db,
		       LabelStr(DEFAULT_PREDICATE),
		       true,
		       false,
		       IntervalIntDomain(0, 0),
		       IntervalIntDomain(),
		       IntervalIntDomain(1, 1));
      CPPUNIT_ASSERT(ce->propagate());
      t0.terminate();
      t1.terminate();
    }

    CPPUNIT_ASSERT(ce->constraintConsistent());

    // Now make sure that we can correctly delete a slave that has been terminated
    TokenId t1;
    {
      IntervalToken t0(db,
		       LabelStr(DEFAULT_PREDICATE),
		       true,
		       false,
		       IntervalIntDomain(0, 0),
		       IntervalIntDomain(1, 1),
		       IntervalIntDomain(1, 1));

      t0.activate();

      t1 = (new IntervalToken(t0.getId(), "any",
				      LabelStr(DEFAULT_PREDICATE),
				      IntervalIntDomain(0, 0),
				      IntervalIntDomain(1, 1),
				      IntervalIntDomain(1, 1)))->getId();

      t1->activate();
      CPPUNIT_ASSERT(ce->propagate());
      t1->commit();
      t0.restrictBaseDomains();
      t1->restrictBaseDomains();
      CPPUNIT_ASSERT(ce->propagate());
      t0.terminate();
    }

    // Make sure the slave remains, since it was committed explicitly
    CPPUNIT_ASSERT(t1.isValid());
    CPPUNIT_ASSERT(ce->constraintConsistent());
    CPPUNIT_ASSERT(t1->master().isNoId());
    t1->terminate();
    delete (Token*) t1;
    CPPUNIT_ASSERT(ce->constraintConsistent());

    DEFAULT_TEARDOWN();
    return true;
  }

  // Added for GNATS 3077
  static bool testMergingWithEmptyDomains() {
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();
    // Create 2 mergeable tokens.

    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000),
		     Token::noObject(), false);

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);

    // add parameters and merge
    t0.addParameter(LabelSet(), "LabelSetParam");
    t1.addParameter(LabelSet(), "LabelSetParam");

    t0.close();
    t1.close();

    // activate t0
    t0.activate();

    // propagate
    bool res = ce->propagate();
    CPPUNIT_ASSERT(res);

    // look for compatible tokens for t1. Should find one - t0
    std::vector<TokenId> compatibleTokens;
    db->getCompatibleTokens(t1.getId(), compatibleTokens);

    CPPUNIT_ASSERT(compatibleTokens.size() == 1);
    CPPUNIT_ASSERT(compatibleTokens[0] == t0.getId());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testBasicMerging(){
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();
    // Create 2 mergeable tokens - predicates, types and base domaiuns match
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    CPPUNIT_ASSERT(t0.duration()->getDerivedDomain().getUpperBound() == 20);

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    t1.duration()->restrictBaseDomain(IntervalIntDomain(5, 7));

    // Activate & deactivate - ensure proper handling of rejectability variable
    CPPUNIT_ASSERT(!t0.getState()->getDerivedDomain().isSingleton());
    t0.activate();
    CPPUNIT_ASSERT(t0.getState()->getDerivedDomain().isSingleton());
    CPPUNIT_ASSERT(t0.getState()->getDerivedDomain().getSingletonValue() == Token::ACTIVE);
    t0.cancel();
    CPPUNIT_ASSERT(!t0.getState()->getDerivedDomain().isSingleton());

    // Now activate and merge
    t0.activate();
    t1.doMerge(t0.getId());

    // Make sure the necessary restrictions have been imposed due to merging i.e. restruction due to specified domain
    CPPUNIT_ASSERT_MESSAGE(t0.duration()->toString(), t0.duration()->getDerivedDomain().getUpperBound() == 7);
    CPPUNIT_ASSERT(t1.isMerged());

    // Do a split and make sure the old values are reinstated.
    t1.cancel();
    CPPUNIT_ASSERT_MESSAGE(t0.duration()->toString(), t0.duration()->getDerivedDomain().getUpperBound() == 20);
    CPPUNIT_ASSERT(t1.isInactive());

    // Now post equality constraint between t1 and extra token t2 and remerge
    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    t2.end()->restrictBaseDomain(IntervalIntDomain(8, 10));

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(t1.end());
    temp.push_back(t2.end());
    ConstraintId equalityConstraint = db->getConstraintEngine()->createConstraint("concurrent",
                                                                          temp);
    t1.doMerge(t0.getId());

    CPPUNIT_ASSERT(!t0.getMergedTokens().empty());

    // Verify that the equality constraint has migrated and original has been deactivated.
    //TBW: when stacking instead of merging tokens, the next check is not true
    // CPPUNIT_ASSERT(!equalityConstraint->isActive());
    CPPUNIT_ASSERT(t0.end()->getDerivedDomain().getLowerBound() == 8);
    CPPUNIT_ASSERT(t0.end()->getDerivedDomain() == t2.end()->getDerivedDomain());

    // Undo the merge and check for initial conditions being established
    t1.cancel();
    CPPUNIT_ASSERT(equalityConstraint->isActive());

    // Redo the merge
    t1.doMerge(t0.getId());

    // Confirm deletion of the constraint is handled correctly
    delete (Constraint*) equalityConstraint;
    CPPUNIT_ASSERT(t0.end()->getDerivedDomain() != t2.end()->getDerivedDomain());


    // Test subset path
    t1.cancel();
    Variable<IntervalIntDomain> superset(db->getConstraintEngine(), IntervalIntDomain(5, 6));

    ConstraintId subsetOfConstraint = db->getConstraintEngine()->createConstraint("SubsetOf",
                                                                          makeScope(t1.duration(), superset.getId()));
    t1.doMerge(t0.getId());
    CPPUNIT_ASSERT(t0.duration()->getDerivedDomain().getUpperBound() == 6);
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
    ObjectId timeline1 = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "timeline1"))->getId();
    new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "timeline2");
    db->close();

    // Create two base tokens
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));


    // Create 2 mergeable tokens - predicates, types and base domains match
    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t3(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));


    LessThanEqualConstraint c0("leq", "Default", db->getConstraintEngine(), makeScope(t1.start(), t3.start()));

    t0.activate();
    t2.activate();
    // Test base case of insertion into an empty sequence
    timeline1->constrain(t0.getId(), t2.getId());

    db->getConstraintEngine()->propagate();

    t1.doMerge(t0.getId());
    t3.doMerge(t2.getId());

    t3.cancel();
    t1.cancel();

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief This test ensures that if a new constraint is added to a variable of a merged token that the
   * constraint is properly migrated and deactivated.
   */
  static bool testConstraintAdditionAfterMerging(){
    DEFAULT_SETUP(ce, db, false);
    ObjectId timeline1 = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "timeline1"))->getId();
    new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "timeline2");
    db->close();


    // Create two base tokens
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));


    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    t0.activate();
    t1.doMerge(t0.getId());

    // Post a constraint between an active variable (t1.start) and a deactivated variable (t2.start)
    LessThanEqualConstraint c0("leq", "Default", db->getConstraintEngine(), makeScope(t1.start(), t2.start()));

    CPPUNIT_ASSERT_MESSAGE("Failed to deactive constraint on merged token", !c0.isActive());

    // Now restrict t2.start and verify that t0.start is impacted
    t2.start()->specify(5);
    CPPUNIT_ASSERT_MESSAGE("Failed to migrate constraint", t0.start()->getDerivedDomain().getUpperBound() == 5);


    // Split and verify the new constraint is activated
    t1.cancel();
    CPPUNIT_ASSERT_MESSAGE("Failed to reactivate migrated constraint.", c0.isActive());
    CPPUNIT_ASSERT_MESSAGE("Failed to clean up migrated constraint", t0.start()->getDerivedDomain().getUpperBound() == 10);
    CPPUNIT_ASSERT_MESSAGE("Failed to reinstate migrated constraint", t1.start()->getDerivedDomain().getUpperBound() == 5);

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testNonChronGNATS2439() {
    DEFAULT_SETUP(ce, db, false);
    new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "timeline1");
    db->close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    IntervalToken token0(db,
			 LabelStr(DEFAULT_PREDICATE),
			 false,
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    token0.addParameter(LabelSet(values), "LabelSetParam");
    token0.close();

    IntervalToken token1(db,
			 LabelStr(DEFAULT_PREDICATE),
			 false,
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    token1.addParameter(LabelSet(values), "LabelSetParam");
    token1.close();

    IntervalToken token2(db,
			 LabelStr(DEFAULT_PREDICATE),
			 false,
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
			 LabelStr(DEFAULT_PREDICATE),
			 false,
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
    ce->createConstraint(LabelStr("precedes"),makeScope(token2.end(),token3.start()));

    CPPUNIT_ASSERT(ce->propagate());

    // after constraining t2 to come before t3, only t2 and t3 start and
    // end domains should've changed.

    CPPUNIT_ASSERT(token0.start()->lastDomain().getLowerBound() == 0);
    CPPUNIT_ASSERT(token0.start()->lastDomain().getUpperBound() == 10);
    CPPUNIT_ASSERT(token0.end()->lastDomain().getLowerBound() == 1);
    CPPUNIT_ASSERT(token0.end()->lastDomain().getUpperBound() == 200);

    CPPUNIT_ASSERT(token1.start()->lastDomain().getLowerBound() == 0);
    CPPUNIT_ASSERT(token1.start()->lastDomain().getUpperBound() == 10);
    CPPUNIT_ASSERT(token1.end()->lastDomain().getLowerBound() == 1);
    CPPUNIT_ASSERT(token1.end()->lastDomain().getUpperBound() == 200);

    CPPUNIT_ASSERT(token2.start()->lastDomain().getLowerBound() == 0);
    CPPUNIT_ASSERT(token2.start()->lastDomain().getUpperBound() == 9);
    CPPUNIT_ASSERT(token2.end()->lastDomain().getLowerBound() == 1);
    CPPUNIT_ASSERT(token2.end()->lastDomain().getUpperBound() == 10);

    CPPUNIT_ASSERT(token3.start()->lastDomain().getLowerBound() == 1);
    CPPUNIT_ASSERT(token3.start()->lastDomain().getUpperBound() == 10);
    CPPUNIT_ASSERT(token3.end()->lastDomain().getLowerBound() == 2);
    CPPUNIT_ASSERT(token3.end()->lastDomain().getUpperBound() == 200);

    token0.activate();
    token2.doMerge(token0.getId());
    CPPUNIT_ASSERT(ce->propagate());
    token1.activate();
    token3.doMerge(token1.getId());
    CPPUNIT_ASSERT(ce->propagate());

    // after merging t2->t0 and t3->t1, all parameters should be
    // singletons. Also, t0 should now be before t1 (inheriting the
    // relation between t2 and t3).


    CPPUNIT_ASSERT(token0.parameters()[0]->lastDomain().isSingleton());
    CPPUNIT_ASSERT(token1.parameters()[0]->lastDomain().isSingleton());
    CPPUNIT_ASSERT(token2.parameters()[0]->lastDomain().isSingleton());
    CPPUNIT_ASSERT(token3.parameters()[0]->lastDomain().isSingleton());

    CPPUNIT_ASSERT(token0.start()->lastDomain().getLowerBound() == 0);
    CPPUNIT_ASSERT(token0.start()->lastDomain().getUpperBound() == 9);
    CPPUNIT_ASSERT(token0.end()->lastDomain().getLowerBound() == 1);
    CPPUNIT_ASSERT(token0.end()->lastDomain().getUpperBound() == 10);

    CPPUNIT_ASSERT(token1.start()->lastDomain().getLowerBound() == 1);
    CPPUNIT_ASSERT(token1.start()->lastDomain().getUpperBound() == 10);
    CPPUNIT_ASSERT(token1.end()->lastDomain().getLowerBound() == 2);
    CPPUNIT_ASSERT(token1.end()->lastDomain().getUpperBound() == 200);

    token2.cancel();
    CPPUNIT_ASSERT(ce->propagate());

    // after cancelling t2->t0, all parameters remain singleton except for
    // t0's since it no longer inherits the singleton domain from t2.
    // Furthermore, t0 should no longer be constrained to be before t1.
    // However, t1 should remain constrained to be before t2 since it still
    // inherits the before constraint between t2 and t3.

    CPPUNIT_ASSERT(!token0.parameters()[0]->lastDomain().isSingleton());
    CPPUNIT_ASSERT_MESSAGE(token1.parameters()[0]->toString(), token1.parameters()[0]->lastDomain().isSingleton());
    CPPUNIT_ASSERT(token2.parameters()[0]->lastDomain().isSingleton());
    CPPUNIT_ASSERT(token3.parameters()[0]->lastDomain().isSingleton());

    CPPUNIT_ASSERT(token0.start()->lastDomain().getLowerBound() == 0);
    CPPUNIT_ASSERT(token0.start()->lastDomain().getUpperBound() == 10);
    CPPUNIT_ASSERT(token0.end()->lastDomain().getLowerBound() == 1);
    CPPUNIT_ASSERT(token0.end()->lastDomain().getUpperBound() == 200);

    CPPUNIT_ASSERT(token3.isMerged());
    token3.cancel();
    CPPUNIT_ASSERT(!token3.isMerged());

    DEFAULT_TEARDOWN();
    return true;
  }

  // add backtracking and longer chain, also add a before constraint
  static bool testMergingPerformance(){
    DEFAULT_SETUP(ce, db, false);
    ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
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
      schema->addMember(LabelStr(DEFAULT_PREDICATE), IntDT::NAME(), LabelStr("P" + i).c_str());

    for (int i=0; i < NUMTOKS; i++) {
      std::vector<IntervalTokenId> tmp;
      for (int j=0; j < UNIFIED; j++) {
        IntervalTokenId t = (new IntervalToken(db,
                                               LabelStr(DEFAULT_PREDICATE),
                                               true,
                                               false,
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

    IntervalIntDomain sdom1(tokens[0][0]->start()->getDerivedDomain());
    CPPUNIT_ASSERT(sdom1.getLowerBound() == 0);
    CPPUNIT_ASSERT(sdom1.getUpperBound() == 210);

    IntervalIntDomain edom1(tokens[0][0]->end()->getDerivedDomain());
    CPPUNIT_ASSERT(edom1.getLowerBound() == 1);
    CPPUNIT_ASSERT(edom1.getUpperBound() == 220);

    Id<TokenVariable<IntervalIntDomain> > pvar1(tokens[0][0]->parameters()[0]);
    IntervalIntDomain pdom1(pvar1->getDerivedDomain());
    CPPUNIT_ASSERT(pdom1.getLowerBound() == 500);
    CPPUNIT_ASSERT(pdom1.getUpperBound() == 1000);

    TokenId predecessor = tokens[0][0];
    predecessor->activate();
    for (int i=1; i < NUMTOKS; i++) {
      tokens[i][0]->activate();
      timeline->constrain(tokens[i-1][0], tokens[i][0]);
    }

    IntervalIntDomain sdom2(tokens[0][0]->start()->getDerivedDomain());
    CPPUNIT_ASSERT(sdom2.getLowerBound() == 0);
    CPPUNIT_ASSERT(sdom2.getUpperBound() == 208);

    IntervalIntDomain edom2(tokens[0][0]->end()->getDerivedDomain());
    CPPUNIT_ASSERT(edom2.getLowerBound() == 1);
    CPPUNIT_ASSERT(edom2.getUpperBound() == 209);

    Id<TokenVariable<IntervalIntDomain> > pvar2(tokens[0][0]->parameters()[0]);
    IntervalIntDomain pdom2(pvar2->getDerivedDomain());
    CPPUNIT_ASSERT(pdom2.getLowerBound() == 500);
    CPPUNIT_ASSERT(pdom2.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) {
        tokens[i][j]->doMerge(tokens[i][0]);
        ce->propagate();
      }

    IntervalIntDomain sdom3(tokens[0][0]->start()->getDerivedDomain());
    CPPUNIT_ASSERT(sdom3.getLowerBound() == 0);
    CPPUNIT_ASSERT(sdom3.getUpperBound() == 208);

    IntervalIntDomain edom3(tokens[0][0]->end()->getDerivedDomain());
    CPPUNIT_ASSERT(edom3.getLowerBound() == 1);
    CPPUNIT_ASSERT(edom3.getUpperBound() == 209);

    Id<TokenVariable<IntervalIntDomain> > pvar3(tokens[0][0]->parameters()[0]);
    IntervalIntDomain pdom3(pvar3->getDerivedDomain());
    CPPUNIT_ASSERT(pdom3.getLowerBound() == 500+UNIFIED-1);
    CPPUNIT_ASSERT(pdom3.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) {
        tokens[i][j]->cancel();
        ce->propagate();
      }

    IntervalIntDomain sdom4(tokens[0][0]->start()->getDerivedDomain());
    CPPUNIT_ASSERT(sdom4.getLowerBound() == sdom2.getLowerBound());
    CPPUNIT_ASSERT(sdom4.getUpperBound() == sdom2.getUpperBound());

    IntervalIntDomain edom4(tokens[0][0]->end()->getDerivedDomain());
    CPPUNIT_ASSERT(edom4.getLowerBound() == edom2.getLowerBound());
    CPPUNIT_ASSERT(edom4.getUpperBound() == edom2.getUpperBound());

    Id<TokenVariable<IntervalIntDomain> > pvar4(tokens[0][0]->parameters()[0]);
    IntervalIntDomain pdom4(pvar4->getDerivedDomain());
    CPPUNIT_ASSERT(pdom4.getLowerBound() == pdom2.getLowerBound());
    CPPUNIT_ASSERT(pdom4.getUpperBound() == pdom2.getUpperBound());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTokenCompatibility(){
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();

    // Create 2 mergeable tokens - predicates, types and base domaiuns match
    IntervalToken t0(db,
                     LabelStr(LabelStr(DEFAULT_PREDICATE)),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t0.addParameter(IntervalDomain(1, 20), "IntervalParam");
    t0.close();

    // Same predicate and has an intersection
    IntervalToken t1(db,
                     LabelStr(LabelStr(DEFAULT_PREDICATE)),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t1.addParameter(IntervalDomain(10, 40), "IntervalParam"); // There is an intersection - but it is not a subset. Still should match
    t1.close();

    t0.activate();
    std::vector<TokenId> compatibleTokens;
    bool res = ce->propagate();
    CPPUNIT_ASSERT(res);
    db->getCompatibleTokens(t1.getId(), compatibleTokens);
    CPPUNIT_ASSERT(compatibleTokens.size() == 1);
    CPPUNIT_ASSERT(db->hasCompatibleTokens(t1.getId()));
    CPPUNIT_ASSERT(compatibleTokens[0] == t0.getId());

    compatibleTokens.clear();
    t0.cancel();
    res = ce->propagate();
    CPPUNIT_ASSERT(res);
    db->getCompatibleTokens(t1.getId(), compatibleTokens);
    CPPUNIT_ASSERT(compatibleTokens.empty()); // No match since no tokens are active
    CPPUNIT_ASSERT(!db->hasCompatibleTokens(t1.getId()));

    IntervalToken t2(db,
                     LabelStr(LabelStr(DEFAULT_PREDICATE)),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t2.addParameter(IntervalDomain(0, 0), "IntervalParam"); // Force no intersection
    t2.close();

    t0.activate();
    res = ce->propagate();
    CPPUNIT_ASSERT(res);
    compatibleTokens.clear();
    db->getCompatibleTokens(t2.getId(), compatibleTokens);
    CPPUNIT_ASSERT(compatibleTokens.empty()); // No match since parameter variable has no intersection


    IntervalToken t3(db,
                     LabelStr(LabelStr(DEFAULT_PREDICATE)),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t3.addParameter(IntervalDomain(), "IntervalParam"); // Force no intersection
    t3.close();

    // Post equality constraint between t3 and t0. Should permit a match since it is a binary constraint
    EqualConstraint c0("eq", "Default", db->getConstraintEngine(), makeScope(t0.start(), t3.start()));
    db->getConstraintEngine()->propagate();
    compatibleTokens.clear();
    db->getCompatibleTokens(t3.getId(), compatibleTokens);
    CPPUNIT_ASSERT(compatibleTokens.size() == 1); // Expect a single match


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
    schema->addObjectType("A", "Object");
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
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t0.close();
    t0.activate();

    IntervalToken t1(db,
                     LabelStr("A.b"),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t1.close();
    t1.activate();

    IntervalToken t2(db,
                     LabelStr("B.a"),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t2.close();
    t2.activate();

    IntervalToken t3(db,
                     LabelStr("C.a"),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t3.close();
    t3.activate();

    IntervalToken t4(db,
                     LabelStr("C.b"),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t4.close();
    t4.activate();

    IntervalToken t5(db,
                     LabelStr("C.c"),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t5.close();
    t5.activate();

    IntervalToken t6(db,
                     LabelStr("D.a"),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);
    t6.close();
    t6.activate();

    IntervalToken t7(db,
                     LabelStr("D.b"),
                     true,
                     false,
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
		      false,
		      IntervalIntDomain(0, 10),
		      IntervalIntDomain(0, 20),
		      IntervalIntDomain(1, 1000),
		      Token::noObject(), false);
      t.close();


      CPPUNIT_ASSERT(ce->propagate());
      db->getCompatibleTokens(t.getId(), results);

      LabelStr encodedNames = encodePredicateNames(results);
      CPPUNIT_ASSERT_MESSAGE("Expected = A.a:B.a:C.a:D.a:, Actual =  " + encodedNames.toString(),
          encodedNames == LabelStr("A.a:B.a:C.a:D.a:"));
    }

    // A.a => A.a, B.a, C.a., D.a. This is the case for GNATS 2837
    {
      std::vector<TokenId> results;
      IntervalToken t(db,
		      LabelStr("D.b"),
		      true,
		      false,
		      IntervalIntDomain(0, 10),
		      IntervalIntDomain(0, 20),
		      IntervalIntDomain(1, 1000),
		      Token::noObject(), false);
      t.close();


      CPPUNIT_ASSERT(ce->propagate());
      db->getCompatibleTokens(t.getId(), results);

      LabelStr encodedNames = encodePredicateNames(results);
      CPPUNIT_ASSERT_MESSAGE("Expected = D.b':, Actual =  " + encodedNames.toString(),
          encodedNames == LabelStr("D.b:"));
    }

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTokenFactory(){
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();

      TokenId master = db->createToken(LabelStr(DEFAULT_PREDICATE), true);
    master->activate();
    TokenId slave = db->createSlaveToken(master, LabelStr(DEFAULT_PREDICATE), LabelStr("any"));
    CPPUNIT_ASSERT(slave->master() == master);
    TokenId rejectable = db->createToken(LabelStr(DEFAULT_PREDICATE), false);
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
    DEFAULT_SETUP(ce, db, false);
    ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
    db->close();

    IntervalToken tokenA(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    // Change to base class to excercise problem with wrong signature on TokenVariable
    ConstrainedVariableId start = tokenA.start();
    start->specify(5);

    tokenA.activate();
    CPPUNIT_ASSERT(ce->propagate());

    IntervalToken tokenB(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    // Post a constraint on tokenB so that it will always fail when it gets merged
    ForceFailureConstraint c0("ForceFailure", "Default", ce, makeScope(tokenC.getState()));

    // Propagate and test our specified value
    CPPUNIT_ASSERT(ce->propagate());
    CPPUNIT_ASSERT(tokenA.start()->lastDomain().getSingletonValue() == 5);

    // Now do the merges and test
    tokenB.doMerge(tokenA.getId());
    CPPUNIT_ASSERT(ce->propagate());
    CPPUNIT_ASSERT(tokenA.start()->lastDomain().getSingletonValue() == 5);

    tokenC.doMerge(tokenA.getId());
    CPPUNIT_ASSERT(!ce->propagate()); // Should always fail

    // Now split it and test that the specified domain is unchanged
    tokenC.cancel();
    CPPUNIT_ASSERT(ce->propagate()); // Should be OK now
    CPPUNIT_ASSERT(tokenA.start()->lastDomain().getSingletonValue() == 5);


    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testOpenMerge() {
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();

    schema->addMember(LabelStr(DEFAULT_PREDICATE),"int",LabelStr("FOO"));

    EnumeratedDomain zero(IntDT::instance());
    zero.insert(0); zero.close();

    EnumeratedDomain one(IntDT::instance());
    one.insert(1); one.close();

    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
										 Token::noObject(),
										 false);
    t0.addParameter(zero, LabelStr("FOO"));
    t0.close();

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
										 Token::noObject(),
										 false);
    t1.addParameter(zero, LabelStr("FOO"));
    t1.getVariable(LabelStr("FOO"))->open();
    t1.close();

    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
										 Token::noObject(),
										 false);
    t2.addParameter(zero, LabelStr("FOO"));
    t2.getVariable(LabelStr("FOO"))->open();
    t2.close();

    IntervalToken t3(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
					 Token::noObject(),
					false);
    t3.addParameter(one, LabelStr("FOO"));
    t3.getVariable(LabelStr("FOO"))->open();
    t3.close();

    IntervalToken t4(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
					 Token::noObject(),
					 false);
    t4.addParameter(one, LabelStr("FOO"));
    t4.getVariable(LabelStr("FOO"))->open();
    t4.close();

    CPPUNIT_ASSERT(t0.getObject()->isClosed());
    t1.getObject()->open();
    CPPUNIT_ASSERT(!t1.getObject()->isClosed());

    t0.activate();
    t2.activate();
    t4.activate();
    CPPUNIT_ASSERT(ce->propagate());

    std::vector<TokenId> compatibleTokens0, compatibleTokens1, compatibleTokens3;

    CPPUNIT_ASSERT(db->hasCompatibleTokens(t1.getId()));
    db->getCompatibleTokens(t1.getId(), compatibleTokens1);
    CPPUNIT_ASSERT(compatibleTokens1.size() == 3); // open {0} intersects with open domains and closed domains containing {0}
    CPPUNIT_ASSERT((compatibleTokens1[0] == t0.getId() || compatibleTokens1[0] == t2.getId() || compatibleTokens1[0] == t4.getId()) &&
               (compatibleTokens1[1] == t0.getId() || compatibleTokens1[1] == t2.getId() || compatibleTokens1[1] == t4.getId()) &&
               (compatibleTokens1[2] == t0.getId() || compatibleTokens1[2] == t2.getId() || compatibleTokens1[2] == t4.getId()));

    CPPUNIT_ASSERT(db->hasCompatibleTokens(t3.getId()));
    db->getCompatibleTokens(t3.getId(), compatibleTokens3);

    CPPUNIT_ASSERT(compatibleTokens3.size() == 3); // open {0} intersects with open domains and closed domains containing {0}
    CPPUNIT_ASSERT((compatibleTokens3[0] == t0.getId() || compatibleTokens3[0] == t2.getId() || compatibleTokens3[0] == t4.getId()) &&
               (compatibleTokens3[1] == t0.getId() || compatibleTokens3[1] == t2.getId() || compatibleTokens3[1] == t4.getId()) &&
               (compatibleTokens3[2] == t0.getId() || compatibleTokens3[2] == t2.getId() || compatibleTokens3[2] == t4.getId()));

    t1.doMerge(t0.getId());
    t3.doMerge(t2.getId());

    DEFAULT_TEARDOWN();
    return true;
  }



  static bool testGNATS_3086() {
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();

    LabelSet lbl;
    lbl.insert("L1");

    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);

    ConstrainedVariableId param0 = t0.addParameter(lbl, "LabelSetParam");
    t0.close();

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000),
                     Token::noObject(), false);

    ConstrainedVariableId param1 = t1.addParameter(lbl, "LabelSetParam");
    t1.close();

    CPPUNIT_ASSERT(!param0->lastDomain().isClosed());
    CPPUNIT_ASSERT(!param1->lastDomain().isClosed());

    param0->specify(LabelStr("L1"));
    CPPUNIT_ASSERT(param0->lastDomain().isClosed());

    // Now activate and merge onto it.
    t1.activate();
    t0.doMerge(t1.getId());

    CPPUNIT_ASSERT(param0->lastDomain().isClosed());
    CPPUNIT_ASSERT(param1->lastDomain().isClosed());

    // Reset, thus reverting to the base domain which should be open
    param0->reset();
    CPPUNIT_ASSERT_MESSAGE(param0->toString(), !param0->lastDomain().isClosed());
    CPPUNIT_ASSERT_MESSAGE(param1->toString(), !param1->lastDomain().isClosed());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCompatCacheReset() {
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();
    //create a regular token
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    //create a token that ends slightly later
    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(10, 20),
                     IntervalIntDomain(1, 1000));

    t0.activate();
    t1.activate();

    //equate the end times
    std::vector<ConstrainedVariableId> temp;
    temp.push_back(t0.end());
    temp.push_back(t1.end());
    ConstraintId eq = db->getConstraintEngine()->createConstraint("concurrent",
                                                          temp);


    //create a token that has to end later
    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 9),
                     IntervalIntDomain(1, 1000));

    ce->propagate(); //propagate the change

    //shouldn't be able to merge with anything
    CPPUNIT_ASSERT(db->countCompatibleTokens(t2.getId(), PLUS_INFINITY, true) == 0);

    delete (Constraint *) eq; //remove the constraint

    ce->propagate();

    // Should now be able to merge
    CPPUNIT_ASSERT(db->countCompatibleTokens(t2.getId(), PLUS_INFINITY, true) > 0);

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testAssignemnt(){
      DEFAULT_SETUP(ce, db, false);
    Object o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "o1");
    Object o2(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    db->close();

    //create a token that has to end later
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 9),
                     IntervalIntDomain(1, 1000));

    ce->propagate();

    // Should not be assigned since inactive and object domain not a sinleton
    CPPUNIT_ASSERT(!t0.isAssigned());

    // Should not have this token for the same reasons
    CPPUNIT_ASSERT(!o1.hasToken(t0.getId()));

    // Now activate it, situation should be unchanged
    t0.activate();

    ce->propagate();

    // Should not be assigned since inactive and object domain not a singleton
    CPPUNIT_ASSERT(!t0.isAssigned());
    CPPUNIT_ASSERT(!o1.hasToken(t0.getId()));

    // Now specify the object value. Expect it to be assigned.
    t0.getObject()->specify(o1.getId());
    ce->propagate();

    CPPUNIT_ASSERT(t0.isAssigned());
    CPPUNIT_ASSERT(o1.hasToken(t0.getId()));

    // Now we can reset and expect it to go back
    t0.getObject()->reset();
    ce->propagate();

    CPPUNIT_ASSERT(!t0.isAssigned());
    CPPUNIT_ASSERT(!o1.hasToken(t0.getId()));
    DEFAULT_TEARDOWN();
    return true;
  }


  /**
   * @brief Test master-slave relationships are built and maintained correctly in the face of non-chronological
   * retractions.
   */
  static bool testDeleteMasterAndPreserveSlave(){
      DEFAULT_SETUP(ce, db, false);
    Timeline o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl1");
    db->close();

    TokenId master = (new IntervalToken(db,
				       LabelStr(DEFAULT_PREDICATE),
				       true,
				       false,
				       IntervalIntDomain(0, 10),
				       IntervalIntDomain(0, 20),
				       IntervalIntDomain(1, 1000)))->getId();
    master->activate();

    TokenId slaveA = (new IntervalToken(master,
					"any",
					LabelStr(DEFAULT_PREDICATE),
					IntervalIntDomain(0, 10),
					IntervalIntDomain(0, 20),
					IntervalIntDomain(1, 1000)))->getId();

    TokenId slaveB = (new IntervalToken(master,
					"any",
					LabelStr(DEFAULT_PREDICATE),
					IntervalIntDomain(0, 10),
					IntervalIntDomain(0, 20),
					IntervalIntDomain(1, 1000)))->getId();

    slaveB->activate();
    slaveB->commit();


    // Now delete the master and expect the uncommitted slave to be discarded but the committed slave to
    // be retained.
    delete (Token*) master;
    CPPUNIT_ASSERT(slaveA->isDiscarded());
    CPPUNIT_ASSERT(slaveB->isCommitted());

    delete (Token*) slaveB;
    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief Test merge relationships are built and maintained correctly in the face of non-chronological
   * retractions.
   */
  static bool testPreserveMergeWithNonChronSplit(){
      DEFAULT_SETUP(ce, db, false);
    Timeline o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl1");
    db->close();

    TokenId master = (new IntervalToken(db,
				       LabelStr(DEFAULT_PREDICATE),
				       true,
				       false,
				       IntervalIntDomain(0, 10),
				       IntervalIntDomain(0, 20),
				       IntervalIntDomain(1, 1000)))->getId();
    master->activate();



    TokenId orphan = (new IntervalToken(db,
					LabelStr(DEFAULT_PREDICATE),
					true,
					false,
					IntervalIntDomain(0, 10),
					IntervalIntDomain(0, 20),
					IntervalIntDomain(1, 1000)))->getId();

    TokenId slaveA = (new IntervalToken(master,
					"any",
					LabelStr(DEFAULT_PREDICATE),
					IntervalIntDomain(0, 10),
					IntervalIntDomain(0, 20),
					IntervalIntDomain(1, 1000)))->getId();

    TokenId slaveB = (new IntervalToken(master,
					"any",
					LabelStr(DEFAULT_PREDICATE),
					IntervalIntDomain(0, 10),
					IntervalIntDomain(0, 20),
					IntervalIntDomain(1, 1000)))->getId();

    TokenId slaveC = (new IntervalToken(master,
					"any",
					LabelStr(DEFAULT_PREDICATE),
					IntervalIntDomain(0, 10),
					IntervalIntDomain(0, 20),
					IntervalIntDomain(1, 1000)))->getId();

    // Activate C and merge other 2 slaves onto it
    slaveC->activate();
    slaveB->doMerge(slaveC);
    slaveA->doMerge(slaveC);

    // Commit C so it will survive deletion of the master
    slaveC->commit();

    // Now merge the orphan onto it
    orphan->doMerge(slaveC);

    // Now delete the master - should force slaves A and B to be deleted
    delete (Token*) master;
    CPPUNIT_ASSERT(slaveA->isDiscarded());
    CPPUNIT_ASSERT(slaveB->isDiscarded());
    CPPUNIT_ASSERT(slaveC->isCommitted());
    CPPUNIT_ASSERT(orphan->isMerged());

    delete (Token*) slaveC;
    CPPUNIT_ASSERT(orphan->isInactive());

    delete (Token*) orphan;
    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief Test that when a committed token is cancelled, it remains a candidate for merging
   */
  static bool testKeepingCommittedTokensInActiveSet(){
    DEFAULT_SETUP(ce, db, false);
    Timeline o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl1");
    db->close();

    // Allocate and commit a token
    TokenId a = (new IntervalToken(db,
				       LabelStr(DEFAULT_PREDICATE),
				       true,
				       false,
				       IntervalIntDomain(0, 10),
				       IntervalIntDomain(0, 20),
				       IntervalIntDomain(1, 1000)))->getId();
    a->activate();
    a->commit();

    // Now cancel the commit
    a->cancel();

    // We should still have an active token
    CPPUNIT_ASSERT(db->getActiveTokens(LabelStr(DEFAULT_PREDICATE)).size() == 1);

    // Delete the token and ensure that we no longer have active tokens
    delete (Token*) a;
    CPPUNIT_ASSERT(db->getActiveTokens(LabelStr(DEFAULT_PREDICATE)).empty());

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * After some work, we now have it boiled down to a failure to handle the case of restricting
   * a base domain correctly. Problem is that the change to use the base domain being a singleton
   * as permiting 'isSpecified' to be true. Consequently, when restricting the base domain,
   * we over-ride 'specify' and it thinks the variable is already specified.
   */
  static bool testGNATS_3163(){
      DEFAULT_SETUP(ce, db, false);
    Timeline o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl1");
    db->close();

    TokenId master = (new IntervalToken(db,
				       LabelStr(DEFAULT_PREDICATE),
				       true,
				       false,
				       IntervalIntDomain(0, 10),
				       IntervalIntDomain(0, 20),
				       IntervalIntDomain(1, 1000)))->getId();

    master->start()->restrictBaseDomain(IntervalIntDomain(1, 1));
    CPPUNIT_ASSERT(master->start()->specifiedFlag());
    master->discard();
    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Should be able to post a constraint on a state variable of a token which would then get merged. Confirm that
   * the constraint remains active after the merge (although it will be basically moot since if consistent the state
   * variable will be grounded). Also want to ensure that if we delete the constraint, it will be removed safely.
   */
  static bool testGNATS_3193(){
      DEFAULT_SETUP(ce, db, false);
    Timeline o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl1");
    db->close();

    TokenId t0 = (new IntervalToken(db,
				       LabelStr(DEFAULT_PREDICATE),
				       true,
				       false,
				       IntervalIntDomain(0, 10),
				       IntervalIntDomain(0, 20),
				       IntervalIntDomain(1, 1000)))->getId();
    t0->activate();

    TokenId t1 = (new IntervalToken(db,
				       LabelStr(DEFAULT_PREDICATE),
				       true,
				       false,
				       IntervalIntDomain(0, 10),
				       IntervalIntDomain(0, 20),
				       IntervalIntDomain(1, 1000)))->getId();
    {
      StateDomain dom;
      dom.insert(Token::MERGED);
      dom.close();

      Variable<StateDomain> v(ce, dom);
      EqualConstraint c0("eq", "Default", ce, makeScope(t1->getState(), v.getId()));

      t1->doMerge(t0);
    }

    t1->discard();
    t0->discard();

    DEFAULT_TEARDOWN();
    return true;
  }
};

class TimelineTest {
public:
  static bool test(){
    EUROPA_runTest(testFullInsertion);
    EUROPA_runTest(testBasicInsertion);
    EUROPA_runTest(testObjectTokenRelation);
    EUROPA_runTest(testTokenOrderQuery);
    EUROPA_runTest(testEventTokenInsertion);
    EUROPA_runTest(testNoChoicesThatFit);
    EUROPA_runTest(testAssignment);
    EUROPA_runTest(testFreeAndConstrain);
    EUROPA_runTest(testRemovalOfMasterAndSlave);

    /* The archiving algorithm needs to be rewritten in EUROPA. Or better still, taken out of EUROPA. We can keep these tests for reference but they are both
       incomplete and incorrect. CMG
       EUROPA_runTest(testArchiving1);
       EUROPA_runTest(testArchiving2);
       EUROPA_runTest(testArchiving3);
       EUROPA_runTest(testGNATS_3162);
    */
    return true;
  }

private:

  static bool testBasicInsertion(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    db->close();

    IntervalToken tokenA(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenD(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    CPPUNIT_ASSERT(!timeline.hasTokensToOrder());
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    tokenD.activate();

    // Establish preliminaries
    std::vector<TokenId> tokens;
    timeline.getTokensToOrder(tokens);
    CPPUNIT_ASSERT(tokens.size() == 4);
    CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 0);
    CPPUNIT_ASSERT(timeline.hasTokensToOrder());
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
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 2);

      timeline.constrain(tokenB.getId(), tokenC.getId());
      num_constraints += 2;
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 3);

      timeline.free(tokenB.getId(), tokenC.getId());
      num_constraints -= 2;
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 2);

      timeline.free(tokenA.getId(), tokenB.getId());
      num_constraints -= 3;
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 0);
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
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);

      timeline.constrain(tokenC.getId(), tokenA.getId());
      num_constraints += 2; // Object variable and a single temporal constraint since placing at the beginning
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);

      CPPUNIT_ASSERT(tokenA.end()->getDerivedDomain().getUpperBound() <= tokenB.start()->getDerivedDomain().getUpperBound());
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 3);

      timeline.free(tokenA.getId(), tokenB.getId());
      num_constraints -= 2; // Should remove 1 object constraint and 1 temporal constraint
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 2);

      timeline.free(tokenC.getId(), tokenA.getId());
      num_constraints -= 3;
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 0);
    }

    /**
     * UNSUPPORTED MIDDLE PAIR
     */
    {
      timeline.constrain(tokenA.getId(), tokenB.getId());
      timeline.constrain(tokenB.getId(), tokenC.getId());
      timeline.constrain(tokenC.getId(), tokenD.getId());
      num_constraints += 7; // 4 object constraints and 3 temporal constraints
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      timeline.free(tokenB.getId(), tokenC.getId());
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints); // No change. Middle link unsupported
      timeline.free(tokenC.getId(), tokenD.getId()); // Should remove C, and D.
      num_constraints -= 4; // Objects constraints for C, and D, and implict constraint for B->C
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
      timeline.free(tokenA.getId(), tokenB.getId());
      num_constraints -= 3;
      CPPUNIT_ASSERT_MESSAGE(toString(ce->getConstraints().size()) + " " + toString(num_constraints),
          ce->getConstraints().size() == num_constraints);
      CPPUNIT_ASSERT(timeline.getTokenSequence().size() == 0);
    }

    /**
     * REMOVAL OF SPANNING CONSTRAINT BETWEEN START AND END
     */
    {
      timeline.constrain(tokenA.getId(), tokenD.getId()); // +3
      timeline.constrain(tokenC.getId(), tokenD.getId()); // +3
      timeline.constrain(tokenB.getId(), tokenC.getId()); // +3
      num_constraints += 9;
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);

      // Remove spanning link.
      timeline.free(tokenA.getId(), tokenD.getId());
      num_constraints -= 4; // One object constraint, one explciit constraint, and 2 implicit constraints
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);

      // Remove B,C and expect to get rid of A and B from the sequence
      timeline.free(tokenB.getId(), tokenC.getId());
      num_constraints -= 2; // 1 object and 1 temporal constraints
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);

      // Remove B,C and expect to get rid of A and B from the sequence
      timeline.free(tokenC.getId(), tokenD.getId());
      num_constraints -= 3; // 2 object and 1 temporal constraints
      CPPUNIT_ASSERT(ce->getConstraints().size() == num_constraints);
    }

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testObjectTokenRelation(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    db->close();

    IntervalToken tokenA(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    // Object variables are not singletons - so query for tokens to order should return nothing
    std::vector<TokenId> tokensToOrder;
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.empty());

    // Specify the object variable of one - but still should return no tokens since they are all inactive
    tokenA.getObject()->specify(timeline.getId());
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.empty());

    // Now activate all of them
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.size() == 3);

    // Set remainders so they are singeltons and get all back
    tokenB.getObject()->specify(timeline.getId());
    tokenC.getObject()->specify(timeline.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.size() == 3);

    // Now incrementally constrain and show reduction in tokens to order
    timeline.constrain(tokenA.getId(), tokenB.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.size() == 1);

    timeline.constrain(tokenB.getId(), tokenC.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.empty());


    // Test destruction call path
    Token* tokenD = new IntervalToken(db,
                                      LabelStr(LabelStr(DEFAULT_PREDICATE)),
                                      true,
                                      false,
                                      IntervalIntDomain(0, 10),
                                      IntervalIntDomain(0, 20),
                                      IntervalIntDomain(1, 1000));
    tokenD->activate();
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.size() == 1);
    timeline.constrain(tokenC.getId(), tokenD->getId());
    delete tokenD;
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    CPPUNIT_ASSERT(tokensToOrder.empty());
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTokenOrderQuery(){
    DEFAULT_SETUP(ce, db, false);

    Id<Timeline> timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
    db->close();

    const int COUNT = 5;
    const int DURATION = 10;

    for (int i=0;i<COUNT;i++){
      int start = i*DURATION;
      TokenId token = (new IntervalToken(db,
                                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                                         true,
                                         false,
                                         IntervalIntDomain(start, start),
                                         IntervalIntDomain(start+DURATION, start+DURATION),
                                         IntervalIntDomain(DURATION, DURATION)))->getId();
      CPPUNIT_ASSERT(token->getObject()->getBaseDomain().isSingleton());
      token->getObject()->specify(timeline->getId());
      token->activate();
    }

    CPPUNIT_ASSERT(timeline->tokens().size() == (unsigned int) COUNT);
    ce->propagate(); // Should not alter the count. Relationship updated eagerly
    CPPUNIT_ASSERT(timeline->tokens().size() == (unsigned int) COUNT);

    int i = 0;
    std::vector<TokenId> tokensToOrder;
    timeline->getTokensToOrder(tokensToOrder);

    while(!tokensToOrder.empty()){
      CPPUNIT_ASSERT(timeline->getTokenSequence().size() == (unsigned int) i);
      CPPUNIT_ASSERT(tokensToOrder.size() == (unsigned int) (COUNT - i));
      std::vector< std::pair<TokenId, TokenId> > choices;
      TokenId toConstrain = tokensToOrder.front();
      timeline->getOrderingChoices(toConstrain, choices);
      CPPUNIT_ASSERT(!choices.empty());
      TokenId predecessor = choices.front().first;
      TokenId successor = choices.front().second;

      CPPUNIT_ASSERT_MESSAGE("The token from the tokens to order must be a predecessor or a successor.",
          toConstrain == predecessor || toConstrain == successor);

      timeline->constrain(predecessor, successor);
      bool res = ce->propagate();
      CPPUNIT_ASSERT(res);
      tokensToOrder.clear();
      timeline->getTokensToOrder(tokensToOrder);
      i++;
      res = ce->propagate();
      CPPUNIT_ASSERT(res);
    }

    const std::list<TokenId>& tokenSequence = timeline->getTokenSequence();
    CPPUNIT_ASSERT(tokenSequence.front()->start()->getDerivedDomain().getSingletonValue() == 0);
    CPPUNIT_ASSERT(tokenSequence.back()->end()->getDerivedDomain().getSingletonValue() == COUNT*DURATION);

    // Now ensure the query can correctly indicate no options available
    TokenId token = (new IntervalToken(db,
                                       LabelStr(LabelStr(DEFAULT_PREDICATE)),
                                       true,
                                       false,
                                       IntervalIntDomain(),
                                       IntervalIntDomain(),
                                       IntervalIntDomain(DURATION, DURATION)))->getId();
    token->getObject()->specify(timeline->getId());
    token->start()->specify(0);
    token->activate();
    std::vector<std::pair<TokenId, TokenId> > choices;
    timeline->getOrderingChoices(token, choices);
    CPPUNIT_ASSERT(choices.empty());

    // Now back off restrictions to token and try patterns which excercise cache management in the face of merging and rejection
    token->cancel();
    token->getObject()->reset();
    token->start()->reset();
    CPPUNIT_ASSERT(ce->propagate());


    TokenId token2 = (new IntervalToken(db,
					LabelStr(LabelStr(DEFAULT_PREDICATE)),
					true,
					false,
					IntervalIntDomain(),
					IntervalIntDomain(),
					IntervalIntDomain(DURATION, DURATION)))->getId();

    choices.clear();
    timeline->getOrderingChoices(token2, choices);
    CPPUNIT_ASSERT(!choices.empty());
    token->reject();
    CPPUNIT_ASSERT(ce->propagate());
    choices.clear();
    timeline->getOrderingChoices(token2, choices);
    CPPUNIT_ASSERT(!choices.empty());

    token->cancel();
    CPPUNIT_ASSERT(ce->propagate());
    choices.clear();
    timeline->getOrderingChoices(token, choices);
    CPPUNIT_ASSERT(!choices.empty());

    std::vector< TokenId > mergeChoices;
    db->getCompatibleTokens(token, mergeChoices);
    CPPUNIT_ASSERT(!mergeChoices.empty());
    token->doMerge(mergeChoices.front());
    CPPUNIT_ASSERT(ce->propagate());

    choices.clear();
    timeline->getOrderingChoices(token2, choices);
    CPPUNIT_ASSERT(!choices.empty());

    token->cancel();
    CPPUNIT_ASSERT(ce->propagate());
    choices.clear();
    timeline->getOrderingChoices(token, choices);
    CPPUNIT_ASSERT(!choices.empty());

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testEventTokenInsertion(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    db->close();

    IntervalToken it1(db,
                      LabelStr(LabelStr(DEFAULT_PREDICATE)),
                      true,
                      false,
                      IntervalIntDomain(0, 10),
                      IntervalIntDomain(0, 1000),
                      IntervalIntDomain(1, 1000));

    it1.getObject()->specify(timeline.getId());
    it1.activate();
    timeline.constrain(it1.getId(), it1.getId());

    // Insert at the end after a token
    EventToken et1(db,
                   LabelStr(DEFAULT_PREDICATE),
                   true,
                   false,
                   IntervalIntDomain(0, 100),
                   Token::noObject());

    et1.getObject()->specify(timeline.getId());
    et1.activate();
    timeline.constrain(it1.getId(), et1.getId());
    CPPUNIT_ASSERT(it1.end()->getDerivedDomain().getUpperBound() == 100);

    // Insert between a token and an event
    EventToken et2(db,
                   LabelStr(DEFAULT_PREDICATE),
                   true,
                   false,
                   IntervalIntDomain(0, 100),
                   Token::noObject());

    et2.getObject()->specify(timeline.getId());
    et2.activate();
    timeline.constrain(et2.getId(), et1.getId());
    CPPUNIT_ASSERT(it1.end()->getDerivedDomain().getUpperBound() == 100);

    // Insert before a token
    EventToken et3(db,
                   LabelStr(DEFAULT_PREDICATE),
                   true,
                   false,
                   IntervalIntDomain(10, 100),
                   Token::noObject());

    et3.getObject()->specify(timeline.getId());
    et3.activate();
    timeline.constrain(et3.getId(), it1.getId());
    CPPUNIT_ASSERT(it1.start()->getDerivedDomain().getLowerBound() == 10);

    // Insert between events
    EventToken et4(db,
                   LabelStr(DEFAULT_PREDICATE),
                   true,
                   false,
                   IntervalIntDomain(0, 100),
                   Token::noObject());

    et4.getObject()->specify(timeline.getId());
    et4.activate();
    timeline.constrain(et4.getId(), et1.getId());
    bool res = ce->propagate();
    CPPUNIT_ASSERT(res);
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testFullInsertion(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2");
    db->close();

    IntervalToken tokenA(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(0, 10),
                         IntervalIntDomain(0, 20),
                         IntervalIntDomain(1, 1000));

    CPPUNIT_ASSERT(!timeline.hasTokensToOrder());
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();

    timeline.constrain(tokenA.getId(), tokenB.getId()); // Insert A and B.
    CPPUNIT_ASSERT(tokenA.end()->getDerivedDomain().getUpperBound() <= tokenB.start()->getDerivedDomain().getUpperBound());

    // Now insert token C in the middle.
    timeline.constrain(tokenC.getId(), tokenB.getId());
    CPPUNIT_ASSERT(tokenA.end()->getDerivedDomain().getUpperBound() <= tokenC.start()->getDerivedDomain().getUpperBound());
    CPPUNIT_ASSERT(tokenC.end()->getDerivedDomain().getUpperBound() <= tokenB.start()->getDerivedDomain().getUpperBound());
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testNoChoicesThatFit(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE) , "o2");
    db->close();

    IntervalToken tokenA(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(10, 10),
                         IntervalIntDomain(20, 20),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(100, 100),
                         IntervalIntDomain(120, 120),
                         IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db,
                         LabelStr(LabelStr(DEFAULT_PREDICATE)),
                         true,
                         false,
                         IntervalIntDomain(9, 9),
                         IntervalIntDomain(11, 11),
                         IntervalIntDomain(1, 1000));

    tokenA.activate();
    tokenB.activate();
    tokenC.activate();

    timeline.constrain(tokenA.getId(), tokenB.getId());
    bool res = ce->propagate();
    CPPUNIT_ASSERT(res);

    std::vector<std::pair<TokenId, TokenId> > choices;
    timeline.getOrderingChoices(tokenC.getId(), choices);
    CPPUNIT_ASSERT(choices.empty());
    timeline.constrain(tokenC.getId(), tokenB.getId());
    res = ce->propagate();
    CPPUNIT_ASSERT(!res);

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testAssignment(){
      DEFAULT_SETUP(ce, db, false);
    Timeline o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl1");
    Timeline o2(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl2");
    db->close();

    //create a token that has to end later
    IntervalToken t0(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 9),
                     IntervalIntDomain(1, 1000));

    ce->propagate();

    // Should not be assigned since inactive and object domain not a sinleton
    CPPUNIT_ASSERT(!t0.isAssigned());

    // Should not have this token for the same reasons
    CPPUNIT_ASSERT(!o1.hasToken(t0.getId()));

    // Now activate it, situation should be unchanged
    t0.activate();

    ce->propagate();

    // Should not be assigned since inactive and object domain not a singleton
    CPPUNIT_ASSERT(!t0.isAssigned());
    CPPUNIT_ASSERT(!o1.hasToken(t0.getId()));

    // Now specify the object value.
    t0.getObject()->specify(o1.getId());
    ce->propagate();

    // It should still not be assigned
    CPPUNIT_ASSERT(!t0.isAssigned());
    CPPUNIT_ASSERT(!o1.hasToken(t0.getId()));

    // Now constrain the token and finnaly expect it to be assigned
    o1.constrain(t0.getId(), t0.getId());
    ce->propagate();

    CPPUNIT_ASSERT(t0.isAssigned());
    CPPUNIT_ASSERT(o1.hasToken(t0.getId()));

    // Free the token and it should be un assigned
    o1.free(t0.getId(), t0.getId());
    CPPUNIT_ASSERT(!t0.isAssigned());
    CPPUNIT_ASSERT(!o1.hasToken(t0.getId()));
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testFreeAndConstrain(){
      DEFAULT_SETUP(ce, db, false);
    Timeline o1(db, LabelStr(DEFAULT_OBJECT_TYPE), "tl1");
    db->close();

    IntervalToken t1(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t2(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    IntervalToken t3(db,
                     LabelStr(DEFAULT_PREDICATE),
                     true,
                     false,
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 20),
                     IntervalIntDomain(1, 1000));

    t1.activate();
    t2.activate();
    t3.activate();

    // Insert, but keep putting in the middle
    o1.constrain(t1.getId(), t3.getId());
    o1.constrain(t2.getId(), t3.getId());

    o1.free(t1.getId(), t3.getId());
    o1.free(t2.getId(), t3.getId());

    // Constrain again to leave all cleanup automatic
    o1.constrain(t1.getId(), t3.getId());

    // Also use a locally scoped token to force a different deletion path
    TokenId t4 = (new IntervalToken(db,
				    LabelStr(DEFAULT_PREDICATE),
				    true,
				    false,
				    IntervalIntDomain(0, 10),
				    IntervalIntDomain(0, 20),
				    IntervalIntDomain(1, 1000)))->getId();

    t4->activate();
    o1.constrain(t4, t3.getId());
    o1.constrain(t2.getId(), t4);
    CPPUNIT_ASSERT(ce->propagate());

    // Now delete t4 to leave a hole which will require repair
    delete (Token*) t4;
    CPPUNIT_ASSERT(ce->propagate());
    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * This test makes sure we can correctly remove a master-slave pair on a timeline
   * when the slave is forced to be removed because the master is being deleted.
   */
  static bool testRemovalOfMasterAndSlave(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE) , "o2");
    db->close();

    {
      TokenId master = (new IntervalToken(db,
					  LabelStr(LabelStr(DEFAULT_PREDICATE)),
					  true,
					  false,
					  IntervalIntDomain(0, 0),
					  IntervalIntDomain(),
					  IntervalIntDomain(1, 1)))->getId();
      master->activate();

      TokenId slave = (new IntervalToken(master, "any" ,
					 LabelStr(LabelStr(DEFAULT_PREDICATE)),
					 IntervalIntDomain(1, 1),
					 IntervalIntDomain(),
					 IntervalIntDomain(1, 1)))->getId();


      slave->activate();

      timeline.constrain(master, slave); // Place at the end

      // Propagate to consistency first
      ce->propagate();

      // Now nuke the master and make sure we safely delete both.
      delete (Token*) master;

      CPPUNIT_ASSERT(db->getTokens().empty());
    }


    {
      TokenId master = (new IntervalToken(db,
					  LabelStr(LabelStr(DEFAULT_PREDICATE)),
					  true,
					  false,
					  IntervalIntDomain(1, 1),
					  IntervalIntDomain(),
					  IntervalIntDomain(1, 1)))->getId();
      master->activate();

      TokenId slave = (new IntervalToken(master, "any" ,
					 LabelStr(LabelStr(DEFAULT_PREDICATE)),
					 IntervalIntDomain(0, 0),
					 IntervalIntDomain(),
					 IntervalIntDomain(1, 1)))->getId();


      slave->activate();

      timeline.constrain(slave,master); // Place at the end

      // Propagate to consistency first
      ce->propagate();

      // Now nuke the master and make sure we safely delete both.
      delete (Token*) master;

      CPPUNIT_ASSERT(db->getTokens().empty());
    }

    DEFAULT_TEARDOWN();
    return true;
  }


  /**
   * @brief This test will address the need to be able to remove active and inactive tokens
   * in the database cleanly, without propagation, in the event that no consequenmces should arise.
   * Tokens do not reach into the furture so there is no risk of conflicts there. The idea is to
   * allocate in incrementing ticks, 4 tokens at a time. Then archive over the same tick increments
   * and verify the database is reduced 4 tokens at a time. This will cover active and assigned, active and free,
   * inactive orphans, and inactive slaves.
   */
  static bool testArchiving1() {
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE) , "o2");
    db->close();

    const unsigned int startTick(0);
    const unsigned int endTick(10);

    // Allocate tokens in each tick
    TokenId lastToken;
    for(unsigned int i=startTick;i<endTick;i++){
      TokenId tokenA = (new IntervalToken(db,
					  LabelStr(LabelStr(DEFAULT_PREDICATE)),
					  true,
					  false,
					  IntervalIntDomain(i, i),
					  IntervalIntDomain(),
					  IntervalIntDomain(1, 1)))->getId();
      tokenA->activate();

      // Allocate an inactive orphan
      new IntervalToken(db,
			LabelStr(LabelStr(DEFAULT_PREDICATE)),
			true,
			false,
			IntervalIntDomain(i, i),
			IntervalIntDomain(),
			IntervalIntDomain(1, 1));

      // Allocate an inactive slave
      new IntervalToken(tokenA, "any",
			LabelStr(LabelStr(DEFAULT_PREDICATE)),
			IntervalIntDomain(i, i),
			IntervalIntDomain(),
			IntervalIntDomain(1, 1));

      // Allocate an active slave
      Token* slaveB = new IntervalToken(tokenA, "any",
					LabelStr(LabelStr(DEFAULT_PREDICATE)),
					IntervalIntDomain(i, i),
					IntervalIntDomain(),
					IntervalIntDomain(1, 1));

      slaveB->activate();

      // Allocate a committed slave
      Token* slaveC = new IntervalToken(tokenA, "any",
					LabelStr(LabelStr(DEFAULT_PREDICATE)),
					IntervalIntDomain(i, i),
					IntervalIntDomain(),
					IntervalIntDomain(1, 1));

      slaveC->activate();
      slaveC->commit();

      if(lastToken.isNoId())
	lastToken = tokenA;

      timeline.constrain(lastToken, tokenA); // Place at the end
      slaveB->getObject()->specify(timeline.getId());
      slaveC->getObject()->specify(timeline.getId());
      ce->propagate();
      tokenA->restrictBaseDomains();
      slaveB->restrictBaseDomains();
      slaveC->restrictBaseDomains();

      lastToken = tokenA;
    }

    // Propagate to consistency first
    ce->propagate();

    // Now incrementally archive, verifying that no propagation is required after each
    for(unsigned int i=startTick;i<endTick;i++){
      db->archive(i+1);
      CPPUNIT_ASSERT(ce->constraintConsistent());
    }


    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief Now we want to verify that uncommitted dependent tokens reaching into the future are correctly
   * preserved.
   */
  static bool testArchiving2() {
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE) , "o2");
    db->close();

    const unsigned int startTick(0);
    const unsigned int endTick(10);

    TokenId tokenA = (new IntervalToken(db,
					LabelStr(LabelStr(DEFAULT_PREDICATE)),
					true,
					false,
					IntervalIntDomain(startTick, startTick),
					IntervalIntDomain(),
					IntervalIntDomain(1, 1)))->getId();
    tokenA->activate();
    tokenA->commit();

    for(unsigned int i=startTick+1;i<endTick;i++){
      TokenId tokenB = (new IntervalToken(tokenA, "any" ,
					  LabelStr(LabelStr(DEFAULT_PREDICATE)),
					  IntervalIntDomain(i, i),
					  IntervalIntDomain(),
					  IntervalIntDomain(1, 1)))->getId();


      tokenB->activate();
      timeline.constrain(tokenA, tokenB); // Place at the end
      ce->propagate();
      tokenB->restrictBaseDomains();
      tokenA->restrictBaseDomains();
      tokenA = tokenB;
    }

    // Propagate to consistency first
    ce->propagate();

    // Nuke one at a time, without any committing
    for(unsigned int i=startTick+1;i<=endTick;i++){
      CPPUNIT_ASSERT(db->archive(startTick+i) == 1);
    }

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * @brief Simliar to prior example but now we commit as we go. This should prevent automatic deallocaation
   * until we archove on each increment
   */
  static bool testArchiving3() {
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE) , "o2");
    db->close();

    const unsigned int startTick(0);
    const unsigned int endTick(10);

    TokenId tokenA = (new IntervalToken(db,
					LabelStr(LabelStr(DEFAULT_PREDICATE)),
					true,
					false,
					IntervalIntDomain(startTick, startTick),
					IntervalIntDomain(),
					IntervalIntDomain(1, 1)))->getId();
    tokenA->activate();

    for(unsigned int i=startTick+1;i<endTick;i++){
      TokenId tokenB = (new IntervalToken(tokenA, "any" ,
					  LabelStr(LabelStr(DEFAULT_PREDICATE)),
					  IntervalIntDomain(i, i),
					  IntervalIntDomain(),
					  IntervalIntDomain(1, 1)))->getId();


      tokenB->activate();
      timeline.constrain(tokenA, tokenB); // Place at the end
      // Commit it. Should prevent deallocation until it is archived within
      // its own tick
      tokenB->commit();
      ce->propagate();
      tokenA->restrictBaseDomains();
      tokenB->restrictBaseDomains();
      tokenA = tokenB;
    }

    // Propagate to consistency first
    ce->propagate();


    // Now incrementally archive, verifying that no propagation is required afetr each
    const unsigned int TOKENS_PER_TICK(1);
    for(unsigned int i=startTick;i<endTick;i++){
      CPPUNIT_ASSERT(db->getTokens().size() == (endTick - i) * TOKENS_PER_TICK);
      unsigned int deletionCount = db->archive(i+1);
      CPPUNIT_ASSERT_MESSAGE(toString(deletionCount), deletionCount == TOKENS_PER_TICK);
      CPPUNIT_ASSERT(ce->constraintConsistent());
    }
    CPPUNIT_ASSERT(db->getTokens().empty());

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Test archiving a token which is spporting other tokens, and then archive the supported tokens.
   * Note that this case fails to reproduce the problem.
   */
  static bool testGNATS_3162(){
    DEFAULT_SETUP(ce, db, false);
    Timeline timeline(db, LabelStr(DEFAULT_OBJECT_TYPE) , "o2");
    db->close();

    const unsigned int startTick(0);

    TokenId tokenA = (new IntervalToken(db,
					LabelStr(LabelStr(DEFAULT_PREDICATE)),
					true,
					false,
					IntervalIntDomain(startTick, startTick),
					IntervalIntDomain(),
					IntervalIntDomain(1, 1)))->getId();
    tokenA->activate();
    timeline.constrain(tokenA, tokenA);
    ce->propagate();

    TokenId tokenB = (new IntervalToken(db,
					LabelStr(LabelStr(DEFAULT_PREDICATE)),
					true,
					false,
					IntervalIntDomain(startTick, startTick),
					IntervalIntDomain(),
					IntervalIntDomain(1, 1)))->getId();
    tokenB->doMerge(tokenA);
    tokenA->restrictBaseDomains();
    tokenA->commit();
    ce->propagate();
    //CPPUNIT_ASSERT(tokenA->canBeTerminated());
    //CPPUNIT_ASSERT(tokenB->canBeTerminated());

    tokenA->terminate();
    tokenA->discard();
    ce->propagate();
    DEFAULT_TEARDOWN();
    return true;
  }
};

class DbClientTest {
public:
  static bool test(){
    EUROPA_runTest(testFactoryMethods);
    EUROPA_runTest(testBasicAllocation);
    EUROPA_runTest(testPathBasedRetrieval);
    EUROPA_runTest(testGlobalVariables);
    return true;
  }
private:
  static bool testFactoryMethods(){
    std::vector<const AbstractDomain*> arguments;
    IntervalIntDomain arg0(10, 10);
    LabelSet arg1(LabelStr("Label"));
    arguments.push_back(&arg0);
    arguments.push_back(&arg1);
    LabelStr factoryName = ObjectTypeMgr::makeFactoryName(LabelStr("Foo"), arguments);
    CPPUNIT_ASSERT(factoryName == LabelStr("Foo:int:string"));
    return true;
  }

  static bool testBasicAllocation(){
    DEFAULT_SETUP(ce, db, false);

    DbClientId client = db->getClient();
    client->enableTransactionLogging();
    DbClientTransactionLog* txLog = new DbClientTransactionLog(client);

    DBFooId foo1 = client->createObject(LabelStr(DEFAULT_OBJECT_TYPE).c_str(), "foo1");
    CPPUNIT_ASSERT(foo1.isValid());

    std::vector<const AbstractDomain*> arguments;
    IntervalIntDomain arg0(10);
    LabelSet arg1(LabelStr("Label"));
    arguments.push_back(&arg0);
    arguments.push_back(&arg1);
    DBFooId foo2 = client->createObject(LabelStr(DEFAULT_OBJECT_TYPE).c_str(), "foo2", arguments);
    CPPUNIT_ASSERT(foo2.isValid());

    TokenId token = client->createToken(LabelStr(DEFAULT_PREDICATE).c_str());
    CPPUNIT_ASSERT(token.isValid());

    // Constrain the token duration
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(token->start());
    scope.push_back(token->duration());
    client->createConstraint("eq", scope);

    delete txLog;
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testPathBasedRetrieval(){
      DEFAULT_SETUP(ce, db, false);
      ObjectId timeline = (new Timeline(db, LabelStr(DEFAULT_OBJECT_TYPE), "o2"))->getId();
      db->close();

    db->getClient()->enableTransactionLogging();
    TokenId t0 = db->getClient()->createToken(LabelStr(DEFAULT_PREDICATE).c_str());
    t0->activate();

    TokenId t1 = db->getClient()->createToken(LabelStr(DEFAULT_PREDICATE).c_str());
    t1->activate();

    TokenId t0_0 = (new IntervalToken(t0, "any",
                                      LabelStr(DEFAULT_PREDICATE),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_0->activate();

    TokenId t0_1 = (new IntervalToken(t0, "any",
                                      LabelStr(DEFAULT_PREDICATE),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_1->activate();

    TokenId t0_2 = (new IntervalToken(t0, "any",
                                      LabelStr(DEFAULT_PREDICATE),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t0_2->activate();

    TokenId t1_0 = (new IntervalToken(t1, "any",
                                      LabelStr(DEFAULT_PREDICATE),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(0, 1),
                                      IntervalIntDomain(1, 1)))->getId();
    t1_0->activate();

    TokenId t0_1_0 = (new EventToken(t0_1, "any",
                                     LabelStr(DEFAULT_PREDICATE),
                                     IntervalIntDomain(0, 1)))->getId();
    t0_1_0->activate();

    TokenId t0_1_1 = (new EventToken(t0_1, "any",
                                     LabelStr(DEFAULT_PREDICATE),
                                     IntervalIntDomain(0, 1)))->getId();
    t0_1_1->activate();

    // Test paths
    std::vector<int> path;
    path.push_back(0); // Start with the index of the token key in the path


    // Base case with just the root
    CPPUNIT_ASSERT(db->getClient()->getTokenByPath(path) == t0);
    CPPUNIT_ASSERT(db->getClient()->getPathByToken(t0).size() == 1);

    // Now test a more convoluted path
    path.push_back(1);
    path.push_back(1);
    CPPUNIT_ASSERT(db->getClient()->getTokenByPath(path) == t0_1_1);

    path.clear();
    path = db->getClient()->getPathByToken(t0_1_1);
    CPPUNIT_ASSERT(path.size() == 3);
    CPPUNIT_ASSERT(path[0] == 0);
    CPPUNIT_ASSERT(path[1] == 1);
    CPPUNIT_ASSERT(path[2] == 1);


    // Negative tests
    path.push_back(100);
    CPPUNIT_ASSERT(db->getClient()->getTokenByPath(path) == TokenId::noId());
    path[0] = 99999;
    CPPUNIT_ASSERT(db->getClient()->getTokenByPath(path) == TokenId::noId());
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testGlobalVariables(){
    DEFAULT_SETUP(ce, db, true);

    DbClientId client = db->getClient();

    // Allocate
    client->createVariable(IntDT::NAME().c_str(), "v1");
    client->createVariable(IntDT::NAME().c_str(), "v2");
    client->createVariable(IntDT::NAME().c_str(), "v3");

    // Retrieve
    ConstrainedVariableId v1 = client->getGlobalVariable("v1");
    ConstrainedVariableId v2 = client->getGlobalVariable("v2");
    ConstrainedVariableId v3 = client->getGlobalVariable("v3");


    // Use
    client->createConstraint("eq", makeScope(v1, v2));
    client->createConstraint("neq", makeScope(v1, v3));
    client->createConstraint("eq", makeScope(v2, v3));

    v1->specify(10);

    CPPUNIT_ASSERT(!client->propagate());

    DEFAULT_TEARDOWN();
    return true;
  }
};

/**
 * Locations enumeration's base domain, as required by the schema.
 * @note Copied from System/test/basic-model-transaction.cc
 * as created from basic-model-transaction.nddl v1.3 with the NDDL compiler.
 */
const Locations& LocationsBaseDomain() {
  static RestrictedDT dt("Locations",SymbolDT::instance(),SymbolDomain());
  static Locations sl_enum(dt.getId());
  if (sl_enum.isOpen()) {
    sl_enum.insert(LabelStr("Hill"));
    sl_enum.insert(LabelStr("Rock"));
    sl_enum.insert(LabelStr("Lander"));
    sl_enum.close();
  }
  return(sl_enum);
}


/**
 * @class DbTransPlayerTest
 * Test the DbClientTransactionPlayer class's interface and semantics in minor ways.
 */
class DbTransPlayerTest {

public:

  /** Run the tests. */
  static bool test() {
    EUROPA_runTest(testImpl);
    EUROPA_runTest(provokeErrors);
    return(true);
  }

  /*
   * For lists of arguments, by type and name (or type and string (even if "1") value).
   * @note This has a different use and purpose than a list or vector of AbstractDomain*.
   * That is for a type name and abstract domain; this is for a type name and a variable name.
   */
  typedef std::list<std::pair<std::string, std::string> > ArgList;
  typedef ArgList::const_iterator ArgIter;

  static StateDomain& getMandatoryStateDom()
  {
	  static StateDomain s_mandatoryStateDom;
	  return s_mandatoryStateDom;
  }

  static StateDomain& getRejectableStateDom()
  {
	  static StateDomain s_rejectableStateDom;
	  return s_rejectableStateDom;
  }

  /** Run all tests within this class that do not try to provoke errors. */
  static bool testImpl() {
    DEFAULT_SETUP(ce, db, false);
    s_ce = ce;
    s_db = db;
    s_dbPlayer = new DbClientTransactionPlayer((s_db)->getClient());
    CPPUNIT_ASSERT(s_dbPlayer != 0);

    REGISTER_OBJECT_FACTORY(db->getSchema(),TestClass2Factory, TestClass2);
    REGISTER_OBJECT_FACTORY(db->getSchema(),TestClass2Factory, TestClass2:string:int:float:Locations);

    /* Token factory for predicate Sample */
    db->getSchema()->registerTokenFactory((new TestClass2::Sample::Factory())->getId());

    /* Initialize state-domain-at-creation of mandatory and rejectable tokens.  Const after this. */
    getMandatoryStateDom().remove(Token::REJECTED);
    getMandatoryStateDom().close();
    getRejectableStateDom().close();

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

    testDefineEnumeration();
    testCreateVariable();
    testDeleteVariable();
    testUndeleteVariable();
    testDefineClass();
    testCreateObject();
    testDeleteObject();
    testUndeleteObject();
    testSpecifyVariable();
    testResetVariable();
    testUnresetVariable();
    testCreateTokens();
    testDeleteTokens();
    testUndeleteTokens(); //I
    testInvokeConstraint();
    testReinvokeConstraint(); //I
    testConstrain();
    testFree();
    testUnfree();
    testActivate();
    testMerge();
    testReject();
    testCancel();
    testUncancel();

    delete s_dbPlayer;
    DEFAULT_TEARDOWN();
    return(true);
  }

  /** Run all tests within this class that do try to provoke errors. */
  static bool provokeErrors() {
    //!! None implemented yet
    return(true);
  }

  class TestClass2;
  typedef Id<TestClass2> TestClass2Id;

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
      Sample(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false, bool isFact = false)
        : IntervalToken(planDb, name, rejectable, isFact, IntervalIntDomain(), IntervalIntDomain(),
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
      // ... but that is in NDDL/base/NddlUtils.hh, which this should not depend on, so:
      class Factory : public TokenFactory {
      public:
        Factory()
          : TokenFactory(LabelStr("TestClass2.Sample")) {
        }
      private:
        TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable = false, bool isFact = false) const {
          TokenId token = (new Sample(planDb, name, rejectable, isFact))->getId();
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

  class TestClass2Factory: public ObjectFactory {
  public:
    TestClass2Factory(const LabelStr& name)
      : ObjectFactory(name) {
    }
  private:
    ObjectId createInstance(const PlanDatabaseId& planDb,
                            const LabelStr& objectType,
                            const LabelStr& objectName,
                            const std::vector<const AbstractDomain*>& arguments) const {
      CPPUNIT_ASSERT(arguments.size() == 0 || arguments.size() == 4);
      if (arguments.size() == 4) {
        //!!I'm not sure why this first one is passed in; it appears to be the object's type info.
        //!!--wedgingt@email.arc.nasa.gov 2004 Nov 1
        CPPUNIT_ASSERT(arguments[0]->getTypeName() == LabelStr(StringDT::NAME()));
        CPPUNIT_ASSERT(arguments[1]->getTypeName() == LabelStr(IntDT::NAME()));
        CPPUNIT_ASSERT(arguments[2]->getTypeName() == LabelStr(FloatDT::NAME()));
        CPPUNIT_ASSERT(arguments[3]->getTypeName() == LabelStr("Locations"));
      }
      TestClass2Id instance = (new TestClass2(planDb, objectType, objectName))->getId();
      instance->handleDefaults();
      std::vector<ConstrainedVariableId> vars = instance->getVariables();
      for (unsigned int i = 1; i < arguments.size(); i++){
	if(arguments[i]->isSingleton())
	   vars[i - 1]->specify(arguments[i]->getSingletonValue());
	else
	  vars[i - 1]->restrictBaseDomain(*(arguments[i]));
      }
      debugMsg("TestClass2:createInstance", "TestClass2 objectId " << instance->getId() << ' ' << instance->getName().toString()
                << " has varIds " << vars[0] << ' ' << vars[1] << ' ' << vars[2] << '\n');
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

  static void testRewindingXML(const std::string& xml, const char* file, const int& line,
			       bool breakpoint = false);

  /**
   * @def TEST_PLAYING_XML
   * Call testPlayingXML with __FILE__ and __LINE__.
   */
#define TEST_PLAYING_XML(xml) (testPlayingXML(xml, __FILE__, __LINE__))

#define TEST_REWINDING_XML(xml) (testRewindingXML(xml, __FILE__, __LINE__))

  /** Test defining an enumeration. */
  static void testDefineEnumeration() {
    std::list<std::string> locs;
    locs.push_back(std::string("Hill"));
    locs.push_back(std::string("Rock"));
    locs.push_back(std::string("Lander"));

    /* Create the XML string and play it. */
    TEST_PLAYING_XML(buildXMLEnumStr(std::string("Locations"), locs, __FILE__, __LINE__));
    // Nothing to verify, since the player cannot actually create enumerations.
  }

  /** Test creating variables. */
  static void testCreateVariable() {
    TEST_PLAYING_XML(buildXMLNameTypeStr("var", "g_int", IntDT::NAME(), __FILE__, __LINE__));
    TEST_PLAYING_XML(buildXMLNameTypeStr("var", "g_float", FloatDT::NAME(), __FILE__, __LINE__));
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

    CPPUNIT_ASSERT(!sg_int.isNoId() && sg_int.isValid());
    CPPUNIT_ASSERT(sg_int->lastDomain() == IntervalIntDomain());
    CPPUNIT_ASSERT(!sg_float.isNoId() && sg_float.isValid());
    CPPUNIT_ASSERT(sg_float->lastDomain() == IntervalDomain());
    CPPUNIT_ASSERT(!sg_location.isNoId() && sg_location.isValid());
    CPPUNIT_ASSERT(sg_location->lastDomain() == LocationsBaseDomain());
    CPPUNIT_ASSERT(g_int2.isNoId());
    CPPUNIT_ASSERT(g_float2.isNoId());
    CPPUNIT_ASSERT(g_location2.isNoId());
  }

  static void testDeleteVariable() {
    CPPUNIT_ASSERT(s_db->getClient()->isGlobalVariable("g_int"));
    TEST_REWINDING_XML(buildXMLNameTypeStr("var", "g_int",
					  IntDT::NAME(),
					  __FILE__, __LINE__));
    CPPUNIT_ASSERT(!s_db->getClient()->isGlobalVariable("g_int"));
    ConstrainedVariableSet allVars = s_ce->getVariables();
    for(ConstrainedVariableSet::iterator it = allVars.begin(); it != allVars.end(); ++it) {
      CPPUNIT_ASSERT((*it)->getName() != LabelStr("g_int"));
    }
    //have to re-create the variable because future tests depend on it
    TEST_PLAYING_XML(buildXMLNameTypeStr("var", "g_int",
					  IntDT::NAME(),
					  __FILE__, __LINE__));
    CPPUNIT_ASSERT(s_db->getClient()->isGlobalVariable("g_int"));
    bool found = false;
    allVars = s_ce->getVariables();
    for(ConstrainedVariableSet::iterator it = allVars.begin(); it != allVars.end() && !found;
	++it) {
      if((found = ((*it)->getName() == LabelStr("g_int")))) {
	sg_int = (*it);
      }
    }
    CPPUNIT_ASSERT(found);
  }

  static void testUndeleteVariable() {
    CPPUNIT_ASSERT(s_db->getClient()->isGlobalVariable("g_int"));
    bool found = false;
    ConstrainedVariableSet allVars = s_ce->getVariables();
    for(ConstrainedVariableSet::iterator it = allVars.begin(); it != allVars.end() && !found;
	++it)
      found = ((*it)->getName() == LabelStr("g_int"));
    CPPUNIT_ASSERT(found);

    std::stringstream transactions;
    transactions << "<deletevar index=\"" << s_db->getClient()->getIndexByVariable(sg_int) <<
      "\" name=\"g_int\"/>";
    TEST_PLAYING_XML(transactions.str());

    CPPUNIT_ASSERT(!s_db->getClient()->isGlobalVariable("g_int"));
    allVars = s_ce->getVariables();
    for(ConstrainedVariableSet::iterator it = allVars.begin(); it != allVars.end(); ++it) {
      CPPUNIT_ASSERT((*it)->getName() != LabelStr("g_int"));
    }

    std::stringstream otherTransactions;
    otherTransactions << buildXMLNameTypeStr("var", "g_int",
					     IntDT::NAME(),
					     __FILE__, __LINE__);
    otherTransactions << "<breakpoint/>";
    otherTransactions << transactions.str();

    testRewindingXML(otherTransactions.str(), __FILE__, __LINE__, true);
    CPPUNIT_ASSERT(s_db->getClient()->isGlobalVariable("g_int"));
    found = false;
    allVars = s_ce->getVariables();
    for(ConstrainedVariableSet::iterator it = allVars.begin(); it != allVars.end() && !found;
	++it) {
      if((found = ((*it)->getName() == LabelStr("g_int")))) {
	sg_int = (*it);
      }
    }
    CPPUNIT_ASSERT(found);
  }

  /** Test defining classes. */
  static void testDefineClass() {
    s_db->getSchema()->addObjectType("TestClass1");
    CPPUNIT_ASSERT(s_db->getSchema()->isObjectType("TestClass1"));

    TEST_PLAYING_XML(buildXMLNameStr("class", "TestClass1", __FILE__, __LINE__));

    s_db->getSchema()->addObjectType("TestClass2");
    CPPUNIT_ASSERT(s_db->getSchema()->isObjectType("TestClass2"));
    s_db->getSchema()->addMember("TestClass2", IntDT::NAME(), "int1");
    s_db->getSchema()->addMember("TestClass2", FloatDT::NAME(), "float2");
    s_db->getSchema()->addMember("TestClass2", "Locations", "where");
    s_db->getSchema()->addPredicate("TestClass2.Sample");
    s_db->getSchema()->addMember("TestClass2.Sample", FloatDT::NAME(), "m_x");
    s_db->getSchema()->addMember("TestClass2.Sample", FloatDT::NAME(), "m_y");
    s_db->getSchema()->addMember("TestClass2.Sample", "Locations", "m_closest");

    ArgList args;
    args.push_back(std::make_pair(IntDT::NAME(), std::string("int1")));
    args.push_back(std::make_pair(FloatDT::NAME(), std::string("float2")));
    args.push_back(std::make_pair(std::string("Locations"), std::string("where")));
    TEST_PLAYING_XML(buildXMLCreateClassStr("TestClass2", args, __FILE__, __LINE__));
    // Nothing to verify, since the player cannot actually create classes.
  }

  /** Helper function to clean up after otherwise memory leaking calls to new. */
  inline static void cleanDomains(std::vector<const AbstractDomain*>& doms) {
    for (unsigned i = 0; i < doms.size(); i++)
      delete doms[i];
    doms.clear();
  }

  /**
   * Test creating an object.
   * @note Side-effect: leaves two objects, "testObj2a" and "testObj2b", in plan db.
   */
  static void testCreateObject() {
    std::vector<const AbstractDomain*> domains;
    domains.push_back(new IntervalIntDomain(1));
    domains.push_back(new IntervalDomain(1.414));
    Locations* ld1 = new Locations(LocationsBaseDomain());
    ld1->set(LabelStr("Hill"));
    domains.push_back(ld1);
    //!!Other types?  Has to match class's definition in the schema.
    TEST_PLAYING_XML(buildXMLCreateObjectStr("TestClass2", "testObj2a", domains));
    cleanDomains(domains);
    ObjectId obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(!obj2a.isNoId() && obj2a.isValid());
    CPPUNIT_ASSERT(obj2a->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2a->getName() == LabelStr("testObj2a"));
    std::vector<ConstrainedVariableId> obj2vars = obj2a->getVariables();
    CPPUNIT_ASSERT(obj2vars.size() == 3);
    for (int i = 0; i < 3; i++) {
      ConstrainedVariableId var = obj2vars[i];
      CPPUNIT_ASSERT(!var.isNoId() && var.isValid());
      CPPUNIT_ASSERT(var->isValid());
      std::set<ConstraintId> constraints;
      var->constraints(constraints);
      CPPUNIT_ASSERT(constraints.empty());
      CPPUNIT_ASSERT(var->parent() == obj2a);
      CPPUNIT_ASSERT(i == var->getIndex());
      switch (i) {
      case 0:
        CPPUNIT_ASSERT(var->lastDomain() == IntervalIntDomain(1));
        break;
      case 1:
        CPPUNIT_ASSERT(var->lastDomain() == IntervalDomain(1.414));
        break;
      case 2: {
          Locations* ld1 = new Locations(LocationsBaseDomain());
          ld1->set(LabelStr("Hill"));
        CPPUNIT_ASSERT(var->lastDomain() == *ld1);
        break;
      }
      default:
        CPPUNIT_ASSERT_MESSAGE("erroneous variable index within obj2a", false);
      }
    }
    domains.push_back(new IntervalIntDomain(2));
    domains.push_back(new IntervalDomain(3.14159265358979));
    std::list<double> locs;
    locs.push_back(LabelStr("Rock"));
    Locations* ld2 = new Locations(LocationsBaseDomain());
    ld2->set(LabelStr("Rock"));
    domains.push_back(ld2);
    Locations toCompare(*ld2);
    //!!Other types?  Has to match class's definition in the schema.
    TEST_PLAYING_XML(buildXMLCreateObjectStr("TestClass2", "testObj2b", domains));
    cleanDomains(domains);
    ObjectId obj2b = s_db->getObject("testObj2b");
    CPPUNIT_ASSERT(!obj2b.isNoId() && obj2b.isValid());
    CPPUNIT_ASSERT(obj2b->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2b->getName() == LabelStr("testObj2b"));
    obj2vars = obj2b->getVariables();
    CPPUNIT_ASSERT(obj2vars.size() == 3);
    for (int i = 0; i < 3; i++) {
      ConstrainedVariableId var = obj2vars[i];
      CPPUNIT_ASSERT(!var.isNoId() && var.isValid());
      CPPUNIT_ASSERT(var->isValid());
      std::set<ConstraintId> constraints;
      var->constraints(constraints);
      CPPUNIT_ASSERT(constraints.empty());
      CPPUNIT_ASSERT(var->parent() == obj2b);
      CPPUNIT_ASSERT(i == var->getIndex());
      switch (i) {
      case 0:
        CPPUNIT_ASSERT(var->lastDomain() == IntervalIntDomain(2));
        break;
      case 1:
        CPPUNIT_ASSERT(var->lastDomain() == IntervalDomain(3.14159265358979));
        break;
      case 2:
        CPPUNIT_ASSERT(var->lastDomain() == toCompare);
        break;
      default:
        CPPUNIT_ASSERT_MESSAGE("erroneous variable index within obj2b", false);
        ;
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

  static void testDeleteObject() {
    ObjectId obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(!obj2a.isNoId() && obj2a.isValid());
    CPPUNIT_ASSERT(obj2a->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2a->getName() == LabelStr("testObj2a"));

    std::vector<const AbstractDomain*> domains;
    domains.push_back(new IntervalIntDomain(1));
    domains.push_back(new IntervalDomain(1.414));
    Locations* ld1 = new Locations(LocationsBaseDomain());
    ld1->set(LabelStr("Hill"));
    domains.push_back(ld1);
    std::string transaction = buildXMLCreateObjectStr("TestClass2", "testObj2a", domains);
    cleanDomains(domains);

    TEST_REWINDING_XML(transaction);

    obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(obj2a.isNoId());

    //have to re-play in case something needs this object
    TEST_PLAYING_XML(transaction);
    obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(!obj2a.isNoId() && obj2a.isValid());
    CPPUNIT_ASSERT(obj2a->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2a->getName() == LabelStr("testObj2a"));

  }

  static void testUndeleteObject() {
    ObjectId obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(!obj2a.isNoId() && obj2a.isValid());
    CPPUNIT_ASSERT(obj2a->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2a->getName() == LabelStr("testObj2a"));

    std::vector<const AbstractDomain*> domains;
    domains.push_back(new IntervalIntDomain(1));
    domains.push_back(new IntervalDomain(1.414));
    Locations* ld1 = new Locations(LocationsBaseDomain());
    ld1->set(LabelStr("Hill"));
    domains.push_back(ld1);
    std::string createTransaction =
      buildXMLCreateObjectStr("TestClass2", "testObj2a", domains);
    cleanDomains(domains);


    std::string deleteTransaction = "<deleteobject name=\"testObj2a\"/>";

    TEST_PLAYING_XML(deleteTransaction);

    obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(obj2a.isNoId());

    std::stringstream transactions;
    transactions << createTransaction;
    transactions << "<breakpoint/>";
    transactions << deleteTransaction;

    testRewindingXML(transactions.str(), __FILE__, __LINE__, true);

    obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(!obj2a.isNoId() && obj2a.isValid());
    CPPUNIT_ASSERT(obj2a->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2a->getName() == LabelStr("testObj2a"));

  }

  /** Test specifying variables. */
  static void testSpecifyVariable() {
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_int, IntervalIntDomain(-5)));
    CPPUNIT_ASSERT(sg_int->lastDomain() == IntervalIntDomain(-5));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_float, IntervalDomain(-5.0)));
    CPPUNIT_ASSERT(sg_float->lastDomain() == IntervalDomain(-5.0));

    Locations ld1(LocationsBaseDomain());
    ld1.set(LabelStr("Lander"));
    TEST_PLAYING_XML(buildXMLSpecifyVariableStr(sg_location, ld1));
    CPPUNIT_ASSERT(sg_location->lastDomain() == Locations(ld1));
  }

  /** Test resetting variables. */
  static void testResetVariable() {
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_int));
    CPPUNIT_ASSERT_MESSAGE(sg_int->toString(), sg_int->lastDomain() == IntervalIntDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_float));
    CPPUNIT_ASSERT(sg_float->lastDomain() == IntervalDomain());

    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_location));
    CPPUNIT_ASSERT(sg_location->lastDomain() == LocationsBaseDomain());

    ObjectId obj2b = s_db->getObject("testObj2b");
    CPPUNIT_ASSERT(!obj2b.isNoId() && obj2b.isValid());
    CPPUNIT_ASSERT(obj2b->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2b->getName() == LabelStr("testObj2b"));
    std::vector<ConstrainedVariableId> obj2vars = obj2b->getVariables();
    CPPUNIT_ASSERT(obj2vars.size() == 3);
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[0]));
    CPPUNIT_ASSERT_MESSAGE(obj2vars[0]->toString(), obj2vars[0]->lastDomain() == IntervalIntDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[1]));
    CPPUNIT_ASSERT_MESSAGE(obj2vars[1]->toString(), obj2vars[1]->lastDomain() == IntervalDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[2]));
    CPPUNIT_ASSERT_MESSAGE(obj2vars[2]->toString(), obj2vars[2]->lastDomain() == LocationsBaseDomain());


    std::string transaction = buildXMLSpecifyVariableStr(sg_int, IntervalIntDomain(-5));
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT(sg_int->lastDomain() == IntervalIntDomain(-5));

    TEST_REWINDING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE(sg_int->toString(), sg_int->lastDomain() == IntervalIntDomain());
  }

  static void testUnresetVariable() {
    std::string specify = buildXMLSpecifyVariableStr(sg_int, IntervalIntDomain(-5));
    std::stringstream transactions;
    transactions << specify;
    transactions << "<breakpoint/>";
    transactions << buildXMLResetVariableStr(sg_int);
    testRewindingXML(transactions.str(), __FILE__, __LINE__, true);

    CPPUNIT_ASSERT(sg_int->lastDomain() == IntervalIntDomain(-5));
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_int));
  }

  /** Test invoking constraints, including "special cases" (as the player calls them). */
  static void testInvokeConstraint() {
    // First section: constraints between variables
    ObjectId obj2b = s_db->getObject("testObj2b");
    CPPUNIT_ASSERT(!obj2b.isNoId() && obj2b.isValid());
    CPPUNIT_ASSERT(obj2b->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2b->getName() == LabelStr("testObj2b"));
    std::vector<ConstrainedVariableId> obj2vars = obj2b->getVariables();
    CPPUNIT_ASSERT(obj2vars.size() == 3);

    // First constraint
    std::list<ConstrainedVariableId> vars;
    vars.push_back(sg_int);
    vars.push_back(obj2vars[0]);
    std::string transaction = buildXMLInvokeConstrainVarsStr("Equal", vars);
    TEST_PLAYING_XML(transaction);
    std::set<ConstraintId> constraints;
    sg_int->constraints(constraints);
    CPPUNIT_ASSERT(constraints.size() == 1);
    ConstraintId constr = *(constraints.begin());
    CPPUNIT_ASSERT(constr->getName() == LabelStr("Equal"));
    CPPUNIT_ASSERT(constr->getScope().size() == 2);
    CPPUNIT_ASSERT(constr->isVariableOf(sg_int));
    CPPUNIT_ASSERT(constr->isVariableOf(obj2vars[0]));
    constraints.clear();
    obj2vars[0]->constraints(constraints);
    CPPUNIT_ASSERT(constraints.size() == 1);
    CPPUNIT_ASSERT(constr == *(constraints.begin()));


    // Second constraint
    vars.clear();
    vars.push_back(sg_int);
    vars.push_back(sg_float);
    TEST_PLAYING_XML(buildXMLInvokeConstrainVarsStr("LessThanEqual", vars));
    constraints.clear();
    sg_int->constraints(constraints);
    CPPUNIT_ASSERT(constraints.size() == 2);
    CPPUNIT_ASSERT(constraints.find(constr) != constraints.end());
    constraints.erase(constraints.find(constr));
    constr = *(constraints.begin());
    CPPUNIT_ASSERT(constr->getName() == LabelStr("LessThanEqual"));
    CPPUNIT_ASSERT(constr->getScope().size() == 2);
    CPPUNIT_ASSERT(constr->isVariableOf(sg_int));
    CPPUNIT_ASSERT(constr->isVariableOf(sg_float));
    constraints.clear();
    sg_float->constraints(constraints);
    CPPUNIT_ASSERT(constraints.size() == 1);
    CPPUNIT_ASSERT(constr == *(constraints.begin()));

    // Third constraint
    vars.clear();
    vars.push_back(sg_location);
    vars.push_back(obj2vars[2]);
    TEST_PLAYING_XML(buildXMLInvokeConstrainVarsStr("NotEqual", vars));
    constraints.clear();
    sg_location->constraints(constraints);
    CPPUNIT_ASSERT(constraints.size() == 1);
    constr = *(constraints.begin());
    CPPUNIT_ASSERT(constr->getName() == LabelStr("NotEqual"));
    CPPUNIT_ASSERT(constr->getScope().size() == 2);
    CPPUNIT_ASSERT(constr->isVariableOf(sg_location));
    CPPUNIT_ASSERT(constr->isVariableOf(obj2vars[2]));
    constraints.clear();
    obj2vars[2]->constraints(constraints);
    CPPUNIT_ASSERT(constraints.size() == 1);
    CPPUNIT_ASSERT(constr == *(constraints.begin()));

    Locations ld1(LocationsBaseDomain());
    ld1.set(LabelStr("Hill"));
    // Specifying variables is one of the special cases.
    TEST_PLAYING_XML(buildXMLInvokeSpecifyVariableStr(sg_location, Locations(ld1)));
    CPPUNIT_ASSERT(sg_location->lastDomain() == Locations(ld1));
    std::list<double> locs;
    locs.push_back(LabelStr("Hill"));
    locs.push_back(LabelStr("Rock"));

    // Resetting variables via invoke is _not_ supported by the player, so do it the other way:
    TEST_PLAYING_XML(buildXMLResetVariableStr(sg_location));
    CPPUNIT_ASSERT(sg_location->lastDomain() == LocationsBaseDomain());
    TEST_PLAYING_XML(buildXMLResetVariableStr(obj2vars[2]));
    CPPUNIT_ASSERT(obj2vars[2]->lastDomain() == LocationsBaseDomain());

    //!!Most special cases involve tokens: constrain, free, activate, merge, reject, cancel
    //!!  Of these, only activate and constrain are used in any of PLASMA/System/test/*.xml.
    //!!  So try each of those two once here to try to catch problems sooner but presume that
    //!!    the other variants of <invoke> are obsolete, at least for now.

    // Activate a token.  Will have to create one and identify it first.
    TokenSet oldTokens = s_db->getTokens();
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", false, "invokeActivateTestToken"));
    TokenSet newTokens = s_db->getTokens();
    CPPUNIT_ASSERT(oldTokens.size() + 1 == newTokens.size());
    TokenId tok = *(newTokens.begin());
    while (oldTokens.find(tok) != oldTokens.end()) {
      newTokens.erase(newTokens.begin());
      tok = *(newTokens.begin());
    }
    CPPUNIT_ASSERT(!tok->isActive());
    TEST_PLAYING_XML(buildXMLInvokeActivateTokenStr("invokeActivateTestToken"));
    CPPUNIT_ASSERT(tok->isActive());
    // Now, destroy it so it doesn't affect later tests.
    delete (Token*) tok;
    tok = TokenId::noId();

    // Special case #1: close an class object domain
    CPPUNIT_ASSERT(!s_db->isClosed("TestClass2"));
    TEST_PLAYING_XML("<invoke name=\"close\" identifier=\"TestClass2\"/>");
    CPPUNIT_ASSERT(s_db->isClosed("TestClass2"));
    debugMsg("testInvokeconstraint", __FILE__ << ':' << __LINE__ << ": TestClass2 object domain is "
              << ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")) << " (size " << ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")).getSize()
              << "); should be 2 members");

    //!!This is failing, despite the prior checks passing, because the domain is still open.
    //!!CPPUNIT_ASSERT(ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")).getSize() == 2);
    if (ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")).isOpen()) {
    	debugMsg("testInvokeconstraint", __FILE__ << ':' << __LINE__ << ": TestClass2 base domain is still open, despite plan database saying otherwise");
    }
    //!!See if closing the entire database takes care of this as well:

    /* Closing the database is the last special case. */
    TEST_PLAYING_XML("<invoke name=\"close\"/>");

    //!!See just above
    debugMsg("testInvokeconstraint", __FILE__ << ':' << __LINE__ << ": After closing db, TestClass2 object domain is "
              << ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")) << " (size " << ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")).getSize()
              << "); should be 2 members");
    //!!Still fails
    //!!CPPUNIT_ASSERT(ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")).getSize() == 2);
    if (ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")).isOpen()) {
    	debugMsg("testInvokeconstraint", __FILE__ << ':' << __LINE__ << ": TestClass2 base domain is still open, despite plan database saying otherwise");
    }
  }

  static void testReinvokeConstraint() {

  }

  /** Test creating tokens. */
  static void testCreateTokens() {
    testCreateGoalTokens();
    testCreateSubgoalTokens();
  }

  /**
   * Test that the given token matches the criteria.
   * @note If only used as condition in CPPUNIT_ASSERT() (or changed to
   * return void), this could itself use 'CPPUNIT_ASSERT(cond)' rather
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
    CPPUNIT_ASSERT(tokens.size() == 1);
    TokenId token = *(tokens.begin());
    CPPUNIT_ASSERT(checkToken(token, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                          TokenId::noId(), getMandatoryStateDom()));

    /* Create a rejectable token. */
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", false, "sample2"));
    /* Find and verify it. */
    tokens = s_db->getTokens();
    CPPUNIT_ASSERT(tokens.size() == 2);
    TokenId token2 = *(tokens.begin());
    if (token2 == token) {
      tokens.erase(tokens.begin());
      token2 = *(tokens.begin());
    }
    CPPUNIT_ASSERT(checkToken(token2, LabelStr("TestClass2.Sample"),
			  LabelStr("TestClass2.Sample"),
			  TokenId::noId(), getRejectableStateDom()));

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
    CPPUNIT_ASSERT(oldTokens.size() + 1 == currentTokens.size());
    TokenId goal = *(currentTokens.begin());
    while (oldTokens.find(goal) != oldTokens.end()) {
      currentTokens.erase(currentTokens.begin());
      goal = *(currentTokens.begin());
    }
    CPPUNIT_ASSERT(checkToken(goal, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                          TokenId::noId(), getMandatoryStateDom()));
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
      CPPUNIT_ASSERT(oldTokens.size() + 1 == currentTokens.size());
      TokenId subgoal = *(currentTokens.begin());
      while (oldTokens.find(subgoal) != oldTokens.end()) {
        currentTokens.erase(currentTokens.begin());
        subgoal = *(currentTokens.begin());
      }
      /* Check it. */
      //!!Should use the master token's Id rather than TokenId::noId() here, but the player doesn't behave that way.
      //!!Is that a bug in the player or not?
      //!!  May mean that this is an inappropriate overloading of the '<goal>' XML tag per Tania and I (17 Nov 2004)
      CPPUNIT_ASSERT(checkToken(subgoal, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                            TokenId::noId(), getMandatoryStateDom()));
      CPPUNIT_ASSERT(verifyTokenRelation(goal, subgoal, *which));
      /* Update list of old tokens. */
      oldTokens.insert(subgoal);
    }
  }

  static void testDeleteTokens() {
    TokenSet::size_type oldTokenCount = s_db->getTokens().size();
    std::stringstream transactions;
    transactions << buildXMLCreateGoalStr("TestClass2.Sample", true, "sample1");
    for(std::set<LabelStr>::const_iterator it = s_tempRels.begin(); it != s_tempRels.end();
	++it) {
      std::string subgoalName = "subgoal1" + it->toString();
      transactions << buildXMLCreateSubgoalStr("sample1", "TestClass2.Sample", subgoalName,
					       *it);
    }
    TEST_REWINDING_XML(transactions.str());
    TokenSet::size_type newTokenCount = s_db->getTokens().size();
    CPPUNIT_ASSERT(newTokenCount < oldTokenCount);
    CPPUNIT_ASSERT((oldTokenCount - newTokenCount) == (TokenSet::size_type)(s_tempRels.size() + 1));

    TEST_PLAYING_XML(transactions.str());
    CPPUNIT_ASSERT(oldTokenCount == s_db->getTokens().size());
  }

  static void testUndeleteTokens() {
  }

  /**
   * Test constraining tokens to an object and ordering tokens on an object.
   */
  static void testConstrain() {
    /* Get an existing object.  See testCreateObject(). */
    ObjectId obj2b = s_db->getObject("testObj2b");
    CPPUNIT_ASSERT(!obj2b.isNoId() && obj2b.isValid());
    CPPUNIT_ASSERT(obj2b->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2b->getName() == LabelStr("testObj2b"));
    const unsigned int initialObjectTokenCount_B = obj2b->tokens().size();

    TokenId constrainedToken = createToken("constrainedSample", true);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "constrainedSample", ""));
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": constrainedToken is " << constrainedToken);
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": constrainedSample's derived object domain is " << constrainedToken->getObject()->derivedDomain());
    CPPUNIT_ASSERT_MESSAGE("token already constrained to one object",
        !constrainedToken->getObject()->derivedDomain().isSingleton());
    /* Create the constraint. */
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2b", "constrainedSample", ""));
    /* Verify its intended effect. */
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": constrainedSample's derived object domain is " << constrainedToken->getObject()->derivedDomain());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to one object",
        constrainedToken->getObject()->derivedDomain().isSingleton());
    ObjectDomain objDom2b(GET_DATA_TYPE(s_db,"TestClass2"),obj2b);
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to expected object",
        constrainedToken->getObject()->derivedDomain() == objDom2b);
    /* Leave it in plan db for testFree(). */

    /* Again, but also constrain it with the prior token. */
    TokenId constrained2 = createToken("constrainedSample2", true);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "constrainedSample2", ""));
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": constrained2 is " << constrained2);
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain());
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain());
    CPPUNIT_ASSERT_MESSAGE("token already constrained to one object",
        !constrained2->getObject()->derivedDomain().isSingleton());
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2b", "constrainedSample2", "constrainedSample"));
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": constrained2's derived object domain is " << constrained2->getObject()->derivedDomain());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to one object",
        constrained2->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to expected object",
        constrained2->getObject()->derivedDomain() == objDom2b);
    CPPUNIT_ASSERT(verifyTokenRelation(constrainedToken, constrained2, "before")); //!! "precedes" ?
    CPPUNIT_ASSERT(obj2b->tokens().size() == initialObjectTokenCount_B + 2);
    /* Leave them in plan db for testFree(). */

    /* Create two rejectable tokens and do the same tests, but with testObj2a. */
    ObjectId obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(!obj2a.isNoId() && obj2a.isValid());
    CPPUNIT_ASSERT(obj2a->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2a->getName() == LabelStr("testObj2a"));
    ObjectDomain objDom2a(GET_DATA_TYPE(s_db,"TestClass2"),obj2a);
    const unsigned int initialObjectTokenCount_A = obj2a->tokens().size();

    TokenId rejectable = createToken("rejectableConstrainedSample", false);
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": rejectable is " << rejectable);
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain());
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "rejectableConstrainedSample", ""));
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain());
    CPPUNIT_ASSERT_MESSAGE("token already constrained to one object",
        !rejectable->getObject()->derivedDomain().isSingleton());
    TEST_PLAYING_XML(buildXMLObjTokTokStr("constrain", "testObj2a", "rejectableConstrainedSample", ""));
    debugMsg("testConstraint", __FILE__ << ':' << __LINE__ << ": rejectable's derived object domain is " << rejectable->getObject()->derivedDomain());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to one object",
        rejectable->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to expected object",
        rejectable->getObject()->derivedDomain() == objDom2a);

    TokenId rejectable2 = createToken("rejectable2", false);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("activate", "", "rejectable2", ""));
    CPPUNIT_ASSERT_MESSAGE("token already constrained to one object",
        !rejectable2->getObject()->derivedDomain().isSingleton());
    std::string transaction =
      buildXMLObjTokTokStr("constrain", "testObj2a", "rejectable2",
			   "rejectableConstrainedSample");
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to one object",
        rejectable2->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to expected object",
        rejectable2->getObject()->derivedDomain() == objDom2a);
    CPPUNIT_ASSERT(verifyTokenRelation(rejectable, rejectable2, "before")); //!! "precedes" ?
    CPPUNIT_ASSERT(obj2a->tokens().size() == initialObjectTokenCount_A + 2);
    /* Leave them in plan db for testFree(). */

    TEST_REWINDING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to one object",
        rejectable->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to expected object",
        rejectable->getObject()->derivedDomain() == objDom2a);
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to one object",
        !rejectable2->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to expected object",
        rejectable2->getObject()->derivedDomain() != objDom2a);
    CPPUNIT_ASSERT(obj2a->tokens().size() == initialObjectTokenCount_A + 2);

    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to one object",
        rejectable2->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT_MESSAGE("player did not constrain token to expected object",
        rejectable2->getObject()->derivedDomain() == objDom2a);
    CPPUNIT_ASSERT(verifyTokenRelation(rejectable, rejectable2, "before")); //!! "precedes" ?
    CPPUNIT_ASSERT(obj2a->tokens().size() == initialObjectTokenCount_A + 2);
  }

  /** Test freeing tokens. */
  static void testFree() {
    ObjectId obj2a = s_db->getObject("testObj2a");
    CPPUNIT_ASSERT(!obj2a.isNoId() && obj2a.isValid());
    CPPUNIT_ASSERT(obj2a->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2a->getName() == LabelStr("testObj2a"));
    ObjectDomain objDom2a(GET_DATA_TYPE(s_db,"TestClass2"),obj2a);
    const unsigned int initialObjectTokenCount_A = obj2a->tokens().size();

    ObjectId obj2b = s_db->getObject("testObj2b");
    CPPUNIT_ASSERT(!obj2b.isNoId() && obj2b.isValid());
    CPPUNIT_ASSERT(obj2b->getType() == LabelStr("TestClass2"));
    CPPUNIT_ASSERT(obj2b->getName() == LabelStr("testObj2b"));
    ObjectDomain objDom2b(GET_DATA_TYPE(s_db,"TestClass2"),obj2b);
    TokenSet tokens = obj2b->tokens();
    debugMsg("testFree", __FILE__ << ':' << __LINE__ << ": there are " << tokens.size() << " tokens on testObj2b; should be 2.");
    /*!!For debugging:
    TokenSet tokens2 = tokens;
    for (int i = 1; !tokens2.empty(); i++) {
      TokenId tok = *(tokens2.begin());
      CPPUNIT_ASSERT(tok.isValid());
      std::cout << __FILE__ << ':' << __LINE__ << ": testFree(): on obj2b, token " << i << " is " << *(tokens2.begin()) << '\n';
      tokens2.erase(tokens2.begin());
    }
    !!*/
    TokenId one = *(tokens.begin());
    CPPUNIT_ASSERT(one.isValid());
    CPPUNIT_ASSERT(one->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT(one->getObject()->derivedDomain() == objDom2b);
    TokenId two = *(tokens.rbegin());
    CPPUNIT_ASSERT(two.isValid() && one != two);
    CPPUNIT_ASSERT(two->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT(two->getObject()->derivedDomain() == objDom2b);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample2", "constrainedSample"));
    CPPUNIT_ASSERT(one->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT(one->getObject()->derivedDomain() == objDom2b);

    //!!Next fails because the base domain is still open
    //!!CPPUNIT_ASSERT(one->getObject()->derivedDomain() == ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")));

    CPPUNIT_ASSERT(!two->getObject()->derivedDomain().isSingleton());
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample", ""));
    CPPUNIT_ASSERT(!two->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!CPPUNIT_ASSERT(one->getObject()->derivedDomain() == ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")));

    CPPUNIT_ASSERT(!one->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!CPPUNIT_ASSERT(two->getObject()->derivedDomain() == ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")));

    tokens = obj2a->tokens();
    /*!!For debugging:
    TokenSet tokens3 = tokens;
    for (int i = 1; !tokens3.empty(); i++) {
      std::cout << __FILE__ << ':' << __LINE__ << ": testFree(): on obj2a, token " << i << " is " << *(tokens3.begin()) << '\n';
      tokens3.erase(tokens3.begin());
    }
    !!*/
    // This is correct because Object::tokens() returns tokens that _could_ go on the object,
    //   not just the tokens that _are_ on the object.
    CPPUNIT_ASSERT(tokens.size() == initialObjectTokenCount_A + 2);
    TokenId three, four;
    tokens.erase(one);
    tokens.erase(two);
    three = *(--tokens.rbegin());
    tokens.erase(three);
    four = *(tokens.rbegin());
    CPPUNIT_ASSERT(three->getObject()->derivedDomain().isSingleton());
    CPPUNIT_ASSERT(four->getObject()->derivedDomain() == objDom2a);
    TEST_PLAYING_XML(buildXMLObjTokTokStr("free", "testObj2a", "rejectableConstrainedSample", ""));
    CPPUNIT_ASSERT_MESSAGE("Should still be a singleton, since still required.",
        three->getObject()->derivedDomain().isSingleton());

    //!!Next fails because the base domain is still open
    //!!CPPUNIT_ASSERT(three->getObject()->derivedDomain() == ObjectDomain(GET_DATA_TYPE(s_db,"TestClass2")));
  }

  static void testUnfree() {
    std::stringstream transactions;
    transactions << buildXMLObjTokTokStr("constrain", "testObj2b", "constrainedSample", "");
    transactions << buildXMLObjTokTokStr("constrain", "testObj2b", "constrainedSample2",
					 "constrainedSample");
    transactions << buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample2",
					 "constrainedSample");
    transactions << buildXMLObjTokTokStr("free", "testObj2b", "constrainedSample", "");
    TEST_REWINDING_XML(transactions.str());
  }

  /** Test activating a token. */
  static void testActivate() {
    s_activatedToken = createToken("activateSample", false);
    debugMsg("testActivate", __FILE__ << ':' << __LINE__ << ": s_activatedToken is " << s_activatedToken);
    std::string transaction = buildXMLObjTokTokStr("activate", "", "activateSample", "");
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("token not activated by player", s_activatedToken->isActive());
    TEST_REWINDING_XML(transaction);
    CPPUNIT_ASSERT(s_activatedToken->isInactive());
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("token not activated by player", s_activatedToken->isActive());
    /* Leave activated for testMerge(). */
  }

  /** Test merging tokens. */
  static void testMerge() {
    s_mergedToken = createToken("mergeSample", true);
    debugMsg("testMerge", __FILE__ << ':' << __LINE__ << ": s_mergedToken is " << s_mergedToken);
    std::string transaction = buildXMLObjTokTokStr("merge", "", "mergeSample",
						   "activateSample");
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("token not merged by player", s_mergedToken->isMerged());
    TEST_REWINDING_XML(transaction);
    CPPUNIT_ASSERT(s_mergedToken->isInactive());
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("token not merged by player", s_mergedToken->isMerged());
    /* Leave merged for testCancel(). */
  }

  /** Test rejecting a token. */
  static void testReject() {
    s_rejectedToken = createToken("rejectSample", false);
    debugMsg("testReject", __FILE__ << ':' << __LINE__ << ": s_rejectedToken is " << s_rejectedToken);
    std::string transaction = buildXMLObjTokTokStr("reject", "", "rejectSample", "");
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("token not rejected by player", s_rejectedToken->isRejected());
    TEST_REWINDING_XML(transaction);
    CPPUNIT_ASSERT(s_rejectedToken->isInactive());
    TEST_PLAYING_XML(transaction);
    CPPUNIT_ASSERT_MESSAGE("token not rejected by player", s_rejectedToken->isRejected());
    /* Leave rejected for testCancel(). */
  }

  /** Test cancelling tokens. */
  static void testCancel() {
    TEST_PLAYING_XML(buildXMLObjTokTokStr("cancel", "", "rejectSample", ""));
    CPPUNIT_ASSERT_MESSAGE("token not unrejected by player", !s_rejectedToken->isRejected());
    TEST_PLAYING_XML(buildXMLObjTokTokStr("cancel", "", "mergeSample", ""));
    CPPUNIT_ASSERT_MESSAGE("token not unmerged by player", !s_mergedToken->isMerged());
    TEST_PLAYING_XML(buildXMLObjTokTokStr("cancel", "", "activateSample", ""));
    CPPUNIT_ASSERT_MESSAGE("token not unactivated by player", !s_activatedToken->isActive());
  }

  static void testUncancel() {
    std::stringstream transactions;
    transactions << buildXMLObjTokTokStr("activate", "", "activateSample", "");
    transactions << buildXMLObjTokTokStr("merge", "", "mergeSample",
						   "activateSample");
    transactions << buildXMLObjTokTokStr("reject", "", "rejectSample", "");
    transactions << "<breakpoint/>";
    transactions << buildXMLObjTokTokStr("cancel", "", "rejectSample", "");
    transactions << buildXMLObjTokTokStr("cancel", "", "mergeSample", "");
    transactions << buildXMLObjTokTokStr("cancel", "", "activateSample", "");
    testRewindingXML(transactions.str(), __FILE__, __LINE__, true);
    CPPUNIT_ASSERT(s_rejectedToken->isRejected());
    CPPUNIT_ASSERT(s_mergedToken->isMerged());
    CPPUNIT_ASSERT(s_activatedToken->isActive());
    testCancel(); //just to clean up
  }

  static TokenId createToken(const LabelStr& name, bool mandatory) {
    /* Get the list of tokens so the new one can be identified. */
    TokenSet oldTokens = s_db->getTokens();
    /* Create the token using the player so its name will be recorded there. */
    TEST_PLAYING_XML(buildXMLCreateGoalStr("TestClass2.Sample", mandatory, name));
    /* Find it. */
    TokenSet currentTokens = s_db->getTokens();
    CPPUNIT_ASSERT(oldTokens.size() + 1 == currentTokens.size());
    TokenId tok = *(currentTokens.begin());
    while (oldTokens.find(tok) != oldTokens.end()) {
      currentTokens.erase(currentTokens.begin());
      tok = *(currentTokens.begin());
    }
    /* Check it. */
    CPPUNIT_ASSERT(checkToken(tok, LabelStr("TestClass2.Sample"), LabelStr("TestClass2.Sample"),
                          TokenId::noId(), mandatory ? getMandatoryStateDom() : getRejectableStateDom()));
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
   * @note I think that this should be part of the base API, not part of the test code.
   * --wedgingt@email.arc.nasa.gov 2004 Dec 13
   */
  static void getConstraintsFromRelations(const TokenId& master, const TokenId& slave, const LabelStr& relation,
                                          std::list<ConstrainedVariableId>& firsts,
                                          std::list<ConstrainedVariableId>& seconds,
                                          std::list<AbstractDomain*>& intervals) {
    CPPUNIT_ASSERT_MESSAGE("unknown temporal relation name given",
        s_tempRels.find(relation) != s_tempRels.end());
    if (relation == LabelStr("after")) {
      ADD_TR_DESC(slave->end(), master->start(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("any")) {
      // Not an actual constraint, so it would be incorrect to require one.
      return;
    }
    if (relation == LabelStr("before")) {
      ADD_TR_DESC(master->end(), slave->start(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contained_by")) {
      ADD_TR_DESC(slave->start(), master->start(), 0, PLUS_INFINITY);
      ADD_TR_DESC(master->end(), slave->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contains")) {
      ADD_TR_DESC(master->start(), slave->start(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->end(), master->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contains_end")) {
      ADD_TR_DESC(master->start(), slave->end(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->end(), master->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("contains_start")) {
      ADD_TR_DESC(master->start(), slave->start(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->end(), master->start(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends")) {
      ADD_TR_DESC(master->end(), slave->end(), 0, 0);
      return;
    }
    if (relation == LabelStr("ends_after")) {
      ADD_TR_DESC(slave->end(), master->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends_after_start")) {
      ADD_TR_DESC(slave->start(), master->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends_before")) {
      ADD_TR_DESC(master->end(), slave->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("ends_during")) {
      ADD_TR_DESC(slave->start(), master->end(), 0, PLUS_INFINITY);
      ADD_TR_DESC(master->end(), slave->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("equal") || relation == LabelStr("equals")) {
      ADD_TR_DESC(master->start(), slave->start(), 0, 0);
      ADD_TR_DESC(master->end(), slave->end(), 0, 0);
      return;
    }
    if (relation == LabelStr("meets")) {
      ADD_TR_DESC(master->end(), slave->start(), 0, 0);
      return;
    }
    if (relation == LabelStr("met_by")) {
      ADD_TR_DESC(slave->end(), master->start(), 0, 0);
      return;
    }
    if (relation == LabelStr("paralleled_by")) {
      ADD_TR_DESC(slave->start(), master->start(), 0, PLUS_INFINITY);
      ADD_TR_DESC(slave->end(), master->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("parallels")) {
      ADD_TR_DESC(master->start(), slave->start(), 0, PLUS_INFINITY);
      ADD_TR_DESC(master->end(), slave->end(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("starts")) {
      ADD_TR_DESC(master->start(), slave->start(), 0, 0);
      return;
    }
    if (relation == LabelStr("starts_after")) {
      ADD_TR_DESC(slave->start(), master->start(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("starts_before")) {
      ADD_TR_DESC(master->start(), slave->start(), 0, PLUS_INFINITY);
      return;
    }
    if (relation == LabelStr("starts_before_end")) {
      ADD_TR_DESC(master->start(), slave->end(), 0, PLUS_INFINITY);
      return;
    }
    CPPUNIT_ASSERT_MESSAGE("when a new temporal relation name was added, s_tempRels was updated but getConstraintsFromRelation() was not",
        relation == LabelStr("starts_during"));
    ADD_TR_DESC(slave->start(), master->start(), 0, PLUS_INFINITY);
    ADD_TR_DESC(master->start(), slave->end(), 0, PLUS_INFINITY);
    return;
  }

  static bool verifyTokenRelation(const TokenId& master, const TokenId& slave, const LabelStr& relation) {
    std::list<ConstrainedVariableId> firstVars;
    std::list<ConstrainedVariableId> secondVars;
    std::list<AbstractDomain*> intervals;
    // Get the appropriate list of timepoint pairs and bounds from the relation name.
    getConstraintsFromRelations(master, slave, relation, firstVars, secondVars, intervals);
    CPPUNIT_ASSERT(firstVars.size() == secondVars.size());
    CPPUNIT_ASSERT(firstVars.size() == intervals.size());
    while (!firstVars.empty()) {
      ConstrainedVariableId one = *(firstVars.begin());
      ConstrainedVariableId two = *(secondVars.begin());
      AbstractDomain *dom = *(intervals.begin());
      firstVars.erase(firstVars.begin());
      secondVars.erase(secondVars.begin());
      intervals.erase(intervals.begin());
      std::set<ConstraintId> oneConstraints, twoConstraints;
      one->constraints(oneConstraints);
      CPPUNIT_ASSERT(!oneConstraints.empty());
      two->constraints(twoConstraints);
      // Look for a constraint in both lists.
      for ( ; !twoConstraints.empty(); twoConstraints.erase(twoConstraints.begin())) {
        if (oneConstraints.find(*(twoConstraints.begin())) == oneConstraints.end())
          continue;
        // Got one.  Does it have other variables?
        ConstraintId both = *(twoConstraints.begin());
        if (both->getScope().size() > 2)
          continue; // Yes: can't be the one we're looking for.
        CPPUNIT_ASSERT(both->getScope().size() == 2);
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
                                             const std::vector<const AbstractDomain*>& args);

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
std::set<LabelStr> DbTransPlayerTest::s_tempRels;

/** Run a single test, reading the XML from the given string. */
void DbTransPlayerTest::testPlayingXML(const std::string& xml, const char *file, const int& line) {
  CPPUNIT_ASSERT(s_dbPlayer != 0);
  std::istringstream iss(xml);
  s_dbPlayer->play(iss);
}

void DbTransPlayerTest::testRewindingXML(const std::string& xml, const char* file, const int & line,
					 bool breakpoint) {
  CPPUNIT_ASSERT(s_dbPlayer != 0);
  std::istringstream iss(xml);
  s_dbPlayer->rewind(iss, breakpoint);
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
  CPPUNIT_ASSERT(it != entries.end());
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
  CPPUNIT_ASSERT(it != args.end());
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
CPPUNIT_ASSERT(line == l_line);
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
                                                       const std::vector<const AbstractDomain*>& domains) {
  CPPUNIT_ASSERT(!domains.empty());
  std::string str("<new type=\"");
  str += className;
  str += "\" name=\"";
  str += objName;
  str += "\"> <value ";
  str += " name=\"";
  str += objName;
  str += "\"";
  str += " type=\"";
  str += StringDT::NAME();
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
  if (var->parent().isNoId())
    str += var->getName().toString();
  else {
    if (TokenId::convertable(var->parent())) {
      //!!For token variables, the player's name for the token is needed: identifier="tokenName.varName"
      //!!  To implement this, a map of the ids to the names will have to be kept as they are created.
      CPPUNIT_ASSERT_MESSAGE("sorry: specifying variables of tokens in tests of <invoke> is presently unsupported", false);
    }
    CPPUNIT_ASSERT_MESSAGE("var's parent is neither token nor object", ObjectId::convertable(var->parent()));
    //!!I don't understand the details in DbClientTransactionPlayer.cc:parseVariable() well enough to figure this out yet
    //!!But here's a guess:
    str += var->parent()->getName();
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
  if (var->parent().isNoId()) {
    str += "id name =\"";
    str += var->getName().toString();
  } else {
    str += "variable index=\"";
    std::ostringstream oss;
    oss << var->getIndex() << "\" ";
    if (ObjectId::convertable(var->parent()))
      oss << "object=\"" << var->parent()->getName().toString();
    else {
      CPPUNIT_ASSERT_MESSAGE("unknown or unsupported (C++) type of parent of variable",
          TokenId::convertable(var->parent()));
      oss << "token=\"";
      TokenId token = var->parent();
      if (token->master().isNoId())
        oss << token->getKey();
      else {
        TokenId rootToken = token->master();
        while (!rootToken->master().isNoId())
          rootToken = rootToken->master();
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
  CPPUNIT_ASSERT_MESSAGE("domain is not singleton, interval, nor enumerated", dom.isEnumerated());
  str += "set> ";
  std::list<double> vals;
  for (dom.getValues(vals); !vals.empty(); vals.pop_front()) {
    str += "<";
    if (dom.isSymbolic()) {
      str += "symbol value=\"";
    } else {
      str += "value name=\"";
    }
    if (dom.isSymbolic())
      str += LabelStr(*(vals.begin())).toString();
    else {
      CPPUNIT_ASSERT_MESSAGE("sorry: only string, symbol, and real enumerations are supported",
          !dom.isInterval());
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

void PlanDatabaseModuleTests::schemaTest(void)
{
  SchemaTest::test();
}

void PlanDatabaseModuleTests::objectTest(void)
{
  ObjectTest::test();
}

void PlanDatabaseModuleTests::tokenTest(void)
{
  TokenTest::test();
}

void PlanDatabaseModuleTests::timelineTest(void)
{
  TimelineTest::test();
}

void PlanDatabaseModuleTests::DBClientTest(void)
{
  DbClientTest::test();
}

void PlanDatabaseModuleTests::DBTransPlayerTest(void)
{
  DbTransPlayerTest::test();
}

void PlanDatabaseModuleTests::cppSetup(void)
{
  setTestLoadLibraryPath(".");
}


