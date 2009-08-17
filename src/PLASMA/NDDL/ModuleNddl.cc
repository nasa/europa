#include "ModuleNddl.hh"
#include "PlanDatabase.hh"
#include "Rule.hh"
#include "NddlInterpreter.hh"

namespace EUROPA {

  ModuleNddl::ModuleNddl()
      : Module("NDDL")
  {

  }

  ModuleNddl::~ModuleNddl()
  {
  }

  void ModuleNddl::initialize()
  {
  }

  void ModuleNddl::uninitialize()
  {
  }

  void ModuleNddl::initialize(EngineId engine)
  {
	  engine->addLanguageInterpreter("nddl", new NddlInterpreter(engine));
	  engine->addLanguageInterpreter("nddl-ast", new NddlToASTInterpreter(engine));
  }

  void ModuleNddl::uninitialize(EngineId engine)
  {
	  LanguageInterpreter *old;

	  old = engine->removeLanguageInterpreter("nddl");
	  check_error(old != NULL);
      delete old;

	  old = engine->removeLanguageInterpreter("nddl-ast");
	  check_error(old != NULL);
	  delete old;
	  // TODO: Finish cleanup
  }
}
