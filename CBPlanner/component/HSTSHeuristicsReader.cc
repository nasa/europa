#include "HSTSHeuristicsReader.hh"
#include <fstream>
#include "tinyxml.h"
#include "HSTSHeuristics.hh"
#include "LabelStr.hh"
#include "Schema.hh"

namespace PLASMA {

#define IS_TAG(x) (strcmp (tagName, x) == 0)

  HSTSHeuristicsReader::HSTSHeuristicsReader(HSTSHeuristicsId& heuristics, const SchemaId& schema) : m_heuristics(heuristics), m_schema(schema) { }

  HSTSHeuristicsReader::~HSTSHeuristicsReader() { check_error(m_heuristics.isValid()); m_heuristics.remove();}

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
      if (IS_TAG("DefaultPriorityPref")) 
	readDefaultPriorityPref(*child);
      else if (IS_TAG("DefaultCompatibility"))
	readDefaultCompatibility(*child);
      else if (IS_TAG("DefaultToken"))
	readDefaultToken(*child);
      else if (IS_TAG("DefaultConstrainedVariable"))
	readDefaultConstrainedVariable(*child);
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

  void HSTSHeuristicsReader::readDefaultPriorityPref(const TiXmlElement& element) {
    std::cout << "in ReadDefaultPriorityPref" << std::endl;

    std::string tmp = getTextChild(element);
    HSTSHeuristics::PriorityPref pp;
    if ((tmp == "low") || (tmp == "LOW"))
      pp = HSTSHeuristics::LOW;
    else if ((tmp == "high") || (tmp == "HIGH"))
      pp = HSTSHeuristics::HIGH;
    else
      check_error(false, "Unexpected value for DefaultPriorityPref.");

    std::cout << "     priority pref = " << pp << std::endl;
    m_heuristics->setDefaultPriorityPreference(pp);
    check_error (m_heuristics->getDefaultPriorityPreference() == 1, "Expected priority preference to be high on Heuristics-HSTS.xml.");
  }

  void HSTSHeuristicsReader::readVariableSpecification(const TiXmlElement& element){
    std::cout << "in ReadVariableSpecification" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected ConstrainedVariable.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected ConstrainedVariable tag.");
      if (IS_TAG("ConstrainedVariable")) 
	readConstrainedVariable(*child);
      else
	check_error(false, "ConstrainedVariable is the only tag allowed in Variable Specification.");
    }
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
	check_error(false,  "Token is the only tag allowed in Token Specification.");
    }
  }

  void HSTSHeuristicsReader::readDefaultCompatibility(const TiXmlElement& element){
    std::cout << "in ReadDefaultCompatibility" << std::endl;

    HSTSHeuristics::Priority p;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Compatibility.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Compatibility tag.");
      if (IS_TAG("Priority")) {
	readPriority(*child,p);
      }
      else if (IS_TAG("PredicateSpec")) {
	LabelStr pred(NO_STRING); 
	std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
	readPredicateSpec(*child, pred, domainSpec);
	TokenType tt(pred,domainSpec);
	std::cout << " initialized predicate spec with predicate = " << tt.getPredicate().c_str() << std::endl;
	m_heuristics->setDefaultPriorityForTokenDPsWithParent(p, tt.getId());
      }
      else
	check_error(false, "Expected Priority or PredicateSpec tag in compatibility specification.");
    }

  }

  void HSTSHeuristicsReader::readDefaultToken(const TiXmlElement& element) {
    std::cout << "in ReadDefaultToken" << std::endl;

    HSTSHeuristics::Priority p;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Token.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Token tag.");
      if (IS_TAG("Priority")) {
	readPriority(*child,p);
	m_heuristics->setDefaultPriorityForTokenDPs(p);
      }
      else if (IS_TAG("DecisionPreference")) {
	std::vector<LabelStr> states;
	std::vector<HSTSHeuristics::CandidateOrder> orders;
	states.reserve(5); // type allows for at most 5 states and orders
	orders.reserve(5);
	readDecisionPreference(*child, states, orders);
	m_heuristics->setDefaultPreferenceForTokenDPs(states,orders);
      }
    }
  }

  void HSTSHeuristicsReader::readDefaultConstrainedVariable(const TiXmlElement& element){
    std::cout << "in ReadDefaultConstrainedVariable" << std::endl;

    HSTSHeuristics::Priority p;
    LabelStr genName(NO_STRING);
    HSTSHeuristics::DomainOrder dorder;
    std::vector<LabelStr> values;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected State Order");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected State Order tag");
      if (IS_TAG("Priority")) {
	readPriority(*child,p);
	m_heuristics->setDefaultPriorityForConstrainedVariableDPs(p);
      }
      else if (IS_TAG("Preference")) {
	readPreference(*child, genName, dorder, values);
	check_error(genName == NO_STRING, "Expected Generator Name to be empty.");
	check_error(values.empty(), "Expected values to be empty.");
	m_heuristics->setDefaultPreferenceForConstrainedVariableDPs(dorder);
      }
      else
	check_error(false, "Expected Priority or Preference in DefaultConstrainedVariable.");
    }
  }

  void HSTSHeuristicsReader::readPriority(const TiXmlElement& element, HSTSHeuristics::Priority& p) {
    std::cout << "in ReadPriority" << std::endl;
    std::string tmp = getTextChild(element);
    p = atof(tmp.c_str()); // priority is a double!!
    std::cout << "  Priority = " << p << std::endl;
  }

  void HSTSHeuristicsReader::readPredicateSpec(const TiXmlElement& element, LabelStr& pred, std::vector<std::pair<LabelStr,LabelStr> >& domainSpec) {
    std::cout << "in ReadPredicateSpec" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
      if (IS_TAG("PredicateName")) {
	pred = getTextChild(*child);
	std::cout << "   Predicate = " << pred.c_str() << std::endl;
      }
      else if (IS_TAG("PredicateParameters")) {
	readPredicateParameters(*child, domainSpec);
      }
      else
	check_error(false, "Expected PredicateName or PredicateParameters in PredicateSpec.");
    }
  }

  void HSTSHeuristicsReader::readPredicateParameters(const TiXmlElement& element, std::vector<std::pair<LabelStr,LabelStr> >& domainSpecs) {
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected PredicateParameters.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected PredicateParameters tag.");
      if (IS_TAG("Parameter")) {
	int index;
	char* value;
	readParameter(*child, index, value);
      }
      else 
	check_error(false, "Expected Parameter tag in PredicateParameters.");
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
	readIndex(*child,index);
      }
      else if (IS_TAG("Value")) {
	value = const_cast<char*>(getTextChild(*child).c_str());
	std::cout << "   Value = " << value << std::endl; 
      }
      else
	check_error(false, "Expected Index or Value tag in Parameter.");
    }
  }

  void HSTSHeuristicsReader::readIndex(const TiXmlElement& element, int& index) {
    index = atoi(getTextChild(element).c_str());
    std::cout << "   Index = " << index << std::endl;
  }

  void HSTSHeuristicsReader::readDecisionPreference(const TiXmlElement& element, std::vector<LabelStr>& states, std::vector<HSTSHeuristics::CandidateOrder>& orders){
    std::cout << "in ReadDecisionPreference" << std::endl;

    check_error(states.empty(), "States should be empty.");
    check_error(orders.empty(), "Orders should be empty.");
    std::vector<LabelStr>::iterator s_pos = states.begin();
    std::vector<HSTSHeuristics::CandidateOrder>::iterator o_pos = orders.begin();
    LabelStr state(NO_STRING);
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
	check_error(false, "Expected StateOrder or State tag in DecisionPreference.");
    }
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
	check_error(false, "Expected Order or State tag in StateOrder.");
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
    int index=-10;// any negative number to symbolize index has not been initialized
    LabelStr varName(NO_STRING);
    LabelStr pred(NO_STRING); 
    std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
    LabelStr genName(NO_STRING);
    HSTSHeuristics::DomainOrder dorder;
    std::vector<LabelStr> values;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected ConstrainedVariable.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected ConstrainedVariable tag");
      if (IS_TAG("Priority"))
	readPriority(*child,p);
      else if (IS_TAG("Preference")) 
	readPreference(*child, genName, dorder, values);
      else if (IS_TAG("VariableSpec"))
	readVariableSpec(*child, index, varName);
      else if (IS_TAG("PredicateSpec"))
	readPredicateSpec(*child, pred, domainSpec);
      else
	check_error(false, "Expected Priority, Preference, VariableSpec, or PredicateSpec in ConstrainedVariable");
    }
    if (index>=0) {
      check_error(varName == NO_STRING, "Expected varName to be empty.");
      varName = m_schema->getNameFromIndex(pred,index);
    } else {
      check_error(varName != NO_STRING, "Expected varName to not be empty.");
      check_error(index == -10, "Index should be uninitialized.");
    }
    TokenType tt(pred,domainSpec);
    m_heuristics->setHeuristicsForConstrainedVariableDP(p, varName, tt.getId(), dorder, genName, values);
  }

  void HSTSHeuristicsReader::readVariableSpec(const TiXmlElement& element, int& index, LabelStr& varName) {
    std::cout << "in ReadVariableSpec" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected VariableSpec");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected VariableSpec tag");
      if (IS_TAG("Index"))
	readIndex(*child,index);
      else if (IS_TAG("Name")) {
	varName = getTextChild(*child);
	std::cout << "   Name = " << varName.c_str() << std::endl;
      }
      else
	check_error(false, "Unexpected Stuff!");
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
    std::cout << "     valueOrder = " << order << std::endl;
  }
    
  void HSTSHeuristicsReader::readPreference(const TiXmlElement& element, LabelStr& genName, HSTSHeuristics::DomainOrder& dorder, std::vector<LabelStr>& values) {
    std::cout << "in ReadPreference" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Preference.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Preference tag.");
      if (IS_TAG("Generator")) {
	dorder = HSTSHeuristics::VGENERATOR;
	LabelStr genName = getTextChild(*child);
      }
      else if (IS_TAG("ValueOrder")) {
	readValueOrder(*child,dorder);
	check_error(dorder != HSTSHeuristics::VGENERATOR &&
		    dorder != HSTSHeuristics::ENUMERATION, "Wrong type of value order, expected ascending or descending");
      }
      else if (IS_TAG("DomainOrder")) {
	dorder = HSTSHeuristics::ENUMERATION;
	readDomainOrder(*child,values);
      }
      else 
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readDomainOrder(const TiXmlElement& element, std::vector<LabelStr>& values) {
    std::cout << "in ReadDomainOrder" << std::endl;

    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected DomainOrder.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected DomainOrder tag.");
      if (IS_TAG("Value")) {
	values.push_back(getTextChild(*child));
      }
      else 
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readToken(const TiXmlElement& element) {
    std::cout << "in ReadToken" << std::endl;

    HSTSHeuristics::Priority p;
    LabelStr pred(NO_STRING); 
    std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
    std::vector<LabelStr> states;
    std::vector<HSTSHeuristics::CandidateOrder> orders;
    states.reserve(5); // type allows for at most 5 states and orders
    orders.reserve(5);
    TokenTypeId ttm;
    HSTSHeuristics::Relationship rel;
    for (TiXmlElement* child = element.FirstChildElement(); child;
	 child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
      if (IS_TAG("Priority"))
	readPriority(*child,p);
      else if (IS_TAG("PredicateSpec"))
	readPredicateSpec(*child, pred, domainSpec);
      else if (IS_TAG("Master"))
	readMaster(*child, rel, ttm);
      else if (IS_TAG("DecisionPreference"))
	readDecisionPreference(*child, states, orders);
    }
    TokenType tt(pred,domainSpec);
    m_heuristics->setHeuristicsForTokenDP(p, tt.getId(), rel, ttm, states, orders);
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
	LabelStr pred(NO_STRING); 
	std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
	readPredicateSpec(*child, pred, domainSpec);
      }
      else
	check_error(false, "Unexpected stuff!");
    }
  }

  void HSTSHeuristicsReader::readRelation(const TiXmlElement& element, HSTSHeuristics::Relationship& rel) {
    std::cout << " in ReadRelation " << std::endl;

    std::string srel = getTextChild(element);
    switch (srel.c_str()[0]) {
    case 'a':
      if (srel.c_str()[1] == 'f')
	rel = HSTSHeuristics::AFTER;
      else if (srel.c_str()[1] == 'n')
	rel = HSTSHeuristics::ANY;
      else check_error(false, "Unknown relation");
      break;
            break;
    case 'b':
      rel = HSTSHeuristics::BEFORE;
      break;
    case 'o':
      rel = HSTSHeuristics::OTHER;
      break;
    default:
      check_error(false, "Unknown relation");
      break;
    }
    std::cout << "     relation = " << rel << std::endl;
  }

  GeneratorId HSTSHeuristicsReader::getGeneratorFromName(std::string genName){ 
    check_error(false, "Not implemented yet.");
    return GeneratorId::noId(); 
  }
}
