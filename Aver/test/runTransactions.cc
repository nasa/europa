#include "Nddl.hh"

#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "ResourceOpenDecisionManager.hh"
#include "PlanDatabaseWriter.hh"
#include "Constraints.hh"
#include "Debug.hh"
#include "AverInterp.hh"
#include "EventAggregator.hh"
#include "AverTestAssembly.hh"
#include "Pdlfcn.hh"

#include <iostream>
#include <stdlib.h>

SchemaId schema;
//static const char* s_tx_init = "aver-test-model.xml";
static const char* s_tx_log = "TransactionLog.xml";
static const char* s_aver_test = "modtest.xml";

bool runTransactions() {
  AverTestAssembly::initialize();
  AverTestAssembly assembly(schema, s_aver_test);
  
  //AverInterp::init(s_aver_test, assembly.getPlanner()->getDecisionManager(), assembly.getPlanDatabase()->getConstraintEngine(), assembly.getPlanDatabase(),
  //                 assembly.getRulesEngine());
  
  assembly.playTransactions(s_tx_log);
  EventAggregator::instance()->notifyStep();
  AverTestAssembly::terminate();
  //AverInterp::terminate();
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
  
  //std::cerr << argv[0] << ": p_dlopen() file: " << libPath << std::endl;
  //std::cerr.flush();
  
  libHandle = p_dlopen(libPath, RTLD_NOW);
  
  if (!libHandle) {
    error_msg = p_dlerror();
    std::cerr << "Error during p_dlopen() of " << libPath << ":" << std::endl;
    assertTrue(!error_msg, error_msg);
  }
  
  fcn_schema = (SchemaId (*)())p_dlsym(libHandle, "loadSchema");
  if (!fcn_schema) {
    error_msg = p_dlerror();
    std::cerr << "p_dlsym: Error locating NDDL::schema:" << std::endl;
    assertTrue(!error_msg, error_msg);
  }
  
  assert(Schema::instance().isValid());
  //SamplePlanDatabase::initialize();
  schema = (*fcn_schema)();
#else //STANDALONE
  if (argc != 1) {
    std::cerr << "usage: " << argv[0] << std::endl;
    exit(1);
  }
  //initialTransactions = argv[1];
  //SamplePlanDatabase::initialize();
  schema = NDDL::loadSchema();
#endif //STANDALONE

  runTest(runTransactions);

  //SamplePlanDatabase::terminate();

#ifdef STANDALONE
  if (p_dlclose(libHandle)) {
    error_msg = p_dlerror();
    std::cerr << "Error during p_dlclose():" << std::endl;
    assertTrue(!error_msg, error_msg);
  }
  
  //std::cerr << "Model Library Unloaded" << std::endl;
  //std::cerr.flush();
#endif
  
  //std::cerr << "Finished" << std::endl;
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
