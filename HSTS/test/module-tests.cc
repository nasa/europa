#include "HSTSModuleTests.hh"
#include "CBPlannerDefs.hh"
#include "DMLogger.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "Utils.hh"
#include "TestSupport.hh"
#include "HSTSNoBranch.hh"
#include "HSTSNoBranchCondition.hh"
#include "HSTSPlanIdReader.hh"

#include "DefaultPropagator.hh"
#include "ConstraintEngine.hh"
#include "Constraint.hh"
#include "CeLogger.hh"

#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "DbLogger.hh"

#include "RulesEngine.hh"
#include "Rule.hh"

#include "DNPConstraints.hh"

#include "test/ConstraintTesting.hh"

#include <iostream>
#include <string>

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0,200); \
    DecisionManager dm(db.getId()); \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce.getId()); \
      dbLId = (new DbLogger(std::cout, db.getId()))->getId(); \
      new DMLogger(std::cout, dm.getId()); \
    } \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN()			\
  delete (DbLogger*) dbLId;

#define DEFAULT_SETUP_PLAN(ce, db, autoClose)			\
  ConstraintEngine ce;						\
  initCBPTestSchema();						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  RulesEngine re(db.getId());					\
  Horizon hor(0, 200);						\
  CBPlanner planner(db.getId(), hor.getId());			\
  Id<DbLogger> dbLId;						\
  if (loggingEnabled()) {					\
    new CeLogger(std::cout, ce.getId());			\
    dbLId = (new DbLogger(std::cout, db.getId()))->getId();	\
    new DMLogger(std::cout, planner.getDecisionManager());	\
  }								\
  if (autoClose)						\
    db.close();

#define DEFAULT_TEARDOWN_PLAN()			\
  delete (DbLogger*) dbLId;

#define DEFAULT_SETUP_HEURISTICS()			\
  ConstraintEngine ce;					\
  initCBPTestSchema();					\
  PlanDatabase db(ce.getId(), Schema::instance());	\
  HSTSHeuristics heuristics(db.getId()); 

#define DEFAULT_TEARDOWN_HEURISTICS()

#define DEFAULT_SETUP_PLAN_HEURISTICS()				\
  ConstraintEngine ce;						\
  initCBPTestSchema();						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  RulesEngine re(db.getId());					\
  Horizon hor(0, 200);						\
  CBPlanner planner(db.getId(), hor.getId());			\
  Id<DbLogger> dbLId;						\
  if (loggingEnabled()) {					\
    new CeLogger(std::cout, ce.getId());			\
    dbLId = (new DbLogger(std::cout, db.getId()))->getId();	\
    new DMLogger(std::cout, planner.getDecisionManager());	\
  }								\
  HSTSHeuristics heuristics(db.getId()); 

#define DEFAULT_TEARDOWN_PLAN_HEURISTICS()	\
  delete (DbLogger*) dbLId;


class ConditionTest {
public:
  static bool test() {
    runTest(testHSTSNoBranchCondition);
    return(true);
  }
private:
  static bool testHSTSNoBranchCondition() {
    bool retval = false;
    DEFAULT_SETUP(ce,db,false);
    retval = testHSTSNoBranchConditionImpl(ce,db,dm);
    DEFAULT_TEARDOWN();
    return retval;
  }

};

class HeuristicsTest {
public:
  static bool test() {
    runTest(testTokenType);
    runTest(testDefaultInitialization);
    runTest(testTokenInitialization);
    runTest(testVariableInitialization);
    runTest(testReader);
    runTest(testHSTSPlanIdReader);
    runTest(testHSTSNoBranch);
    runTest(testHSTSHeuristicsAssembly);
    runTest(testDNPConstraints);
    return(true);
  }
private:
  static bool testTokenType() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testTokenTypeImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testDefaultInitialization() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testDefaultInitializationImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testTokenInitialization() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testTokenInitializationImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testVariableInitialization() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testVariableInitializationImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testReader() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testReaderImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testHSTSPlanIdReader() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testHSTSPlanIdReaderImpl();
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testHSTSNoBranch() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce,db,true);
    retval = testHSTSNoBranchImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
  static bool testHSTSHeuristicsAssembly() {
    bool retval = false;
    DEFAULT_SETUP_PLAN_HEURISTICS();
    retval = testHSTSHeuristicsAssemblyImpl(ce, db, planner, heuristics);
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return retval;
  }

  /**
   * Test the DNP specific constraint functions.
   * @note Almost a copy of ConstraintEngine/test/module-tests.cc's testArbitraryCosntraints().
   */
  static bool testDNPConstraints() {
    DEFAULT_SETUP_PLAN_HEURISTICS();
    std::list<ConstraintTestCase> tests;
    assertTrue(readTestCases(std::string("DNPTestCases"), tests) ||
               readTestCases(std::string("HSTS/test/DNPTestCases"), tests));
    assertTrue(executeTestCases(ce.getId(), tests));
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return(true);
  }
};

int main() {
  Schema::instance();

  //!!initConstraintEngine(); // Needed ?
  //!!initConstraintLibrary(); // Needed ?  May interfere with some of the later register constraint calls.

  REGISTER_CONSTRAINT(EqualConstraint, "concurrent", "Default");
  REGISTER_CONSTRAINT(LessThanEqualConstraint, "precedes", "Default");
  REGISTER_CONSTRAINT(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
  REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
  REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
  REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");

  // Register the DNP constraint functions.
  REGISTER_CONSTRAINT(BOUNDS_PLAYBACK_START_STORAGE, "BOUNDS_PLAYBACK_START_STORAGE", "Default");
  REGISTER_CONSTRAINT(BOUNDS_PLAYBACK_END_STORAGE, "BOUNDS_PLAYBACK_END_STORAGE", "Default");
  REGISTER_CONSTRAINT(BOUNDS_RECORD_END_STORAGE, "BOUNDS_RECORD_END_STORAGE", "Default");
  REGISTER_CONSTRAINT(BOUNDS_RECORD_START_STORAGE, "BOUNDS_RECORD_START_STORAGE", "Default");
  REGISTER_CONSTRAINT(COMPUTE_PLAYBACK_DURATION, "COMPUTE_PLAYBACK_DURATION", "Default");
  REGISTER_CONSTRAINT(FIGURE_EARLIER_OP_IDS, "FIGURE_EARLIER_OP_IDS", "Default");

  //!!Add calls to readTestCases(), etc., from ConstraintEngine/test/module-tests.cc

  for (int i = 0; i < 1; i++) {
    runTestSuite(ConditionTest::test);
    runTestSuite(HeuristicsTest::test);
  }
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
}
