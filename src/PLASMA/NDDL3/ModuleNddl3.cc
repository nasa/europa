#include "ModuleNddl3.hh"
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

  class NddlInterpreter : public LanguageInterpreter
  {
    public:
      virtual ~NddlInterpreter() {}
      virtual std::string interpret(std::istream& input, const std::string& source);
  };

  std::string NddlInterpreter::interpret(std::istream& input, const std::string& script)
  {
      // TODO: plug in nddl3 interpreter here
	  check_error(ALWAYS_FAIL,"nddl parser is only available in Java for now. nddl-xml is supported in C++, you can use the nddl parser to generate nddl-xml from nddl source.");
      return "";
  }

  void ModuleNddl3::initialize(EngineId engine)
  {
      // These are Nddl specific, so they belong here
      /*
      CESchema* ces = (CESchema*)engine->getComponent("CESchema");
      ces->registerFactory((new intTypeFactory())->getId());
      ces->registerFactory((new floatTypeFactory())->getId());
      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");
      RuleSchema* rs = (RuleSchema*)engine->getComponent("RuleSchema");
      */
	  engine->addLanguageInterpreter("nddl3", new NddlInterpreter(/*pdb->getClient(),rs->getId()*/));
  }

  void ModuleNddl3::uninitialize(EngineId engine)
  {
	  engine->removeLanguageInterpreter("nddl3");
  }
}
