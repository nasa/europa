#include "HSTSHeuristicsReader.hh"
#include <fstream>
#include "tinyxml.h"
#include "HSTSHeuristics.hh"

namespace Prototype {

  HSTSHeuristicsReader::HSTSHeuristicsReader(HSTSHeuristics& heuristics) : m_heuristics(heuristics) { }

  HSTSHeuristicsReader::~HSTSHeuristicsReader() { }

  void HSTSHeuristicsReader::read(const std::string& fileName) {

    std::ifstream is(fileName.c_str());

    int elements = 0;
    while (!is.eof()) {
      if (is.peek() != '<') {
	is.get(); // discard characters up to '<'
	continue;
      }
      TiXmlElement element("");
      is >> element;
      const char * tagname = element.Value();    
      if (strcmp(tagname, "Document") == 0) {
	for (TiXmlElement* child = element.FirstChildElement(); child;
	     child = child->NextSiblingElement()) {
	  check_error(child != NULL);
	  readElement(*child);
	}
      }
      else
	readElement(element);
      elements++;
    }
  }

  void HSTSHeuristicsReader::readElement(const TiXmlElement& element) {
    const char * tagName = element.Value();
    if (strcmp(tagName, "Defaults") == 0)
      readDefaults(element);
    else if (strcmp(tagName, "VariableSpecification") == 0)
      readVariableSpecification(element);
    else if (strcmp(tagName, "TokenSpecification") == 0)
      readTokenSpecification(element);
  }

  void HSTSHeuristicsReader::readDefaults(const TiXmlElement& element) {
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL);
      const char* tagName = child->Value();
      check_error(tagName != NULL);
      if (strcmp(tagName, "PriorityPref") == 0) 
	readPriorityPref(*child);
      else if (strcmp(tagName, "Compatibility") == 0)
	readCompatibility(*child);
      else if (strcmp(tagName, "Token") == 0)
	readTokenSpecification(*child);
      else if (strcmp(tagName, "ConstrainedVariable") == 0)
	readConstrainedVariable(*child);
    }
  }

  const std::string HSTSHeuristicsReader::getTextChild (const TiXmlElement& element) {
    check_error (element.FirstChild() && element.FirstChild()->ToText() && element.FirstChild()->ToText()->Value());
    return element.FirstChild()->ToText()->Value();
  }

  void HSTSHeuristicsReader::readPriorityPref(const TiXmlElement& element) {
    std::string tmp = getTextChild(element);
    HSTSHeuristics::PriorityPref pp;
    if ((tmp == "low") || (tmp == "LOW"))
      pp = HSTSHeuristics::LOW;
    else if ((tmp == "high") || (tmp == "HIGH"))
      pp = HSTSHeuristics::HIGH;
    else
      check_error(false);

    std::cout << "read PriorityPref = " << tmp << std::endl;
    m_heuristics.setDefaultPriorityPreference(pp);
    std::cout << "getting PriorityPref = " << m_heuristics.getDefaultPriorityPreference() << std::endl;
  }

  void HSTSHeuristicsReader::readVariableSpecification(const TiXmlElement& element){}
  void HSTSHeuristicsReader::readTokenSpecification(const TiXmlElement& element){}
  void HSTSHeuristicsReader::readCompatibility(const TiXmlElement& element){}
  void HSTSHeuristicsReader::readPredicateSpec(const TiXmlElement& element){}
  void HSTSHeuristicsReader::readDecisionPreference(const TiXmlElement& element){}
  void HSTSHeuristicsReader::readConstrainedVariable(const TiXmlElement& element){}
  void HSTSHeuristicsReader::readPreference(const TiXmlElement& element){}
  void HSTSHeuristicsReader::readMaster(const TiXmlElement& element){}

  GeneratorId HSTSHeuristicsReader::getGeneratorFromName(std::string genName){}
}
