#ifndef _H_BoolDomain
#define _H_BoolDomain

/**
 * @file BoolDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalIntDomain.hh"

namespace EUROPA {

  // TODO! : this class seems unnecessary now that DataType has been factored out of AbstractDomain

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

    BoolDomain(const DataTypeId& dt = BoolDT::instance());
    BoolDomain(bool value, const DataTypeId& dt = BoolDT::instance());
    BoolDomain(const AbstractDomain& org);

    bool isFinite() const;

    bool isFalse() const;

    bool isTrue() const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual BoolDomain *copy() const;

    /**
     * @brief Convert to true or false as needed
     */
    std::string toString(double value) const;

    bool intersect(const AbstractDomain& dom);

    bool intersect(double lb, double ub);

  private:
    virtual void testPrecision(const double& value) const;
  };
}
#endif
