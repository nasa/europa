#include "re-test-module.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "RulesEngine.hh"
#include "Rule.hh"
#include "RuleInstance.hh"
#include "TestSupport.hh"
#include "TestRule.hh"
#include "Utils.hh"
#include "IntervalIntDomain.hh"
#include "DefaultPropagator.hh"
#include "StringDomain.hh"
#include "NumericDomain.hh"
#include "ProxyVariableRelation.hh"

#include "Constraints.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"

#include <iostream>
#include <string>

using namespace EUROPA;

class SimpleSubGoal: public Rule {
public:
  SimpleSubGoal()
      : Rule(LabelStr("AllObjects.Predicate"))
  {
  }

  RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb,
                                const RulesEngineId &rulesEngine) const{
    RuleInstanceId rootInstance = (new RootInstance(m_id, token, planDb))->getId();
    std::vector<ConstrainedVariableId> vars = rootInstance->getVariables("start:end:duration:object:state");
    CPPUNIT_ASSERT(vars.size() == 5);
    CPPUNIT_ASSERT(vars[0] == token->start());
    CPPUNIT_ASSERT(vars[4] == token->getState());
    rootInstance->setRulesEngine(rulesEngine);
    return rootInstance;
  }

private:
  class RootInstance: public RuleInstance{
  public:
    RootInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
      : RuleInstance(rule, token, planDb) {}

    void handleExecute(){
      m_onlySlave = addSlave(new IntervalToken(m_token, "met_by", LabelStr("AllObjects.Predicate")));
      addConstraint(LabelStr("eq"), makeScope(m_token->end(), m_onlySlave->start()));
    }

    TokenId m_onlySlave;
  };
};

class NestedGuards_0: public Rule {
public:
  NestedGuards_0();
  RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb,
                                const RulesEngineId &rulesEngine) const;
};

class NestedGuards_0_Root: public RuleInstance{
public:
  NestedGuards_0_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb);
  void handleExecute();
  TokenId m_onlySlave;
};

class NestedGuards_0_0: public RuleInstance{
public:
  NestedGuards_0_0(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, const AbstractDomain& domain);
  void handleExecute();
  TokenId m_onlySlave;
};

class NestedGuards_0_1: public RuleInstance{
public:
  NestedGuards_0_1(const RuleInstanceId& parentInstance, const std::vector<ConstrainedVariableId>& guards);
  void handleExecute();
  TokenId m_onlySlave;
};


NestedGuards_0::NestedGuards_0()
    : Rule(LabelStr("AllObjects.Predicate"))
{
}
RuleInstanceId NestedGuards_0::createInstance(const TokenId& token, const PlanDatabaseId& planDb,
                                              const RulesEngineId &rulesEngine) const{
  RuleInstanceId rootInstance = (new NestedGuards_0_Root(m_id, token, planDb))->getId();
  rootInstance->setRulesEngine(rulesEngine);
  return rootInstance;
}

NestedGuards_0_Root::NestedGuards_0_Root(const RuleId& rule, const TokenId& token,
                                         const PlanDatabaseId& planDb)
  : RuleInstance(rule, token, planDb, makeScope(token->getObject())) {}

void NestedGuards_0_Root::handleExecute(){
  m_onlySlave = addSlave(new IntervalToken(m_token, "met_by", LabelStr("AllObjects.Predicate")));
  addConstraint(LabelStr("eq"), makeScope(m_token->end(), m_onlySlave->start()));
  addChildRule(new NestedGuards_0_0(m_id, m_token->start(), IntervalIntDomain(8, 12))); /*!< Add child context with guards - start == 10 */
  addChildRule(new NestedGuards_0_1(m_id, makeScope(m_onlySlave->getObject()))); /*!< Add child context with guards - object set to singleton */
}

NestedGuards_0_0::NestedGuards_0_0(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, const AbstractDomain& domain)
  : RuleInstance(parentInstance, guard, domain){}

void NestedGuards_0_0::handleExecute(){
  m_onlySlave = addSlave(new IntervalToken(m_token, "met_by", LabelStr("AllObjects.Predicate")));
  addConstraint(LabelStr("eq"), makeScope(m_token->start(), m_onlySlave->end())); // Place before
}

NestedGuards_0_1::NestedGuards_0_1(const RuleInstanceId& parentInstance, const std::vector<ConstrainedVariableId>& guards)
  : RuleInstance(parentInstance, guards){}

void NestedGuards_0_1::handleExecute(){
  m_onlySlave = addSlave(new IntervalToken(m_token, "met_by",  LabelStr("AllObjects.Predicate")));
  addConstraint(LabelStr("eq"), makeScope(m_token->start(), m_onlySlave->end())); // Place before
}

class LocalVariableGuard_0: public Rule {
public:
  LocalVariableGuard_0();
  RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb,
                                const RulesEngineId &rulesEngine) const;
};

class LocalVariableGuard_0_Root: public RuleInstance{
public:
  LocalVariableGuard_0_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb);
  void handleExecute();
  static const ConstrainedVariableId& getGuard() {return s_guard;}
  static ConstrainedVariableId s_guard;
};

class LocalVariableGuard_0_0: public RuleInstance{
public:
  LocalVariableGuard_0_0(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, const AbstractDomain& domain)
    : RuleInstance(parentInstance, guard, domain){}
  void handleExecute();
};

ConstrainedVariableId LocalVariableGuard_0_Root::s_guard;

LocalVariableGuard_0::LocalVariableGuard_0()
    : Rule(LabelStr("AllObjects.Predicate"))
{
}

RuleInstanceId LocalVariableGuard_0::createInstance(const TokenId& token,
                                                    const PlanDatabaseId& planDb,
                                                    const RulesEngineId &rulesEngine) const{
  RuleInstanceId rootInstance = (new LocalVariableGuard_0_Root(m_id, token, planDb))->getId();
  rootInstance->setRulesEngine(rulesEngine);
  return rootInstance;
}

LocalVariableGuard_0_Root::LocalVariableGuard_0_Root(const RuleId& rule, const TokenId& token,
                                                     const PlanDatabaseId& planDb)
  : RuleInstance(rule, token, planDb){}

void LocalVariableGuard_0_Root::handleExecute(){
  // Add the guard
  StringDomain baseDomain("TestDomainType");
  baseDomain.insert(LabelStr("A"));
  baseDomain.insert(LabelStr("B"));
  baseDomain.insert(LabelStr("C"));
  baseDomain.insert(LabelStr("D"));
  baseDomain.insert(LabelStr("E"));
  baseDomain.close();
  ConstrainedVariableId guard = addVariable(baseDomain, true, LabelStr("b"));
  s_guard = guard; // To allow it to be set

  // Now allocate the guard domain.
  StringDomain guardDomain("TestDomainType");
  guardDomain.insert(LabelStr("B"));
  guardDomain.insert(LabelStr("C"));
  guardDomain.insert(LabelStr("E"));
  guardDomain.close();

  addChildRule(new LocalVariableGuard_0_0(m_id, guard, guardDomain));
}

void LocalVariableGuard_0_0::handleExecute(){
  addSlave(new IntervalToken(m_token, "any", LabelStr("AllObjects.Predicate")));
}

class RETestEngine : public EngineBase
{
  public:
    RETestEngine();
    virtual ~RETestEngine();

    const ConstraintEngineId& getConstraintEngine() const;
    const SchemaId& getSchema() const;
    const PlanDatabaseId& getPlanDatabase() const;
    const RulesEngineId& getRulesEngine() const;

  protected:
    void createModules();
};

RETestEngine::RETestEngine()
{
    createModules();
    doStart();
    SchemaId sch = getSchema();
    sch->reset();
    sch->addObjectType(LabelStr("AllObjects"));
    sch->addObjectType(LabelStr("Objects"));
    sch->addMember(LabelStr("Objects"), IntervalIntDomain::getDefaultTypeName(), "m_int");
    sch->addPredicate(LabelStr("AllObjects.Predicate"));
    Object* objectPtr = new Object(getPlanDatabase(), "AllObjects", LabelStr("defaultObj"));
    assert(objectPtr != 0);
    Object& object = *objectPtr;
    assert(objectPtr->getId() == object.getId());
    CESchema* ces = (CESchema*)getComponent("CESchema");
    REGISTER_SYSTEM_CONSTRAINT(ces,EqualConstraint, "concurrent", "Default");
    REGISTER_SYSTEM_CONSTRAINT(ces,LessThanEqualConstraint, "precedes", "Default");
    REGISTER_SYSTEM_CONSTRAINT(ces,AddEqualConstraint, "temporaldistance", "Default");
    REGISTER_SYSTEM_CONSTRAINT(ces,AddEqualConstraint, "temporalDistance", "Default");
}

RETestEngine::~RETestEngine()
{
    doShutdown();
}

const ConstraintEngineId& RETestEngine::getConstraintEngine() const
{
    return ((ConstraintEngine*)getComponent("ConstraintEngine"))->getId();
}

const SchemaId& RETestEngine::getSchema() const
{
    return ((Schema*)getComponent("Schema"))->getId();
}

const PlanDatabaseId& RETestEngine::getPlanDatabase() const
{
    return ((PlanDatabase*)getComponent("PlanDatabase"))->getId();
}

const RulesEngineId& RETestEngine::getRulesEngine() const
{
    return ((RulesEngine*)getComponent("RulesEngine"))->getId();
}

void RETestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
    addModule((new ModuleRulesEngine())->getId());
}

ConstraintEngineId ce;
SchemaId schema;
PlanDatabaseId db;
RulesEngineId re;

#define RE_DEFAULT_SETUP(ce, db, autoClose) \
    RETestEngine testEngine; \
    ce = testEngine.getConstraintEngine(); \
    schema = testEngine.getSchema(); \
    db = testEngine.getPlanDatabase(); \
    re = testEngine.getRulesEngine(); \
    if (autoClose) \
      db->close();

#define RE_DEFAULT_TEARDOWN()

class RulesEngineTest {
public:
  static bool test(){
    EUROPA_runTest(testSimpleSubGoal);
    EUROPA_runTest(testNestedGuards);
    EUROPA_runTest(testLocalVariable);
    EUROPA_runTest(testTestRule);
    EUROPA_runTest(testPurge);
    EUROPA_runTest(testGNATS_3157);
    EUROPA_runTest(testProxyVariableRelation);
    return true;
  }
private:

  static bool testSimpleSubGoal(){
    RE_DEFAULT_SETUP(ce, db, false);
    db->close();

    re->getRuleSchema()->registerRule((new SimpleSubGoal())->getId());
    // Create a token of an expected type

    IntervalToken t0(db,
		     LabelStr("AllObjects.Predicate"),
		     true,
		     false,
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));
    // Activate it and confirm we are getting a subgoal and that the expected constraint holds.
    CPPUNIT_ASSERT(t0.slaves().empty());
    t0.activate();
    CPPUNIT_ASSERT(db->getTokens().size() == 2);
    CPPUNIT_ASSERT(t0.slaves().size() == 1);

    TokenId slaveToken = *(t0.slaves().begin());
    CPPUNIT_ASSERT(t0.end()->getDerivedDomain() == slaveToken->start()->getDerivedDomain());

    RE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testNestedGuards(){
    RE_DEFAULT_SETUP(ce, db, false);
    Object o1(db, LabelStr("AllObjects"), LabelStr("o1"));
    Object o2(db, LabelStr("AllObjects"), LabelStr("o2"));
    db->close();

    re->getRuleSchema()->registerRule((new NestedGuards_0())->getId());
    // Create a token of an expected type

    IntervalToken t0(db,
		     LabelStr("AllObjects.Predicate"),
		     true,
		     false,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));
    // Activate it and confirm we are getting a subgoal and that the expected constraint holds.
    CPPUNIT_ASSERT(t0.slaves().empty());
    t0.activate();
    CPPUNIT_ASSERT(db->getTokens().size() == 1);
    t0.getObject()->specify(o1.getId());
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().size() == 1);
    CPPUNIT_ASSERT(db->getTokens().size() == 2);

    TokenId slaveToken = *(t0.slaves().begin());

    // Set start time to 10 will trigger another guard
    t0.start()->specify(10); // Will trigger nested guard
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().size() == 2);

    // Now set the object variable of the slaveToken to trigger additional guard
    slaveToken->getObject()->specify(o2.getId());
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().size() == 3);

    // Now retract a decision and confirm the slave is removed
    t0.start()->reset();
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().size() == 2);

    // Now deactivate the master token and confirm all salves are gone
    t0.cancel();
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().empty());
    RE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testLocalVariable(){
    RE_DEFAULT_SETUP(ce, db, false);
    db->close();

    re->getRuleSchema()->registerRule((new LocalVariableGuard_0())->getId());

    IntervalToken t0(db,
		     LabelStr("AllObjects.Predicate"),
		     true,
		     false,
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));
    // Activate it and confirm we are not sub-goaling yet
    ConstrainedVariableId guard = LocalVariableGuard_0_Root::getGuard();
    CPPUNIT_ASSERT(guard.isNoId());

    t0.activate();
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().empty());

    guard = LocalVariableGuard_0_Root::getGuard();
    CPPUNIT_ASSERT(guard.isValid());
    guard->specify(LabelStr("A")); // Should not succeed
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().empty());

    guard->reset(); // Reset and try correct value
    guard->specify(LabelStr("B")); // Should succeed
    ce->propagate();
    CPPUNIT_ASSERT(t0.slaves().size() == 1);

    RE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTestRule(){
    RE_DEFAULT_SETUP(ce, db, false);
    db->close();

    re->getRuleSchema()->registerRule((new TestRule(LabelStr("AllObjects.Predicate")))->getId());

    IntervalToken t0(db,
		     LabelStr("AllObjects.Predicate"),
		     true,
		     false,
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));

    t0.getObject()->specify(t0.getObject()->lastDomain().getSingletonValue());

    /* Force first level of execution based on object variable being specified to a singleton on activation.
       second level of execution should also occur, since by default, the local guard base domain
       is a singleton. Note that this addresses a case for GNATS_ */
    t0.activate();
    ce->propagate();
    CPPUNIT_ASSERT_MESSAGE(toString(t0.slaves().size()), t0.slaves().size() == 2);

    RE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testPurge(){
    RE_DEFAULT_SETUP(ce, db, false);
    db->close();

    RuleSchema rs;
    rs.registerRule((new TestRule(LabelStr("AllObjects.Predicate")))->getId());
    rs.purgeAll();

    RE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testGNATS_3157(){
    RE_DEFAULT_SETUP(ce, db, false);
    db->close();

    re->getRuleSchema()->registerRule((new SimpleSubGoal())->getId());

    // Case where we have a master's rule that remains even though slaves and constraints are removed.
    {
      // Create a token of an expected type
      IntervalToken t0(db,
		       LabelStr("AllObjects.Predicate"),
		       true,
		       false,
		       IntervalIntDomain(0, 1000),
		       IntervalIntDomain(0, 1000),
		       IntervalIntDomain(1, 1000));
      // Activate it and confirm we are getting a subgoal and that the expected constraint holds.
      CPPUNIT_ASSERT(t0.slaves().empty());
      t0.activate();
      CPPUNIT_ASSERT(db->getTokens().size() == 2);
      CPPUNIT_ASSERT(t0.slaves().size() == 1);

      TokenId slaveToken = *(t0.slaves().begin());
      CPPUNIT_ASSERT(t0.end()->getDerivedDomain() == slaveToken->start()->getDerivedDomain());

      t0.commit();
      delete (Token*) slaveToken;
      Entity::garbageCollect();
    }

    // Case now where the slave remains and master is deleted. Make sure we disconnext dependents
    {
      TokenId slaveToken;
      {
	// Create a token of an expected type
	IntervalToken t0(db,
			 LabelStr("AllObjects.Predicate"),
			 true,
			 false,
			 IntervalIntDomain(0, 1000),
			 IntervalIntDomain(0, 1000),
			 IntervalIntDomain(1, 1000));
	// Activate it and confirm we are getting a subgoal and that the expected constraint holds.
	CPPUNIT_ASSERT(t0.slaves().empty());
	t0.activate();
	CPPUNIT_ASSERT(db->getTokens().size() == 2);
	CPPUNIT_ASSERT(t0.slaves().size() == 1);

	slaveToken = *(t0.slaves().begin());
	CPPUNIT_ASSERT(t0.end()->getDerivedDomain() == slaveToken->start()->getDerivedDomain());

	slaveToken->activate();
	slaveToken->commit();
      }

      Entity::garbageCollect();
      delete (Token*) slaveToken;
    }

    RE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testProxyVariableRelation(){
    RE_DEFAULT_SETUP(ce, db, false);
    Object obj0(db, "Objects", "obj0", true);
    CPPUNIT_ASSERT(!obj0.isComplete());
    obj0.addVariable(IntervalIntDomain(0, 0), "m_int");
    obj0.close();
    Object obj1(db, "Objects", "obj1", true);
    CPPUNIT_ASSERT(!obj1.isComplete());
    obj1.addVariable(IntervalIntDomain(1, 1), "m_int");
    obj1.close();
    Object obj2(db, "Objects", "obj2", true);
    CPPUNIT_ASSERT(!obj2.isComplete());
    obj2.addVariable(IntervalIntDomain(2, 2), "m_int");
    obj2.close();

    ObjectDomain emptyDomain("Objects");

    // Allocate an object variable with an empty domain
    Variable<ObjectDomain> objVar(ce, emptyDomain);

    // populate the domain, leaving it open
    db->makeObjectVariableFromType("Objects", objVar.getId(), true);
    CPPUNIT_ASSERT_MESSAGE(objVar.toString(), objVar.lastDomain().getSize() == 3);

    // Create the initial proxy variable
    NumericDomain dom(IntervalIntDomain::getDefaultTypeName().c_str());
    dom.insert(0);
    dom.insert(1);
    dom.insert(2);
    Variable<NumericDomain> proxyVar(ce, dom);
    CPPUNIT_ASSERT(!proxyVar.isClosed());

    // Allocate the constraint
    std::vector<unsigned int> path;
    path.push_back(0);
    ProxyVariableRelation c(objVar.getId(), proxyVar.getId(), path);

    CPPUNIT_ASSERT(ce->propagate());

    // Specify the proxy and ensure the object variable is propagated
    proxyVar.specify(1);
    CPPUNIT_ASSERT(ce->propagate());
    CPPUNIT_ASSERT(objVar.lastDomain().isSingleton());
    CPPUNIT_ASSERT(objVar.lastDomain().getSingletonValue() == obj1.getId());

    // Reset and ensure things go back to normal
    proxyVar.reset();
    ce->propagate();
    CPPUNIT_ASSERT_MESSAGE(objVar.toString(), objVar.lastDomain().getSize() == 3);

    // Specify the object var and ensure the proxy var also becomes specified
    objVar.specify(obj2.getId());
    CPPUNIT_ASSERT(ce->propagate());
    CPPUNIT_ASSERT(proxyVar.isSpecified());

    // Reset and ensure things go back to normal
    objVar.reset();
    ce->propagate();
    CPPUNIT_ASSERT(!proxyVar.isSpecified());

    // First set the proxy, then set the object. Retract the proxy but ensure it is not reset
    proxyVar.specify(1);
    objVar.specify(obj1.getId());
    proxyVar.reset();
    CPPUNIT_ASSERT(proxyVar.isSpecified());

    // Now reset the object var also, and ensure all is back to normal
    objVar.reset();
    CPPUNIT_ASSERT(!proxyVar.isSpecified());

    // specify both such that there is an inconsistency
    proxyVar.specify(2);
    objVar.specify(obj1.getId());
    CPPUNIT_ASSERT(!ce->propagate());

    // Back off an fix it
    proxyVar.reset();
    CPPUNIT_ASSERT(ce->propagate());

    return true;
  }
};

/*void RulesEngineModuleTests::runTests(std::string path)
{
    setTestLoadLibraryPath(path);
    runTestSuite(RulesEngineTest::test);
    std::cout << "Finished" << std::endl;
}*/

void RulesEngineModuleTests::cppTest()
{
  setTestLoadLibraryPath(".");
}

void RulesEngineModuleTests::rulesEngineTests()
{
  RulesEngineTest::test();
}

