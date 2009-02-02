#ifndef _H_Number
#define _H_Number

#include <cmath>
#include <ostream>
#include <limits>

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))
#include <ext/hash_fun.h>
#else
#include <ext/stl_hash_fun.h>
#endif

#if (__LONG_MAX__ > __INT_MAX__)
#define E2_LONG_INT
#else
#undef E2_LONG_INT
#endif

/**
 * Pre-declarations.
 */

namespace EUROPA {
  class eint;
  class edouble;
}


namespace __gnu_cxx {
  template<> struct hash<EUROPA::edouble> {
    inline size_t operator()(EUROPA::edouble __x) const;
  };
}

namespace std {
  template<>
  struct numeric_limits<EUROPA::eint> {
    static const bool is_specialized = true;
    //I'm not sure if these numbers are correct, so be careful
#ifdef E2_LONG_INT
    static const int digits = __DBL_MANT_DIG__;
    static const int digits10 = __DBL_DIG__;
#else
    static const int digits = numeric_limits<long>::digits;
    static const int digits10 = numeric_limits<long>::digits10;
#endif
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
    static const bool traps = __glibcxx_integral_traps;
    static const bool tinyness_before = false;
    static const float_round_style round_style = round_toward_zero;

    static EUROPA::eint min() throw();
    static EUROPA::eint max() throw();
    static EUROPA::eint epsilon() throw();
    static EUROPA::eint round_error() throw();
    static EUROPA::eint infinity() throw();
    static EUROPA::eint quiet_NaN() throw();
    static EUROPA::eint signaling_NaN() throw();
    static EUROPA::eint denorm_min() throw();
  };

  template<>
  struct numeric_limits<EUROPA::edouble> {
    static const bool is_specialized = true;
    //I'm not sure if these numbers are correct, so be careful
#ifdef E2_LONG_INT
    static const int digits = __DBL_MANT_DIG__;
    static const int digits10 = __DBL_DIG__;
#else
    static const int digits = numeric_limits<long>::digits;
    static const int digits10 = numeric_limits<long>::digits10;
#endif
    static const bool is_signed = true;
    static const bool is_integer = false;
    static const bool is_exact = false;
    static const int radix = __FLT_RADIX__;
    //Not sure what these numbers should be
    static const int min_exponent = 0;
    static const int min_exponent10 = 0;
    static const int max_exponent = 0;
    static const int max_exponent10 = 0;

    static const bool has_infinity = true;
    static const bool has_quiet_NaN = __DBL_HAS_QUIET_NAN__;
    static const bool has_signaling_NaN = has_quiet_NaN;
    static const float_denorm_style has_denorm = denorm_absent;
    static const bool has_denorm_loss = false;
    static const bool is_iec559 = has_infinity && has_quiet_NaN && has_denorm == denorm_present;
    static const bool is_bounded = true;
    static const bool is_modulo = false;
    static const bool traps = __glibcxx_integral_traps;
    static const bool tinyness_before = numeric_limits<double>::tinyness_before;
    static const float_round_style round_style = round_to_nearest;

    static EUROPA::edouble min() throw();
    static EUROPA::edouble max() throw();
    static EUROPA::edouble epsilon() throw();
    static EUROPA::edouble round_error() throw();
    static EUROPA::edouble infinity() throw();
    static EUROPA::edouble quiet_NaN() throw();
    static EUROPA::edouble signaling_NaN() throw();
    static EUROPA::edouble denorm_min() throw();
  };

  inline EUROPA::edouble abs(const EUROPA::edouble d);
  inline EUROPA::edouble sqrt(const EUROPA::edouble d);
  inline EUROPA::edouble pow(const EUROPA::edouble d, const EUROPA::eint i);
  inline EUROPA::edouble sin(const EUROPA::edouble d);
  inline EUROPA::edouble ceil(const EUROPA::edouble d);
  inline EUROPA::edouble floor(const EUROPA::edouble d);
}



namespace EUROPA {

#ifdef OVERFLOW_CHECK
  template<typename T>
  bool add_over(const T x, const T y) {return (std::numeric_limits<T>::infinity() - x < y;}
#else
#endif

  class edouble;

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
   *  foo(eint(0));
   */

  class eint {
  public:
    eint(const int v) : m_v(v) {}
    eint(const long v) : m_v(v) {}
    eint(const unsigned int v) : m_v((long)v) {}
    eint(const unsigned long v) : m_v((long)v) {}

    eint() : m_v(0) {}
    inline eint operator+() const {return eint(+m_v);}
    inline eint operator-() const {return eint(-m_v);}
    inline eint operator~() const {return eint(~m_v);}
    inline bool operator!() const {return !m_v;}
    inline eint operator++() {return eint(++m_v);}
    inline eint operator++(int) {return eint(m_v++);}
    inline eint operator--() {return eint(--m_v);}
    inline eint operator--(int) {return eint(m_v--);}
    inline eint operator+(const int o) const {return eint(m_v + o);}
    inline eint operator+(const eint o) const {return eint(m_v + o.m_v);}
    inline eint operator-(const int o) const {return eint(m_v - o);}
    inline eint operator-(const eint o) const {return eint(m_v - o.m_v);}
    inline eint operator*(const int o) const {return eint(m_v * o);}
    inline eint operator*(const eint o) const {return eint(m_v * o.m_v);}
    inline eint operator/(const int o) const {return eint(m_v / o);}
    inline eint operator/(const eint o) const {return eint(m_v / o.m_v);}
    inline eint operator%(const int o) const {return eint(m_v % o);}
    inline eint operator%(const eint o) const {return eint(m_v % o.m_v);}
    inline eint operator^(const int o) const {return eint(m_v ^ o);}
    inline eint operator^(const eint o) const {return eint(m_v ^ o.m_v);}
    inline eint operator&(const int o) const {return eint(m_v & o);}
    inline eint operator&(const eint o) const {return eint(m_v & o.m_v);}
    inline eint operator|(const int o) const {return eint(m_v | o);}
    inline eint operator|(const eint o) const {return eint(m_v | o.m_v);}
    inline eint operator<<(const int o) const {return eint(m_v << o);}
    inline eint operator<<(const eint o) const {return eint(m_v << o.m_v);}
    inline eint operator>>(const int o) const {return eint(m_v >> o);}
    inline eint operator>>(const eint o) const {return eint(m_v >> o.m_v);}
    inline eint operator+=(const int o) {return eint(m_v += o);}
    inline eint operator+=(const eint o) {return eint(m_v += o.m_v);}
    inline eint operator-=(const int o) {return eint(m_v -= o);}
    inline eint operator-=(const eint o) {return eint(m_v -= o.m_v);}
    inline eint operator*=(const int o) {return eint(m_v *= o);}
    inline eint operator*=(const eint o) {return eint(m_v *= o.m_v);}
    inline eint operator/=(const int o) {return eint(m_v /= o);}
    inline eint operator/=(const eint o) {return eint(m_v /= o.m_v);}
    inline eint operator%=(const eint o) {return eint(m_v %= o.m_v);}
    inline eint operator%=(const int o) {return eint(m_v %= o);}
    inline eint operator^=(const eint o) {return eint(m_v ^= o.m_v);}
    inline eint operator^=(const int o) {return eint(m_v ^= o);}
    inline eint operator&=(const int o) {return eint(m_v &= o);}
    inline eint operator&=(const eint o) {return eint(m_v &= o.m_v);}
    inline eint operator|=(const int o) {return eint(m_v |= o);}
    inline eint operator|=(const eint o) {return eint(m_v |= o.m_v);}
    inline eint operator<<=(const int o) {return eint(m_v <<= o);}
    inline eint operator<<=(const eint o) {return eint(m_v <<= o.m_v);}
    inline eint operator>>=(const int o) {return eint(m_v >>= o);}
    inline eint operator>>=(const eint o) {return eint(m_v >>= o.m_v);}
    inline bool operator<(const int o) const {return m_v < o;}
    inline bool operator<(const eint o) const {return m_v < o.m_v;}
    inline bool operator<=(const int o) const {return m_v <= o;}
    inline bool operator<=(const eint o) const {return m_v <= o.m_v;}
    inline bool operator==(const int o) const {return m_v == o;}
    inline bool operator==(const eint o) const {return m_v == o.m_v;}
    inline bool operator>=(const int o) const {return m_v >= o;}
    inline bool operator>=(const eint o) const {return m_v >= o.m_v;}
    inline bool operator>(const int o) const {return m_v > o;}
    inline bool operator>(const eint o) const {return m_v > o.m_v;}
    inline bool operator!=(const int o) const {return m_v != o;}
    inline bool operator!=(const eint o) const {return m_v != o.m_v;}

    inline edouble operator+(const double o) const;
    inline edouble operator+(const edouble o) const;
    inline edouble operator-(const double o) const;
    inline edouble operator-(const edouble o) const;
    inline edouble operator*(const double o) const;
    inline edouble operator*(const edouble o) const;
    inline edouble operator/(const double o) const;
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

    friend double cast_double(const eint e);
    friend int cast_int(const eint e);
    friend long cast_long(const eint e);
  private:

    friend class edouble;
  
    friend eint operator+(const long o, const eint e);
    friend eint operator-(const long o, const eint e);
    friend eint operator*(const long o, const eint e);
    friend eint operator/(const long o, const eint e);
    friend eint operator%(const long o, const eint e);
    friend eint operator^(const long o, const eint e);
    friend eint operator&(const long o, const eint e);
    friend eint operator|(const long o, const eint e);
    friend eint operator<<(const long o, const eint e);
    friend eint operator>>(const long o, const eint e);
//     friend eint operator+=(const long o, const eint e);
//     friend eint operator-=(const long o, const eint e);
//     friend eint operator*=(const long o, const eint e);
//     friend eint operator/=(const long o, const eint e);
//     friend eint operator%=(const long o, const eint e);
//     friend eint operator^=(const long o, const eint e);
//     friend eint operator&=(const long o, const eint e);
//     friend eint operator|=(const long o, const eint e);
//     friend eint operator<<=(const long o, const eint e);
//     friend eint operator>>=(const long o, const eint e);
    friend bool operator<(const long o, const eint e);
    friend bool operator<=(const long o, const eint e);
    friend bool operator==(const long o, const eint e);
    friend bool operator>=(const long o, const eint e);
    friend bool operator>(const long o, const eint e);
    friend bool operator!=(const long o, const eint e);
    friend std::ostream& operator<<(std::ostream& o, const eint e);

    friend edouble std::pow(const EUROPA::edouble d, const eint i);

    long m_v;
  };

  inline eint operator+(const long o, const eint e) {return eint(o + e.m_v);}
  inline eint operator-(const long o, const eint e) {return eint(o - e.m_v);}
  inline eint operator*(const long o, const eint e) {return eint(o * e.m_v);}
  inline eint operator/(const long o, const eint e) {return eint(o / e.m_v);}
  inline eint operator%(const long o, const eint e) {return eint(o % e.m_v);}
  inline eint operator^(const long o, const eint e) {return eint(o ^ e.m_v);}
  inline eint operator&(const long o, const eint e) {return eint(o & e.m_v);}
  inline eint operator|(const long o, const eint e) {return eint(o | e.m_v);}
  inline eint operator<<(const long o, const eint e) {return eint(o << e.m_v);}
  inline eint operator>>(const long o, const eint e) {return eint(o >> e.m_v);}
  inline bool operator<(const long o, const eint e) {return o < e.m_v;}
  inline bool operator<=(const long o, const eint e) {return o <= e.m_v;}
  inline bool operator==(const long o, const eint e) {return o == e.m_v;}
  inline bool operator>=(const long o, const eint e) {return o >= e.m_v;}
  inline bool operator>(const long o, const eint e) {return o > e.m_v;}
  inline bool operator!=(const long o, const eint e) {return o != e.m_v;}
  inline std::ostream& operator<<(std::ostream& o, const eint e) {return(o << e.m_v);}

  class edouble {
  public:
    edouble(const eint v) : m_v(v.m_v) {}
    edouble(const double v) : m_v(v) {}
    edouble() : m_v(0.0) {}
    inline edouble operator+() const {return edouble(+m_v);}
    inline edouble operator-() const {return edouble(-m_v);}
    inline bool operator!() const {return !m_v;}
    inline edouble operator++() {return edouble(++m_v);}
    inline edouble operator++(int) {return edouble(m_v++);}
    inline edouble operator--() {return edouble(--m_v);}
    inline edouble operator--(int) {return edouble(m_v--);}
    inline edouble operator+(const edouble o) const {return edouble(m_v + o.m_v);}
    inline edouble operator-(const edouble o) const {return edouble(m_v - o.m_v);}
    inline edouble operator*(const edouble o) const {return edouble(m_v * o.m_v);}
    inline edouble operator/(const edouble o) const {return edouble(m_v / o.m_v);}
    inline edouble operator+=(const edouble o) {return edouble(m_v += o.m_v);}
    inline edouble operator-=(const edouble o) {return edouble(m_v -= o.m_v);}
    inline edouble operator*=(const edouble o) {return edouble(m_v *= o.m_v);}
    inline edouble operator/=(const edouble o) {return edouble(m_v /= o.m_v);}
    inline bool operator<(const edouble o) const {return m_v < o.m_v;}
    inline bool operator<=(const edouble o) const {return m_v <= o.m_v;}
    inline bool operator==(const edouble o) const {return m_v == o.m_v;}
    inline bool operator>=(const edouble o) const {return m_v >= o.m_v;}
    inline bool operator>(const edouble o) const {return m_v > o.m_v;}
    inline bool operator!=(const edouble o) const {return m_v != o.m_v;}
    inline operator eint() const {return eint((long)m_v);}

    friend int cast_int(const edouble e);
    friend long cast_long(const edouble e);
    friend double cast_double(const edouble e);
  private:
    friend class eint;
    friend class __gnu_cxx::hash<edouble>;

    friend edouble std::abs(const edouble d);
    friend edouble std::sqrt(const edouble d);
    friend edouble std::pow(const edouble d, const EUROPA::eint i);
    friend edouble std::sin(const edouble d);
    friend edouble std::ceil(const edouble d);
    friend edouble std::floor(const edouble d);


    friend std::ostream& operator<<(std::ostream& o, const edouble e);

    double m_v;
  };

  edouble eint::operator+(const edouble o) const {return edouble(((double)m_v) + o.m_v);}
  edouble eint::operator-(const edouble o) const {return edouble(((double)m_v) - o.m_v);}
  edouble eint::operator*(const edouble o) const {return edouble(((double)m_v) * o.m_v);}
  edouble eint::operator/(const edouble o) const {return edouble(((double)m_v) / o.m_v);}
  bool eint::operator<(const edouble o) const {return m_v < o.m_v;}
  bool eint::operator<=(const edouble o) const {return m_v <= o.m_v;}
  bool eint::operator==(const edouble o) const {return m_v == o.m_v;}
  bool eint::operator>=(const edouble o) const {return m_v >= o.m_v;}
  bool eint::operator>(const edouble o) const {return m_v > o.m_v;}
  bool eint::operator!=(const edouble o) const {return m_v != o.m_v;}

  edouble eint::operator+(const double o) const {return edouble(((double)m_v) + o);}
  edouble eint::operator-(const double o) const {return edouble(((double)m_v) - o);}
  edouble eint::operator*(const double o) const {return edouble(((double)m_v) * o);}
  edouble eint::operator/(const double o) const {return edouble(((double)m_v) / o);}
  bool eint::operator<(const double o) const {return m_v < o;}
  bool eint::operator<=(const double o) const {return m_v <= o;}
  bool eint::operator==(const double o) const {return m_v == o;}
  bool eint::operator>=(const double o) const {return m_v >= o;}
  bool eint::operator>(const double o) const {return m_v > o;}
  bool eint::operator!=(const double o) const {return m_v != o;}

  inline int cast_int(const eint e) {return static_cast<int>(e.m_v);}
  inline int cast_int(const edouble e) {return static_cast<int>(e.m_v);}
  inline long cast_long(const eint e) {return static_cast<long>(e.m_v);}
  inline long cast_long(const edouble e) {return static_cast<long>(e.m_v);}
  inline double cast_double(const eint e) {return static_cast<double>(e.m_v);}
  inline double cast_double(const edouble e) {return e.m_v;}

  inline std::ostream& operator<<(std::ostream& o, const edouble e) {return(o << e.m_v);}
}

namespace std {
  EUROPA::eint numeric_limits<EUROPA::eint>::infinity() throw()
  {return std::min((long) pow((double)2, 51) - 1 + pow((double)2, 51), numeric_limits<long>::max());}
  EUROPA::eint numeric_limits<EUROPA::eint>::max() throw() {return infinity() - 1;}
  EUROPA::eint numeric_limits<EUROPA::eint>::min() throw() {return -max();}
  EUROPA::eint numeric_limits<EUROPA::eint>::epsilon() throw() {return 0;}
  EUROPA::eint numeric_limits<EUROPA::eint>::round_error() throw() {return 0;}
  EUROPA::eint numeric_limits<EUROPA::eint>::quiet_NaN() throw() {return 0;}
  EUROPA::eint numeric_limits<EUROPA::eint>::signaling_NaN() throw() {return 0;}
  EUROPA::eint numeric_limits<EUROPA::eint>::denorm_min() throw() {return 0;}

  EUROPA::edouble numeric_limits<EUROPA::edouble>::infinity() throw()
  {return std::min(pow((double)2, 51) - 1.0 + pow((double)2, 51), (double) numeric_limits<long>::max());}
  EUROPA::edouble numeric_limits<EUROPA::edouble>::max() throw() {return infinity() - 1.0;}
  EUROPA::edouble numeric_limits<EUROPA::edouble>::min() throw() {return -max();}
  EUROPA::edouble numeric_limits<EUROPA::edouble>::epsilon() throw() {return 0.00001;}
  EUROPA::edouble numeric_limits<EUROPA::edouble>::round_error() throw() {return 0.5;}
  EUROPA::edouble numeric_limits<EUROPA::edouble>::quiet_NaN() throw() {return __builtin_nan("");}
  EUROPA::edouble numeric_limits<EUROPA::edouble>::signaling_NaN() throw() {return __builtin_nans("");}
  EUROPA::edouble numeric_limits<EUROPA::edouble>::denorm_min() throw() {return __DBL_DENORM_MIN__;}

  inline EUROPA::edouble abs(const EUROPA::edouble d) { return EUROPA::edouble(abs(d.m_v));}
  inline EUROPA::edouble sqrt(const EUROPA::edouble d) { return EUROPA::edouble(sqrt(d.m_v));}
  inline EUROPA::edouble pow(const EUROPA::edouble d, const EUROPA::eint i) {return EUROPA::edouble(std::pow(d.m_v, (int)i.m_v));}
  inline EUROPA::edouble sin(const EUROPA::edouble d) {return EUROPA::edouble(std::sin(d.m_v));}
  inline EUROPA::edouble ceil(const EUROPA::edouble d) {return EUROPA::edouble(std::ceil(d.m_v));}
  inline EUROPA::edouble floor(const EUROPA::edouble d) {return EUROPA::edouble(std::floor(d.m_v));}

}

namespace __gnu_cxx {
  //I'm not entirely sure this is safe, but it's worked so far.  Maybe this should be changed to
  //*((size_t*)&(__x.m_v))
  size_t hash<EUROPA::edouble>::operator()(EUROPA::edouble __x) const {return (size_t) (__x.m_v);}

}



#endif /* _H_Number */
