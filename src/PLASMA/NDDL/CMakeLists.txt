include(EuropaModule)
set(ANTLR_FIND_REQUIRED TRUE)
include(FindANTLR)
if(NOT ANTLR_FOUND)
  message(FATAL_ERROR "ANTLR not found!")
endif(NOT ANTLR_FOUND)

find_package(Java REQUIRED)
include(UseJava RESULT_VARIABLE java_included)
if(NOT java_included)
  message(FATAL_ERROR "Failed to include UseJava")
endif(NOT java_included)


set(nddl_parser_sources NDDL3.tokens NDDL3Lexer.cpp NDDL3Lexer.h NDDL3Parser.cpp NDDL3Parser.h)
set(nddl_tree_sources NDDL3Tree.cpp NDDL3Tree.h NDDL3Tree.tokens)

set(antlr_generated_parser_files
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3.tokens
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Lexer.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Lexer.h
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Parser.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Parser.h)
set(antlr_generated_tree_files
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Tree.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Tree.h
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Tree.tokens
  )
set(antlr_generated_source_files
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Lexer.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Parser.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Tree.c)

set_property(SOURCE ${antlr_generated_source_files} PROPERTY LANGUAGE CXX)
#TEMPORARY
#set(ANTLR3_JAR /opt/local/share/java/antlr.jar)
set(nddl_parser_sources_path
  #${CMAKE_CURRENT_BINARY_DIR}/NDDL3.tokens
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Lexer.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Lexer.h
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Parser.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Parser.h)
set(nddl_tree_sources_path
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Tree.c
  ${CMAKE_CURRENT_BINARY_DIR}/NDDL3Tree.h
  #${CMAKE_CURRENT_BINARY_DIR}/NDDL3Tree.tokens
  )

# add_custom_command(OUTPUT ${nddl_parser_sources}
add_custom_command(OUTPUT ${antlr_generated_parser_files}
  COMMAND ${Java_JAVA_EXECUTABLE} -cp ${ANTLR_JAR} ${JREFLAGS} org.antlr.Tool -fo ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/base/antlr/NDDL3.g
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/base/antlr/NDDL3.g)
add_custom_command(OUTPUT ${antlr_generated_tree_files}
  COMMAND ${Java_JAVA_EXECUTABLE} -cp ${ANTLR_JAR} ${JREFLAGS} org.antlr.Tool -fo ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/base/antlr/NDDL3Tree.g
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/base/antlr/NDDL3Tree.g
  ${antlr_generated_parser_files})


    
set(internal_dependencies RulesEngine PlanDatabase TemporalNetwork ConstraintEngine Utils)
set(root_sources ModuleNddl.cc)
set(base_sources NddlRules.cc NddlToken.cc NddlUtils.cc)
set(component_sources Interpreter.cc NddlInterpreter.cc NddlTestEngine.cc)
set(test_sources module-tests.cc nddl-test-module.cc)

common_module_prepends("${base_sources}" "${component_sources}" "${test_sources}" base_sources component_sources test_sources)
list(APPEND base_sources ${antlr_generated_source_files})
declare_module(NDDL "${root_sources}" "${base_sources}" "${component_sources}" "${test_sources}" "${internal_dependencies}" "")
#setup ANTLR link/headers?
append_target_property("NDDL${EUROPA_SUFFIX}" INCLUDE_DIRECTORIES ";${ANTLR_INCLUDE_DIR}")
append_target_property("NDDL${EUROPA_SUFFIX}" INCLUDE_DIRECTORIES ";${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries("NDDL${EUROPA_SUFFIX}" ${ANTLR_LIBRARIES})
target_link_libraries("NDDL-test${EUROPA_SUFFIX}" ${ANTLR_LIBRARIES})


file(COPY test/ErrorCheckingTests.txt DESTINATION .)
file(COPY test/nddl DESTINATION .)
file(COPY test/parser DESTINATION .)
file(COPY test/parser.nddl DESTINATION .)
file(GLOB nddl_core base/*.nddl)
file(COPY ${nddl_core} DESTINATION .)