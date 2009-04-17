#ifndef _H_NumericDomain
#define _H_NumericDomain

/**
 * @file NumericDomain.hh
 * @author Conor McGann
 * @brief Declares an enumerated domain of numeric values
 */

#include "EnumeratedDomain.hh"
#include "DataTypes.hh"

namespace EUROPA {

  // TODO! : this class seems unnecessary now that DataType has been factored out of AbstractDomain
  /**
   * @class NumericDomain
   * @brief an enumerated domain of numbers
   */
  class NumericDomain : public EnumeratedDomain {
  public:

    NumericDomain(const DataTypeId& dt = FloatDT::instance());
    NumericDomain(double value, const DataTypeId& dt = FloatDT::instance());
    NumericDomain(const std::list<double>& values, const DataTypeId& dt = FloatDT::instance());
    NumericDomain(const AbstractDomain& org);

    virtual NumericDomain *copy() const;
  };

} // namespace EUROPA

#endif
