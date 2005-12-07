#include "Nddl.hh"
#include "PlanDatabase.hh"
#include "TestSupport.hh"
#include "Debug.hh"
#include "Pdlfcn.hh"
#include "PlanDatabaseWriter.hh"
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "SolverAssembly.hh"
#include "CBPlannerAssembly.hh"

SchemaId schema;
const char* initialTransactions = NULL;
const char* averTestFile = NULL;
const char* plannerConfig = NULL;
const char* heuristics = NULL;
bool replayRequired = false;


template<class ASSEMBLY>
void replay(const PlanDatabaseId& db, const DbClientTransactionLogId& txLog) {
  std::string s1 = PlanDatabaseWriter::toString(db, false);
  std::ofstream out(ASSEMBLY::TX_LOG());
  txLog->flush(out);
  out.close();
  ASSEMBLY replayed(Schema::instance());
  replayed.playTransactions(ASSEMBLY::TX_LOG());
  std::string s2 = PlanDatabaseWriter::toString(replayed.getPlanDatabase(), false);
  condDebugMsg(s1 != s2, "Main", "S1" << std::endl << s1 << std::endl << "S2" << std::endl << s2);
  // TO FIX: assertTrue(s1 == s2);
}

template<class ASSEMBLY>
bool runPlanner(){
  check_error(DebugMessage::isGood());

  ASSEMBLY assembly(schema);

  DbClientTransactionLogId txLog;
  if(replayRequired)
    txLog = (new DbClientTransactionLog(assembly.getPlanDatabase()->getClient()))->getId();

  check_error(plannerConfig != NULL, "Must have a planner config argument.");
  TiXmlDocument doc(plannerConfig);
  doc.LoadFile();
  

  assert(assembly.plan(initialTransactions,*(doc.RootElement()), heuristics, averTestFile));

  debugMsg("Main:runPlanner", "Found a plan at depth " 
	   << assembly.getDepthReached() << " after " << assembly.getTotalNodesSearched());

  if (replayRequired)  // this ensures we're not running the performance tests.
    assembly.write(std::cout);

  // Store transactions for recreation of database

  if(replayRequired)
    replay<ASSEMBLY>(assembly.getPlanDatabase(), txLog);

  debugStmt("IdTypeCounts", IdTable::printTypeCnts(std::cerr));

  return true;
}


template<class ASSEMBLY>
bool copyFromFile(){
  // Populate plan database from transaction log
  std::string s1;
  {
    ASSEMBLY assembly(schema);
    assembly.playTransactions(ASSEMBLY::TX_LOG());
    s1 = PlanDatabaseWriter::toString(assembly.getPlanDatabase(), false);
    assembly.getPlanDatabase()->archive();
  }

  std::string s2;
  {
    ASSEMBLY assembly(schema);
    assembly.playTransactions(ASSEMBLY::TX_LOG());
    s2 = PlanDatabaseWriter::toString(assembly.getPlanDatabase(), false);
    assembly.getPlanDatabase()->archive();
  }

  assert(s1 == s2);

  return true;
}

template<class ASSEMBLY>
int internalMain(int argc, const char** argv){
#ifdef STANDALONE
#define MODEL_INDEX 1
#define TRANS_INDEX 2
#define PCONF_INDEX 3
#define HCONF_INDEX 4
#define AVER_INDEX 5
#define ARGC 6

  const char* error_msg;
  void* libHandle;
  const char* libPath;
  SchemaId (*fcn_schema)();

  if(argc != ARGC && argc != ARGC - 1) {
    std::cout << "usage: runProblem <model shared library path>" <<
      " <initial transaction file> <planner config file> <heuristics> [Aver test file]" << std::endl;
    return 1;
  }
  
  libPath = argv[MODEL_INDEX];
  initialTransactions = argv[TRANS_INDEX];
  plannerConfig = argv[PCONF_INDEX];
  heuristics = argv[HCONF_INDEX];

  if(argc == ARGC)
    averTestFile = argv[AVER_INDEX];

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
  ASSEMBLY::initialize();
  schema = (*fcn_schema)();
#else //STANDALONE
#define TRANS_INDEX 1
#define PCONF_INDEX 2
#define HCONF_INDEX 3
#define AVER_INDEX 4
#define ARGC 5
  if(argc != ARGC && argc != ARGC-1) {
    std::cout << "usage: runProblem <initial transaction file> "
	      << "<planner config> <heuristics file> [Aver test file]" << std::endl;
    std::cout << ARGC << " " << argc << std::endl;
    return 1;
  }
  initialTransactions = argv[TRANS_INDEX];
  plannerConfig = argv[PCONF_INDEX];
  heuristics = argv[HCONF_INDEX];

  if(argc == ARGC)
    averTestFile = argv[AVER_INDEX];

  ASSEMBLY::initialize();
  schema = NDDL::loadSchema();
#endif //STANDALONE

  const char* performanceTest = getenv("EUROPA_PERFORMANCE");

  if (performanceTest != NULL && strcmp(performanceTest, "1") == 0) {
    replayRequired = false;
    for(int i = 0; i < 1; i++) {
      runTest(runPlanner<ASSEMBLY>);
    }
  }
  else {
    for(int i = 0; i < 1; i++) {
      replayRequired = true;
      runTest(runPlanner<ASSEMBLY>);
      runTest(copyFromFile<ASSEMBLY>);
    }
  }

  ASSEMBLY::terminate();

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


int main(int argc, const char** argv) {

#define ONE_ASSEMBLY_ONLY
#ifdef ONE_ASSEMBLY_ONLY
#ifdef CBPLANNER
  return internalMain<CBPlannerAssembly>(argc, argv);
#else
  return internalMain<SolverAssembly>(argc, argv);
#endif
#else
#ifdef CBPLANNER
  bool result = internalMain<CBPlannerAssembly>(argc, argv);
#else
  bool result = internalMain<SolverAssembly>(argc, argv);
#endif

  if(result != 0)
    return result;
  else {
#ifdef CBPLANNER
    return internalMain<SolverAssembly>(argc, argv);
#else
    return internalMain<CBPlannerAssembly>(argc, argv);
#endif
  }
#endif
}
