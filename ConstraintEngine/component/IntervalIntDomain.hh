#ifndef _H_IntervalIntDomain
#define _H_IntervalIntDomain

/**
 * @file IntervalIntDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalDomain.hh"

namespace Prototype{

  /**
   * @class IntervalIntDomain
   * @brief Imposes restrictions on the more generic super class.
   *
   * Restrictions are:
   * @li If closed then it is finite.
   * @li All modification operations on the bounds must be checked to ensure they are integers
   */
  class IntervalIntDomain: public IntervalDomain {
  public:

    IntervalIntDomain(const DomainListenerId& listener = DomainListenerId::noId());

    IntervalIntDomain(int lb, 
		      int ub, 
		      const DomainListenerId& listener = DomainListenerId::noId());

    IntervalIntDomain(int value, 
		      const DomainListenerId& listener = DomainListenerId::noId());

    IntervalIntDomain(const IntervalIntDomain& org);

    bool isFinite() const;

    const DomainType& getType() const;

    /**
     * @brief Add an element to the domain.
     * @param value The value to insert.
     * @note Will generate a domain relaxation if value is not already in the domain.
     * @note Will generate an error if value not within or "next to" the existing interval.
     * @see DomainListener::DOMAIN_RELAXED, AbstractDomain::insert
     * @note This implementation might also work in IntervalDomain
     * since it uses minDelta().
     */
    virtual void insert(double value);

    /**
     * @brief Remove a value from the domain.
     * @param value The value to remove.
     * @note Will generate an error if value is in "middle" of interval.
     * @note This implementation might also work in IntervalDomain
     * since it uses minDelta().
     */
    virtual void remove(double value);

    /**
     * @brief Enforces integer semantics by restricting changes to units.
     */
    double minDelta() const;

    double translateNumber(double number, bool asMin = true) const;

  protected:
    /**
     * @brief Enforces integer semantics.
     * @note Will be compiled out for fast version.
     */
    virtual void testPrecision(const double& value) const;

    /**
     * @brief Enforces integer semantics.
     */
    double convert(const double& value) const;
  };
}
#endif
