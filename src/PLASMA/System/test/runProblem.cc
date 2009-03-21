
#include "Debug.hh"
#include "Pdlfcn.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "DbClientTransactionLog.hh"
#include "Rule.hh"
#include "Nddl.hh"
#include "TestSupport.hh"
#include "EuropaEngine.hh"
#include <fstream>
#include <iostream>
#include <stdlib.h>


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
#elif FULLY_INTERPRETED
    return true;
#else
    return false;
#endif
}

const char* initialTransactions = NULL;
const char* plannerConfig = NULL;
bool replayRequired = false;

void initSchema(const SchemaId& schema, const RuleSchemaId& ruleSchema);

class TestEngine : public EuropaEngine
{
  public:
    TestEngine()
    {
        m_config->setProperty("nddl.includePath","../../NDDL/test/nddl");
        doStart();
    }

    ~TestEngine()
    {
        doShutdown();
    }

    virtual bool playTransactions(const char* txSource, bool interp = false)
    {
#ifdef FULLY_INTERPRETED
        std::string errors = executeScript("nddl",txSource,true /*isFile*/);
        return (errors.size()==0 && getConstraintEnginePtr()->constraintConsistent());
#else
        return EuropaEngine::playTransactions(txSource,interp);
#endif
    }
};

/**
   REPLAY IS BROKEN WITH THE INTERPRETER AND NEEDS TO BE FIXED!!!!
 */
void replay(const std::string& s1,const DbClientTransactionLogId& txLog)
{
  TestEngine replayed;
  initSchema(((Schema*)replayed.getComponent("Schema"))->getId(),
             ((RuleSchema*)replayed.getComponent("RuleSchema"))->getId());

  replayed.playTransactions(TestEngine::TX_LOG());
  std::string s2 = PlanDatabaseWriter::toString(replayed.getPlanDatabase(), false);
  condDebugMsg(s1 != s2, "Main", "S1" << std::endl << s1 << std::endl << "S2" << std::endl << s2);
  // TO FIX: assertTrue(s1 == s2);
}

std::string dumpIdTable(const char* title)
{
  std::ostringstream os;
  os << title << ":" << IdTable::size() << std::endl;
  IdTable::printTypeCnts(os);

  return os.str();
}

bool runPlanner()
{
  check_error(DebugMessage::isGood());

  TestEngine engine;
  initSchema(((Schema*)engine.getComponent("Schema"))->getId(),
             ((RuleSchema*)engine.getComponent("RuleSchema"))->getId());

  debugMsg("IdTypeCounts", dumpIdTable("before"));

  DbClientTransactionLogId txLog;
  if(replayRequired)
    txLog = (new DbClientTransactionLog(engine.getPlanDatabase()->getClient()))->getId();

  check_error(plannerConfig != NULL, "Must have a planner config argument.");
  TiXmlDocument doc(plannerConfig);
  doc.LoadFile();

  assert(engine.plan(initialTransactions,*(doc.RootElement()), isInterpreted()));

  debugMsg("Main:runPlanner", "Found a plan at depth "
	   << engine.getDepthReached() << " after " << engine.getTotalNodesSearched());

  if(replayRequired) {
      engine.write(std::cout);
      std::string s1 = PlanDatabaseWriter::toString(engine.getPlanDatabase(), false);
      std::ofstream out(TestEngine::TX_LOG());
      txLog->flush(out);
      out.close();
      engine.doShutdown(); // TODO: remove this when all static data structures are gone

      replay(s1, txLog);
  }

  debugMsg("IdTypeCounts", dumpIdTable("after"));

  return true;
}


bool copyFromFile(){
  // Populate plan database from transaction log
  std::string s1;
  {
    TestEngine engine;
    engine.playTransactions(TestEngine::TX_LOG());
    s1 = PlanDatabaseWriter::toString(engine.getPlanDatabase(), false);
    engine.getPlanDatabase()->archive();
  }

  std::string s2;
  {
    TestEngine engine;

    engine.playTransactions(TestEngine::TX_LOG());
    s2 = PlanDatabaseWriter::toString(engine.getPlanDatabase(), false);
    engine.getPlanDatabase()->archive();
  }

  assert(s1 == s2);

  return true;
}

// Args to main()
#ifdef STANDALONE

#define ARGC 4
#define MODEL_INDEX 1
#define TRANS_INDEX 2
#define PCONF_INDEX 3

#else
#define ARGC 3
#define TRANS_INDEX 1
#define PCONF_INDEX 2

#endif

const char* error_msg;
void* libHandle;
SchemaId (*fcn_schema)(const SchemaId&,const RuleSchemaId&);


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

    fcn_schema = (SchemaId (*)(const SchemaId&,const RuleSchemaId&))p_dlsym(libHandle, "loadSchema");
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

void initSchema(const SchemaId& schema, const RuleSchemaId& ruleSchema)
{
#ifdef STANDALONE
    (*fcn_schema)(schema,ruleSchema);
#elif INTERPRETED
    // Interpreter doesn't need to load external initialization for schema
#elif FULLY_INTERPRETED
    // Interpreter doesn't need to load external initialization for schema
#else
    NDDL::loadSchema(schema,ruleSchema);
#endif
}

int main(int argc, const char** argv)
{
    if(argc != ARGC) {
      std::cout << "usage: "
                << "runProblem "
                << "<model shared library path> "
                << "<initial transaction file> "
                << "<planner config file> "
                << std::endl;
      return 1;
    }

    setup(argv);

    const char* performanceTest = getenv("EUROPA_PERFORMANCE");

    if (performanceTest != NULL && strcmp(performanceTest, "1") == 0) {
        replayRequired = false;
        EUROPA_runTest(runPlanner);
    }
    else {
        replayRequired = false; //= true;
        EUROPA_runTest(runPlanner);
        //EUROPA_runTest(copyFromFile);
    }

    cleanup();

    std::cout << "Finished" << std::endl;
    return 0;
}
