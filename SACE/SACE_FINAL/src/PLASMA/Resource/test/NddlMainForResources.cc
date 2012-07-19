
#include "ModuleResource.hh"
#include "ModuleSolvers.hh"
#include "NddlTestEngine.hh"

using namespace EUROPA;

class NddlResourceTestEngine : public NddlTestEngine
{
protected:
    virtual void createModules();
};

void NddlResourceTestEngine::createModules()
{
    addModule((new ModuleSolvers())->getId());
    addModule((new ModuleResource())->getId());
}

/**
 * @file Provides main execution program to run a test which loads a model and dumps the resulting database
 */
int main(int argc, const char** argv)
{
    NddlTestEngine engine;
    return engine.run(argc,argv);
}

