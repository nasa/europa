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
      NddlInterpreter* interp = new NddlInterpreter(engine);
	  engine->addLanguageInterpreter("nddl", interp);
      // TODO: This is only to make it visible in java
	  // drop this when current Java parser gets downgraded to nddl2
	  engine->addLanguageInterpreter("nddl3", interp);

      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");
      RuleSchema* rs = (RuleSchema*)engine->getComponent("RuleSchema");
	  engine->addLanguageInterpreter("nddl-xml", new NddlXmlInterpreter(pdb->getClient(),rs->getId()));
  }

  void ModuleNddl::uninitialize(EngineId engine)
  {
	  engine->removeLanguageInterpreter("nddl");
	  engine->removeLanguageInterpreter("nddl-xml");
	  // TODO: Finish cleanup
  }
}
