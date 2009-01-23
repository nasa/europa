#include "ModuleNddl3.hh"
#include "NddlInterpreter.hh"
#include "intType.hh"
#include "floatType.hh"
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
      // These are Nddl specific, so they belong here. TODO: this shouldn't be necessary, include int and float in default CE types
      CESchema* ces = (CESchema*)engine->getComponent("CESchema");
      ces->registerFactory((new intTypeFactory())->getId());
      ces->registerFactory((new floatTypeFactory())->getId());
	  engine->addLanguageInterpreter("nddl3", new NddlInterpreter(engine));
  }

  void ModuleNddl3::uninitialize(EngineId engine)
  {
	  engine->removeLanguageInterpreter("nddl3");
  }
}
