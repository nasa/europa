#ifndef _H_PlanDatabaseDefs
#define _H_PlanDatabaseDefs

#include "./ConstraintEngine/ConstraintEngineDefs.hh"

namespace Prototype {
  class Object;
  typedef Europa::Id<Object> ObjectId;

  class Token;
  typedef Europa::Id<Token> TokenId;

  class PlanDatabaseListener;
  typedef Europa::Id<PlanDatabaseListener> PlanDatabaseListenerId;

  template<class DomainType> class TokenVariable;
  typedef Europa::Id< TokenVariable<IntervalIntDomain> > TempVarId;

  typedef IntervalIntDomain BooleanDomain;
  typedef Europa::Id< TokenVariable<BooleanDomain> > BoolVarId; /*!< Typedef it as just an int domain for now and change later */

  class ObjectSet;
  typedef Europa::Id< TokenVariable<ObjectSet> > ObjectVarId;

  class Schema;
  typedef Europa::Id<Schema> SchemaId;

  class PlanDatabase;
  typedef Europa::Id<PlanDatabase> PlanDatabaseId;
}

#endif
