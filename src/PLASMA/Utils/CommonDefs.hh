#ifndef H_CommonDefs
#define H_CommonDefs

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
    return(sl_data); \
  }

#define ALWAYS_FAILS (false)


#ifdef _MSC_VER
#  define EUROPA_ATTRIBUTE_UNUSED
#else
#  define EUROPA_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
#endif 

#ifdef _MSC_VER
#  define SPRINTF_FUNC sprintf_s
#else
#  define SPRINTF_FUNC sprintf
#endif
#endif
