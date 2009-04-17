#ifndef _H_IntervalIntDomain
#define _H_IntervalIntDomain

/**
 * @file IntervalIntDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalDomain.hh"

namespace EUROPA{

  // TODO! : this class seems unnecessary now that DataType has been factored out of AbstractDomain

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

    IntervalIntDomain(const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(int lb, int ub, const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(int value, const DataTypeId& dt = IntDT::instance());
    IntervalIntDomain(const AbstractDomain& org);

    virtual ~IntervalIntDomain();

    virtual bool isFinite() const;

    virtual bool isSingleton() const;

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
     * @brief Fill the given list with the contents of the set.
     * @note Should only be called on finite (and thus closed) domains.
     * @param results The target collection to fill with all values in the set.
     */
    virtual void getValues(std::list<double>& results) const;

    virtual double translateNumber(double number, bool asMin = true) const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual IntervalIntDomain *copy() const;

    virtual bool intersect(double lb, double ub);

    virtual bool intersect(const AbstractDomain& dom);

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
