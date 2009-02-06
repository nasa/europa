#include "ModuleNddl3.hh"
#include "NddlInterpreter.hh"
#include "PlanDatabase.hh"
#include "Rule.hh"

namespace EUROPA {

  ModuleNddl3::ModuleNddl3()
      : Module("NDDL3")
  {

  }

  ModuleNddl3::~ModuleNddl3()
  {
  }

  void ModuleNddl3::initialize()
  {
  }

  void ModuleNddl3::uninitialize()
  {
  }

  void ModuleNddl3::initialize(EngineId engine)
  {
	  engine->addLanguageInterpreter("nddl3", new NddlInterpreter(engine));
  }

  void ModuleNddl3::uninitialize(EngineId engine)
  {
	  engine->removeLanguageInterpreter("nddl3");
  }
}
