#ifndef H_XML_Utils
#define H_XML_Utils

/**
 * @author Conor McGann
 */

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include <cstddef>
#include <string>

namespace EUROPA {
class TiXmlElement;

#define IS_TAG(x) (strcmp (tagName, x) == 0)

  /**
   * @brief Utility to extract an argument from an xml element
   */
  std::string extractData(const TiXmlElement& element, const std::string& argName);

  /**
   * @brief Helper method to get the first xml element in the file
   */
  TiXmlElement* initXml(const char* sourceFile, const char* element = NULL);

  /**
   * @brief Helper method to parse a given XML string
   */
  TiXmlElement* initXml(std::string& xmlStr);

  /**
   * @brief Extract text at this node, adding error checks
   */
  const char* getTextChild (const TiXmlElement& element);


  /**
   * @brief Helper method to test if a char* is numeric.
   */
  bool isNumber(const char* data);

  /**
   * @brief Helper method to test if a char* is numeric. If it is, will write it to data.
   */
  bool isNumber(const char* data, double& value);
}

#endif
