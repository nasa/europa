
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "Debug.hh"
#include "Utils.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "DbClientTransactionLog.hh"
#include "EuropaEngine.hh"

#ifdef __BEOS__
void __assert_fail(const char *__assertion,
                   const char *__file,
                   unsigned int __line,
                   const char *__function)
{
  debugger(__assertion);
}
#endif

using namespace EUROPA;

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
};

/**
   TODO: REPLAY IS BROKEN WITH THE INTERPRETER AND NEEDS TO BE FIXED!!!!
 */
void replay(const std::string& s1,const DbClientTransactionLogId& txLog, const char* language)
{
  TestEngine replayed;
  replayed.playTransactions(TestEngine::TX_LOG(),language);
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

bool runPlanner(const char* modelFile,
                const char* plannerConfig,
                const char* language,
                bool replayRequired)
{
  CHECK_DEBUG_STREAM

  TestEngine engine;

  debugMsg("IdTypeCounts", dumpIdTable("before"));

  DbClientTransactionLogId txLog;
  if(replayRequired)
    txLog = (new DbClientTransactionLog(engine.getPlanDatabase()->getClient()))->getId();

  assert(engine.plan(modelFile,plannerConfig,language));

  debugMsg("Main:runPlanner", "Found a plan at depth "
	   << engine.getDepthReached() << " after " << engine.getTotalNodesSearched());

  if(replayRequired) {
      engine.write(std::cout);
      std::string s1 = PlanDatabaseWriter::toString(engine.getPlanDatabase(), false);
      std::ofstream out(TestEngine::TX_LOG());
      txLog->flush(out);
      out.close();
      replay(s1, txLog,language);
  }

  debugMsg("IdTypeCounts", dumpIdTable("after"));

  return true;
}


bool copyFromFile(const char* language){
  // Populate plan database from transaction log
  std::string s1;
  {
    TestEngine engine;
    engine.playTransactions(TestEngine::TX_LOG(),language);
    s1 = PlanDatabaseWriter::toString(engine.getPlanDatabase(), false);
    engine.getPlanDatabase()->archive();
  }

  std::string s2;
  {
    TestEngine engine;

    engine.playTransactions(TestEngine::TX_LOG(),language);
    s2 = PlanDatabaseWriter::toString(engine.getPlanDatabase(), false);
    engine.getPlanDatabase()->archive();
  }

  assert(s1 == s2);

  return true;
}

// Args to main()
#define ARGC 4
#define MODEL_INDEX 1
#define PCONF_INDEX 2
#define LANG_INDEX 3

int main(int argc, const char** argv)
{
    if(argc != ARGC) {
      std::cout << "usage: "
                << "runProblem "
                << "<model file> "
                << "<planner config file> "
                << "<language to interpret> "
                << std::endl;
      return 1;
    }

    const char* modelFile = argv[MODEL_INDEX];
    const char* plannerConfig = argv[PCONF_INDEX];
    const char* language = argv[LANG_INDEX];
    bool replayRequired = false;

    // Init data types so that id counts don't fail
    VoidDT::instance();
    BoolDT::instance();
    IntDT::instance();
    FloatDT::instance();
    StringDT::instance();
    SymbolDT::instance();

    const char* performanceTest = getenv("EUROPA_PERFORMANCE");

    if (performanceTest != NULL && strcmp(performanceTest, "1") == 0) {
        replayRequired = false;
        EUROPA_runTest(runPlanner,modelFile,plannerConfig,language,replayRequired);
    }
    else {
        replayRequired = false; //= true;
        EUROPA_runTest(runPlanner,modelFile,plannerConfig,language,replayRequired);
        //EUROPA_runTest(copyFromFile,language);
    }

    std::cout << "Finished" << std::endl;
    return 0;
}
