#ifndef _H_StringDomain
#define _H_StringDomain

/**
 * @file StringDomain.hh
 * @author Andrew Bachmann
 * @brief Declares an enumerated domain of Strings
 */
#include "EnumeratedDomain.hh"
#include "Debug.hh"
#include "LabelStr.hh"

namespace EUROPA {

  /**
   * @class StringDomain
   * @brief an enumerated domain of strings
   */
  class StringDomain : public EnumeratedDomain {
  public:

    /**
     * @brief Constructs an initially empty and open domain, with a default type name
     */
    StringDomain();

    /**
     * @brief Initially empty but open, with special type name
     */
    StringDomain(const char* typeName);

    /**
     * @brief Constructs an initial singleton domain with the given type name
     */
    StringDomain(double value, const char* typeName = getDefaultTypeName().toString().c_str());

    /**
     * @brief Constructs an initial singleton domain with the given type name
     */
    StringDomain(const std::string& value, const char* typeName = getDefaultTypeName().toString().c_str());

    /**
     * @brief Constructor.
     * @param values The initial set of values to populate the domain.
     * @param typeName
     * @see AbstractDomain::isDynamic()
     */
    StringDomain(const std::list<double>& values, 
                 const char* typeName = getDefaultTypeName().toString().c_str());

    /**
     * @brief Copy constructor.
     * @param org The source domain.
     */
    StringDomain(const AbstractDomain& org);

    /**
     * @brief Get the default name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    static const LabelStr& getDefaultTypeName();

    /**
     * @brief Copy the concrete C++ object into new memory and return a pointer to it.
     */
    virtual StringDomain *copy() const;

    /**
     * @brief Sets a singleton value.
     * @param value The value to set. Must be a LabelStr.
     */
    void set(double value);

    bool isMember(double value) const;    
    
    /** String specific bindings for user convenience **/
    void set(const std::string& value);
    bool isMember(const std::string& value) const;
    void insert(const std::string& value);
    void insert(double value);

  };

} // namespace EUROPA

#endif // _H_StringDomain

