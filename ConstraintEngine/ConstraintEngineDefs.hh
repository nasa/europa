#ifndef _H_ConstraintEngineDefs
#define _H_ConstraintEngineDefs

/**
 * Just provide forward declarations of classes
 */

#include "Id.hh"
#include <list>
#include <cassert>

const bool ALWAYS_FAILS = false;

#ifndef PROTOTYPE_FAST_VERSION
#define check_error(cond) assert(cond);
#else
#define check_error(cond)
#endif

namespace Prototype
{
  static const int MAX_INT = 1999999999;
  static const int PLUS_INFINITY = MAX_INT;
  static const int MINUS_INFINITY = -PLUS_INFINITY;
  static const double EPSILON = 0.00001; /*!< Used when computing differences on REAL NUMBERS. Smallest increment */

  class Entity;
  typedef Id<Entity> EntityId;

  class AbstractDomain;
  class IntervalDomain;
  class IntervalIntDomain;
  class BoolDomain;
  class EnumeratedDomain;

  class DomainListener;
  typedef Id<DomainListener> DomainListenerId;
  class ConstrainedVariable;
  typedef Id<ConstrainedVariable> ConstrainedVariableId;
  class VariableChangeListener;
  typedef Id<VariableChangeListener> VariableChangeListenerId;

  class Constraint;
  typedef Id<Constraint> ConstraintId;
  class UnaryConstraint;
  typedef Id<UnaryConstraint> UnaryConstraintId;
  class Propagator;
  typedef Id<Propagator> PropagatorId;
  class ConstraintEngine;
  typedef Id<ConstraintEngine> ConstraintEngineId;
  class ConstraintEngineListener;
  typedef Id<ConstraintEngineListener> ConstraintEngineListenerId;
  typedef std::pair<ConstraintId, int> ConstraintEntry;
  typedef std::list<ConstraintEntry> ConstraintList;
}// End namespace
#endif
