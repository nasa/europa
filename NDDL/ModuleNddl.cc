#include "ModuleNddl.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "intType.hh"
#include "floatType.hh"
#include "TransactionInterpreter.hh"
#include "TransactionInterpreterInitializer.hh"

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

    /* Allocate NDDL Type Factories */
    // Default names are uppercase, TODO: change default names so that this isn't necessary
    new BoolTypeFactory("bool");
    new StringTypeFactory("string");
    new SymbolTypeFactory("symbol");
      
    // These are Nddl specific, so they belong here
    new intTypeFactory();
    new floatTypeFactory();
      
    TransactionInterpreterInitializer::initialize();      
      
    nddlInitialized() = true;
  }

  void uninitNDDL()
  {
    if(!nddlInitialized())
    	return;
    
    TypeFactory::purgeAll();
    TransactionInterpreterInitializer::uninitialize();      
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
	  check_error(ALWAYS_FAIL,"nddl parser is only available in Java for now");
      return "";
  } 
  
  class NddlXmlInterpreter : public LanguageInterpreter 
  {
    public:
      NddlXmlInterpreter(const DbClientId& client) : m_interpreter(client) {}	
      virtual ~NddlXmlInterpreter() {}	
      virtual std::string interpret(std::istream& input, const std::string& source);
      
    protected:
      InterpretedDbClientTransactionPlayer m_interpreter;	
  };

  std::string NddlXmlInterpreter::interpret(std::istream& input, const std::string& script) 
  {
	  m_interpreter.play(input);
      return "";
  } 
  
  void ModuleNddl::initialize(EngineId engine)
  {
	  PlanDatabaseId& pdb = (PlanDatabaseId&)(engine->getComponent("PlanDatabase"));	  
	  engine->addLanguageInterpreter("nddl", new NddlInterpreter());
	  engine->addLanguageInterpreter("nddl-xml", new NddlXmlInterpreter(pdb->getClient()));
  }
  
  void ModuleNddl::uninitialize(EngineId engine)
  {	  
	  engine->removeLanguageInterpreter("nddl"); 
	  engine->removeLanguageInterpreter("nddl-xml"); 
  }  
}
