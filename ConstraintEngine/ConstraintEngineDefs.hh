#ifndef _H_ConstraintEngineDefs
#define _H_ConstraintEngineDefs

/**
 * Just provide forward declarations of classes and certain global declarations related to error handling.
 */

#include "CommonDefs.hh"
#include "Id.hh"
#include "Entity.hh"

#include <list>

namespace Prototype {
  class AbstractDomain;
  class IntervalDomain;
  class IntervalIntDomain;
  class BoolDomain;
  class EnumeratedDomain;

  class DomainListener;
  typedef Id<DomainListener> DomainListenerId;

  class ConstrainedVariable;
  typedef Id<ConstrainedVariable> ConstrainedVariableId;
  class ConstrainedVariableListener;
  typedef Id<ConstrainedVariableListener> ConstrainedVariableListenerId;
  class VariableChangeListener;
  typedef Id<VariableChangeListener> VariableChangeListenerId;

  class Constraint;
  typedef Id<Constraint> ConstraintId;
  typedef std::set<ConstraintId, EntityComparator<ConstraintId> > ConstraintSet;
  class Propagator;
  typedef Id<Propagator> PropagatorId;
  class ConstraintEngine;
  typedef Id<ConstraintEngine> ConstraintEngineId;
  class ConstraintEngineListener;
  typedef Id<ConstraintEngineListener> ConstraintEngineListenerId;
  typedef std::pair<ConstraintId, int> ConstraintEntry;
  typedef std::list<ConstraintEntry> ConstraintList;
} /* namespace Prototype */

#endif /* #ifndef _H_ConstraintEngineDefs */
