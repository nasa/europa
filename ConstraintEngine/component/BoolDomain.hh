#ifndef _H_BoolDomain
#define _H_BoolDomain

/**
 * @file BoolDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalDomain.hh"

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
  class BoolDomain: public IntervalDomain {
  public:
    BoolDomain(bool singletonValue, const DomainListenerId& listener = DomainListenerId::noId());
    BoolDomain(const DomainListenerId& listener = DomainListenerId::noId());
    BoolDomain(const BoolDomain& org);
    const DomainType& getType() const;
    bool isFinite() const;
    bool isFalse() const;
    bool isTrue() const;

  private:
    /**
     * @brief Enforces integer semantics.
     * @note Might be compiled out for fast version, depending on the compiler.
     * Only explicitly inline'ing it can ensure it is always completely compiled out.
     * --wedgingt 2004 Feb 25
     */
    void testPrecision(const double& value) const;

    /**
     * @brief Carries out the conversion of the given double to do appropriate rounding.
     * @param value The value to be converted.
     * @return The value subject to any rounding required for the sub-type (e.g. int).
     */
    double convert(const double& value) const;
  };
}
#endif
