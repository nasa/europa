#ifndef _H_BoolDomain
#define _H_BoolDomain

/**
 * @file BoolDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalIntDomain.hh"

namespace Prototype {

  /**
   * @class BoolDomain
   * @brief Imposes restrictions on the more generic super class.
   *
   * Restrictions are:
   * @li Always closed and so always finite.
   * @li All modification operations on the bounds must be checked to ensure they are integers.
   * @li Only values of 0 and 1 allowed.
   */
  class BoolDomain : public IntervalIntDomain {
  public:
    BoolDomain(bool singletonValue, const DomainListenerId& listener = DomainListenerId::noId());
    BoolDomain(const DomainListenerId& listener = DomainListenerId::noId());
    BoolDomain(const BoolDomain& org);
    const DomainType& getType() const;
    bool isFinite() const;
    bool isFalse() const;
    bool isTrue() const;
  private:
    virtual void testPrecision(const double& value) const;
  };
}
#endif
