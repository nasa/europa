#include "PLASMAPerformanceConstraint.hh"
#include "Nddl.hh"
#include "TestAssembly.hh"
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "ResourceOpenDecisionManager.hh"
#include "PlanDatabaseWriter.hh"
#include "Constraints.hh"
#include "Debug.hh"
#include "Pdlfcn.hh"

//#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>

SchemaId schema;
const char* initialTransactions = NULL;
const char* averTestFile = NULL;
bool replay = false;
extern const char* TX_LOG;

bool runPlanner(){

  TestAssembly assembly(schema);

  DbClientTransactionLogId txLog;
  if(replay)
    txLog = (new DbClientTransactionLog(assembly.getPlanDatabase()->getClient()))->getId();
  
  CBPlanner::Status result = assembly.plan(initialTransactions, averTestFile);

  assert(result == CBPlanner::PLAN_FOUND);

  std::cout << " found a plan at depth " << assembly.getDepthReached() << " after " << assembly.getTotalNodesSearched() << std::endl;

  if (replay)  // this ensures we're not running the performance tests.
    assembly.write(std::cout);

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
  const char* error_msg;
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

  std::cout << "runProblem: p_dlopen() file: " << libPath << std::endl;
  std::cout.flush();
  
  libHandle = p_dlopen(libPath, RTLD_NOW);
  
  std::cout << "runProblem: returned from p_dlopen() file: " << libPath << std::endl;
  std::cout.flush();

  if(!libHandle) {
    error_msg = p_dlerror();
    std::cout << "Error during p_dlopen() of " << libPath << ":" << std::endl;
    check_error(!error_msg, error_msg);
  }
  std::cout << "runProblem: p_dlsym() symbol: loadSchema" << std::endl;
  std::cout.flush();
  
  fcn_schema = (SchemaId (*)())p_dlsym(libHandle, "loadSchema");
  if(!fcn_schema) {
    error_msg = p_dlerror();
    std::cout << "p_dlsym: Error locating NDDL::schema:" << std::endl;
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

  const char* performanceTest = getenv("EUROPA_PERFORMANCE");

  if (performanceTest != NULL && strcmp(performanceTest, "1") == 0) {
    replay = false;
    for(int i = 0; i < 1; i++) {
      runTest(runPlanner);
    }
  }
  else {
    for(int i = 0; i < 1; i++) {
      replay = true;
      runTest(runPlanner);
      runTest(copyFromFile);
    }
  }

  TestAssembly::terminate();

#ifdef STANDALONE
  if(p_dlclose(libHandle)) {
    error_msg = p_dlerror();
    std::cout << "Error during p_dlclose():" << std::endl;
    check_error(!error_msg, error_msg);
  }
  
  std::cout << "Model Library Unloaded" << std::endl;
  std::cout.flush();
#endif
  
  std::cout << "Finished" << std::endl;
  exit(0);
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
