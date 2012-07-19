#ifndef _H_ConstraintEngineDefs
#define _H_ConstraintEngineDefs

/**
 * Just provide forward declarations of classes and certain global declarations related to error handling.
 */

#include "CommonDefs.hh"
#include "Id.hh"
#include "Entity.hh"

#include <list>
#include <set>

namespace EUROPA {
  class AbstractDomain;
  class IntervalDomain;
  class IntervalIntDomain;
  class BoolDomain;
  class EnumeratedDomain;

  class DomainListener;
  typedef Id<DomainListener> DomainListenerId;

  class ConstrainedVariable;
  typedef Id<ConstrainedVariable> ConstrainedVariableId;
  typedef std::set<ConstrainedVariableId, EntityComparator<ConstrainedVariableId> > ConstrainedVariableSet;

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

  /**
   * @brief Initialize all default elements of the module e.g. factories
   */
  void initConstraintEngine();

  /**
   * @brief Uninitialize all default elements of the module e.g. factories
   */
  void uninitConstraintEngine();

  /**
   * @brief Helper method to cast singleton values
   */
  template<class T>
  Id<T> id(const ConstrainedVariableId& var);
  
  class StringDomain;
  typedef StringDomain LabelSet;

  /** Constants **/
  DECLARE_GLOBAL_CONST(char*, g_noVarName);
  #define NO_VAR_NAME (g_noVarName())

} /* namespace EUROPA */

#endif /* #ifndef _H_ConstraintEngineDefs */
