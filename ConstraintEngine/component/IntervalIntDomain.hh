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

    /**
     * @brief Get the type of the domain to aid in type checking.
     * @see AbstractDomain::DomainType
     */
    virtual const DomainType& getType() const;

    /**
     * @brief Get the name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    virtual const LabelStr& getTypeName() const;

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
     * @brief Fill the given list with the contents of the set.
     * @note Should only be called on finite (and thus closed) domains.
     * @param results The target collection to fill with all values in the set.
     */
    void getValues(std::list<double>& results) const;

    /**
     * @brief Enforces integer semantics by restricting changes to units.
     */
    double minDelta() const;

    double translateNumber(double number, bool asMin = true) const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual IntervalIntDomain *copy() const;

  protected:

    /**
     * @brief Enforce integer semantics.
     * @note Will be compiled out for fast version.
     */
    virtual void testPrecision(const double& value) const;

    /**
     * @brief Enforce integer semantics.
     */
    double convert(const double& value) const;
  };
}
#endif
