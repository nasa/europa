#include "HSTSHeuristicsReader.hh"
#include <fstream>
#include "tinyxml.h"
#include "HSTSHeuristics.hh"
#include "LabelStr.hh"

namespace PLASMA {

#define IS_TAG(x) (strcmp (tagName, x) == 0)

  HSTSHeuristicsReader::HSTSHeuristicsReader(HSTSHeuristics& heuristics) : m_heuristics(heuristics) { }

  HSTSHeuristicsReader::~HSTSHeuristicsReader() { }

  void HSTSHeuristicsReader::read(const std::string& fileName) {

    check_error(fileName != "", "File name is empty.");
    std::cout << "Reading " << fileName << std::endl;

    TiXmlNode *main_node = NULL;
  
    TiXmlDocument doc(fileName);
    bool loadOkay = doc.LoadFile();
    check_error(loadOkay, "File didn't load properly.");

    main_node = doc.FirstChild();
  
    TiXmlElement* element = NULL;
    element = main_node->ToElement();
    const char * tagName = element->Value();    
    check_error(IS_TAG("Heuristics-HSTS"), "Expected Heursitics-HSTS tag.");

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
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
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
    check_error(element.FirstChild(), "FirstChild is empty.");
    //    check_error(element.FirstChild()->ToComment(), "FirstChild->ToComment is empty");
    check_error(element.FirstChild()->ToText(), "FirstChild->ToText is empty.");
    check_error(element.FirstChild()->ToText()->Value(), "FirstChild->ToText->Value is empty.");

    check_error (element.FirstChild() && element.FirstChild()->ToText() && element.FirstChild()->ToText()->Value(), "Element empty, expected data.");
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
      check_error(false, "Unexpected value for PriorityPref.");

    m_heuristics.setDefaultPriorityPreference(pp);
    check_error (m_heuristics.getDefaultPriorityPreference() == 0);
  }

  void HSTSHeuristicsReader::readVariableSpecification(const TiXmlElement& element){
    std::cout << "in ReadVariableSpecification" << std::endl;
  }
  void HSTSHeuristicsReader::readTokenSpecification(const TiXmlElement& element){
    std::cout << "in ReadTokenSpecification" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Token Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Token Specification tag.");
      if (IS_TAG("Token"))
	readToken(*child);
      else 
	check_error(false,  "Unexpected stuff!");
    }
  }
  void HSTSHeuristicsReader::readCompatibility(const TiXmlElement& element){
    std::cout << "in ReadCompatibility" << std::endl;

    HSTSHeuristics::Priority p;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
      if (IS_TAG("Priority")) {
	readPriority(*child,p);
      }
      else if (IS_TAG("PredicateSpec")) {
	TokenTypeId tt;
	readPredicateSpec(*child, tt);
	//	m_heuristics.setDefaultPriorityForTokenDPsWithParent(p, tt);
	//	check_error (m_heuristics.getDefaultPriorityForTokenDPsWithParent(tt) == 24.5);
      }
      else
	check_error(false, "Unexpected tag in compatibility specification.");
    }

  }
  void HSTSHeuristicsReader::readPriority(const TiXmlElement& element, HSTSHeuristics::Priority& p) {
    std::cout << "in ReadPriority" << std::endl;
    std::string tmp = getTextChild(element);
    p = atof(tmp.c_str()); // priority is a double!!
    std::cout << "  Priority = " << p << std::endl;
  }
  void HSTSHeuristicsReader::readPredicateSpec(const TiXmlElement& element, TokenTypeId& tt){
    std::cout << "in ReadPredicateSpec" << std::endl;

    LabelStr pred("");
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
      std::cout << "DefaultSpec tagName = " << tagName << std::endl;
      if (IS_TAG("PredicateName")) {
	/*
	std::cout << "Getting predicate name" << std::endl;
	check_error((*child).FirstChild()->ToElement()->FirstChild()->ToElement()->ToText(), "FirstChild->ToElement is empty");
	std::cout << (*child).FirstChild()->ToElement()->FirstChild()->ToElement()->ToText() << std::endl;
	//	pred = getTextChild(*child);
	std::cout << "Finished getting predicate name" << std::endl;
	*/
      }
      else if (IS_TAG("PredicateParameters")) {
	std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
	readPredicateParameters(*child, domainSpec);
	// read each parameter: for each, create a pair - name,value and
	// add it to the spec. (a vector)
	// when all parameters have been handled, take the predicate name
	// and the domainSpecs and create a token type and assign it to tt.
	//check_error(false, "Not handling predicate parameters yet.");
      }
      else
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readPredicateParameters(const TiXmlElement& element, std::vector<std::pair<LabelStr,LabelStr> >& domainSpecs) {
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
      if (IS_TAG("Parameter")) {
	int index;
	char* value;
	readParameter(*child, index, value);
      }
      else 
	check_error(false, "Unexpected Stuff!");
    }
  }

  void HSTSHeuristicsReader::readParameter(const TiXmlElement& element, int& index, char* value) {
    std::cout << "in ReadParameter" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Parameter");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Parameter tag");
      if (IS_TAG("Index")) {
	index = atoi(getTextChild(*child).c_str());
	std::cout << "   Index = " << index << std::endl;
      }
      else if (IS_TAG("Value")) {
	value = const_cast<char*>(getTextChild(*child).c_str());
	std::cout << "   Value = " << value << std::endl; 
      }
      else
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readDecisionPreference(const TiXmlElement& element){
    std::cout << "in ReadDecisionPreference" << std::endl;

    std::vector<LabelStr> states; 
    states.reserve(5);
    std::vector<HSTSHeuristics::CandidateOrder> orders;
    orders.reserve(5);
    std::vector<LabelStr>::iterator s_pos = states.begin();
    std::vector<HSTSHeuristics::CandidateOrder>::iterator o_pos = orders.begin();
    LabelStr state("");
    HSTSHeuristics::CandidateOrder order;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Decision Preference"); 
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Decision Preference tag.");
      if (IS_TAG("StateOrder")) {
	readStateOrder(*child, state, order);
	states.insert(s_pos,state);
	orders.insert(o_pos,order);
	s_pos++; o_pos++;
      } 
      else if (IS_TAG("State")) {
	readState(*child, state);
	states.insert(s_pos,state);
	orders.insert(o_pos,HSTSHeuristics::NONE);
	s_pos++; o_pos++;
      }
      else
	check_error(false, "Unexpected stuff!");
    }
    m_heuristics.setDefaultPreferenceForTokenDPs(states,orders);
  }

  void HSTSHeuristicsReader::readStateOrder(const TiXmlElement& element, LabelStr& state, HSTSHeuristics::CandidateOrder& order) {
    std::cout << "in ReadStateOrder" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected State Order");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected State Order tag");
      if (IS_TAG("State"))
	readState(*child, state);
      else if (IS_TAG("Order")) {
	std::string ostr = getTextChild(*child);
	std::cout << "   Order = " << ostr << std::endl; 
	switch(ostr.c_str()[0]) {
	case 't': order = HSTSHeuristics::TGENERATOR; break;
	case 'n': 
	  if (ostr.c_str()[1] == 'e')
	    order = HSTSHeuristics::NEAR;
	  else if (ostr.c_str()[1] == 'o')
	    order = HSTSHeuristics::NONE;
	  else check_error(false, "Unknown order");
	  break;
	case 'f': order = HSTSHeuristics::FAR; break;
	case 'e': order = HSTSHeuristics::EARLY; break;
	case 'l': 
	  if (ostr.c_str()[1] == 'a')
	    order = HSTSHeuristics::LATE;
	  else if (ostr.c_str()[1] == 'e')
	    order = HSTSHeuristics::LEAST_SPECIFIED; 
	  else check_error(false, "Unknown order");
	  break;
	case 'm':
	  if (ostr.c_str()[1] == 'a')
	    order = HSTSHeuristics::MAX_FLEXIBLE;
	  else if (ostr.c_str()[1] == 'i')
	    order = HSTSHeuristics::MIN_FLEXIBLE;
	  else if (ostr.c_str()[1] == 'o')
	    order = HSTSHeuristics::MOST_SPECIFIED;
	  else check_error(false, "Unknown order");
	  break;
	default:
	  check_error(false, "Unknown order");
	  break;
	}
      }
      else
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readState(const TiXmlElement& element, LabelStr& state) {
    std::cout << "in ReadState" << std::endl;

    state = getTextChild(element);
    std::cout << "   State = " << state.toString() << std::endl;
  }

  void HSTSHeuristicsReader::readConstrainedVariable(const TiXmlElement& element){
    std::cout << "in ReadConstrainedVariable" << std::endl;

    HSTSHeuristics::Priority p;
    HSTSHeuristics::DomainOrder order;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected State Order");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected State Order tag");
      if (IS_TAG("Priority")) {
	readPriority(*child,p);
	m_heuristics.setDefaultPriorityForConstrainedVariableDPs(p);
      }
      else if (IS_TAG("ValueOrder")) {
	readValueOrder(*child,order);
	m_heuristics.setDefaultPreferenceForConstrainedVariableDPs(order);
      }
      else
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readValueOrder(const TiXmlElement& element, HSTSHeuristics::DomainOrder& order) {
    std::cout << "in ReadValueOrder" << std::endl;

    std::string dorder = getTextChild(element);
    switch (dorder.c_str()[0]) {
    case 'a':
      order = HSTSHeuristics::ASCENDING;
      break;
    case 'd':
      order = HSTSHeuristics::DESCENDING;
      break;
    case 'e':
      order = HSTSHeuristics::ENUMERATION;
      break;
    case 'v':
      order = HSTSHeuristics::VGENERATOR;
      break;
    default:
      check_error(false, "Unknown Value Order");
      break;
    }
  }
    
  void HSTSHeuristicsReader::readPreference(const TiXmlElement& element){
    std::cout << "in ReadPreference" << std::endl;
    std::cout << "  must read domain order for constrained varible " << std::endl;
  }

  void HSTSHeuristicsReader::readToken(const TiXmlElement& element) {
    std::cout << "in ReadToken" << std::endl;

    HSTSHeuristics::Priority p;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
      if (IS_TAG("Priority")) {
	readPriority(*child,p);
	m_heuristics.setDefaultPriorityForTokenDPs(p);
      }
      else if (IS_TAG("PredicateSpec")) {
	TokenTypeId tt;
	readPredicateSpec(*child, tt);
      }
      else if (IS_TAG("Master")) {
	TokenTypeId ttm;
	HSTSHeuristics::Relationship rel;
	readMaster(*child, rel, ttm);
      }
      else if (IS_TAG("DecisionPreference")) {
	readDecisionPreference(*child);
      }
    }
  }

  void HSTSHeuristicsReader::readMaster(const TiXmlElement& element, HSTSHeuristics::Relationship& rel, TokenTypeId& ttm) {
    std::cout << " in ReadMaster " << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Master.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Master tag.");
      if (IS_TAG("Relation")) {
	readRelation(*child,rel);
      }
      else if (IS_TAG("PredicateSpec")) {
	readPredicateSpec(*child, ttm);
      }
      else
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readRelation(const TiXmlElement& element, HSTSHeuristics::Relationship& rel) {
    std::cout << " in ReadRelation " << std::endl;
    std::cout << "    must parse according to all values of Relationship " << std::endl;
  }

  GeneratorId HSTSHeuristicsReader::getGeneratorFromName(std::string genName){ return GeneratorId::noId(); }
}
