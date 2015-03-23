include(EuropaModule)
set(internal_dependencies RulesEngine PlanDatabase ConstraintEngine Utils)
set(root_sources ModuleTemporalNetwork.cc)
set(base_sources DistanceGraph.cc TemporalNetwork.cc queues.cc)
set(component_sources STNTemporalAdvisor.cc TemporalNetworkListener.cc TemporalPropagator.cc TimepointWrapper.cc)
set(test_sources TestSubgoalRule.cc module-tests.cc tn-test-module.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(TemporalNetwork "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")
