#ifndef _H_CommonDefs
#define _H_CommonDefs

#include <cassert>


//#define PROTOTYPE_FAST_VERSION

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

// The C++ standard changed; use std::stringstream and streamIsEmpty()
// to be compatible with compilers that are compliant to different versions.
#ifdef __GNUC__
#if __GNUC__ > 2
#include <sstream>
//typedef std::stringstream sstream;
#define streamIsEmpty(s) (s).str() == ""
#else
#include <strstream>
typedef std::strstream std::stringstream;
#define streamIsEmpty(s) !(s).str()
#endif
#else
#include <strstream>
typedef std::strstream std::stringstream;
#define streamIsEmpty(s) !(s).str()
#endif

namespace Prototype {

  /**
   * @def DECLARE_GLOBAL_CONST(TYPE,NAME)
   * @brief Declare a global constant via a global function to ensure initialization
   * occurs before use with all linkers.
   */
#define DECLARE_GLOBAL_CONST(TYPE, NAME) \
  extern const TYPE& NAME();

  /**
   * @def DEFINE_GLOBAL_CONST(TYPE,NAME,VALUE)
   * @brief Define a global constant to have the given value via a
   * global function to ensure initialization occurs before use with all
   * linkers.
   */
#define DEFINE_GLOBAL_CONST(TYPE, NAME, VALUE) \
  const TYPE& NAME() { \
    static const TYPE sl_data(VALUE); \
    return sl_data; \
  }

  /**
   * @def DEFINE_GLOBAL_EMPTY_CONST(TYPE,NAME)
   * @brief Define a global constant via a global function to ensure
   * initialization occurs before use with all linkers.
   */
#define DEFINE_GLOBAL_EMPTY_CONST(TYPE, NAME) \
  const TYPE& NAME() { \
    static const TYPE sl_data; \
    return sl_data; \
  }

  DECLARE_GLOBAL_CONST(int, g_maxFiniteTime); /**< 268435455 */
  DECLARE_GLOBAL_CONST(int, g_infiniteTime);
  DECLARE_GLOBAL_CONST(int, g_noTime);
  DECLARE_GLOBAL_CONST(double, g_epsilon);
  DECLARE_GLOBAL_CONST(int, g_infinity);
  DECLARE_GLOBAL_CONST(int, g_maxInt);

  #define MAX_INT (g_maxInt())
  #define PLUS_INFINITY (g_infiniteTime())
  #define MINUS_INFINITY (-g_infiniteTime())
  #define EPSILON (g_epsilon()) /*!< Used when computing differences and comparing real numbers: smallest recognized increment. */

  /*!< Magic number to enforce assumption that memory addresses and string keys will never collide */
  #define ASSUMED_MINIMUM_MEMORY_ADDRESS 1000000 
  #define MAXIMUM_STRING_COUNT ASSUMED_MINIMUM_MEMORY_ADDRESS 
}
#endif
