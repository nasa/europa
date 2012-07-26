
#include "AnmlTestEngine.hh"

// Modules
#include "ModuleAnml.hh"

AnmlTestEngine::AnmlTestEngine()
{
}

AnmlTestEngine::~AnmlTestEngine()
{
}

void AnmlTestEngine::createModules()
{
	NddlTestEngine::createModules();
    addModule((new ModuleAnml())->getId());
}
