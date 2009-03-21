
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleNddl.hh"
#include "ModuleResource.hh"
#include "ModuleSolvers.hh"
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"

using namespace EUROPA;

class NddlResourceTestEngine : public EngineBase
{
public:
    NddlResourceTestEngine();
    virtual ~NddlResourceTestEngine();

    virtual void init();
    void run(const char* txSource, const std::string& language);

protected:
    virtual void createModules();
};

NddlResourceTestEngine::NddlResourceTestEngine()
{
}

void NddlResourceTestEngine::init()
{
    createModules();
    doStart();
    //initialize((CESchema*)getComponent("CESchema"));
}

NddlResourceTestEngine::~NddlResourceTestEngine()
{
    doShutdown();
}

void NddlResourceTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
    addModule((new ModuleRulesEngine())->getId());
    addModule((new ModuleTemporalNetwork())->getId());
    addModule((new ModuleNddl())->getId());
    addModule((new ModuleSolvers())->getId());
    addModule((new ModuleResource())->getId());
}

void NddlResourceTestEngine::run(const char* txSource, const std::string& language)
{
    PlanDatabase* planDatabase = (PlanDatabase*) getComponent("PlanDatabase");
    planDatabase->getClient()->enableTransactionLogging();

    executeScript(language,txSource,true /*isFile*/);

    ConstraintEngine* ce = (ConstraintEngine*)getComponent("ConstraintEngine");
    assert(ce->constraintConsistent());

    PlanDatabaseWriter::write(planDatabase->getId(), std::cout);
    std::cout << "Finished\n";
}


/**  (Copied from NddlMain.cc)
 * @file Provides main execution program to run a test which plays transactions
 * on a database for a given model. The model must currently be linked to the executable
 * statically, but we will eventually get to dynamically linking to a model as an argument.
 */
int main(int argc, const char ** argv)
{
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file and language to interpret" << std::endl;
    return -1;
  }

  NddlResourceTestEngine engine;

  engine.init();

  const char* txSource = argv[1];
  std::string language = argv[2];
  engine.run(txSource,language);

  return 0;
}

