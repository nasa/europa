#ifndef _H_PlanDbModuleTests
#define _H_PlanDbModuleTests

#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "Timeline.hh"
#include "IntervalIntDomain.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "IntervalToken.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"

namespace EUROPA {

  const LabelStr& DEFAULT_OBJECT_TYPE();

  const LabelStr& DEFAULT_PREDICATE();

  #define SCHEMA Schema::instance()

  void initDbTestSchema(const SchemaId& schema);
  void initDbModuleTests();

  bool testBasicObjectAllocationImpl();
  bool testObjectDomainImpl();
  bool testObjectVariablesImpl();
  bool testObjectObjectTokenRelationImpl();
  bool testCommonAncestorConstraintImpl();
  bool testHasAncestorConstraintImpl();
  bool testMakeObjectVariableImpl();
  bool testTokenObjectVariableImpl();
  bool testTokenWithNoObjectOnCreationImpl();
  bool testFreeAndConstrainImpl();

  bool testBasicTokenAllocationImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testBasicTokenCreationImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testStateModelImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testMasterSlaveRelationshipImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testBasicMergingImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testConstraintMigrationDuringMergeImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testNonChronGNATS2439Impl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testMergingPerformanceImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testTokenCompatibilityImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testTokenFactoryImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testCorrectSplit_Gnats2450impl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testBasicInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testObjectTokenRelationImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testTokenOrderQueryImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testEventTokenInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testFullInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testNoChoicesThatFitImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testBasicAllocationImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
  bool testPathBasedRetrievalImpl(ConstraintEngineId &ce, PlanDatabaseId &db);
}

#endif
