#ifndef _H_ConstraintEngineDefs
#define _H_ConstraintEngineDefs

/**
 * Just provide forward declarations of classes and certain global declarations related to error handling.
 */

#include "CommonDefs.hh"
#include "Id.hh"
#include <list>
#include <cassert>

/**
 * @brief Condition indicating the surrounding call to check_error() or similar should always fail.
 * @note Should only be used as an argument to check_error(), assert(), assertTrue(), etc.
 * @note Note also that assert() should only be used in test programs and not even there in the long term,
 *   since some compilers implement it using a #define, which prevents setting a break point in it.
 * --wedgingt 2004 Mar 3
 * @note Why is this a C++ variable rather than a #define? --wedgingt 2004 Mar 3
 */
const bool ALWAYS_FAILS = false;

/**
 * @def check_error
 * @brief If the condition is false, generate an error.
 * @param cond The condition to test.
 * @note Current, preliminary, implementation simply calls assert().
 * @note Once the new error handling support is is use, this will instead
 *   throw an exception when the condition is false.
 * @note Should only be used in 'core' code and in test programs when the
 *   condition cannot be tested when the 'core' code has been compiled 'fast'.
 *   Otherwise - in test programs where the condition can always be checked,
 *   no matter how the code was compiled - use assertTrue(), assertFalse(), etc.
 * @see assertTrue, assertFalse, ALWAYS_FAIL
 */
#ifndef PROTOTYPE_FAST_VERSION
#define check_error(cond) assert(cond)
#else
#define check_error(cond)
#endif

/**
 * @def assertTrue
 * @brief Require the condition to be true, aborting the program otherwise.
 * @note Should only be used in test programs.
 * @note Likely precursor to new error handling support.
 */
#define assertTrue(cond) assert(cond)

/**
 * @def assertFalse
 * @brief Require the condition to be false, aborting the program otherwise.
 * @note Should only be used in test programs.
 * @note Likely precursor to new error handling support.
 */
#define assertFalse(cond) assert(!(cond))

namespace Prototype {
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
} /* namespace Prototype */

#endif /* #ifndef _H_ConstraintEngineDefs */
