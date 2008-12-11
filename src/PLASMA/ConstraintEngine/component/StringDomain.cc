#include "StringDomain.hh"

namespace EUROPA {

  StringDomain::StringDomain()
    :EnumeratedDomain(false, getDefaultTypeName().toString()){m_isString = true;}

  StringDomain::StringDomain(const std::string& typeName)
    :EnumeratedDomain(false, typeName){m_isString = true;}

  StringDomain::StringDomain(edouble value, const std::string& typeName) 
    : EnumeratedDomain(value, false, typeName){m_isString = true;}

  StringDomain::StringDomain(const std::list<edouble>& values, 
                             const std::string& typeName)
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
    static const LabelStr sl_typeName("string");
    return(sl_typeName);
  }


  StringDomain *
  StringDomain::copy() const
  {
    StringDomain * ptr = new StringDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }
  
  void StringDomain::set(edouble value) {
    check_error(LabelStr::isString(value));
    checkError(isEmpty() || isMember(value), value << " is not a member of the domain :" << toString());

    // Insert the value into the set as a special behavior for strings
    m_values.insert(value);
    EnumeratedDomain::set(value);
  }

  bool StringDomain::isMember(edouble value) const {
      // This is a hack so that specify() will work
      // string domain needs to be able to handle all situations that involve literal string gracefully
      // for example :
      // string str; str.specify("foo"); eq(str,"bar");
      if (isOpen())
          return true;
      
      return EnumeratedDomain::isMember(value);
  }
  
} // namespace EUROPA
