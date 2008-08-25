#include "ModuleNddl.hh"
#include "intType.hh"
#include "floatType.hh"
#include "NddlXml.hh"
#include "PlanDatabase.hh"
#include "Rule.hh"

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

  class NddlInterpreter : public LanguageInterpreter
  {
    public:
      virtual ~NddlInterpreter() {}
      virtual std::string interpret(std::istream& input, const std::string& source);
  };

  std::string NddlInterpreter::interpret(std::istream& input, const std::string& script)
  {
	  check_error(ALWAYS_FAIL,"nddl parser is only available in Java for now. nddl-xml is supported in C++, you can use the nddl parser to generate nddl-xml from nddl source.");
      return "";
  }

  void ModuleNddl::initialize(EngineId engine)
  {
      // These are Nddl specific, so they belong here
      CESchema* ces = (CESchema*)engine->getComponent("CESchema");
      ces->registerFactory((new intTypeFactory())->getId());
      ces->registerFactory((new floatTypeFactory())->getId());

      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");
	  engine->addLanguageInterpreter("nddl", new NddlInterpreter());

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
