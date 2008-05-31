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

	  planDatabaseInitialized() = true;
  }  

  void ModulePlanDatabase::uninitialize()
  {
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
      CESchema* tfm = (CESchema*)engine->getComponent("CESchema");
      
      Schema* schema = new Schema("EngineSchema",tfm->getId()); // TODO: use engine name
      engine->addComponent("Schema",schema);
      schema->addObjectType("Timeline");

      ConstraintEngine* ce = (ConstraintEngine*)engine->getComponent("ConstraintEngine");
      PlanDatabase* pdb = new PlanDatabase(ce->getId(), schema->getId());      
      engine->addComponent("PlanDatabase",pdb);

      REGISTER_SYSTEM_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
      REGISTER_SYSTEM_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");
      REGISTER_SYSTEM_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
      
	  engine->addLanguageInterpreter("nddl-xml-txn", new NddlXmlTxnInterpreter(pdb->getClient()));
  }
  
  void ModulePlanDatabase::uninitialize(EngineId engine)
  {	  
	  engine->removeLanguageInterpreter("nddl-xml-txn"); 

      PlanDatabase* pdb = (PlanDatabase*)engine->removeComponent("PlanDatabase");      
      delete pdb;

      Schema* schema = (Schema*)engine->removeComponent("Schema");      
      delete schema;
	  
	  // TODO: these need to be member variables in PlanDatabase
      ObjectFactory::purgeAll();
      TokenFactory::purgeAll();         
  }  
}
