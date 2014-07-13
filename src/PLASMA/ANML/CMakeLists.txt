include(EuropaModule)

set(anml_parser_sources ANML.tokens ANMLLexer.cpp ANMLLexer.h ANMLParser.cpp ANMLParser.h)

#TEMPORARY
set(ANTLR3_JAR /opt/local/share/java/antlr.jar)

add_custom_command(OUTPUT ${anml_parser_sources}
  COMMAND ${JAVA} -cp ${ANTLR3_JAR} ${JREFLAGS} org.antlr.Tool -fo ${CMAKE_CURRENT_SOURCE_DIR}/base antlr/ANML.g
  DEPENDS antlr/ANML.g)

#TODO: abstract the stuff it depends on in NDDL so languages aren't inter-dependent
set(internal_dependencies NDDL RulesEngine TemporalNetwork PlanDatabase ConstraintEngine Utils TinyXml)
set(root_sources ModuleAnml.cc)
set(base_sources ${anml_parser_sources} ANMLTranslator.cc)
set(component_sources AnmlInterpreter.cc AnmlTestEngine.cc)
set(test_sources module-tests.cc anml-test-module.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(ANML "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")
#setup ANTLR link/headers?
target_link_libraries("ANML${EUROPA_SUFFIX}" antlr3c)
target_link_libraries("ANML-test${EUROPA_SUFFIX}" antlr3c)
#file(COPY test/ErrorCheckingTests.txt DESTINATION .)
#file(COPY test/anml DESTINATION .)
#file(COPY test/parser DESTINATION .)
#file(COPY test/parser.anml DESTINATION .)
#file(GLOB anml_core base/*.anml)
#file(COPY ${anml_core} DESTINATION .)