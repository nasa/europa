#include "PLASMAPerformanceConstraint.hh"
#include "Nddl.hh"
#include "TestAssembly.hh"
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "ResourceOpenDecisionManager.hh"
#include "PlanDatabaseWriter.hh"
#include "Constraints.hh"
#include "Debug.hh"

#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>

SchemaId schema;
const char* initialTransactions = NULL;
const char* averTestFile = NULL;
bool replay = false;
extern const char* TX_LOG;

#define PERFORMANCE

bool runPlanner(){

  TestAssembly assembly(schema);

  DbClientTransactionLogId txLog;
  if(replay)
    txLog = (new DbClientTransactionLog(assembly.getPlanDatabase()->getClient()))->getId();
  
  CBPlanner::Status result = assembly.plan(initialTransactions, averTestFile);

  assert(result == CBPlanner::PLAN_FOUND);

  // std::cout << "assembly.plan status = " << result << std::endl;

  std::cout << " found a plan at depth " << assembly.getDepthReached() << " after " << assembly.getTotalNodesSearched() << std::endl;

#ifndef PERFORMANCE
  assembly.write(std::cout);
#endif

  // Store transactions for recreation of database

  if(replay)
    assembly.replay(txLog);

  debugStmt("IdTypeCounts", IdTable::printTypeCnts(std::cerr));

  return true;
}


bool copyFromFile(){
  // Populate plan database from transaction log
  std::stringstream os1;
  {
    TestAssembly assembly(schema);
    assembly.playTransactions(TX_LOG);
    assembly.getPlanDatabase()->getClient()->toStream(os1);
  }
  std::stringstream os2;
  {
    TestAssembly assembly(schema);
    assembly.playTransactions(TX_LOG);
    assembly.getPlanDatabase()->getClient()->toStream(os2);
  }

  std::string s1 = os1.str();
  std::string s2 = os2.str();
  assert(s1 == s2);

  return true;
}

//namespace NDDL { SchemaId loadSchema() {return Schema::instance();}}

int main(int argc, const char** argv) {
#ifdef STANDALONE
  char* error_msg;
  void* libHandle;
  const char* libPath;
  SchemaId (*fcn_schema)();

  if(argc != 3 && argc != 4) {
    std::cout << "usage: runProblem <model shared library path> <initial transaction file> [Aver test file]" << std::endl;
    exit(1);
  }
  
  libPath = argv[1];
  initialTransactions = argv[2];
  
  if(argc == 4)
    averTestFile = argv[3];

  std::cout << "runProblem: dlopen() file: " << libPath << std::endl;
  std::cout.flush();
  
  libHandle = dlopen(libPath, RTLD_NOW);
  
  if(!libHandle) {
    error_msg = dlerror();
    std::cout << "Error during dlopen() of " << libPath << ":" << std::endl;
    check_error(!error_msg, error_msg);
  }
  
  fcn_schema = (SchemaId (*)())dlsym(libHandle, "loadSchema");
  if(!fcn_schema) {
    error_msg = dlerror();
    std::cout << "dlsym: Error locating NDDL::schema:" << std::endl;
    check_error(!error_msg, error_msg);
  }
  
  assert(Schema::instance().isValid());
  TestAssembly::initialize();
  schema = (*fcn_schema)();
#else //STANDALONE
  if(argc != 2 && argc != 3) {
    std::cout << "usage: runProblem <initial transaction file> [Aver test file]" << std::endl;
    exit(1);
  }
  initialTransactions = argv[1];

  if(argc == 3)
    averTestFile = argv[2];

  TestAssembly::initialize();
  schema = NDDL::loadSchema();
#endif //STANDALONE

#ifdef PERFORMANCE
  replay = false;
  for(int i = 0; i < 1; i++) {
    runTest(runPlanner);
  }
#else
  for(int i = 0; i < 1; i++) {
    replay = true;
    runTest(runPlanner);
    runTest(copyFromFile);
  }
#endif

  TestAssembly::terminate();

#ifdef STANDALONE
  if(dlclose(libHandle)) {
    error_msg = dlerror();
    std::cout << "Error during dlclose():" << std::endl;
    check_error(!error_msg, error_msg);
  }
  
  std::cout << "Model Library Unloaded" << std::endl;
  std::cout.flush();
#endif
  
  std::cout << "Finished" << std::endl;
  return 0;
}

#ifdef __BEOS__

void __assert_fail(const char *__assertion,
                   const char *__file,
                   unsigned int __line,
                   const char *__function)
{
  debugger(__assertion);
}

#endif
