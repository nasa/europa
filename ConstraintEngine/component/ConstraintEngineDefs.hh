#ifndef _H_ConstraintEngineDefs
#define _H_ConstraintEngineDefs

/**
 * Just provide forward declarations of classes
 */

#include "Id.hh"
#include <list>
#include <cassert>

//#define _PROTOTYPE_FAST_VALUE_

const bool ALWAYS_FAILS = false;

#ifndef _PROTOTYPE_FAST_VALUE_
#define check_error(cond) assert(cond);
#else
#define check_error(cond)
#endif

namespace Prototype
{

  static const int MAX_INT = 9999999;
  static const int PLUS_INFINITY = MAX_INT;
  static const int MINUS_INFINITY = -PLUS_INFINITY;

  class AbstractDomain;
  class IntervalIntDomain;
  class IntervalRealDomain;
  class LabelSet;
  class BoolDomain;

  class DomainListener;
  typedef Europa::Id<DomainListener> DomainListenerId;
  class ConstrainedVariable;
  typedef Europa::Id<ConstrainedVariable> ConstrainedVariableId;
  class VariableChangeListener;
  typedef Europa::Id<VariableChangeListener> VariableChangeListenerId;

  class Constraint;
  typedef Europa::Id<Constraint> ConstraintId;
  class Propagator;
  typedef Europa::Id<Propagator> PropagatorId;
  class ConstraintEngine;
  typedef Europa::Id<ConstraintEngine> ConstraintEngineId;
  typedef std::pair<ConstraintId, int> ConstraintEntry;
  typedef std::list<ConstraintEntry> ConstraintList;
}// End namespace
#endif
