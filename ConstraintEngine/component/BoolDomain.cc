#include "BoolDomain.hh"

namespace Prototype {

  BoolDomain::BoolDomain(const DomainListenerId& listener)
    :IntervalDomain(0, 1, true, listener){}

  BoolDomain::BoolDomain(const BoolDomain& org)
    :IntervalDomain(org){}

  BoolDomain& BoolDomain::operator=(const BoolDomain& org){
    IntervalDomain::operator=(org);
    return(*this);
  }

  bool BoolDomain::isFinite() const {
    return true;
  }

  bool BoolDomain::operator==(const BoolDomain& dom) const{
    return IntervalDomain::operator==(dom);
  }

  bool BoolDomain::intersect(const BoolDomain& dom){
    return IntervalDomain::intersect(dom);
  }

  bool BoolDomain::isSubsetOf(const BoolDomain& dom) const{
    return IntervalDomain::isSubsetOf(dom);
  }

  void BoolDomain::testPrecision(const double& value) const {
    int intValue = (int) value;
    double dblValue =(double) intValue;
    check_error(dblValue == value); // confirms no loss in precision
    check_error(intValue == 0 || intValue == 1);
  }

  const AbstractDomain::DomainType& BoolDomain::getType() const{
    static const AbstractDomain::DomainType s_type = BOOL;
    return s_type;
  }
}
