#include "HSTSHeuristicsReader.hh"
#include <fstream>
#include "tinyxml.h"
#include "HSTSHeuristics.hh"

namespace Prototype {

#define IS_TAG(x) (strcmp (tagName, x) == 0)

  HSTSHeuristicsReader::HSTSHeuristicsReader(HSTSHeuristics& heuristics) : m_heuristics(heuristics) { }

  HSTSHeuristicsReader::~HSTSHeuristicsReader() { }

  void HSTSHeuristicsReader::read(const std::string& fileName) {

    check_error(fileName != "");
    std::cout << "Reading " << fileName << std::endl;

    TiXmlNode *main_node = NULL;
  
    TiXmlDocument doc(fileName);
    bool loadOkay = doc.LoadFile();
    check_error(loadOkay);

    main_node = doc.FirstChild();
  
    TiXmlElement* element = NULL;
    element = main_node->ToElement();
    const char * tagName = element->Value();    
    check_error(IS_TAG("Heuristics-HSTS"));

    element = main_node->FirstChildElement();

    while(element) {
      tagName = element->Value();
      check_error(IS_TAG("Defaults") || IS_TAG("VariableSpecification") || IS_TAG("TokenSpecification"));
      readElement(*element);
      element = element->NextSiblingElement();
    }
  }

  void HSTSHeuristicsReader::readElement(const TiXmlElement& element) {
    std::cout << " in ReadElement" << std::endl;
    const char * tagName = element.Value();
    if (IS_TAG("Defaults"))
      readDefaults(element);
    else if (IS_TAG("VariableSpecification"))
      readVariableSpecification(element);
    else if (IS_TAG("TokenSpecification"))
      readTokenSpecification(element);
  }

  void HSTSHeuristicsReader::readDefaults(const TiXmlElement& element) {
    std::cout << "in ReadDefaults" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL);
      const char* tagName = child->Value();
      check_error(tagName != NULL);
      if (IS_TAG("PriorityPref")) 
	readPriorityPref(*child);
      else if (IS_TAG("Compatibility"))
	readCompatibility(*child);
      else if (IS_TAG("Token"))
	readToken(*child);
      else if (IS_TAG("ConstrainedVariable"))
	readConstrainedVariable(*child);
    }
  }

  const std::string HSTSHeuristicsReader::getTextChild (const TiXmlElement& element) {
    check_error (element.FirstChild() && element.FirstChild()->ToText() && element.FirstChild()->ToText()->Value());
    return element.FirstChild()->ToText()->Value();
  }

  void HSTSHeuristicsReader::readPriorityPref(const TiXmlElement& element) {
    std::cout << "in ReadPriorityPref" << std::endl;

    std::string tmp = getTextChild(element);
    HSTSHeuristics::PriorityPref pp;
    if ((tmp == "low") || (tmp == "LOW"))
      pp = HSTSHeuristics::LOW;
    else if ((tmp == "high") || (tmp == "HIGH"))
      pp = HSTSHeuristics::HIGH;
    else
      check_error(false);

    m_heuristics.setDefaultPriorityPreference(pp);
    check_error (m_heuristics.getDefaultPriorityPreference() == 0);
  }

  void HSTSHeuristicsReader::readVariableSpecification(const TiXmlElement& element){
    std::cout << "in ReadVariableSpecification" << std::endl;
  }
  void HSTSHeuristicsReader::readTokenSpecification(const TiXmlElement& element){
    std::cout << "in ReadTokenSpecification" << std::endl;
  }
  void HSTSHeuristicsReader::readCompatibility(const TiXmlElement& element){
    std::cout << "in ReadCompatibility" << std::endl;
  }
  void HSTSHeuristicsReader::readPredicateSpec(const TiXmlElement& element){
    std::cout << "in ReadPredicateSpec" << std::endl;
  }
  void HSTSHeuristicsReader::readDecisionPreference(const TiXmlElement& element){
    std::cout << "in ReadDecisionPreference" << std::endl;
  }
  void HSTSHeuristicsReader::readConstrainedVariable(const TiXmlElement& element){
    std::cout << "in ReadConstrainedVariable" << std::endl;
  }
  void HSTSHeuristicsReader::readPreference(const TiXmlElement& element){
    std::cout << "in ReadPreference" << std::endl;
  }
  void HSTSHeuristicsReader::readMaster(const TiXmlElement& element){
    std::cout << "in ReadMaster" << std::endl;
  }
  void HSTSHeuristicsReader::readToken(const TiXmlElement& element) {
    std::cout << "in ReadToken" << std::endl;
  }

  GeneratorId HSTSHeuristicsReader::getGeneratorFromName(std::string genName){ return GeneratorId::noId(); }
}
