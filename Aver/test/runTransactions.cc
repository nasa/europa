#include "PLASMAPerformanceConstraint.hh"
#include "Nddl.hh"
#include "SamplePlanDatabase.hh"
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "ResourceOpenDecisionManager.hh"
#include "PlanDatabaseWriter.hh"
#include "Constraints.hh"
#include "Debug.hh"
#include "AverInterp.hh"
#include "EventAggregator.hh"

#include <dlfcn.h>
#include <iostream>
#include <stdlib.h>

SchemaId schema;
//const char* TX_INIT = "aver-test-model.xml";
const char* TX_LOG = "TransactionLog.xml";
const char* AVER_TEST = "modtest.xml";

bool runTransactions() {
  SamplePlanDatabase db1(schema, false);

  DecisionManagerId local_dm = db1.planner->getDecisionManager();
  ResourceOpenDecisionManagerId local_rodm = (new ResourceOpenDecisionManager(local_dm))->getId();
  local_dm->setOpenDecisionManager(local_rodm);

  DbClientId client = db1.planDatabase->getClient();
  DbClientTransactionPlayer player(client);
  
  //std::ifstream init(TX_INIT);
  //player.play(init);

  AverInterp::init(AVER_TEST, db1.planner->getDecisionManager(), db1.constraintEngine, db1.planDatabase,
                   db1.rulesEngine);
  
  std::ifstream trans(TX_LOG);
  player.play(trans);
  EventAggregator::instance()->notifyStep();
  AverInterp::terminate();
  EventAggregator::remove();
  return true;
}

int main(int argc, const char** argv) {
#ifdef STANDALONE
  char* error_msg;
  void* libHandle;
  const char* libPath;
  SchemaId (*fcn_schema)();

  if(argc != 2) {
    std::cout << "usage: runProblem <model shared library path>" << std::endl;
    exit(1);
  }
  
  libPath = argv[1];
  //initialTransactions = argv[2];
  
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
  SamplePlanDatabase::initialize();
  schema = (*fcn_schema)();
#else //STANDALONE
  if(argc != 1) {
    std::cout << "usage: runTransactions" << std::endl;
    exit(1);
  }
  //initialTransactions = argv[1];
  SamplePlanDatabase::initialize();
  schema = NDDL::loadSchema();
#endif //STANDALONE

  runTest(runTransactions);

  SamplePlanDatabase::terminate();

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
