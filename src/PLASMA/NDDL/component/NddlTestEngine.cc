
#include "NddlTestEngine.hh"

#include "Error.hh"
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"

// Modules
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleNddl.hh"

NddlTestEngine::NddlTestEngine()
{
    Error::doThrowExceptions(); // throw exceptions!
}

void NddlTestEngine::init()
{
    createModules();
    doStart();
}

NddlTestEngine::~NddlTestEngine()
{
    doShutdown();
}

void NddlTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
    addModule((new ModuleRulesEngine())->getId());
    addModule((new ModuleTemporalNetwork())->getId());
    addModule((new ModuleNddl())->getId());
}

int NddlTestEngine::run(int argc, const char** argv)
{
  if (argc != 3) {
    std::cerr << "Must provide model file and language to interpret" << std::endl;
    return -1;
  }

  const char* txSource = argv[1];
  const char* language = argv[2];

  init();
  return run(txSource,language);
}

int NddlTestEngine::run(const char* txSource, const char* language)
{
    PlanDatabase* planDatabase = (PlanDatabase*) getComponent("PlanDatabase");
    planDatabase->getClient()->enableTransactionLogging();

    std::string result = executeScript(language,txSource,true /*isFile*/);
    if (result.size()>0) {
        std::cerr << result;
        return -1;
    }

    ConstraintEngine* ce = (ConstraintEngine*)getComponent("ConstraintEngine");
    assert(ce->constraintConsistent());

    PlanDatabaseWriter::write(planDatabase->getId(), std::cout);
    std::cout << "Finished\n";
    return 0;
}
