#ifndef _H_PlanDatabaseDefs
#define _H_PlanDatabaseDefs

#include "ConstraintEngineDefs.hh"
#include "Domain.hh"
#include "Entity.hh"

namespace Prototype {
  class Object;
  typedef Id<Object> ObjectId;

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
  typedef Id< TokenVariable<EnumeratedDomain> > StateVarId;
  
  class TokenTemporalVariable;
  typedef Id< TokenTemporalVariable > TempVarId;

  typedef Domain<ObjectId>  ObjectSet;
  typedef Id< TokenVariable<ObjectSet> > ObjectVarId;

  class Schema;
  typedef Id<Schema> SchemaId;

  class PlanDatabase;
  typedef Id<PlanDatabase> PlanDatabaseId;

  class TemporalAdvisor;
  typedef Id<TemporalAdvisor> TemporalAdvisorId;

  class DbClient;
  typedef Id<DbClient> DbClientId;

  typedef std::pair<LabelStr, AbstractDomain* > ConstructorArgument; /*!< Defines a pair for names and values of arguments for a constructor */
}

#endif
