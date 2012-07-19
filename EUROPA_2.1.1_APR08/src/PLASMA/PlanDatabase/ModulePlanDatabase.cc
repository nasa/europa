#include "ModulePlanDatabase.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "ConstraintLibrary.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "ObjectTokenRelation.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "DbClientTransactionPlayer.hh"

namespace EUROPA {

  static bool & planDatabaseInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModulePlanDatabase::ModulePlanDatabase()
      : Module("PlanDatabase")
  {
	  
  }

  ModulePlanDatabase::~ModulePlanDatabase()
  {	  
  }  
  
  void ModulePlanDatabase::initialize()
  {
      if(planDatabaseInitialized())
    	  return;

      REGISTER_SYSTEM_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
  	  REGISTER_SYSTEM_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");
  	  REGISTER_SYSTEM_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
      
	  planDatabaseInitialized() = true;
  }  

  void ModulePlanDatabase::uninitialize()
  {
      Schema::instance()->reset();
	  ObjectFactory::purgeAll();
	  TokenFactory::purgeAll();	  
	  
	  planDatabaseInitialized() = false;
  }  
  
  class NddlXmlTxnInterpreter : public LanguageInterpreter 
  {
    public:
      NddlXmlTxnInterpreter(const DbClientId& client) : m_interpreter(client) {}	
      virtual ~NddlXmlTxnInterpreter() {}	
      virtual std::string interpret(std::istream& input, const std::string& source);
      
    protected:
      DbClientTransactionPlayer m_interpreter;	
  };

  std::string NddlXmlTxnInterpreter::interpret(std::istream& input, const std::string& script) 
  {
	  m_interpreter.play(input);
      return "";
  } 
  
  void ModulePlanDatabase::initialize(EngineId engine)
  {
	  PlanDatabaseId& pdb = (PlanDatabaseId&)(engine->getComponent("PlanDatabase"));	  
	  engine->addLanguageInterpreter("nddl-xml-txn", new NddlXmlTxnInterpreter(pdb->getClient()));
  }
  
  void ModulePlanDatabase::uninitialize(EngineId engine)
  {	  
	  engine->removeLanguageInterpreter("nddl-xml-txn"); 
  }  
}
