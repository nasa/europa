#include "BoolDomain.hh"

namespace Prototype {

  BoolDomain::BoolDomain(bool singletonValue, const DomainListenerId& listener,
                         const LabelStr& typeName)
    : IntervalIntDomain(singletonValue, singletonValue, listener, typeName) {
    check_value(singletonValue);
  }

  BoolDomain::BoolDomain(const DomainListenerId& listener,
                         const LabelStr& typeName)
    : IntervalIntDomain(0, 1, listener, typeName) {
  }

  BoolDomain::BoolDomain(const BoolDomain& org)
    : IntervalIntDomain(org) {
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

  const AbstractDomain::DomainType& BoolDomain::getType() const {
    static const AbstractDomain::DomainType s_type = BOOL;
    return(s_type);
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
}
