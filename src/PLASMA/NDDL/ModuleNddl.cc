#include "ModuleNddl.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "intType.hh"
#include "floatType.hh"
#include "TransactionInterpreter.hh"

namespace EUROPA {

  static bool & nddlInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleNddl::ModuleNddl()
      : Module("NDDL")
  {
	  
  }

  ModuleNddl::~ModuleNddl()
  {	  
  }  
  
  void initNDDL()
  {
    if(nddlInitialized())
    	return;

    nddlInitialized() = true;
  }

  void uninitNDDL()
  {
    if(!nddlInitialized())
    	return;
    
    nddlInitialized() = false;
  }
  
  // TODO: remove initNDDL() and uninitNDDL()
  void ModuleNddl::initialize()
  {
	  initNDDL();
  }  

  void ModuleNddl::uninitialize()
  {
	  uninitNDDL();
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
      CESchema* tfm = (CESchema*)engine->getComponent("CESchema");
      tfm->registerFactory((new intTypeFactory())->getId());
      tfm->registerFactory((new floatTypeFactory())->getId());      

      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");	  
	  engine->addLanguageInterpreter("nddl", new NddlInterpreter());
	  engine->addLanguageInterpreter("nddl-xml", new NddlXmlInterpreter(pdb->getClient()));	  
  }
  
  void ModuleNddl::uninitialize(EngineId engine)
  {	  
	  engine->removeLanguageInterpreter("nddl"); 
	  engine->removeLanguageInterpreter("nddl-xml"); 
	  // TODO: Finish cleanup
  }  
}
