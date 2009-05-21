#include "ModuleNddl.hh"
#include "PlanDatabase.hh"
#include "Rule.hh"
#include "NddlXml.hh"
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

      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");
      RuleSchema* rs = (RuleSchema*)engine->getComponent("RuleSchema");
	  engine->addLanguageInterpreter("nddl-xml", new NddlXmlInterpreter(pdb->getClient(),rs->getId()));
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

	  old = engine->removeLanguageInterpreter("nddl-xml");
	  check_error(old != NULL);
	  delete old;
	  // TODO: Finish cleanup
  }
}
