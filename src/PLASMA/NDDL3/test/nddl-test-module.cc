#include "nddl-test-module.hh"

#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleNddl3.hh"

using namespace EUROPA;


class NddlTestEngine : public EngineBase
{
  public:
    NddlTestEngine();
    virtual ~NddlTestEngine();

  protected:
    virtual void createModules();
};

NddlTestEngine::NddlTestEngine()
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
    addModule((new ModuleNddl3())->getId());
}

void NDDLModuleTests::syntaxTests()
{
    std::string filename="parser.nddl";

    NddlTestEngine engine;

    std::string result = engine.executeScript("nddl3",filename,true /*isFile*/);
    if (result.size() > 0)
        std::cout << "ERROR, Nddl3 parser reported problems :\n" << result << std::endl;
}


