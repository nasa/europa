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
#include "DataTypes.hh"

namespace EUROPA {

// TODO! : this class seems unnecessary now that DataType has been factored out of AbstractDomain
  /**
   * @class StringDomain
   * @brief an enumerated domain of strings
   */
  class StringDomain : public EnumeratedDomain {
  public:

    StringDomain(const DataTypeId& dt = StringDT::instance());
    StringDomain(double value, const DataTypeId& dt = StringDT::instance());
    StringDomain(const std::string& value, const DataTypeId& dt = StringDT::instance());
    StringDomain(const std::list<double>& values, const DataTypeId& dt = StringDT::instance());
    StringDomain(const AbstractDomain& org);

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

