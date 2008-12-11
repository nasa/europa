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
  inline EUROPA::edouble abs(const EUROPA::edouble d);
  inline EUROPA::edouble sqrt(const EUROPA::edouble d);
  inline EUROPA::edouble pow(const EUROPA::edouble d, const EUROPA::eint i);
  inline EUROPA::edouble sin(const EUROPA::edouble d);
  inline EUROPA::edouble ceil(const EUROPA::edouble d);
  inline EUROPA::edouble floor(const EUROPA::edouble d);
}

namespace EUROPA {

#if (__LONG_MAX__ > __INT_MAX__)
#define E2_LONG_INT
#else
#undef E2_LONG_INT
#endif

  class edouble;

  /**
   * Caveats:
   *  There is a potential ambiguity with the number 0, because it freely converts to a void*, in this case:
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
  inline double cast_double(const eint e) {return static_cast<double>(e.m_v);}
  inline double cast_double(const edouble e) {return e.m_v;}

  inline std::ostream& operator<<(std::ostream& o, const edouble e) {return(o << e.m_v);}
}

namespace std {
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
