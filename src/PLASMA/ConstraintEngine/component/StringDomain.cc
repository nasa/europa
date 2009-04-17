#include "StringDomain.hh"
#include "DataTypes.hh"

namespace EUROPA {

  StringDomain::StringDomain(const DataTypeId& dt) : EnumeratedDomain(dt) {}
  StringDomain::StringDomain(double value, const DataTypeId& dt) : EnumeratedDomain(dt,value) {}
  StringDomain::StringDomain(const std::string& value, const DataTypeId& dt) : EnumeratedDomain(dt,LabelStr(value)) {}
  StringDomain::StringDomain(const std::list<double>& values, const DataTypeId& dt) : EnumeratedDomain(dt,values) {}

  StringDomain::StringDomain(const AbstractDomain& org) : EnumeratedDomain(org) {}

  StringDomain* StringDomain::copy() const
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

  bool StringDomain::isMember(double value) const {
      // This is a hack so that specify() will work
      // string domain needs to be able to handle all situations that involve literal string gracefully
      // for example :
      // string str; str.specify("foo"); eq(str,"bar");
      if (isOpen())
          return true;

      return EnumeratedDomain::isMember(value);
  }

  void StringDomain::set(const std::string& value){
    LabelStr lbl(value);
    set((double) lbl);
  }

  bool StringDomain::isMember(const std::string& value) const{
    LabelStr lbl(value);
    return isMember((double) lbl);
  }

  void StringDomain::insert(const std::string& value){
    LabelStr lbl(value);
    EnumeratedDomain::insert((double) lbl);
  }

  void StringDomain::insert(double value){
    EnumeratedDomain::insert(value);
  }

} // namespace EUROPA
