#ifndef _H_CommonDefs
#define _H_CommonDefs

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

  DECLARE_GLOBAL_CONST(int, g_maxFiniteTime);
  DECLARE_GLOBAL_CONST(int, g_infiniteTime);
  DECLARE_GLOBAL_CONST(int, g_noTime);
  DECLARE_GLOBAL_CONST(double, g_epsilon);
  DECLARE_GLOBAL_CONST(int, g_infinity);
  DECLARE_GLOBAL_CONST(int, g_maxInt);

  /*!<  g_maxFiniteTime()  is 268435455 */

  #define MAX_INT g_maxInt()
  #define PLUS_INFINITY g_infiniteTime()
  #define MINUS_INFINITY -g_infiniteTime()
  #define EPSILON g_epsilon() /*!< Used when computing differences on REAL NUMBERS. Smallest increment */
}
#endif
