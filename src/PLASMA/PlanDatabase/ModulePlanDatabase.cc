#include "ModulePlanDatabase.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "ConstraintType.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "ObjectTokenRelation.hh"
#include "DbClientTransactionPlayer.hh"
#include "Timeline.hh"
#include "Token.hh"
#include "Methods.hh"
#include "Propagators.hh"
#include "CESchema.hh"

#include <boost/cast.hpp>

namespace EUROPA {

  ModulePlanDatabase::ModulePlanDatabase()
      : Module("PlanDatabase")
  {

  }

  ModulePlanDatabase::~ModulePlanDatabase()
  {
  }

  void ModulePlanDatabase::initialize()
  {
  }

  void ModulePlanDatabase::uninitialize()
  {
  }

  class NddlXmlTxnInterpreter : public LanguageInterpreter
  {
    public:
      NddlXmlTxnInterpreter(const DbClientId client) : m_interpreter(client) {}
      virtual ~NddlXmlTxnInterpreter() {}
      virtual std::string interpret(std::istream& input, const std::string& source);

    protected:
      DbClientTransactionPlayer m_interpreter;
  };

  std::string NddlXmlTxnInterpreter::interpret(std::istream& input, const std::string&)
  {
	  m_interpreter.play(input);
      return "";
  }

void ModulePlanDatabase::initialize(EngineId engine) {
  ConstraintEngine* ce =
      boost::polymorphic_cast<ConstraintEngine*>(engine->getComponent("ConstraintEngine"));
  CESchema* ceSchema = boost::polymorphic_cast<CESchema*>(engine->getComponent("CESchema"));

  new DefaultPropagator("PlanDatabaseSystemPropagator", ce->getId(), SYSTEM_PRIORITY);
  REGISTER_SYSTEM_CONSTRAINT(ceSchema,ObjectTokenRelation, "ObjectTokenRelation", "PlanDatabaseSystemPropagator");
  
  REGISTER_CONSTRAINT_TYPE(ceSchema,CommonAncestorCT, "commonAncestor", "Default");
  REGISTER_CONSTRAINT_TYPE(ceSchema,HasAncestorCT, "hasAncestor", "Default");
  
  Schema* schema = new Schema("EngineSchema",ceSchema->getId()); // TODO: use engine name
  schema->registerEnum("TokenStates",StateDomain());
  engine->addComponent("Schema",schema);
  
  ObjectType* ot;
  const char* rootObjType = Schema::rootObject().c_str();
  
  ot = new ObjectType(rootObjType,ObjectTypeId::noId(),true /*isNative*/);
  schema->registerObjectType(ot->getId());
  
  ot = new ObjectType("Timeline",ot->getId(),true /*isNative*/);
  ot->addObjectFactory((new TimelineObjectFactory(ot->getId()))->getId());
  schema->registerObjectType(ot->getId());

  // Exported Methods
  schema->registerMethod((new PDBClose())->getId());
  schema->registerMethod((new SpecifyVariable())->getId());
  schema->registerMethod((new ResetVariable())->getId());
  schema->registerMethod((new CloseVariable())->getId());
  schema->registerMethod((new ConstrainToken())->getId());
  schema->registerMethod((new FreeToken())->getId());
  schema->registerMethod((new ActivateToken())->getId());
  schema->registerMethod((new MergeToken())->getId());
  schema->registerMethod((new RejectToken())->getId());
  schema->registerMethod((new CancelToken())->getId());
  schema->registerMethod((new CloseClass())->getId());

  PlanDatabase* pdb = new PlanDatabase(ce->getId(), schema->getId());
  engine->addComponent("PlanDatabase",pdb);

  engine->addLanguageInterpreter("nddl-xml-txn", new NddlXmlTxnInterpreter(pdb->getClient()));
}

void ModulePlanDatabase::uninitialize(EngineId engine) {
  LanguageInterpreter* old = engine->removeLanguageInterpreter("nddl-xml-txn");
  if (old)
    delete old;
  
  PlanDatabase* pdb =
      boost::polymorphic_cast<PlanDatabase*>(engine->removeComponent("PlanDatabase"));
  delete pdb;
  
  Schema* schema = boost::polymorphic_cast<Schema*>(engine->removeComponent("Schema"));
  delete schema;
}
}
