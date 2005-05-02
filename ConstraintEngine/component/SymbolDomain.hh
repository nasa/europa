#ifndef _H_SymbolDomain
#define _H_SymbolDomain

/**
 * @file SymbolDomain.hh
 * @author Andrew Bachmann
 * @brief Declares an enumerated domain of Symbols
 */
#include "EnumeratedDomain.hh"
#include "LabelStr.hh"

namespace EUROPA {

  /**
   * @class SymbolDomain
   * @brief an enumerated domain of strings
   */
  class SymbolDomain : public EnumeratedDomain {
  public:

    /**
     * @brief Constructs an initially empty and open domain
     */
    SymbolDomain();

    /**
     * @brief Initially empty and open, with specialized type name
     */
    SymbolDomain(const char* typeName);

    /**
     * @brief Constructs an initial singleton with the given type name
     */
    SymbolDomain(double value, const char* typeName = getDefaultTypeName().toString().c_str());

    /**
     * @brief Constructor.
     * @param values The initial set of values to populate the domain.
     * @param typename The type name to use
     * @see AbstractDomain::isDynamic()
     */
    SymbolDomain(const std::list<double>& values, 
                 const char* typeName = getDefaultTypeName().toString().c_str());

    /**
     * @brief Copy constructor.
     * @param org The source domain.
     */
    SymbolDomain(const AbstractDomain& org);

    /**
     * @brief Get the default name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    static const LabelStr& getDefaultTypeName();

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual SymbolDomain *copy() const;

  };

} // namespace EUROPA

#endif // _H_SymbolDomain

