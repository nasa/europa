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
//static const char* s_tx_init = "aver-test-model.xml";
static const char* s_tx_log = "TransactionLog.xml";
static const char* s_aver_test = "modtest.xml";

bool runTransactions() {
  SamplePlanDatabase db1(schema, false);

  DecisionManagerId local_dm = db1.planner->getDecisionManager();
  ResourceOpenDecisionManagerId local_rodm = (new ResourceOpenDecisionManager(local_dm))->getId();
  local_dm->setOpenDecisionManager(local_rodm);

  DbClientId client = db1.planDatabase->getClient();
  DbClientTransactionPlayer player(client);
  
  //std::ifstream init(s_tx_init);
  //player.play(init);

  AverInterp::init(s_aver_test, db1.planner->getDecisionManager(), db1.constraintEngine, db1.planDatabase,
                   db1.rulesEngine);
  
  std::ifstream trans(s_tx_log);
  player.play(trans);
  EventAggregator::instance()->notifyStep();
  AverInterp::terminate();
  //EventAggregator::remove();
  return(true);
}

int main(int argc, const char** argv) {
#ifdef STANDALONE
  char* error_msg;
  void* libHandle;
  const char* libPath;
  SchemaId (*fcn_schema)();

  if (argc != 2) {
    std::cerr << "usage: " << argv[0] << " <model shared library path>" << std::endl;
    exit(1);
  }
  
  libPath = argv[1];
  //initialTransactions = argv[2];
  
  std::cerr << argv[0] << ": dlopen() file: " << libPath << std::endl;
  std::cerr.flush();
  
  libHandle = dlopen(libPath, RTLD_NOW);
  
  if (!libHandle) {
    error_msg = dlerror();
    std::cerr << "Error during dlopen() of " << libPath << ":" << std::endl;
    assertTrue(!error_msg, error_msg);
  }
  
  fcn_schema = (SchemaId (*)())dlsym(libHandle, "loadSchema");
  if (!fcn_schema) {
    error_msg = dlerror();
    std::cerr << "dlsym: Error locating NDDL::schema:" << std::endl;
    assertTrue(!error_msg, error_msg);
  }
  
  assert(Schema::instance().isValid());
  SamplePlanDatabase::initialize();
  schema = (*fcn_schema)();
#else //STANDALONE
  if (argc != 1) {
    std::cerr << "usage: " << argv[0] << std::endl;
    exit(1);
  }
  //initialTransactions = argv[1];
  SamplePlanDatabase::initialize();
  schema = NDDL::loadSchema();
#endif //STANDALONE

  runTest(runTransactions);

  SamplePlanDatabase::terminate();

#ifdef STANDALONE
  if (dlclose(libHandle)) {
    error_msg = dlerror();
    std::cerr << "Error during dlclose():" << std::endl;
    assertTrue(!error_msg, error_msg);
  }
  
  std::cerr << "Model Library Unloaded" << std::endl;
  std::cerr.flush();
#endif
  
  std::cerr << "Finished" << std::endl;
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
