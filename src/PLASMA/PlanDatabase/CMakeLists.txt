include(EuropaModule)
set(internal_dependencies ConstraintEngine Utils TinyXml)
# set(internal_dependencies ConstraintEngine)
set(root_sources ModulePlanDatabase.cc)
set(base_sources CommonAncestorConstraint.cc DbClient.cc DefaultTemporalAdvisor.cc HasAncestorConstraint.cc MergeMemento.cc Method.cc Object.cc ObjectTokenRelation.cc ObjectType.cc PDBInterpreter.cc PSPlanDatabaseListener.cc PlanDatabase.cc PlanDatabaseListener.cc PlanDatabaseWriter.cc Schema.cc StackMemento.cc Token.cc TokenFactory.cc TokenType.cc TokenTypeMgr.cc UnifyMemento.cc DbClientListener.cc)
set(component_sources DbClientTransactionLog.cc DbClientTransactionPlayer.cc EventToken.cc IntervalToken.cc Methods.cc Timeline.cc)
set(test_sources module-tests.cc db-test-module.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(PlanDatabase "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")
