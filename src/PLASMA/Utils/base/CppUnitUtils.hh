#include <iostream>
#include <fstream>

#include "cppunit/XmlOutputter.h"
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

using namespace std;

// This version available to to NDDL module-tests.cc which does a special-case
// construction of the runner
#define RUN_CPP_UNIT_RUNNER(runner, useLogFile) { \
    ofstream* os = NULL; \
    if(useLogFile) { \
        os = new ofstream("cppunit-results.xml"); \
        CppUnit::XmlOutputter* outputter = \
           new CppUnit::XmlOutputter(&runner.result(), *os); \
        runner.setOutputter(outputter); \
    } \
    bool success = runner.run("", false); \
    if(os != NULL) { \
         delete os; \
    } \
    return success ? 0 : 1; \
}

// This macro available to the rest of the module tests, to build a cpp unit
// runner, and optionally spit the output to an xml file (for autobuild reporting)
#define RUN_CPP_UNIT_MODULE(useLogFile) { \
    CppUnit::TextUi::TestRunner runner; \
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry(); \
    runner.addTest( registry.makeTest() );  \
    RUN_CPP_UNIT_RUNNER(runner, useLogFile); \
}
