#include "Debug.hh"
#include "LabelStr.hh"
#include "Error.hh"
#include "Utils.hh"
#include <string.h>

namespace EUROPA {

  DEFINE_GLOBAL_CONST(LabelStr, EMPTY_LABEL, "");


//   std::map< std::string, double>& LabelStr::keysFromString() {
//     static std::map< std::string, double> sl_keysFromString;
//     return(sl_keysFromString);
//   }

//   std::map< double, std::string>& LabelStr::stringFromKeys() {
//     static std::map< double, std::string> sl_stringFromKeys;
//     return(sl_stringFromKeys);
//   }

  std::map<std::string, edouble>& LabelStr::keysFromString() {
    static std::map<std::string, edouble> sl_keysFromString;
    return sl_keysFromString;
  }

  std::map< edouble, std::string>& LabelStr::stringFromKeys() {
    static std::map<edouble, std::string> sl_stringFromKeys;
    return(sl_stringFromKeys);
  }

  LabelStr::LabelStr() {
    std::string empty("");
    m_key = getKey(empty);
#ifndef EUROPA_FAST
    m_chars = empty.c_str();
#endif
  }

  pthread_mutex_t& LabelStrMutex()
  {
      static pthread_mutex_t sl_mutex = PTHREAD_MUTEX_INITIALIZER;
      return sl_mutex;      
  }
  
  
  /**
   * Construction must obtain a key that is efficient to use for later
   * calculations in the domain and must maintain the ordering defined
   * by the strings.
   */
  LabelStr::LabelStr(const std::string& label) {
    m_key = getKey(label);

#ifndef EUROPA_FAST
    m_chars = label.c_str();
#endif
  }

  LabelStr::LabelStr(const char* label) {
    std::string str(label);
    m_key = getKey(label);

#ifndef EUROPA_FAST
    m_chars = label;
#endif
  }

  LabelStr::LabelStr(edouble key)
    : m_key(key) {
    check_error(isString(m_key), "Invalid key provided.");

#ifndef EUROPA_FAST
    m_chars = toString().c_str();
#endif
  }

  const std::string& LabelStr::toString() const {
    return(getString(m_key));
  }

  const char* LabelStr::c_str() const {
    return(toString().c_str());
  }

#ifndef EUROPA_FAST

  LabelStr::LabelStr(const LabelStr& org)
    : m_key(org.m_key) {
    m_chars = org.m_chars;
  }

  LabelStr::operator edouble () const {
    return(m_key);
  }

#endif

  bool LabelStr::operator <(const LabelStr& lbl) const {
    return toString() < lbl.toString();
  }

  bool LabelStr::operator >(const LabelStr& lbl) const {
    return toString() > lbl.toString();
  }

  bool LabelStr::operator==(const LabelStr& lbl) const {
    return m_key == lbl.m_key;
  }

  bool LabelStr::operator!=(const LabelStr& lbl) const {
    return m_key != lbl.m_key;
  }

  unsigned int LabelStr::getSize() {
    check_error(keysFromString().size() == stringFromKeys().size());
    int toRet = keysFromString().size();
    return toRet;
  }

  edouble LabelStr::getKey(const std::string& label) {
    
    static edouble sl_counter = EPSILON;

    std::map<std::string, edouble>::iterator it = 
      keysFromString().find(label);

    if (it != keysFromString().end()) {
      edouble toRet = it->second;
      return toRet; // Found it; return the key.
    }

    // Given label not found, so allocate it.
    edouble key = sl_counter;
    sl_counter = sl_counter + 2*EPSILON;

    check_error(key < 1.0, "More strings allocated than permitted");
    
    handleInsertion(key, label);
    return(key);
  }

  void LabelStr::handleInsertion(edouble key, const std::string& label) {
    debugMsg("LabelStr:insert", " " << key << " -> " << label);
    keysFromString().insert(std::make_pair(label, key));
    stringFromKeys().insert(std::make_pair(key, label));
  }

  const std::string& LabelStr::getString(edouble key){
    std::map< edouble, std::string >::const_iterator it = stringFromKeys().find(key);
    check_error(it != stringFromKeys().end());
    const std::string& toRet = it->second;
    return toRet;
  }

  bool LabelStr::isString(edouble key) {
    bool toRet = (stringFromKeys().find(key) != stringFromKeys().end());
    return toRet;
  }

  bool LabelStr::isString(const std::string& candidate){
    bool toRet = (keysFromString().find(candidate) != keysFromString().end());
    return toRet;
  }

  bool LabelStr::contains(const LabelStr& lblStr) const{
    const std::string& thisStr = toString();
    int index = thisStr.find(lblStr.c_str());
    return (index >= 0);
  }


  unsigned int LabelStr::countElements(const char* delimiter) const{
    check_error(delimiter != NULL && delimiter != 0 && delimiter[0] != '\0', "'NULL' and '\\0' are not valid delimiters");

    //allocate a results vector
    std::vector<std::string> tokens;

    // Get a std string from the LabelStr
    const std::string& srcStr = toString();

    //create a std string of the delimiter
    std::string delim(delimiter);

    tokenize(srcStr, tokens, delim);

    return tokens.size();
  }

  LabelStr LabelStr::getElement(unsigned int index, const char* delimiter) const{
    check_error(delimiter != NULL && delimiter != 0 && delimiter[0] != '\0', "'NULL' and '\\0' are not valid delimiters");

    //allocate a results vector
    std::vector<std::string> tokens;

    // Get a std string from the LabelStr
    const std::string& srcStr = toString();

    //create a std string of the delimiter
    std::string delim(delimiter);

    tokenize(srcStr, tokens, delim);

    LabelStr result(tokens[index]);

    return result;
  }
}
