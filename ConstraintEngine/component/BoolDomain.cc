#include "BoolDomain.hh"

namespace Prototype {

  BoolDomain::BoolDomain(bool singletonValue, const DomainListenerId& listener)
    :IntervalDomain(singletonValue, singletonValue, true, listener){}

  BoolDomain::BoolDomain(const DomainListenerId& listener)
    :IntervalDomain(0, 1, true, listener){}

  BoolDomain::BoolDomain(const BoolDomain& org)
    :IntervalDomain(org){}

  void BoolDomain::testPrecision(const double& value) const {
    int intValue = (int) value;
    double dblValue =(double) intValue;
    check_error(dblValue == value); // confirms no loss in precision
    check_error(intValue == 0 || intValue == 1);
  }

  bool BoolDomain::isFinite() const{
    return true;
  }

  bool BoolDomain::isFalse() const {
    return m_ub == m_lb == 0;
  }

  bool BoolDomain::isTrue() const {
    return m_ub == m_lb == 1;
  }

  const AbstractDomain::DomainType& BoolDomain::getType() const{
    static const AbstractDomain::DomainType s_type = BOOL;
    return s_type;
  }
}
