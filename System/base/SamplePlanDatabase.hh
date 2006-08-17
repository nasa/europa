#ifndef _H_SamplePlanDatabase
#define _H_SamplePlanDatabase

/**
 * @file   SamplePlanDatabase.hh
 * @author Andrew Bachmann
 * @date   Mon Dec 27 17:29:18 2004
 * @brief  
 * @ingroup System
 */

// Support for registered constraints
#include "ConstraintLibrary.hh"
#include "Constraints.hh"
#include "EqualityConstraintPropagator.hh"
#include "ObjectTokenRelation.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "PLASMAPerformanceConstraint.hh"
#include "LoraxConstraints.hh"

#include "LabelStr.hh"
#include "TestSupport.hh"

// Support fro required plan database components
#include "PlanDatabase.hh"
#include "Object.hh"
#include "ConstraintEngineDefs.hh"
#include "ConstraintEngine.hh"

// Rules Engine Components
#include "RulesEngine.hh"

// Access for registered event loggers for instrumentation
#include "PartialPlanWriter.hh"

#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ResourceDefs.hh"
#include "ResourcePropagator.hh"
#include "Transaction.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"

// Support for Temporal Network
#include "TemporalNetwork.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "TemporalConstraints.hh"

// Integration to NDDL
#include "NddlDefs.hh"

// For cleanup purging
#include "TokenFactory.hh"
#include "ObjectFactory.hh"
#include "Rule.hh"
#include "NddlDefs.hh"

// Support for Planner and Decision Management
#include "Horizon.hh"
#include "CBPlanner.hh"

#include <string>
#include <fstream>

#define PPW_WITH_PLANNER

using namespace EUROPA;

class SamplePlanDatabase {
public:
  ConstraintEngineId constraintEngine;
  PlanDatabaseId planDatabase;
  RulesEngineId rulesEngine;

  HorizonId horizon;
  CBPlannerId planner;

  PlanWriter::PartialPlanWriter* writer;
  DbClientTransactionLogId txLog;

  static inline bool &getAlreadyCalled() {
    static bool sl_alreadyCalled(false);
    return sl_alreadyCalled;
  }

  /**
   * @brief Sets up the necessary constraint factories
   */
  static void initialize() {
    if (!getAlreadyCalled()) {
      getAlreadyCalled() = true;

      // Initialize NDD
      initNDDL();

      REGISTER_CONSTRAINT(ConcurrentConstraint, "concurrent", "Temporal");
      REGISTER_CONSTRAINT(PrecedesConstraint, "precedes", "Temporal"); // Special case to get around NDDL reserved word
      REGISTER_CONSTRAINT(TemporalDistanceConstraint, "StartEndDurationRelation", "Temporal");
      REGISTER_CONSTRAINT(TemporalDistanceConstraint, "temporaldistance", "Temporal");

      REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
      REGISTER_CONSTRAINT(NotEqualConstraint, "neq", "Default");
      REGISTER_CONSTRAINT(LessThanEqualConstraint, "leq", "Default");
      REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
      REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
      REGISTER_CONSTRAINT(NegateConstraint, "neg", "Default");
      REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
      REGISTER_CONSTRAINT(LockConstraint, "Lock", "Default");
      REGISTER_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
      REGISTER_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");
      REGISTER_CONSTRAINT(ResourceConstraint, "ResourceRelation", "Resource");
      REGISTER_CONSTRAINT(ResourceConstraint, "ResourceTransactionRelation", "Default");
      REGISTER_CONSTRAINT(PLASMAPerformanceConstraint, "perf", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");

      /* Names used by some of the tests converted from NewPlan/ModuleTests/Parser. */
      REGISTER_CONSTRAINT(CondAllSameConstraint, "condeq", "Default");
      REGISTER_CONSTRAINT(EqualSumConstraint, "sum", "Default");

      // LoraxConstraints for some of the resources tests.
      REGISTER_CONSTRAINT(SquareOfDifferenceConstraint, "diffSquare", "Default");
      REGISTER_CONSTRAINT(DistanceFromSquaresConstraint, "distanceSquares", "Default");
      REGISTER_CONSTRAINT(DriveBatteryConstraint, "driveBattery", "Default");
      REGISTER_CONSTRAINT(DriveDurationConstraint, "driveDuration", "Default");
      REGISTER_CONSTRAINT(WindPowerConstraint, "windPower", "Default");
      REGISTER_CONSTRAINT(SampleBatteryConstraint, "sampleBattery", "Default");
      REGISTER_CONSTRAINT(SampleDurationConstraint, "sampleDuration", "Default");

      // Rest copied from ConstraintEngine/component/Constraints.cc's initConstraintLibrary().
      //   See also GNATS 2758.
      // Register constraint Factories
      REGISTER_CONSTRAINT(AddEqualConstraint, "AddEqual", "Default");
      REGISTER_CONSTRAINT(AddMultEqualConstraint, "AddMultEqual", "Default");
      REGISTER_CONSTRAINT(AllDiffConstraint, "AllDiff", "Default");
      REGISTER_CONSTRAINT(CardinalityConstraint, "Cardinality", "Default");
      REGISTER_CONSTRAINT(CondAllDiffConstraint, "CondAllDiff", "Default");
      REGISTER_CONSTRAINT(CondAllSameConstraint, "CondAllSame", "Default");
      REGISTER_CONSTRAINT(CondEqualSumConstraint, "CondEqualSum", "Default");
      REGISTER_CONSTRAINT(CountNonZerosConstraint, "CountNonZeros", "Default");
      REGISTER_CONSTRAINT(CountZerosConstraint, "CountZeros", "Default");
      REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
      REGISTER_CONSTRAINT(EqualMaximumConstraint, "EqualMaximum", "Default");
      REGISTER_CONSTRAINT(EqualMinimumConstraint, "EqualMinimum", "Default");
      REGISTER_CONSTRAINT(EqualProductConstraint, "EqualProduct", "Default");
      REGISTER_CONSTRAINT(EqualSumConstraint, "EqualSum", "Default");
      REGISTER_CONSTRAINT(GreaterThanSumConstraint, "GreaterThanSum", "Default");
      REGISTER_CONSTRAINT(GreaterOrEqThanSumConstraint, "GreaterOrEqThanSum", "Default");
      REGISTER_CONSTRAINT(LessOrEqThanSumConstraint, "LessOrEqThanSum", "Default");
      REGISTER_CONSTRAINT(LessThanConstraint, "LessThan", "Default");
      REGISTER_CONSTRAINT(LessThanEqualConstraint, "LessThanEqual", "Default");
      REGISTER_CONSTRAINT(LessThanSumConstraint, "LessThanSum", "Default");
      //REGISTER_CONSTRAINT(LockConstraint, "Lock", "Default");
      REGISTER_CONSTRAINT(MemberImplyConstraint, "MemberImply", "Default");
      REGISTER_CONSTRAINT(MultEqualConstraint, "MultEqual", "Default");
      REGISTER_CONSTRAINT(NotEqualConstraint, "NotEqual", "Default");
      REGISTER_CONSTRAINT(OrConstraint, "Or", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");
      REGISTER_CONSTRAINT(TestEqConstraint, "TestEqual", "Default");

      // Europa (NewPlan/ConstraintNetwork) names for the same constraints:
      REGISTER_CONSTRAINT(AddEqualConstraint, "addeq", "Default");
      REGISTER_CONSTRAINT(AddMultEqualConstraint, "addmuleq", "Default");
      REGISTER_CONSTRAINT(AllDiffConstraint, "adiff", "Default"); // all different
      REGISTER_CONSTRAINT(AllDiffConstraint, "fadiff", "Default"); // flexible all different
      REGISTER_CONSTRAINT(AllDiffConstraint, "fneq", "Default"); // flexible not equal
      REGISTER_CONSTRAINT(CardinalityConstraint, "card", "Default"); // cardinality not more than
      //REGISTER_CONSTRAINT(CondAllSameConstraint, "condeq", "Default");
      REGISTER_CONSTRAINT(CountNonZerosConstraint, "cardeq", "Default"); // cardinality equals
      REGISTER_CONSTRAINT(EqualConstraint, "asame", "Default"); // all same
      //REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
      REGISTER_CONSTRAINT(EqualConstraint, "fasame", "Default"); // flexible all same
      REGISTER_CONSTRAINT(EqualMaximumConstraint, "fallmax", "Default"); // flexible all max
      REGISTER_CONSTRAINT(EqualMinimumConstraint, "fallmin", "Default"); // flexible all min
      REGISTER_CONSTRAINT(EqualProductConstraint, "product", "Default");
      //REGISTER_CONSTRAINT(EqualSumConstraint, "sum", "Default");
      REGISTER_CONSTRAINT(LessOrEqThanSumConstraint, "leqsum", "Default");
      //REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
      //REGISTER_CONSTRAINT(LessThanEqualConstraint, "leq", "Default");
      REGISTER_CONSTRAINT(MemberImplyConstraint, "memberImply", "Default");
      REGISTER_CONSTRAINT(MultEqualConstraint, "mulEq", "Default");
      //REGISTER_CONSTRAINT(NegateConstraint, "neg", "Default");
      //REGISTER_CONSTRAINT(NotEqualConstraint, "neq", "Default");
      REGISTER_CONSTRAINT(OrConstraint, "for", "Default"); // flexible or
      REGISTER_CONSTRAINT(OrConstraint, "or", "Default");

      // Rotate scope right one (last var moves to front) to ...
      // ... change addleq constraint to GreaterOrEqThan constraint:
      REGISTER_ROTATED_CONSTRAINT("addleq", "Default", "GreaterOrEqThanSum", 1);
      // ... change addlt constraint to GreaterThanSum constraint:
      REGISTER_ROTATED_CONSTRAINT("addlt", "Default", "GreaterThanSum", 1);
      // ... change allmax and max constraint to EqualMaximum constraint:
      REGISTER_ROTATED_CONSTRAINT("allmax", "Default", "EqualMaximum", 1);
      REGISTER_ROTATED_CONSTRAINT("max", "Default", "EqualMaximum", 1);
      // ... change allmin and min constraint to EqualMinimum constraint:
      REGISTER_ROTATED_CONSTRAINT("allmin", "Default", "EqualMinimum", 1);
      REGISTER_ROTATED_CONSTRAINT("min", "Default", "EqualMinimum", 1);

      // But addeqcond is harder, requiring two "steps":
      REGISTER_SWAP_TWO_VARS_CONSTRAINT("eqcondsum", "Default", "CondEqualSum", 0, 1);
      REGISTER_ROTATED_CONSTRAINT("addeqcond", "Default", "eqcondsum", 2);

    }
  }

  static void terminate() {
    if (getAlreadyCalled()) {
      ObjectFactory::purgeAll();
      TokenFactory::purgeAll();
      ConstraintLibrary::purgeAll();
      Rule::purgeAll();
      uninitNDDL();
      getAlreadyCalled() = false;
    }
  }

  SamplePlanDatabase(const SchemaId& schema, bool enableTransactionLogging = false) {
    initialize();
    constraintEngine = (new ConstraintEngine())->getId();

    planDatabase = (new PlanDatabase(constraintEngine, schema))->getId();

    // order here is important.
    new DefaultPropagator(LabelStr("Default"), constraintEngine);
    new TemporalPropagator(LabelStr("Temporal"), constraintEngine);
    new ResourcePropagator(LabelStr("Resource"), constraintEngine, planDatabase);
    //new ProfilePropagator(LabelStr("Profile"), constraintEngine);

    PropagatorId temporalPropagator = constraintEngine->getPropagatorByName(LabelStr("Temporal"));
    planDatabase->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());

    rulesEngine = (new RulesEngine(planDatabase))->getId();
    horizon = (new Horizon())->getId();
    planner = (new CBPlanner(planDatabase,horizon))->getId();

    if(enableTransactionLogging)
      txLog = (new DbClientTransactionLog(planDatabase->getClient()))->getId();

#ifdef PPW_WITH_PLANNER
    writer = new PlanWriter::PartialPlanWriter(planDatabase, constraintEngine, rulesEngine, planner);
#else
    writer = new PlanWriter::PartialPlanWriter(planDatabase, constraintEngine, rulesEngine);
#endif

  }

  ~SamplePlanDatabase() {
    Entity::purgeStarted();
    delete writer;
    delete (CBPlanner*) planner;
    delete (Horizon*)horizon;

    delete (RulesEngine*) rulesEngine;
    delete (PlanDatabase*) planDatabase;
    delete (ConstraintEngine*) constraintEngine;
    Entity::purgeEnded();
  }
};
#endif
