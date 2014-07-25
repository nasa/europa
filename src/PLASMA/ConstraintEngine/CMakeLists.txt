include(EuropaModule)
#set(internal_dependencies Utils TinyXml)
set(internal_dependencies Utils)
set(root_sources ModuleConstraintEngine.cc)
set(base_sources CESchema.cc DataType.cc CFunction.cc Domain.cc ConstrainedVariable.cc DomainListener.cc Constraint.cc PSConstraintEngineListener.cc ConstraintEngine.cc PSVarValue.cc ConstraintEngineListener.cc Propagator.cc ConstraintType.cc VariableChangeListener.cc)
set(component_sources Constraints.cc EquivalenceClassCollection.cc DataTypes.cc Propagators.cc Domains.cc)
#set(test_sources ConstraintTesting.cc ce-test-module.cc module-tests.cc DomainTest.cc domain-tests.cc)
set(test_sources ConstraintTesting.cc ce-test-module.cc module-tests.cc domain-tests.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(ConstraintEngine "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")

set(test_inputs test/CLibTestCases test/CLibTestCases.xml test/NewTestCases test/NewTestCases.xml)
file(COPY ${test_inputs} DESTINATION .)
