include(EuropaModule)

set(nddl_parser_sources NDDL3.tokens NDDL3Lexer.cpp NDDL3Lexer.h NDDL3Parser.cpp NDDL3Parser.h)
set(nddl_tree_sources NDDL3Tree.cpp NDDL3Tree.h NDDL3Tree.tokens)

#TEMPORARY
set(ANTLR3_JAR /opt/local/share/java/antlr.jar)

add_custom_command(OUTPUT ${nddl_parser_sources}
  COMMAND ${JAVA} -cp ${ANTLR3_JAR} ${JREFLAGS} org.antlr.Tool -fo ${CMAKE_CURRENT_SOURCE_DIR}/base antlr/NDDL3.g
  DEPENDS antlr/NDDL3.g)
add_custom_command(OUTPUT ${nddl_tree_sources}
  COMMAND ${JAVA} -cp ${ANTLR3_JAR} ${JREFLAGS} org.antlr.Tool -fo ${CMAKE_CURRENT_SOURCE_DIR}/base antlr/NDDL3Tree.g
  DEPENDS antlr/NDDL3Tree.g)


set(internal_dependencies RulesEngine TemporalNetwork PlanDatabase ConstraintEngine Utils TinyXml)
set(root_sources ModuleNddl.cc)
set(base_sources ${nddl_parser_sources} ${nddl_tree_sources} NddlRules.cc NddlToken.cc NddlUtils.cc)
set(component_sources Interpreter.cc NddlInterpreter.cc NddlTestEngine.cc)
set(test_sources module-tests.cc nddl-test-module.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)

declare_module(NDDL "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")
#setup ANTLR link/headers?
target_link_libraries("NDDL${EUROPA_SUFFIX}" antlr3c)
target_link_libraries("NDDL-test${EUROPA_SUFFIX}" antlr3c)
file(COPY test/ErrorCheckingTests.txt DESTINATION .)
file(COPY test/nddl DESTINATION .)
file(COPY test/parser DESTINATION .)
file(COPY test/parser.nddl DESTINATION .)
file(GLOB nddl_core base/*.nddl)
file(COPY ${nddl_core} DESTINATION .)