#ifndef _H_Number
#define _H_Number
#include <iomanip>

#include <cassert>
#include <cmath>
#include <istream>
#include <ostream>
#include <limits>
#include <stdexcept>
#include "hash_map.hh"

#ifdef _MSC_VER
#  if ( LONG_MAX > INT_MAX)
#    define E2_LONG_INT
#  else
#    undef E2_LONG_INT
#  endif
#else
#  if (__LONG_MAX__ > __INT_MAX__)
#    define E2_LONG_INT
#  else
#    undef E2_LONG_INT
#  endif
#endif //_MSC_VER

#ifdef _MSC_VER
#  if ( DBL_MANT_DIG >= 64 )
#    define E2_LONG_DOUBLE
#  else
#    undef E2_LONG_DOUBLE
#  endif
#else
#  if (__DBL_MANT_DIG__ >= 64)
#    define E2_LONG_DOUBLE
#  else
#    undef E2_LONG_DOUBLE
#  endif
#endif //_MSC_VER

/**
 * Pre-declarations.
 */

#define MAX_COPY max
#define MIN_COPY min
#undef min
#undef max

namespace EUROPA {
  class eint;
  class edouble;
}

#ifndef _MSC_VER
namespace __gnu_cxx
{
  template<> struct hash<EUROPA::edouble> {
    inline size_t operator()(EUROPA::edouble __x) const;
  };

  template<> struct hash<EUROPA::eint> {
    inline size_t operator()(EUROPA::eint __x) const;
  };
}
#endif // _MSC_VER

namespace std {
  template<>
  struct numeric_limits<EUROPA::eint> {
    static const bool is_specialized = true;
    //I'm not sure if these numbers are correct, so be careful
#ifdef E2_LONG_INT
#  ifdef _MSC_VER
    static const int digits = DBL_MANT_DIG;
    static const int digits10 = DBL_DIG;
#  else
    static const int digits = __DBL_MANT_DIG__;
    static const int digits10 = __DBL_DIG__;
#  endif //_MSC_VER
#else
    static const int digits = numeric_limits<long>::digits;
    static const int digits10 = numeric_limits<long>::digits10;
#endif // E2_LONG_INT
    static const bool is_signed = true;
    static const bool is_integer = true;
    static const bool is_exact = true;
    static const int radix = 2;
    static const int min_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent = 0;
    static const int max_exponent10 = 0;
    static const bool has_infinity = true;
    static const bool has_quiet_NaN = false;
    static const bool has_signaling_NaN = false;
    static const float_denorm_style has_denorm = denorm_absent;
    static const bool has_denorm_loss = false;
    static const bool is_iec559 = false;
    static const bool is_bounded = true;
    static const bool is_modulo = false;
#ifdef _MSC_VER
    static const bool traps = numeric_limits<long>::traps;
#else
    static const bool traps = __glibcxx_integral_traps;
#endif //_MSC_VER
    static const bool tinyness_before = false;
    static const float_round_style round_style = round_toward_zero;

    inline static EUROPA::eint min() throw();
    inline static EUROPA::eint max() throw();
    inline static EUROPA::eint epsilon() throw();
    inline static EUROPA::eint round_error() throw();
    inline static EUROPA::eint infinity() throw();
    inline static EUROPA::eint minus_infinity() throw();
    inline static EUROPA::eint quiet_NaN() throw();
    inline static EUROPA::eint signaling_NaN() throw();
    inline static EUROPA::eint denorm_min() throw();
  };

  template<>
  struct numeric_limits<EUROPA::edouble> {
    static const bool is_specialized = true;
    //I'm not sure if these numbers are correct, so be careful
#ifdef E2_LONG_INT
#  ifdef _MSC_VER
    static const int digits = DBL_MANT_DIG;
    static const int digits10 = DBL_DIG;
#  else
    static const int digits = __DBL_MANT_DIG__;
    static const int digits10 = __DBL_DIG__;
#  endif //_MSC_VER
#else
    static const int digits = numeric_limits<long>::digits;
    static const int digits10 = numeric_limits<long>::digits10;
#endif
    static const bool is_signed = true;
    static const bool is_integer = false;
    static const bool is_exact = false;
#ifdef _MSC_VER
    static const int radix = numeric_limits<float>::radix;
#else
    static const int radix = __FLT_RADIX__;
#endif //_MSC_VER
    //Not sure what these numbers should be
    static const int min_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent = 0;
    static const int max_exponent10 = 0;

    static const bool has_infinity = true;
#ifdef _MSC_VER
    static const bool has_quiet_NaN = numeric_limits<float>::has_quiet_NaN;
#else
    static const bool has_quiet_NaN = __DBL_HAS_QUIET_NAN__;
#endif //_MSC_VER
    static const bool has_signaling_NaN = has_quiet_NaN;
    static const float_denorm_style has_denorm = denorm_absent;
    static const bool has_denorm_loss = false;
    static const bool is_iec559 = has_infinity && has_quiet_NaN && has_denorm == denorm_present;
    static const bool is_bounded = true;
    static const bool is_modulo = false;
#ifdef _MSC_VER
    static const bool traps = numeric_limits<float>::traps;
#else
    static const bool traps = __glibcxx_integral_traps;
#endif //_MSC_VER
    static const bool tinyness_before = numeric_limits<double>::tinyness_before;
    static const float_round_style round_style = round_to_nearest;

    inline static EUROPA::edouble min() throw();
    inline static EUROPA::edouble max() throw();
    inline static EUROPA::edouble epsilon() throw();
    inline static EUROPA::edouble round_error() throw();
    inline static EUROPA::edouble infinity() throw();
    inline static EUROPA::edouble minus_infinity() throw();
    inline static EUROPA::edouble quiet_NaN() throw();
    inline static EUROPA::edouble signaling_NaN() throw();
    inline static EUROPA::edouble denorm_min() throw();
  };

  inline EUROPA::edouble abs(const EUROPA::edouble d);
  inline EUROPA::edouble sqrt(const EUROPA::edouble d);
  inline EUROPA::edouble pow(const EUROPA::edouble d, const EUROPA::eint i);
  inline EUROPA::edouble sin(const EUROPA::edouble d);
  inline EUROPA::edouble ceil(const EUROPA::edouble d);
  inline EUROPA::edouble floor(const EUROPA::edouble d);

  inline EUROPA::edouble max(const EUROPA::edouble a, const EUROPA::edouble b);
  inline EUROPA::edouble min(const EUROPA::edouble a, const EUROPA::edouble b);
}



namespace EUROPA {

  class edouble;

#define handle_inf_unary(type, v) {                             \
  if(v >= std::numeric_limits<type>::infinity())                \
    return std::numeric_limits<type>::infinity();               \
  else if(v <= std::numeric_limits<type>::minus_infinity())     \
    return std::numeric_limits<type>::minus_infinity();         \
}

#define handle_inf_add(type, v1, v2) {                          \
  if(((type::basis_type) v1) >= std::numeric_limits<type>::infinity()) {             \
    if(((type::basis_type) v2) > std::numeric_limits<type>::minus_infinity())        \
      return std::numeric_limits<type>::infinity();             \
        }                                                       \
  else if(((type::basis_type) v1) <= std::numeric_limits<type>::minus_infinity()) {  \
    if(((type::basis_type) v2) < std::numeric_limits<type>::infinity())              \
      return std::numeric_limits<type>::minus_infinity();       \
  }                                                             \
  if(((type::basis_type) v2) >= std::numeric_limits<type>::infinity()) {             \
    if(((type::basis_type) v1) > std::numeric_limits<type>::minus_infinity())        \
      return std::numeric_limits<type>::infinity();             \
  }                                                             \
  else if(((type::basis_type) v2) <= std::numeric_limits<type>::minus_infinity()) {  \
    if(((type::basis_type) v1) < std::numeric_limits<type>::infinity())              \
      return std::numeric_limits<type>::minus_infinity();       \
  }                                                             \
}

#define handle_inf_sub(type, v1, v2) {                          \
  if(((type::basis_type) v1) >= std::numeric_limits<type>::infinity()) {             \
    if(((type::basis_type) v2) < std::numeric_limits<type>::infinity())              \
      return std::numeric_limits<type>::infinity();             \
  }                                                             \
  else if(((type::basis_type) v1) <= std::numeric_limits<type>::minus_infinity()) {  \
    if(((type::basis_type) v2) > std::numeric_limits<type>::minus_infinity())        \
      return std::numeric_limits<type>::minus_infinity();       \
  }                                                             \
  if(((type::basis_type) v2) >= std::numeric_limits<type>::infinity()) {             \
    if(((type::basis_type) v1) < std::numeric_limits<type>::infinity())              \
      return std::numeric_limits<type>::minus_infinity();       \
  }                                                             \
  else if(((type::basis_type) v2) <= std::numeric_limits<type>::minus_infinity()) {  \
    if(((type::basis_type) v1) > std::numeric_limits<type>::minus_infinity())        \
      return std::numeric_limits<type>::infinity();             \
  }                                                             \
}

#define handle_inf_mul(type, v1, v2) {                          \
  if(((type::basis_type) v1) >= std::numeric_limits<type>::infinity()) {             \
    if(((type::basis_type) v2) > 0)                                                  \
      return std::numeric_limits<type>::infinity();             \
    else if(((type::basis_type) v2) < 0)                                             \
      return std::numeric_limits<type>::minus_infinity();       \
  }                                                             \
  else if(((type::basis_type) v1) <= std::numeric_limits<type>::minus_infinity()) {  \
    if(((type::basis_type) v2) > 0)                                                  \
      return std::numeric_limits<type>::minus_infinity();       \
    else if(((type::basis_type) v2) < 0)                                             \
      return std::numeric_limits<type>::infinity();             \
  }                                                             \
  if(((type::basis_type) v2) >= std::numeric_limits<type>::infinity()) {             \
    if(((type::basis_type) v1) > 0)                                                  \
      return std::numeric_limits<type>::infinity();             \
    else if(((type::basis_type) v1) < 0)                                             \
      return std::numeric_limits<type>::minus_infinity();       \
  }                                                             \
  else if(((type::basis_type) v2) <= std::numeric_limits<type>::minus_infinity()) {  \
    if(((type::basis_type) v1) > 0)                                                  \
      return std::numeric_limits<type>::minus_infinity();       \
    else if(((type::basis_type) v1) < 0)                                             \
      return std::numeric_limits<type>::infinity();             \
  }                                                             \
}

#define handle_inf_div(type, v1, v2) {                                  \
  if(((type::basis_type) v1) >= std::numeric_limits<type>::infinity()) {                     \
    if(((type::basis_type) v2) > 0 && ((type::basis_type) v2) < std::numeric_limits<type>::infinity())            \
      return std::numeric_limits<type>::infinity();                     \
    else if(((type::basis_type) v2) < 0 && ((type::basis_type) v2) > std::numeric_limits<type>::minus_infinity()) \
      return std::numeric_limits<type>::minus_infinity();               \
  }                                                                     \
  else if(((type::basis_type) v1) <= std::numeric_limits<type>::minus_infinity()) {          \
    if(((type::basis_type) v2) > 0 && ((type::basis_type) v2) < std::numeric_limits<type>::infinity())            \
      return std::numeric_limits<type>::minus_infinity();               \
    else if(((type::basis_type) v2) < 0 && ((type::basis_type) v2) > std::numeric_limits<type>::minus_infinity()) \
      return std::numeric_limits<type>::infinity();                     \
  }                                                                     \
  else if(((type::basis_type) v2) >= std::numeric_limits<type>::infinity() || ((type::basis_type) v2) <= std::numeric_limits<type>::minus_infinity()) { \
    if(((type::basis_type) v1) < std::numeric_limits<type>::infinity() && ((type::basis_type) v1) > std::numeric_limits<type>::minus_infinity()) \
      return type(0, true);                                             \
  }                                                                     \
}

#define handle_inf_mod(type, v1, v2) {                                  \
  if(((type::basis_type) v1) >= std::numeric_limits<type>::infinity() || ((type::basis_type) v1) <= std::numeric_limits<type>::minus_infinity()) \
    return type(0, true);                                               \
  else if(((type::basis_type) v2) == std::numeric_limits<type>::infinity())                  \
    return type(((type::basis_type) v1), true);                                              \
  else if(((type::basis_type) v2) == std::numeric_limits<type>::minus_infinity())            \
    return type(-((type::basis_type) v1), true);                                             \
}

  /**
   * For the moment, doing overflow checking by up-promoting.  There are other ways to achieve this that might be more performant.
   */
#ifndef NO_OVERFLOW_CHECKING
#define op(type, a, x, b) {                                             \
  double temp = ((double)(a)) x (b);                                    \
  if(temp > std::numeric_limits<type>::infinity()) {                    \
    throw std::overflow_error("greater-than-infinity error"); \
  }                                                                     \
  else if(temp < std::numeric_limits<type>::minus_infinity()) {         \
    throw std::underflow_error("less-than-minus-infinity error"); \
  }                                                                     \
  return type((type::basis_type)temp, true);                            \
}
#else
#define op(type, a, x, b) return (a) x (b)
#endif

  //it feels a bit dirty doing this this way.  I don't want to make this code un-readable because of all the macros, but I also
  //don't want to make it un-readable because of all the repetition
#define GEN_COMPARISONS(type)                                   \
  inline bool operator<(const type o) {return m_v < o;}         \
  inline bool operator<=(const type o) {return m_v <= o;}       \
  inline bool operator==(const type o) {return m_v == o;}       \
  inline bool operator>=(const type o) {return m_v >= o;}       \
  inline bool operator>(const type o) {return m_v > o;}       \
  inline bool operator!=(const type o) {return m_v != o;}

#define DECL_FRIEND_COMPARISONS(type1, type2)           \
  friend bool operator<(const type1 o, const type2 e);  \
  friend bool operator<=(const type1 o, const type2 e); \
  friend bool operator==(const type1 o, const type2 e); \
  friend bool operator>=(const type1 o, const type2 e); \
  friend bool operator>(const type1 o, const type2 e); \
  friend bool operator!=(const type1 o, const type2 e);

#define GEN_FRIEND_COMPARISONS(type1, type2)                            \
  inline bool operator<(const type1 o, const type2 e) {return o < e.m_v;} \
  inline bool operator<=(const type1 o, const type2 e) {return o <= e.m_v;} \
  inline bool operator==(const type1 o, const type2 e) {return o == e.m_v;} \
  inline bool operator>=(const type1 o, const type2 e) {return o >= e.m_v;} \
  inline bool operator>(const type1 o, const type2 e) {return o > e.m_v;} \
  inline bool operator!=(const type1 o, const type2 e) {return o != e.m_v;}
  
  /**
   * Rationale:
   *
   * EUROPA uses a single representation for numbers, but is multi-platform and so must support differences in the sizes of 
   * numeric types, as well as requiring particular algebraic behaviors of special values like positive and negative infinity.
   * The original representation was an IEEE 754 64-bit double with infinities defined at +/- (MAX_INT/8) and use-point code for
   * dealing with infinities.
   *
   * Caveats:
   *  There is a potential ambiguity with the number 0, because it freely converts to a void*, which can be treated as a reference,
   *  despite the fact that it's supposed to be hard to make references point to NULL...
   *  In this case:
   *
   *  class foo {
   *  public:
   *    foo(eint x);
   *    foo(const std::string& y);
   *  };
   *
   *  foo(0);
   *
   *  It is, type-wise, exactly as expensive to convert 0 to an eint as to a reference, so you have to explicitly disambiguate:
   *
   *  foo(eint(0));
   */

  class eint {
  public:
    typedef long basis_type;

    eint(const int v) : m_v(v) {
      if(m_v > std::numeric_limits<eint>::infinity()) {
        assert(m_v <= std::numeric_limits<eint>::infinity());
        m_v = cast_int(std::numeric_limits<eint>::infinity());
      }
      else if(m_v < std::numeric_limits<eint>::minus_infinity()) {
        assert(m_v >= std::numeric_limits<eint>::minus_infinity());
        m_v = cast_int(std::numeric_limits<eint>::minus_infinity());
      }
    }
    eint(const long v) : m_v(v) { //this should maybe warn of loss of precision on 64-bit platforms?
      if(m_v > std::numeric_limits<eint>::infinity()) {
        assert(m_v <= std::numeric_limits<eint>::infinity());
        m_v = cast_int(std::numeric_limits<eint>::infinity());
      }
      else if(m_v < std::numeric_limits<eint>::minus_infinity()) {
        assert(m_v >= std::numeric_limits<eint>::minus_infinity());
        m_v = cast_int(std::numeric_limits<eint>::minus_infinity());
      }

    } 
    eint(const unsigned int v) : m_v((long)v) {
      if(m_v > std::numeric_limits<eint>::infinity()) {
        assert(m_v <= std::numeric_limits<eint>::infinity());
        m_v = cast_int(std::numeric_limits<eint>::infinity());
      }
      else if(m_v < std::numeric_limits<eint>::minus_infinity()) {
        assert(m_v >= std::numeric_limits<eint>::minus_infinity());
        m_v = cast_int(std::numeric_limits<eint>::minus_infinity());
      }
    }
    eint(const unsigned long v) : m_v((long)v) {
      if(m_v > std::numeric_limits<eint>::infinity()) {
        assert(m_v <= std::numeric_limits<eint>::infinity());
        assert(m_v >= std::numeric_limits<eint>::minus_infinity());
        m_v = cast_int(std::numeric_limits<eint>::infinity());
      }
      else if(m_v < std::numeric_limits<eint>::minus_infinity()) {
        assert(m_v >= std::numeric_limits<eint>::minus_infinity());
        m_v = cast_int(std::numeric_limits<eint>::minus_infinity());
      }
    }

#ifdef _MSC_VER 
#  ifdef E2_LONG_INT 
    eint(const __int64 v) : m_v( (__int64) v ) {
        if(m_v > std::numeric_limits<eint>::infinity()) {
            assert(m_v <= std::numeric_limits<eint>::infinity());
            assert(m_v >= std::numeric_limits<eint>::minus_infinity());
            m_v = cast_int(std::numeric_limits<eint>::infinity());
        }
        else if(m_v < std::numeric_limits<eint>::minus_infinity()) {
            assert(m_v >= std::numeric_limits<eint>::minus_infinity());
            m_v = cast_int(std::numeric_limits<eint>::minus_infinity());
        }
    }
    eint(const unsigned __int64 v) : m_v( (unsigned __int64) v ) {
        if(m_v > std::numeric_limits<eint>::infinity()) {
            assert(m_v <= std::numeric_limits<eint>::infinity());
            assert(m_v >= std::numeric_limits<eint>::minus_infinity());
            m_v = cast_int(std::numeric_limits<eint>::infinity());
        }
        else if(m_v < std::numeric_limits<eint>::minus_infinity()) {
            assert(m_v >= std::numeric_limits<eint>::minus_infinity());
            m_v = cast_int(std::numeric_limits<eint>::minus_infinity());
        }
    }
#  endif //#ifdef E2_LONG_INT
#endif //#ifdef _MSC_VER

    eint() : m_v(0) {}
    inline eint operator+() const {return eint(+m_v, true);}
    inline eint operator-() const {return eint(-m_v, true);}
    inline bool operator!() const {return !m_v;}
    inline eint operator++() {handle_inf_unary(eint, m_v); return eint(++m_v, true);}
    inline eint operator++(int) {handle_inf_unary(eint, m_v); return eint(m_v++, true);}
    inline eint operator--() {handle_inf_unary(eint, m_v); return eint(--m_v, true);}
    inline eint operator--(int) {handle_inf_unary(eint, m_v); return eint(m_v--, true);}
    inline eint operator+(const basis_type o) const {handle_inf_add(eint, m_v, o); op(eint, m_v, +, o);}
    inline eint operator+(const eint o) const {return operator+(o.m_v);}
    inline eint operator-(const basis_type o) const {handle_inf_sub(eint, m_v, o); op(eint, m_v, -, o);}
    inline eint operator-(const eint o) const {return operator-(o.m_v);}
    inline eint operator*(const basis_type o) const {handle_inf_mul(eint, m_v, o); op(eint, m_v, *, o);}
    inline eint operator*(const eint o) const {return operator*(o.m_v);}
    inline eint operator/(const basis_type o) const {handle_inf_div(eint, m_v, o); op(eint, m_v, /, o);}
    inline eint operator/(const eint o) const {return operator/(o.m_v);}

    //have to special-case this, since % isn't defined on doubles
    inline eint operator%(const basis_type o) const {
      handle_inf_mod(eint, m_v, o);
      return eint(m_v % o, true);
    }
    inline eint operator%(const eint o) const {return operator%(o.m_v);}

    //these could be optimized, unless the g++ optimizer is pretty smart
    inline eint operator+=(const basis_type o) {(*this) = operator+(o); return eint(m_v, true);}
    inline eint operator+=(const eint o) {(*this) = operator+(o); return eint(m_v, true);}
    inline eint operator-=(const basis_type o) {(*this) = operator-(o); return eint(m_v, true);}
    inline eint operator-=(const eint o) {(*this) = operator-(o); return eint(m_v, true);}
    inline eint operator*=(const basis_type o) {(*this) = operator*(o); return eint(m_v, true);}
    inline eint operator*=(const eint o) {(*this) = operator*(o); return eint(m_v, true);}
    inline eint operator/=(const basis_type o) {(*this) = operator/(o); return eint(m_v, true);}
    inline eint operator/=(const eint o) {(*this) = operator/(o); return eint(m_v, true);}
    inline eint operator%=(const eint o) {(*this) = operator%(o); return eint(m_v, true);}
    inline eint operator%=(const basis_type o) {(*this) = operator%(o); return eint(m_v, true);}

    GEN_COMPARISONS(int);
    GEN_COMPARISONS(long);
    GEN_COMPARISONS(long long int);
    GEN_COMPARISONS(double);
    GEN_COMPARISONS(long double);

    inline bool operator<(const eint o) const {return m_v < o.m_v;}
    inline bool operator<=(const eint o) const {return m_v <= o.m_v;}
    inline bool operator==(const eint o) const {return m_v == o.m_v;}
    inline bool operator>=(const eint o) const {return m_v >= o.m_v;}
    inline bool operator>(const eint o) const {return m_v > o.m_v;}
    inline bool operator!=(const eint o) const {return m_v != o.m_v;}

    inline long asLong() const {return m_v;}

    inline edouble operator+(const edouble o) const;
    inline edouble operator-(const edouble o) const;
    inline edouble operator*(const edouble o) const;
    inline edouble operator/(const edouble o) const;
    inline bool operator<(const double o) const;
    inline bool operator<(const edouble o) const;
    inline bool operator<=(const double o) const;
    inline bool operator<=(const edouble o) const;
    inline bool operator==(const double o) const;
    inline bool operator==(const edouble o) const;
    inline bool operator>=(const double o) const;
    inline bool operator>=(const edouble o) const;
    inline bool operator>(const double o) const;
    inline bool operator>(const edouble o) const;
    inline bool operator!=(const double o) const;
    inline bool operator!=(const edouble o) const;

    friend eint::basis_type cast_int(const eint e);
    friend long cast_long(const eint e);
    friend double cast_double(const eint e);
    friend long long int cast_llong(const eint e);
    friend long double cast_ldouble(const eint e);
    friend eint::basis_type cast_basis(const eint e);

  private:
    
    friend class edouble;

#ifdef _MSC_VER
    friend struct std::numeric_limits<eint>;
#else
    friend class __gnu_cxx::hash<eint>;
    friend class std::numeric_limits<eint>;
#endif //_MSC_VER
  
    /**
     * Private constructors that don't do infinity checking
     */
    eint(const int v, bool) : m_v(v) {}
    eint(const unsigned int v, bool) : m_v(v) {}
    eint(const long v, bool) : m_v(v) {}
    eint(const unsigned long v, bool) : m_v(v) {}

    friend eint operator+(const long o, const eint e);
    friend eint operator-(const long o, const eint e);
    friend eint operator*(const long o, const eint e);
    friend eint operator/(const long o, const eint e);
    friend eint operator%(const long o, const eint e);
    DECL_FRIEND_COMPARISONS(int, eint);
    DECL_FRIEND_COMPARISONS(long, eint);
    DECL_FRIEND_COMPARISONS(long long int, eint);
    DECL_FRIEND_COMPARISONS(double, eint);
    DECL_FRIEND_COMPARISONS(long double, eint);
    friend std::ostream& operator<<(std::ostream& o, const eint e);
    friend std::istream& operator>>(std::istream& i, eint& e);

    friend edouble std::pow(const EUROPA::edouble d, const eint i);

    long m_v;
  };  //class eint

  inline eint operator+(const long o, const eint e) {handle_inf_add(eint, o, e.m_v); op(eint, o, +, e.m_v);}
  inline eint operator-(const long o, const eint e) {handle_inf_sub(eint, o, e.m_v); op(eint, o, -, e.m_v);}
  inline eint operator*(const long o, const eint e) {handle_inf_mul(eint, o, e.m_v); op(eint, o, *, e.m_v);}
  inline eint operator/(const long o, const eint e) {handle_inf_div(eint, o, e.m_v); op(eint, o, /, e.m_v);}
  inline eint operator%(const long o, const eint e) {
    handle_inf_mod(eint, o, e.m_v);
    return eint(o % e.m_v, true);
  }
  GEN_FRIEND_COMPARISONS(int, eint);
  GEN_FRIEND_COMPARISONS(long, eint);
  GEN_FRIEND_COMPARISONS(long long int, eint);
  GEN_FRIEND_COMPARISONS(double, eint);
  GEN_FRIEND_COMPARISONS(long double, eint);
  inline std::ostream& operator<<(std::ostream& o, const eint e) {return(o << e.m_v);}
  inline std::istream& operator>>(std::istream& o, eint& e) {return(o >> e.m_v);}

  class edouble {
  public:
    typedef double basis_type;

    edouble(const eint v) : m_v(v.m_v) {} //don't have to check infinities with eints
    edouble(const double v) : m_v(v) {
      if(m_v > std::numeric_limits<edouble>::infinity()) {
        assert(m_v <= std::numeric_limits<edouble>::infinity());
        m_v = cast_double(std::numeric_limits<edouble>::infinity());
      }
      else if(m_v < std::numeric_limits<edouble>::minus_infinity()) {
        assert(m_v >= std::numeric_limits<eint>::minus_infinity());
        m_v = cast_double(std::numeric_limits<edouble>::minus_infinity());
      }
    }
    edouble() : m_v(0.0) {}
    inline edouble operator+() const {return edouble(+m_v, true);}
    inline edouble operator-() const {return edouble(-m_v, true);}
    inline bool operator!() const {return !m_v;}
    inline edouble operator++() {handle_inf_unary(edouble, m_v); return edouble(++m_v, true);}
    inline edouble operator++(int) {handle_inf_unary(edouble, m_v); return edouble(m_v++, true);}
    inline edouble operator--() {handle_inf_unary(edouble, m_v); return edouble(--m_v, true);}
    inline edouble operator--(int) {handle_inf_unary(edouble, m_v); return edouble(m_v--, true);}
    inline edouble operator+(const edouble o) const {handle_inf_add(edouble, m_v, o.m_v); op(edouble, m_v, +, o.m_v);}
    inline edouble operator-(const edouble o) const {handle_inf_sub(edouble, m_v, o.m_v); op(edouble, m_v, -, o.m_v);}
    inline edouble operator*(const edouble o) const {handle_inf_mul(edouble, m_v, o.m_v); op(edouble, m_v, *, o.m_v);}
    inline edouble operator/(const edouble o) const {handle_inf_div(edouble, m_v, o.m_v); op(edouble, m_v, /, o.m_v);}
    inline edouble operator+=(const edouble o) {(*this) = operator+(o); return edouble(m_v, true);}
    inline edouble operator-=(const edouble o) {(*this) = operator-(o); return edouble(m_v, true);}
    inline edouble operator*=(const edouble o) {(*this) = operator*(o); return edouble(m_v, true);}
    inline edouble operator/=(const edouble o) {(*this) = operator/(o); return edouble(m_v, true);}

    inline bool operator<(const edouble o) const {return m_v < o.m_v;}
    inline bool operator<=(const edouble o) const {return m_v <= o.m_v;}
    inline bool operator==(const edouble o) const {return m_v == o.m_v;}
    inline bool operator>=(const edouble o) const {return m_v >= o.m_v;}
    inline bool operator>(const edouble o) const {return m_v > o.m_v;}
    inline bool operator!=(const edouble o) const {return m_v != o.m_v;}
    
    GEN_COMPARISONS(double);
    GEN_COMPARISONS(long);
    GEN_COMPARISONS(int);
    GEN_COMPARISONS(unsigned int);
    GEN_COMPARISONS(long long int);
    GEN_COMPARISONS(long double);

    inline operator eint() const {return eint((long)m_v);} //is this a good idea?

    friend eint::basis_type cast_int(const edouble e);
    friend long cast_long(const edouble e);
    friend double cast_double(const edouble e);
    friend long long int cast_llong(const edouble e);
    friend long double cast_ldouble(const edouble e);
    friend edouble::basis_type cast_basis(const edouble e);
  private:
    friend class eint;
#ifdef _MSC_VER
    friend struct std::numeric_limits<edouble>;
#else
    friend class __gnu_cxx::hash<edouble>;
    friend class std::numeric_limits<edouble>;
#endif //_MSC_VER

    //private version that doesn't do infinity checking
    edouble(const double v, bool) : m_v(v) {}


    friend edouble operator+(const double o, const edouble e);
    friend edouble operator-(const double o, const edouble e);
    friend edouble operator*(const double o, const edouble e);
    friend edouble operator/(const double o, const edouble e);
    friend edouble operator%(const double o, const edouble e);

    DECL_FRIEND_COMPARISONS(int, edouble);
    DECL_FRIEND_COMPARISONS(long, edouble);
    DECL_FRIEND_COMPARISONS(double, edouble);
    DECL_FRIEND_COMPARISONS(unsigned int, edouble);
    DECL_FRIEND_COMPARISONS(long long int, edouble);
    DECL_FRIEND_COMPARISONS(long double, edouble);

    friend edouble std::abs(const edouble d);
    friend edouble std::sqrt(const edouble d);
    friend edouble std::pow(const edouble d, const EUROPA::eint i);
    friend edouble std::sin(const edouble d);
    friend edouble std::ceil(const edouble d);
    friend edouble std::floor(const edouble d);
    friend edouble std::max(const edouble a, const edouble b);
    friend edouble std::min(const edouble a, const edouble b);

    friend std::ostream& operator<<(std::ostream& o, const edouble e);
    friend std::istream& operator>>(std::istream& o, edouble& e);

    double m_v;
  };

  edouble eint::operator+(const edouble o) const {handle_inf_add(edouble, m_v, o.m_v); op(edouble, (double)m_v, +, o.m_v);}
  edouble eint::operator-(const edouble o) const {handle_inf_sub(edouble, m_v, o.m_v); op(edouble, (double)m_v, -, o.m_v);}
  edouble eint::operator*(const edouble o) const {handle_inf_mul(edouble, m_v, o.m_v); op(edouble, (double)m_v, *, o.m_v);}
  edouble eint::operator/(const edouble o) const {handle_inf_div(edouble, m_v, o.m_v); op(edouble, (double)m_v, /, o.m_v);}
  bool eint::operator<(const edouble o) const {return m_v < o.m_v;}
  bool eint::operator<=(const edouble o) const {return m_v <= o.m_v;}
  bool eint::operator==(const edouble o) const {return m_v == o.m_v;}
  bool eint::operator>=(const edouble o) const {return m_v >= o.m_v;}
  bool eint::operator>(const edouble o) const {return m_v > o.m_v;}
  bool eint::operator!=(const edouble o) const {return m_v != o.m_v;}

  bool eint::operator<(const double o) const {return m_v < o;}
  bool eint::operator<=(const double o) const {return m_v <= o;}
  bool eint::operator==(const double o) const {return m_v == o;}
  bool eint::operator>=(const double o) const {return m_v >= o;}
  bool eint::operator>(const double o) const {return m_v > o;}
  bool eint::operator!=(const double o) const {return m_v != o;}

  inline edouble operator+(const double o, const edouble e) {handle_inf_add(edouble, o, e.m_v); op(edouble, o, +, e.m_v);}
  inline edouble operator-(const double o, const edouble e) {handle_inf_sub(edouble, o, e.m_v); op(edouble, o, -, e.m_v);}
  inline edouble operator*(const double o, const edouble e) {handle_inf_mul(edouble, o, e.m_v); op(edouble, o, *, e.m_v);}
  inline edouble operator/(const double o, const edouble e) {handle_inf_div(edouble, o, e.m_v); op(edouble, o, /, e.m_v);}

  GEN_FRIEND_COMPARISONS(int, edouble);
  GEN_FRIEND_COMPARISONS(long, edouble);
  GEN_FRIEND_COMPARISONS(double, edouble);
  GEN_FRIEND_COMPARISONS(unsigned int, edouble);
  GEN_FRIEND_COMPARISONS(long long int, edouble);
  GEN_FRIEND_COMPARISONS(long double, edouble);

  inline eint::basis_type cast_int(const eint e) {return static_cast<eint::basis_type>(e.m_v);}
  inline eint::basis_type cast_int(const edouble e) {return static_cast<eint::basis_type>(e.m_v);}
  inline long cast_long(const eint e) {return static_cast<long>(e.m_v);}
  inline long cast_long(const edouble e) {return static_cast<long>(e.m_v);}
  inline double cast_double(const eint e) {return static_cast<double>(e.m_v);}
  inline double cast_double(const edouble e) {return e.m_v;}
  inline long long int cast_llong(const eint e) {return static_cast<long long int>(e.m_v);}
  inline long long int cast_llong(const edouble e) {return static_cast<long long int>(e.m_v);}
  inline long double cast_ldouble(const eint e) {return static_cast<long double>(e.m_v);}
  inline long double cast_ldouble(const edouble e) {return static_cast<long double>(e.m_v);}
  inline eint::basis_type cast_basis(const eint e) {return static_cast<eint::basis_type>(e.m_v);}
  inline edouble::basis_type cast_basis(const edouble e) {return static_cast<edouble::basis_type>(e.m_v);}

  inline std::ostream& operator<<(std::ostream& o, const edouble e) {return(o << e.m_v);}
  inline std::istream& operator>>(std::istream& o, edouble& e) {
    return(o >> e.m_v);}

  template<typename T>
  inline bool finite(const T& v) {
    return (v != std::numeric_limits<eint>::infinity() && v != std::numeric_limits<eint>::minus_infinity());
  }
}

namespace std {

#ifdef E2_LONG_INT //if we're using 64-bit integers
#ifdef E2_LONG_DOUBLE //if we've got doubles with at least 64-bit significands, use max long
  inline EUROPA::eint numeric_limits<EUROPA::eint>::infinity() throw()
  {return EUROPA::eint(numeric_limits<long>::max(), true);}
#else //if we've got doubles with the regular 52-bit significands, use 2^52 - 1
  inline EUROPA::eint numeric_limits<EUROPA::eint>::infinity() throw()
  {return EUROPA::eint(4503599627370495L, true);}
#endif
#else //if we're using 32-bit integers, use max long
  inline EUROPA::eint numeric_limits<EUROPA::eint>::infinity() throw()
  {return EUROPA::eint(numeric_limits<long>::max(), true);}
#endif
  inline EUROPA::eint numeric_limits<EUROPA::eint>::minus_infinity() throw() {return -infinity();}
  inline EUROPA::eint numeric_limits<EUROPA::eint>::max() throw() {return cast_basis(infinity()) - (long)1;}
  inline EUROPA::eint numeric_limits<EUROPA::eint>::min() throw() {return -max();}
  inline EUROPA::eint numeric_limits<EUROPA::eint>::epsilon() throw() {return 0;}
  inline EUROPA::eint numeric_limits<EUROPA::eint>::round_error() throw() {return 0;}
  inline EUROPA::eint numeric_limits<EUROPA::eint>::quiet_NaN() throw() {return 0;}
  inline EUROPA::eint numeric_limits<EUROPA::eint>::signaling_NaN() throw() {return 0;}
  inline EUROPA::eint numeric_limits<EUROPA::eint>::denorm_min() throw() {return 0;}

#ifdef E2_LONG_INT //if we're using 64-bit integers
#ifdef E2_LONG_DOUBLE //if we've got doubles with at least 64-bit significands, use max long
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::infinity() throw()
  {return EUROPA::edouble((double) numeric_limits<long>::max(), true);}
#else //if we've got doubles with the regular 52-bit significands, use 2^52 - 1
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::infinity() throw()
  {return EUROPA::edouble(4503599627370495.0, true);}
#endif
#else //if we're using 32-big integers, use max long
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::infinity() throw()
  {return EUROPA::edouble((double) numeric_limits<long>::max(), true);}
#endif
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::minus_infinity() throw() {return -infinity();}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::max() throw() {return cast_double(infinity()) - 1.0;}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::min() throw() {return -max();}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::epsilon() throw() {return 0.00001;}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::round_error() throw() {return 0.5;}
#ifdef _MSC_VER
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::quiet_NaN() throw() {return numeric_limits<double>::quiet_NaN();}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::signaling_NaN() throw() {return numeric_limits<double>::signaling_NaN();}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::denorm_min() throw() {return numeric_limits<double>::denorm_min();}
#else
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::quiet_NaN() throw() {return __builtin_nan("");}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::signaling_NaN() throw() {return __builtin_nans("");}
  inline EUROPA::edouble numeric_limits<EUROPA::edouble>::denorm_min() throw() {return __DBL_DENORM_MIN__;}
#endif //_MSC_VER


  inline EUROPA::edouble abs(const EUROPA::edouble d) { return EUROPA::edouble(abs(d.m_v), true);}
  inline EUROPA::edouble sqrt(const EUROPA::edouble d) { handle_inf_unary(EUROPA::edouble, d); return EUROPA::edouble(sqrt(d.m_v), true);}
  inline EUROPA::edouble pow(const EUROPA::edouble d, const EUROPA::eint i) {return EUROPA::edouble(std::pow(d.m_v, (int)i.m_v));}
  inline EUROPA::edouble sin(const EUROPA::edouble d) {return EUROPA::edouble(std::sin(d.m_v), true);}
  inline EUROPA::edouble ceil(const EUROPA::edouble d) {return EUROPA::edouble(std::ceil(d.m_v), true);}
  inline EUROPA::edouble floor(const EUROPA::edouble d) {return EUROPA::edouble(std::floor(d.m_v), true);}
  inline EUROPA::edouble max(const EUROPA::edouble a, const EUROPA::edouble b) {return EUROPA::edouble(std::max(a.m_v, b.m_v), true);}
  inline EUROPA::edouble min(const EUROPA::edouble a, const EUROPA::edouble b) {return EUROPA::edouble(std::min(a.m_v, b.m_v), true);}

#undef handle_inf_unary
#undef op


}

#ifndef _MSC_VER
namespace __gnu_cxx
{
  //I'm not entirely sure this is safe, but it's worked so far.  Maybe this should be changed to
  //*((size_t*)&(__x.m_v))
  size_t hash<EUROPA::edouble>::operator()(EUROPA::edouble __x) const {return (size_t) (__x.m_v);}
  size_t hash<EUROPA::eint>::operator()(EUROPA::eint __x) const {return (size_t) (long) (__x.m_v);}
}
#endif //_MSC_VER

#define min MIN_COPY
#define max MAX_COPY

#endif /* _H_Number */
