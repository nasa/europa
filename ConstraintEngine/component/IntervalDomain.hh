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
     * @brief Set to a singleton. May empty the domain if value is not a member of the current domain.
     * @param value the target singleton value.
     */
    void setToSingleton(double value);

    bool intersect(double lb, double ub);
    bool relax(double lb, double ub);
    bool isMember(double value) const;
    bool isEnumerated() const;
    bool isSingleton() const;
    bool isEmpty() const;
    void empty();
    int getSize() const;
  protected:
    IntervalDomain(double lb, double ub, bool finite, bool closed, const DomainListenerId& listener);
    IntervalDomain(const IntervalDomain& org);
    IntervalDomain(Europa::Domain& org);

    /**
     * @brief Relax this domain to that of the given domain
     * @param dom - The domain to relax it to. Must not be empty and must be a superset of this domain.
     */
    IntervalDomain& operator=(const IntervalDomain& dom);
    bool operator==(const IntervalDomain& dom) const;
    bool intersect(const IntervalDomain& dom);
    bool isSubsetOf(const IntervalDomain& dom) const;
    double check(const double& value) const;
    virtual void testPrecision(const double& value) const = 0;

    double m_ub;
    double m_lb;
  };
}
#endif
