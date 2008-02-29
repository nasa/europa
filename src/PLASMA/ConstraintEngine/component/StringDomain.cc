#include "StringDomain.hh"

namespace EUROPA {

  StringDomain::StringDomain()
    :EnumeratedDomain(false, getDefaultTypeName().c_str()){m_isString = true;}

  StringDomain::StringDomain(const char* typeName)
    :EnumeratedDomain(false, typeName){m_isString = true;}

  StringDomain::StringDomain(double value, const char* typeName) 
    : EnumeratedDomain(value, false, typeName){m_isString = true;}

  StringDomain::StringDomain(const std::list<double>& values, 
                             const char* typeName)
    : EnumeratedDomain(values, false, typeName)
  {
    m_isString = true;
  }

  StringDomain::StringDomain(const AbstractDomain& org)
    : EnumeratedDomain(org)
  {
  }

  const LabelStr&
  StringDomain::getDefaultTypeName()
  {
    static const LabelStr sl_typeName("STRING_ENUMERATION");
    return(sl_typeName);
  }


  StringDomain *
  StringDomain::copy() const
  {
    StringDomain * ptr = new StringDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }
  
  void StringDomain::set(double value) {
    check_error(LabelStr::isString(value));
    checkError(isEmpty() || isMember(value), value << " is not a member of the domain :" << toString());

    // Insert the value into the set as a special behavior for strings
    m_values.insert(value);
    EnumeratedDomain::set(value);
  }

} // namespace EUROPA
