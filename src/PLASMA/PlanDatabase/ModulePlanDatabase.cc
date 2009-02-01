#include "ModulePlanDatabase.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "ConstraintFactory.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "ObjectTokenRelation.hh"
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
      ConstraintEngine* ce = (ConstraintEngine*)engine->getComponent("ConstraintEngine");
      CESchema* ceSchema = (CESchema*)engine->getComponent("CESchema");
      REGISTER_SYSTEM_CONSTRAINT(ceSchema,CommonAncestorConstraint, "commonAncestor", "Default");
      REGISTER_SYSTEM_CONSTRAINT(ceSchema,HasAncestorConstraint, "hasAncestor", "Default");
      REGISTER_SYSTEM_CONSTRAINT(ceSchema,ObjectTokenRelation, "ObjectTokenRelation", "Default");

      Schema* schema = new Schema("EngineSchema",ceSchema->getId()); // TODO: use engine name
      engine->addComponent("Schema",schema);

      PlanDatabase* pdb = new PlanDatabase(ce->getId(), schema->getId());
      engine->addComponent("PlanDatabase",pdb);

	  engine->addLanguageInterpreter("nddl-xml-txn", new NddlXmlTxnInterpreter(pdb->getClient()));
  }

  void ModulePlanDatabase::uninitialize(EngineId engine)
  {
	  engine->removeLanguageInterpreter("nddl-xml-txn");

      PlanDatabase* pdb = (PlanDatabase*)engine->removeComponent("PlanDatabase");
      delete pdb;

      Schema* schema = (Schema*)engine->removeComponent("Schema");
      delete schema;
  }
}
