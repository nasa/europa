#ifndef _H_BoolDomain
#define _H_BoolDomain

/**
 * @file BoolDomain.hh
 * @author Conor McGann
 * @brief Declares a restriction to semantics of the IntervalDomain for integers only.
 */
#include "IntervalIntDomain.hh"

namespace EUROPA {

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

    BoolDomain();

    BoolDomain(const char* typeName);

    BoolDomain(bool value);

    BoolDomain(bool value, const char* typeName);

    BoolDomain(const AbstractDomain& org);

    const DomainType& getType() const;

    static const LabelStr& getDefaultTypeName();

    bool isFinite() const;

    bool isFalse() const;

    bool isTrue() const;

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual BoolDomain *copy() const;

  private:
    virtual void testPrecision(const double& value) const;
  };
}
#endif
