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
#include "ObjectFilter.hh"
#include "DbLogger.hh"

#include "TestSupport.hh"
#include "TestRule.hh"
#include "Utils.hh"
#include "IntervalIntDomain.hh"
#include "TokenTemporalVariable.hh"
#include "Domain.hh"
#include "DefaultPropagator.hh"
#include "EqualityConstraintPropagator.hh"

#include <iostream>
#include <string>

#ifdef __sun
#include <strstream>
typedef std::strstream sstream;
#else
#include <sstream>
typedef std::stringstream sstream;
#endif


class SimpleSubGoal: public Rule {
public:
  SimpleSubGoal(): Rule(LabelStr("Predicate")){}

  RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb) const{
    RuleInstanceId rootInstance = (new RootInstance(m_id, token, planDb))->getId();
    return rootInstance;
  }

private:
  class RootInstance: public RuleInstance{
  public:
    RootInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
      : RuleInstance(rule, token, planDb) {}

    void handleExecute(){
      m_onlySlave = addSlave(new IntervalToken(m_token,  LabelStr("Predicate")));
      addConstraint(LabelStr("eq"), makeScope(m_token->getEnd(), m_onlySlave->getStart()));
    }

    TokenId m_onlySlave;
  };
};

class NestedGuards_0: public Rule {
public:
  NestedGuards_0();
  RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb) const;
};

class NestedGuards_0_Root: public RuleInstance{
public:
  NestedGuards_0_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb);
  void handleExecute();
  TokenId m_onlySlave;
};

class NestedGuards_0_0: public RuleInstance{
public:
  NestedGuards_0_0(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, double value);
  void handleExecute();
  TokenId m_onlySlave;
};

class NestedGuards_0_1: public RuleInstance{
public:
  NestedGuards_0_1(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard);
  void handleExecute();
  TokenId m_onlySlave;
};


NestedGuards_0::NestedGuards_0(): Rule(LabelStr("Predicate")){}
RuleInstanceId NestedGuards_0::createInstance(const TokenId& token, const PlanDatabaseId& planDb) const{
  RuleInstanceId rootInstance = (new NestedGuards_0_Root(m_id, token, planDb))->getId();
  return rootInstance;
}

NestedGuards_0_Root::NestedGuards_0_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
  : RuleInstance(rule, token, planDb, token->getObject()) {
}

void NestedGuards_0_Root::handleExecute(){
  m_onlySlave = addSlave(new IntervalToken(m_token,  LabelStr("Predicate")));
  addConstraint(LabelStr("eq"), makeScope(m_token->getEnd(), m_onlySlave->getStart()));
  addChildRule(new NestedGuards_0_0(m_id, m_token->getStart(), 10)); /*!< Add child context with guards - start == 10 */
  addChildRule(new NestedGuards_0_1(m_id, m_onlySlave->getObject())); /*!< Add child context with guards - object set to singleton */
}

NestedGuards_0_0::NestedGuards_0_0(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, double value)
  : RuleInstance(parentInstance, guard, value){}

void NestedGuards_0_0::handleExecute(){
  m_onlySlave = addSlave(new IntervalToken(m_token,  LabelStr("Predicate")));
  addConstraint(LabelStr("eq"), makeScope(m_token->getStart(), m_onlySlave->getEnd())); // Place before
}

NestedGuards_0_1::NestedGuards_0_1(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard)
  : RuleInstance(parentInstance, guard){}

void NestedGuards_0_1::handleExecute(){
  m_onlySlave = addSlave(new IntervalToken(m_token,  LabelStr("Predicate")));
  addConstraint(LabelStr("eq"), makeScope(m_token->getStart(), m_onlySlave->getEnd())); // Place before
}

class LocalVariableGuard_0: public Rule {
public:
  LocalVariableGuard_0();
  RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb) const;
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
  LocalVariableGuard_0_0(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, double value)
    : RuleInstance(parentInstance, guard, value){}
  void handleExecute();
};

ConstrainedVariableId LocalVariableGuard_0_Root::s_guard;

LocalVariableGuard_0::LocalVariableGuard_0(): Rule(LabelStr("Predicate")){}

RuleInstanceId LocalVariableGuard_0::createInstance(const TokenId& token, const PlanDatabaseId& planDb) const{
  RuleInstanceId rootInstance = (new LocalVariableGuard_0_Root(m_id, token, planDb))->getId();
  return rootInstance;
}

LocalVariableGuard_0_Root::LocalVariableGuard_0_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
  : RuleInstance(rule, token, planDb){}

void LocalVariableGuard_0_Root::handleExecute(){
  // Add the guard
  ConstrainedVariableId guard = addVariable(BoolDomain(), true, LabelStr("b"));
  s_guard = guard; // To allow it to be set
  addChildRule(new LocalVariableGuard_0_0(m_id, guard, true));
}

void LocalVariableGuard_0_0::handleExecute(){
  addSlave(new IntervalToken(m_token,  LabelStr("Predicate")));
}

class DefaultSchemaAccessor{
public:
  static const SchemaId& instance(){
    if (s_instance.isNoId()){
      s_instance = (new Schema())->getId();
    }

    return s_instance;
  }

  static void reset(){
    if(!s_instance.isNoId()){
      delete (Schema*) s_instance;
      s_instance = SchemaId::noId();
    }
  }

private:
  static SchemaId s_instance;
};

SchemaId DefaultSchemaAccessor::s_instance;

#define SCHEMA DefaultSchemaAccessor::instance()

#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce; \
    Schema schema; \
    PlanDatabase db(ce.getId(), schema.getId()); \
    { DefaultPropagator* dp = new DefaultPropagator(LabelStr("Default"), ce.getId()); \
      assert(dp != 0); \
    } \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce.getId()); \
      new DbLogger(std::cout, db.getId()); \
    } \
    RulesEngine re(db.getId()); \
    Object* objectPtr = new Object(db.getId(), LabelStr("AllObjects"), LabelStr("o1")); \
    assert(objectPtr != 0); \
    Object& object = *objectPtr; \
    assert(objectPtr->getId() == object.getId()); \
    if (autoClose) \
      db.close();

class RulesEngineTest {
public:
  static bool test(){
    runTest(testSimpleSubGoal);
    runTest(testNestedGuards);
    runTest(testLocalVariable);
    runTest(testTestRule);
    runTest(testPurge);
    return true;
  }
private:

  static bool testSimpleSubGoal(){
    DEFAULT_SETUP(ce, db, schema, false);
    db.close();

    SimpleSubGoal r;
    // Create a token of an expected type

    IntervalToken t0(db.getId(), 
		     LabelStr("Predicate"), 
		     true,
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));
    // Activate it and confirm we are getting a subgoal and that the expected constraint holds.
    check_error(t0.getSlaves().empty());
    t0.activate();
    check_error(db.getTokens().size() == 2);
    check_error(t0.getSlaves().size() == 1);

    TokenId slaveToken = *(t0.getSlaves().begin());
    check_error(t0.getEnd()->getDerivedDomain() == slaveToken->getStart()->getDerivedDomain());

    return true;
  }

  static bool testNestedGuards(){
    DEFAULT_SETUP(ce, db, schema, false);
    Object o2(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    NestedGuards_0 r;
    // Create a token of an expected type

    IntervalToken t0(db.getId(), 
		     LabelStr("Predicate"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));
    // Activate it and confirm we are getting a subgoal and that the expected constraint holds.
    check_error(t0.getSlaves().empty());
    t0.activate();
    check_error(db.getTokens().size() == 1);
    t0.getObject()->specify(object.getId());
    ce.propagate();
    check_error(t0.getSlaves().size() == 1);
    check_error(db.getTokens().size() == 2);

    TokenId slaveToken = *(t0.getSlaves().begin());

    // Set start time to 10 will trigger another guard
    t0.getStart()->specify(10); // Will trigger nested guard
    ce.propagate();
    check_error(t0.getSlaves().size() == 2);

    // Now set the object variable of the slaveToken to trigger additional guard
    slaveToken->getObject()->specify(o2.getId());
    ce.propagate();
    check_error(t0.getSlaves().size() == 3);

    // Now retract a decision and confirm the slave is removed
    t0.getStart()->reset();
    ce.propagate();
    check_error(t0.getSlaves().size() == 2);

    // Now deactivate the master token and confirm all salves are gone
    t0.cancel();
    ce.propagate();
    check_error(t0.getSlaves().empty());
    return true;
  }

  static bool testLocalVariable(){
    DEFAULT_SETUP(ce, db, schema, false);
    db.close();

    LocalVariableGuard_0 r;

    IntervalToken t0(db.getId(), 
		     LabelStr("Predicate"), 
		     true,
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));
    // Activate it and confirm we are not sub-goaling yet
    ConstrainedVariableId guard = LocalVariableGuard_0_Root::getGuard();
    check_error(guard.isNoId());

    t0.activate();
    ce.propagate();
    check_error(t0.getSlaves().empty());

    guard = LocalVariableGuard_0_Root::getGuard();
    check_error(guard.isValid());
    guard->specify(false); // Should not succeed
    ce.propagate();
    check_error(t0.getSlaves().empty());

    guard->reset(); // Reset and try correct value
    guard->specify(true); // Should succeed
    ce.propagate();
    check_error(t0.getSlaves().size() == 1);

    return true;
  }

  static bool testTestRule(){
    DEFAULT_SETUP(ce, db, schema, false);
    db.close();

    TestRule r(LabelStr("Predicate"));

    IntervalToken t0(db.getId(), 
		     LabelStr("Predicate"), 
		     true,
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));

    /* Force first level of execution based on object variable being specified to a singleton on activation.
       second level of execution should also occur through propagation, since by default, the local guard base domain
       is a singleton */

    t0.activate();
    ce.propagate();
    check_error(t0.getSlaves().size() == 2);

    return true;
  }

  static bool testPurge(){
    DEFAULT_SETUP(ce, db, schema, false);
    db.close();

    new TestRule(LabelStr("Predicate"));

    Rule::purgeAll();

    return true;
  }

};

int main() {
  initConstraintLibrary();
  
  // Special designations for temporal relations
  REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "precede", "Default");

  // Support for Token implementations
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");

  // This is now done in ConstraintEngine/test-support.cc::initConstraintLibrary()
  //   for ConstraintEngine/module-tests.cc::testArbitraryConstraints().
  // --wedgingt 2004 Mar 11
  //REGISTER_NARY(EqualConstraint, "eq", "Default");

  // Allocate default schema initially so tests don't fail because of ID's
  SCHEMA;
  runTestSuite(RulesEngineTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
}
