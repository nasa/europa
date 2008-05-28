#include "Nddl.hh"
#include "PlanDatabase.hh"
#include "DbClientTransactionLog.hh"
#include "TestSupport.hh"
#include "Debug.hh"
#include "Pdlfcn.hh"
#include "PlanDatabaseWriter.hh"
#include "ObjectFactory.hh"
#include "TypeFactory.hh"
#include "TokenFactory.hh"
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "SolverAssembly.hh"
#include "Rule.hh"
#ifndef NO_RESOURCES
#include "SAVH_FVDetector.hh"
#include "SAVH_Profile.hh"
#include "SAVH_TimetableFVDetector.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_ReusableFVDetector.hh"
#endif

#ifdef CBPLANNER
#error "CBPlanner is now deprecated."
#endif

#ifdef __BEOS__
void __assert_fail(const char *__assertion,
                   const char *__file,
                   unsigned int __line,
                   const char *__function)
{
  debugger(__assertion);
}
#endif


bool isInterpreted()
{
#ifdef INTERPRETED
    return true;
#else
    return false;
#endif
}

const char* initialTransactions = NULL;
const char* plannerConfig = NULL;
bool replayRequired = false;

void initSchema(const SchemaId& schema);

/**
   REPLAY IS BROKEN WITH THE INTERPRETER AND NEEDS TO BE FIXED!!!!
 */
template<class ASSEMBLY>
void replay(const std::string& s1,const DbClientTransactionLogId& txLog) {
  ASSEMBLY replayed;
  initSchema(((Schema*)replayed.getComponent("Schema"))->getId());
  
  replayed.playTransactions(ASSEMBLY::TX_LOG());
  std::string s2 = PlanDatabaseWriter::toString(replayed.getPlanDatabase(), false);
  condDebugMsg(s1 != s2, "Main", "S1" << std::endl << s1 << std::endl << "S2" << std::endl << s2);
  // TO FIX: assertTrue(s1 == s2);
}

std::string dumpIdTable(const char* title)
{
  std::ostringstream os;
  os << "before:" << IdTable::size() << std::endl;
  IdTable::printTypeCnts(os);
  
  return os.str();
}

template<class ASSEMBLY>
bool runPlanner()
{
  check_error(DebugMessage::isGood());

  ASSEMBLY assembly;
  initSchema(((Schema*)assembly.getComponent("Schema"))->getId());

  debugMsg("IdTypeCounts", dumpIdTable("before"));  
    
  DbClientTransactionLogId txLog;
  if(replayRequired)
    txLog = (new DbClientTransactionLog(assembly.getPlanDatabase()->getClient()))->getId();

  check_error(plannerConfig != NULL, "Must have a planner config argument.");
  TiXmlDocument doc(plannerConfig);
  doc.LoadFile();

  assert(assembly.plan(initialTransactions,*(doc.RootElement()), isInterpreted()));

  debugMsg("Main:runPlanner", "Found a plan at depth " 
	   << assembly.getDepthReached() << " after " << assembly.getTotalNodesSearched());

  if(replayRequired) {
      assembly.write(std::cout);
      std::string s1 = PlanDatabaseWriter::toString(assembly.getPlanDatabase(), false);
      std::ofstream out(ASSEMBLY::TX_LOG());
      txLog->flush(out);
      out.close();
      assembly.doShutdown(); // TODO: remove this when all static data structures are gone
      
      replay<ASSEMBLY>(s1, txLog);
  }

  debugMsg("IdTypeCounts", dumpIdTable("after"));  

  return true;
}


template<class ASSEMBLY>
bool copyFromFile(){
  // Populate plan database from transaction log
  std::string s1;
  {
    ASSEMBLY assembly;
    assembly.playTransactions(ASSEMBLY::TX_LOG());
    s1 = PlanDatabaseWriter::toString(assembly.getPlanDatabase(), false);
    assembly.getPlanDatabase()->archive();
  }

  std::string s2;
  {
    ASSEMBLY assembly;
    assembly.playTransactions(ASSEMBLY::TX_LOG());
    s2 = PlanDatabaseWriter::toString(assembly.getPlanDatabase(), false);
    assembly.getPlanDatabase()->archive();
  }

  assert(s1 == s2);

  return true;
}

#ifdef STANDALONE

#define MODEL_INDEX 1
#define TRANS_INDEX 2
#define PCONF_INDEX 3
#define ARGC 4

#elif INTERPRETED

#define TRANS_INDEX 1
#define PCONF_INDEX 2
#define ARGC 3

#else 

#define TRANS_INDEX 1
#define PCONF_INDEX 2
#define ARGC 3

#endif

const char* error_msg;
void* libHandle;
SchemaId (*fcn_schema)(const SchemaId&);


void loadLibrary(const char* libPath)
{    
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
    
    fcn_schema = (SchemaId (*)(const SchemaId&))p_dlsym(libHandle, "loadSchema");
    if(!fcn_schema) {
      error_msg = p_dlerror();
      std::cout << "p_dlsym: Error locating NDDL::schema:" << std::endl;
      check_error(!error_msg, error_msg);
    }    
}

void unloadLibrary()
{
    if(p_dlclose(libHandle)) {
      error_msg = p_dlerror();
      std::cout << "Error during p_dlclose():" << std::endl;
      check_error(!error_msg, error_msg);
    }
    
    std::cout << "Model Library Unloaded" << std::endl;
    std::cout.flush();    
}

void setup(const char** argv)
{
    initialTransactions = argv[TRANS_INDEX];
    plannerConfig = argv[PCONF_INDEX];
    
#ifdef STANDALONE
    loadLibrary(argv[MODEL_INDEX]);
#endif 
}

void cleanup()
{
#ifdef STANDALONE
    unloadLibrary();
#endif    
}

void initSchema(const SchemaId& schema)
{
#ifdef STANDALONE
    (*fcn_schema)(schema);
#elif INTERPRETED
    std::string modelName(initialTransactions);
    modelName = modelName.substr(0, modelName.find(".xml"));
#else 
    NDDL::loadSchema(schema);
#endif     
}

int main(int argc, const char** argv) 
{
    if(argc != ARGC) {
      std::cout << "usage: runProblem <model shared library path>" <<
        " <initial transaction file> <planner config file>" << std::endl;
      return 1;
    }
        
    setup(argv);
    SolverAssembly::initialize();

    const char* performanceTest = getenv("EUROPA_PERFORMANCE");

    if (performanceTest != NULL && strcmp(performanceTest, "1") == 0) {
        replayRequired = false;
        for (int i = 0; i < 1; i++) 
            runTest(runPlanner<SolverAssembly>);
    }
    else {
        replayRequired = false; //= true;
        for (int i = 0; i < 1; i++) {
            runTest(runPlanner<SolverAssembly>);
            //runTest(copyFromFile<SolverAssembly>);
        }
    }

    SolverAssembly::terminate();
    cleanup();
  
    std::cout << "Finished" << std::endl;
    return 0;
}
