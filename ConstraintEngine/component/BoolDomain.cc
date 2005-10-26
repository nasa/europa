#include "BoolDomain.hh"

namespace EUROPA {


  BoolDomain::BoolDomain()
    : IntervalIntDomain(0, 1, getDefaultTypeName().c_str()) {}

  BoolDomain::BoolDomain(const char* typeName)
    : IntervalIntDomain(typeName) {
    m_ub = 1;
    m_lb = 0;
  }

  BoolDomain::BoolDomain(bool value)
    : IntervalIntDomain(value, value, getDefaultTypeName().c_str()) {
    check_error(check_value(value), "Invalid value");
  }

  BoolDomain::BoolDomain(bool value, const char* typeName)
    : IntervalIntDomain(value, value, typeName) {
    check_error(check_value(value), "Invalid value");
  }

  BoolDomain::BoolDomain(const AbstractDomain& org)
    : IntervalIntDomain(org) {
    check_error(check_value(m_lb), "Invalid lower bound");
    check_error(check_value(m_ub), "Invalid upper bound");
  }

  void BoolDomain::testPrecision(const double& value) const {
    check_error(value == 0 || value == 1);
  }

  // convert(), insert(), and remove() are inherited from IntervalIntDomain.

  bool BoolDomain::isFinite() const{
    return(true);
  }

  bool BoolDomain::isFalse() const {
    return(m_ub == 0 && m_lb == 0);
  }

  bool BoolDomain::isTrue() const {
    return(m_ub == 1 && m_lb == 1);
  }

  const LabelStr& BoolDomain::getDefaultTypeName() {
    static const LabelStr sl_typeName("BOOL");
    return(sl_typeName);
  }

  BoolDomain *BoolDomain::copy() const {
    BoolDomain *ptr = new BoolDomain(*this);
    check_error(ptr != 0);
    return(ptr);
  }

  LabelStr BoolDomain::displayValue(double value) const{
    check_error(isMember(value), "Caught an invalid attempt to display a value not in this domain");

    static const LabelStr sl_true("true");
    static const LabelStr sl_false("false");
    checkError(value == true || value == false, value << "is not a bool value" );
    if(value == true)
      return sl_true;
    else 
      return sl_false;
  }

  bool BoolDomain::intersect(const AbstractDomain& dom) {
    return intersect(dom.getLowerBound(), dom.getUpperBound());
  }

  bool BoolDomain::intersect(double lb, double ub) {
    double boolLb = lb;
    double boolUb = ub;
    if(boolLb > boolUb) {
      boolLb = boolUb;
      boolUb = lb;
    }
    if(boolLb != 0.0)
      boolLb = 1.0;
    if(boolUb != 0.0)
      boolUb = 1.0;
    return IntervalIntDomain::intersect(boolLb, boolUb);
  }
}
