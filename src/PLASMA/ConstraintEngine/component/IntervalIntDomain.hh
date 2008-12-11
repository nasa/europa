#ifndef _H_IntervalIntDomain
#define _H_IntervalIntDomain

/**
 * @file IntervalIntDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalDomain.hh"

namespace EUROPA{

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

    IntervalIntDomain();

    IntervalIntDomain(const std::string& typeName);

    IntervalIntDomain(eint lb, eint ub);

    IntervalIntDomain(eint value);

    IntervalIntDomain(eint lb, eint ub, const std::string& typeName);

    IntervalIntDomain(const AbstractDomain& org);

    virtual ~IntervalIntDomain(){}

    bool isFinite() const;

    bool isSingleton() const;

    virtual bool isBool() const {
      return(false);
    }

    /**
     * @brief Get the default name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    static const LabelStr& getDefaultTypeName();

    /**
     * @brief Add an element to the domain.
     * @param value The value to insert.
     * @note Will generate a domain relaxation if value is not already in the domain.
     * @note Will generate an error if value not within or "next to" the existing interval.
     * @see DomainListener::DOMAIN_RELAXED, AbstractDomain::insert
     * @note This implementation might also work in IntervalDomain
     * since it uses minDelta().
     */
    virtual void insert(edouble value);

    /**
     * @brief Fill the given list with the contents of the set.
     * @note Should only be called on finite (and thus closed) domains.
     * @param results The target collection to fill with all values in the set.
     */
    void getValues(std::list<edouble>& results) const;

    edouble translateNumber(edouble number, bool asMin = true) const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual IntervalIntDomain *copy() const;

    virtual bool intersect(edouble lb, edouble ub);

    virtual bool intersect(const AbstractDomain& dom);

  protected:

    /**
     * @brief Enforce integer semantics.
     * @note Will be compiled out for fast version.
     */
    virtual void testPrecision(const edouble& value) const;

    /**
     * @brief Enforce integer semantics.
     */
    edouble convert(const edouble& value) const;
  };
}
#endif
