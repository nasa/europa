#ifndef _H_SymbolDomain
#define _H_SymbolDomain

/**
 * @file SymbolDomain.hh
 * @author Andrew Bachmann
 * @brief Declares an enumerated domain of Symbols
 */
#include "EnumeratedDomain.hh"
#include "LabelStr.hh"
#include "DataTypes.hh"
namespace EUROPA {

  // TODO! : this class seems unnecessary now that DataType has been factored out of AbstractDomain

  /**
   * @class SymbolDomain
   * @brief an enumerated domain of strings
   */
  class SymbolDomain : public EnumeratedDomain {
  public:
    SymbolDomain(const DataTypeId& dt = SymbolDT::instance());
    SymbolDomain(double value,const DataTypeId& dt = SymbolDT::instance());
    SymbolDomain(const std::list<double>& values,const DataTypeId& dt = SymbolDT::instance());
    SymbolDomain(const AbstractDomain& org);

    virtual SymbolDomain *copy() const;
  };

} // namespace EUROPA

#endif // _H_SymbolDomain

