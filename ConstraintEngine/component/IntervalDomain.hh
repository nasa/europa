#ifndef _H_IntervalDomain
#define _H_IntervalDomain

/**
 * @file IntervalDomain.hh
 * @author Conor McGann
 * @date August, 2003
 */

#include "AbstractDomain.hh"

namespace Prototype{

  /**
   * @class IntervalDomain
   * @brief Abstract base class for all interval domains. Derived classes impose restrictions on the general semantics of this base class.
   */
  class IntervalDomain: public AbstractDomain {
  public:
    /**
     * @brief Destructor
     */
    virtual ~IntervalDomain();

    /**
     * @brief Access upper bound
     */
    double getUpperBound() const;

    /**
     * @brief Access lower bound
     */
    double getLowerBound() const;

    /**
     * @brief Access singleton value. Must be a singleton or this will fail.
     */
    double getSingletonValue() const;

    /**
     * @brief Access both bounds in a convenience method, and indicates if the domain is infinite
     * @param lb update this value with the lower bound
     * @param ub update this value with the upper bound
     * @return true if !isFinite()
     */
    bool getBounds(double& lb, double& ub);

    /**
     * @brief Set to the specified domain. May empty the domain if target does not intersect the current domain.
     * @param value the target singleton value.
     */
    void set(const IntervalDomain& dom);

    /**
     * @brief Set to a singleton. May empty the domain if value is not a member of the current domain.
     * @param value the target singleton value.
     */
    void set(double value);

    /**
     * @brief Indicates assigment to the target domain as a relaxation triggered externally.
     * @param value the target singleton value.
     * @see relax
     */
    void reset(const IntervalDomain& dom);

    /**
     * @brief restricts this domain to the intersection of its values with the given domain.
     * @param dom the domain to intersect with. Must not be empty.
     * @return true if the intersection results in a change to this domain, otherwise false.
     */
    bool intersect(const IntervalDomain& dom);

    /**
     * @brief Convenience version of intersect.
     * @param lb the lower bound of domain to intersect with
     * @param ub the upper bound of domain to intersect with. ub must be >= lb.
     * @return true if the intersection results in a change to this domain, otherwise false.
     * @see (const IntervalDomain& dom
     */
    bool intersect(double lb, double ub);

    /**
     * @brief Force the domain to empty.
     * @see DomainListener::EMPTIED
     */
    void empty();

    /**
     * @brief Relax this domain to that of the given domain
     * @param dom - The domain to relax it to. Must not be empty and must be a superset of this domain.
     */
    IntervalDomain& operator=(const IntervalDomain& dom);

    /**
     * @brief Convenience method for relaxing a domain.
     * @param lb the lower bound of domain to relax to. lb must be <= m_lb
     * @param ub the upper bound of domain to relax to. ub must be >= m_ub
     * @return true if relaxation causes a change to this domain
     * @see operator=(const IntervalDomain& dom)
     */
    bool relax(double lb, double ub);

    /**
     * @brief test for membership
     * @param value to test for
     * @return true if a member of the domain, otherwise false
     */
    bool isMember(double value) const;

    /**
     * @brief Always false
     */
    bool isEnumerated() const;

    /**
     * @brief test for single valued domain.
     */
    bool isSingleton() const;

    /**
     * @brief test for empty domain. Only allowed to call this on closed domains.
     */
    bool isEmpty() const;

    /**
     * @brief return the number of elements in the domain. an only be called on a domain which is finite.
     */
    int getSize() const;

    /**
     * @brief test for equality.
     */
    bool operator==(const IntervalDomain& dom) const;

    /**
     * @brief test for inequality.
     */
    bool operator!=(const IntervalDomain& dom) const;

    /**
     * @brief test if this domain is a subset of dom.
     * @param dom the domain tested against.
     * @param true if all elements of this domain are in dom. Otherwise false.
     */
    bool isSubsetOf(const IntervalDomain& dom) const;

  protected:
    IntervalDomain(double lb, double ub, bool closed, const DomainListenerId& listener);
    IntervalDomain(const IntervalDomain& org);

    /**
     * @brief Helper method to test if the given value can be considered an integer. Used in derived class.
     * @see testPrecision
     */
    double check(const double& value) const;

    /**
     * @brief tests if the given value is of the correct type for the domain type. Mostly used for
     * restricting values of doubles to int. However, we could restrict it in other ways perhaps.
     */
    virtual void testPrecision(const double& value) const = 0;

    double m_ub; /*!< The upper bound of the domain */
    double m_lb; /*!< The lower bound o fthe domain */
  };
}
#endif
