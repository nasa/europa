include(EuropaModule)
#set(internal_dependencies Utils TinyXml)
set(internal_dependencies Utils TinyXml)
set(root_sources ModuleConstraintEngine.cc)
set(base_sources CESchema.cc DataType.cc CFunction.cc Domain.cc ConstrainedVariable.cc DomainListener.cc Constraint.cc PSConstraintEngineListener.cc ConstraintEngine.cc PSVarValue.cc ConstraintEngineListener.cc Propagator.cc ConstraintType.cc VariableChangeListener.cc ConstraintTypeChecking.cc)
set(component_sources Constraints.cc EquivalenceClassCollection.cc DataTypes.cc Propagators.cc Domains.cc CFunctions.cc)
#set(test_sources ConstraintTesting.cc ce-test-module.cc module-tests.cc DomainTest.cc domain-tests.cc)
set(test_sources ConstraintTesting.cc ce-test-module.cc module-tests.cc domain-tests.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(ConstraintEngine "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")

set(test_inputs CLibTestCases CLibTestCases.xml NewTestCases NewTestCases.xml violations-allowed.xml violations-missing.xml violations-unallowed.xml)
foreach(file ${test_inputs})
  add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/test/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file})
  add_custom_target(${file} DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${file})
  add_dependencies(${ConstraintEngine_TEST} ${file})
endforeach(file)
