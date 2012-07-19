#ifndef _H_PlanDatabaseDefs
#define _H_PlanDatabaseDefs

#include "ConstraintEngineDefs.hh"
#include "Entity.hh"
#include "Variable.hh"

namespace EUROPA {
  class Object;
  typedef Id<Object> ObjectId;
  typedef std::set<ObjectId, EntityComparator<ObjectId> > ObjectSet;

  class Timeline;
  typedef Id<Timeline> TimelineId;

  class Token;
  typedef Id<Token> TokenId;
  typedef std::set<TokenId, EntityComparator<TokenId> > TokenSet;

  class UnifyMemento;
  typedef Id<UnifyMemento> UnifyMementoId;

  class MergeMemento;
  typedef Id<MergeMemento> MergeMementoId;

  class StackMemento;
  typedef Id<StackMemento> StackMementoId;

  class PlanDatabaseListener;
  typedef Id<PlanDatabaseListener> PlanDatabaseListenerId;

  template<class DomainType> class TokenVariable;
  class StateDomain;
  typedef Id< TokenVariable<StateDomain> > StateVarId;
  
  typedef Id< Variable<IntervalIntDomain> > TimeVarId;
  typedef Id< TokenVariable<IntervalIntDomain> > TempVarId;

  class ObjectDomain;
  typedef Id< TokenVariable<ObjectDomain> > ObjectVarId;

  class Schema;
  typedef Id<Schema> SchemaId;

  class PlanDatabase;
  typedef Id<PlanDatabase> PlanDatabaseId;

  class TemporalAdvisor;
  typedef Id<TemporalAdvisor> TemporalAdvisorId;

  class DbClient;
  typedef Id<DbClient> DbClientId;

  class DbClientListener;
  typedef Id<DbClientListener> DbClientListenerId;

  class DbClientTransactionLog;
  typedef Id<DbClientTransactionLog> DbClientTransactionLogId;

  class DbClientTransactionPlayer;
  typedef Id<DbClientTransactionPlayer> DbClientTransactionPlayerId;

  class DbClientTransactionTokenMapper;
  typedef Id<DbClientTransactionTokenMapper> DbClientTransactionTokenMapperId;

  typedef std::pair<LabelStr, const AbstractDomain*> ConstructorArgument;

  /**
   * @brief Used for returning ordering choices for tokens on objects
   */
  typedef std::pair< ObjectId, std::pair<TokenId, TokenId> > OrderingChoice;

  DECLARE_GLOBAL_CONST(std::string, g_ClassDelimiter);

#define CLASS_DELIMITER (EUROPA::g_ClassDelimiter())


}

#endif
