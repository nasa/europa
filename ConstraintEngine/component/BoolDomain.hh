#ifndef _H_BoolDomain
#define _H_BoolDomain

/**
 * @file BoolDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalDomain.hh"

namespace Prototype{

  /**
   * @class BoolDomain
   * @brief Imposes restrictions on the more generic super class.
   *
   * Restrictions are:
   * @li Always closed and so always finite.
   * @li All modification operations on the bounds must be checked to ensure they are integers
   * @li Only values of 0 and 1 allowed
   */
  class BoolDomain: public IntervalDomain {
  public:
    BoolDomain(const DomainListenerId& listener = DomainListenerId::noId());
    BoolDomain(const BoolDomain& org);
    bool isFinite() const;
    BoolDomain& operator=(const BoolDomain& org);
    bool operator==(const BoolDomain& dom) const;
    bool intersect(const BoolDomain& dom);
    bool isSubsetOf(const BoolDomain& dom) const;
    const DomainType& getType() const;

  private:
    /**
     * @brief Enforces integer semantics. Will be compiled out for fast version.
     */
    void testPrecision(const double& value) const;
  };
}
#endif
