include(EuropaModule)
set(internal_dependencies PlanDatabase ConstraintEngine Utils TinyXml)
# set(internal_dependencies PlanDatabase)
set(root_sources ModuleRulesEngine.cc)
set(base_sources ProxyVariableRelation.cc Rule.cc RuleInstance.cc RuleVariableListener.cc RulesEngine.cc RulesEngineListener.cc)
set(component_sources "")
set(test_sources TestRule.cc module-tests.cc re-test-module.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(RulesEngine "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")
