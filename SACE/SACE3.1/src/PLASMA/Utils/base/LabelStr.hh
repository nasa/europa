#ifndef _H_LabelStr
#define _H_LabelStr

/**
 * @file LabelStr.hh
 * @brief Declares the LabelStr class
 * @author Conor McGann
 * @date August, 2003
 */

#include "CommonDefs.hh"
#include "Error.hh"
#include <map>
#include <string>
#include "hash_map.hh"

#ifndef _MSC_VER
namespace __gnu_cxx {
  template<> struct hash<double> {
    size_t operator()(double __x) const {return (size_t) (__x);}
  };

  template<> struct hash<std::string> {
    size_t operator()(const std::string& __x) const {return hash<const char*>()(__x.c_str());}
  };
}
#endif //_MSC_VER

namespace EUROPA {

  class LabelStr;
  DECLARE_GLOBAL_CONST(LabelStr, EMPTY_LABEL);

  /**
   * @class LabelStr
   * @brief Provides for a symbolic value to be handled in a domain.
   * 
   * The reader should note that strings are stored in a static data structure so that they can be shared. Access to
   * the store is provided by a key value. This reduces operations on LabelStr to operations on double valued keys
   * which is considerable more efficient. This encoding is largely transparent to users.
   */
  class LabelStr {
  public:
    /**
     * Zero argument constructor.
     * @note Should only be used indirectly, e.g., via std::list.
     */
    LabelStr();
     
    /**
     * @brief Constructor
     * @param str The null terminated string in question
     */
    LabelStr(const char* str);

    /**
     * @brief Constructor
     * @param The symbolic value as a string
     */
    LabelStr(const std::string& label);

    /**
     * @brief Constructor from encoded key
     *
     * Each LabelStr gets encoded as a key such that any 2 instances of a LabelStr constructed from the same
     * string will have the same key and that the key preserves lexicographic ordering.
     * @param key the key value for a previously created LabelStr instance.
     * @see m_key, getString()
     */
    LabelStr(edouble key);

#ifdef EUROPA_FAST

    LabelStr(const LabelStr& org)
      : m_key(org.m_key) {
    }

    inline operator edouble () const {
      return(m_key);
    }

#else

    /**
     * @brief Copy constructor.
     * 
     * Only needs to copy the key since the string value can be recovered from the shared repository.
     * @param org The source LabelStr.
     */
    LabelStr(const LabelStr& org);

    operator edouble () const;

#endif

    /**
     * @brief Lexical ordering test - less than
     */
    bool operator <(const LabelStr& lbl) const;

    /**
     * @brief Lexical ordering test - greater than
     */
    bool operator >(const LabelStr& lbl) const;

    bool operator==(const LabelStr& lbl) const;
    bool operator!=(const LabelStr& lbl) const;

    /**
     * @brief Return the represented string.
     */
    const std::string& toString() const;

    /**
     * @brief Return the represented char*
     */
    const char* c_str() const;

    /**
     * @brief Obtain the encoded key value for the string.
     * @return The key for accessing the store of strings.
     */
    inline edouble getKey() const {
      return(m_key);
    }

    /**
     * @brief Tests if a given string is contained within this string.
     * @param lblStr The string to test for
     * @return true if present, otherwise false.
     */
    bool contains(const LabelStr& lblStr) const;

    /**
     * @brief Return the number of elements in the string delimited by the given delimeter. 
     *
     * Cases:
     * 1. 'A:B:C:DEF' will contain 4 elements
     * 2. 'A' will contain 1 element
     * 3. ....A:' is invalid.
     * 4. ':A... is invalid
     *
     * @param delimiter The delimeter to mark element boundaries
     * @return The number of elements found.
     * @see getElement
     */
    unsigned int countElements(const char* delimiter) const;

    /**
     * @brief Return the requested element in a delimited string
     *
     * Cases:
     * 1. 'A:B:C:DEF', 2  => 'C'
     * 2. 'A:B:C:DEF', < 0 => error
     * 2. 'A:B:C:DEF', > 3 => error
     *
     * @param index The position of the requested element
     * @param delimeter The delimeter to mark
     */
    LabelStr getElement(unsigned int index, const char* delimiter) const;

    /**
     * @brief Return the number of strings stored.
     */
    static unsigned int getSize();

    /**
     * @brief Obtain the key for the given string and possibly conducting an insertion into keysFromString.
     * @param label The string to be added or found in the store of all strings in use.
     * @return The key value, either created or retrieved.
     */
    static edouble getKey(const std::string& label);

    /**
     * @brief Test if the given double valued key is actually a string.
     */
    static bool isString(edouble key);

    /**
     * @brief Tests if the given candidate is actually stored already as a LabelStr
     */
    static bool isString(const std::string& candidate);
  private:

    /**
     * @brief The key value used as a proxy for the original string.
     * @note The only instance data.
     * @see handleInsertion.
     */
    edouble m_key;

    /**
     * @brief Constructs an entry in stringFromKeys to allow lookup of strings from a key.
     * @todo This optimization may not be that significant. Should explore storing the strings in the instance
     * for such retrieval. It will increase the memeory per instance and hurt on copying or assignment but would provide
     * faster access to implement toString().
     * @param key The double value encoding for the given string
     * @param label the string for which the key has been encoded.
     * @see s_stringFromKeys, getString()
     */
    static void handleInsertion(edouble key, const std::string& label);

    /**
     * @brief Obtain the string from the key.
     * @param key The double valued encoding of the string
     * @return a reference to the original string held in the string store.
     * @see s_stringFromKeys
     */
    static const std::string& getString(edouble key);

    /*static std::map< std::string, double>& keysFromString();*/ /**< Map strings to keys for key allocation on construction. */
    /*static std::map< double, std::string>& stringFromKeys();*/ /**< Map keys to strings for string retrieval - i.e. toString(). */
    static std::map<std::string, edouble>& keysFromString();
    static std::map<edouble, std::string>& stringFromKeys();

#ifndef EUROPA_FAST
    const char* m_chars;
#endif

  };
}
#endif
