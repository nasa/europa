#ifndef _H_PlanDbModuleTests
#define _H_PlanDbModuleTests

#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "Timeline.hh"
#include "UnaryConstraint.hh"
#include "IntervalIntDomain.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "IntervalToken.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"

namespace Prototype {

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

  bool testBasicTokenAllocationImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testBasicTokenCreationImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testStateModelImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testMasterSlaveRelationshipImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testBasicMergingImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testConstraintMigrationDuringMergeImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testMergingPerformanceImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testTokenCompatibilityImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testTokenFactoryImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testCorrectSplit_Gnats2450impl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testBasicInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testObjectTokenRelationImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testTokenOrderQueryImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testEventTokenInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testFullInsertionImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testNoChoicesThatFitImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testBasicAllocationImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);
  bool testPathBasedRetrievalImpl(ConstraintEngineId &ce, PlanDatabaseId &db, SchemaId &schema);

}

#endif
