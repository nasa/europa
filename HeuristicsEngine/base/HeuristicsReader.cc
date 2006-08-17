#include "HeuristicsReader.hh"
#include "Heuristic.hh"
#include "HeuristicsEngine.hh"
#include "Schema.hh"
#include "Token.hh"

/**
 * @author Conor McGann
 * @note Borrowed heavily from original HSTSHeuristicsReader from Tania.
 */
namespace EUROPA {

  HeuristicsReader::HeuristicsReader(const HeuristicsEngineId& he) : m_he(he){}

  void HeuristicsReader::read(const std::string& fileName, bool initialize){
    TiXmlElement* configXml = initXml(fileName.c_str());
    checkError(configXml != NULL, "Bad test input data from " << fileName);

    for (TiXmlElement * child = configXml->FirstChildElement(); 
         child != NULL; 
         child = child->NextSiblingElement()) {
      const char* tagName = child->Value();
      if(IS_TAG("Defaults"))
        readDefaults(*child);
      else if(IS_TAG("VariableSpecification") ||
              IS_TAG("TokenSpecification"))
        readHeuristic(m_he, *child);
    }

    if(initialize)
      m_he->initialize();
  }


  void HeuristicsReader::readDefaults(const TiXmlElement& element) {
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");
      if (IS_TAG("DefaultCreationPref"))
        readDefaultCreationPref(*child);
      else if (IS_TAG("DefaultPriorityPref"))
        readDefaultPriorityPref(*child);
      else if (IS_TAG("DefaultCompatibility"))
        readDefaultCompatibility(*child);
      else if (IS_TAG("DefaultToken"))
        readDefaultToken(*child);
      else if (IS_TAG("DefaultConstrainedVariable"))
        readDefaultConstrainedVariable(*child);
    }
  }

  void HeuristicsReader::readDefaultCreationPref(const TiXmlElement& element) {
    std::string tmp = getTextChild(element);
    bool preferNewer = true;
    if ((tmp == "older") || (tmp == "OLDER"))
      preferNewer = false;
    else {
      checkError((tmp == "newer" || tmp == "NEWER"), "Unexpected value " << tmp << " for DefaultCreationPref.");
    }
    m_he->setDefaultCreationPreference(preferNewer);
  }

  void HeuristicsReader::readDefaultPriorityPref(const TiXmlElement& element) {
    std::string tmp = getTextChild(element);
    bool preferLowPriority = true;
    if ((tmp == "high") || (tmp == "HIGH"))
      preferLowPriority = false;
    else {
      checkError((tmp == "low" || tmp == "LOW"), "Unexpected value " << tmp << " for DefaultPriorityPref.");
    }
    m_he->setDefaultPriorityPreference(preferLowPriority);
  }

  void HeuristicsReader::readDefaultCompatibility(const TiXmlElement& element){
    Priority p = 0;
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Compatibility.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Compatibility tag.");
      if (IS_TAG("Priority"))
        readPriority(*child,p);
      else if (IS_TAG("PredicateSpec")) {
        LabelStr pred; 
        std::vector< GuardEntry > domainSpec;
        readPredicateSpec(*child, pred, domainSpec);
        (new DefaultCompatibilityHeuristic(m_he, pred, p, domainSpec))->getId();
      }
      else check_error(false, "Expected Priority or PredicateSpec tag in compatibility specification.");
    }
  }

  void HeuristicsReader::readDefaultToken(const TiXmlElement& element) {
    Priority p;
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Token.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Token tag.");
      if (IS_TAG("Priority")) {
        readPriority(*child,p);
        m_he->setDefaultPriorityForToken(p);
      }
      else if (IS_TAG("DecisionPreference")) {
        std::vector<LabelStr> states;
        std::vector<TokenHeuristic::CandidateOrder> orders;
        states.reserve(5); // type allows for at most 5 states and orders
        orders.reserve(5);
        readDecisionPreference(*child, states, orders);
        m_he->setDefaultPreferenceForToken(states,orders);
      }
    }
  }

  void HeuristicsReader::readDefaultConstrainedVariable(const TiXmlElement& element){
    Priority p;
    VariableHeuristic::DomainOrder dorder;
    std::list<LabelStr> values;
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected State Order");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected State Order tag");
      if (IS_TAG("Priority")) {
        readPriority(*child,p);
        m_he->setDefaultPriorityForConstrainedVariable(p);
      }
      else if (IS_TAG("ValueOrder")) {
        readValueOrder(*child, dorder);
        m_he->setDefaultPreferenceForConstrainedVariable(dorder);
      }
      else
        check_error(false, "Expected Priority or Preference in DefaultConstrainedVariable.");
    }
  }

  void HeuristicsReader::readHeuristic(const HeuristicsEngineId& he, 
                                       const TiXmlElement& configData){
    for (TiXmlElement * child = configData.FirstChildElement(); 
         child != NULL; 
         child = child->NextSiblingElement()) {
      const char* tagName = child->Value();
      if(strcmp(tagName, "ConstrainedVariable") == 0 ||
         strcmp(tagName, "Token") == 0)
        createHeuristic(he, *child);
    }

  }

  HeuristicId HeuristicsReader::createHeuristic(const HeuristicsEngineId& he, 
                                                const TiXmlElement& configData){
    checkError(strcmp(configData.Value(), "ConstrainedVariable") == 0 ||
               strcmp(configData.Value(), "Token") == 0 ,
               "Configuration file error. " << configData.Value());

    bool isToken = (strcmp(configData.Value(), "Token") == 0);
 
    // Initialize the data to gather
    LabelStr variableName(EMPTY_LABEL());
    bool mustBeOrphan(false);
    Priority priority(0);
    LabelStr predicate("NO_PREDICATE");
    std::vector< GuardEntry> guards;
    LabelStr masterPredicate("NO_PREDICATE");
    MasterRelation masterRelation(Heuristic::DONT_CARE);
    std::vector< GuardEntry> masterGuards;

    // Variable specific data
    VariableHeuristic::DomainOrder domainOrder = VariableHeuristic::ASCENDING; // The default basically
    std::list<double> values;

    // Tken specific data
    std::vector< LabelStr> states;
    std::vector< TokenHeuristic::CandidateOrder> orders;

    // Initialize flags to make sure we got all the data we need
    bool priorityFound(false);
    bool preferenceFound(false);
    bool predicateSpecFound(false);

    TiXmlElement * variableSpec = NULL; // Will process this later when assigned.

    for (TiXmlElement * child = configData.FirstChildElement(); 
         child != NULL; 
         child = child->NextSiblingElement()) {
      const char* tagName = child->Value();

      if(strcmp(PRIORITY_TAG(), tagName) == 0){
        check_error(!priorityFound);
        priorityFound = readPriority(*child, priority);
        check_error(priorityFound);
      }
      else if(strcmp(PREFERENCE_TAG(), tagName) == 0){
        check_error(!preferenceFound);
        preferenceFound = readPreference(*child, domainOrder, values);
        check_error(preferenceFound);
      }
      else if(strcmp(VARIABLE_TAG(), tagName) == 0){
        variableSpec = child; // Defer till end
      }
      else if(strcmp(PREDICATE_SPEC_TAG(), tagName) == 0){
        check_error(!predicateSpecFound);
        predicateSpecFound = readPredicateSpec(*child, predicate, guards);
        check_error(predicateSpecFound);
      }
      else if(strcmp(MASTER_SPEC_TAG(), tagName) == 0){
        readMaster(*child, masterRelation, masterPredicate, masterGuards);
      }
      else if(strcmp("DecisionPreference", tagName) == 0){
        readDecisionPreference(*child, states, orders);
      }
    }

    checkError(isToken || variableSpec != NULL, "Variables require a variable target.");
    checkError(!isToken || variableSpec == NULL, "Tokens should not have a variable target.");

    HeuristicId heuristic;

    if(variableSpec != NULL){
      readVariable(*variableSpec, predicate, variableName);

      // Special case - hack for lack of explicit indication of use of default. 
      // If:
      // a) we are using a low priority preference
      // b) priority == -1
      // 
      // Then set the priority to the default variable priority
      if(priority == -1)
        priority = he->getDefaultVariablePriority();

      if(masterRelation == Heuristic::DONT_CARE){
        heuristic = (new VariableHeuristic(he, 
                                           predicate,
                                           variableName,
                                           priority,
                                           values,
                                           domainOrder,
                                           mustBeOrphan,
                                           guards ))->getId();
      }
      else { 
        heuristic = (new VariableHeuristic(he, 
                                           predicate,
                                           variableName,
                                           priority,
                                           values,
                                           domainOrder,
                                           guards,
                                           masterPredicate,
                                           masterRelation,
                                           masterGuards))->getId();
      }
    }
    else {
      // Special case - hack for lack of explicit indication of use of default. 
      // If:
      // a) we are using a low priority preference
      // b) priority == -1
      // 
      // Then set the priority to the default variable priority
      if(priority == -1)
        priority = he->getDefaultTokenPriority();

      if(masterRelation == Heuristic::DONT_CARE){
        heuristic = (new TokenHeuristic(he, 
                                        predicate,
                                        priority,
                                        states,
                                        orders,
                                        mustBeOrphan,
                                        guards ))->getId();
      }
      else { 
        heuristic = (new TokenHeuristic(he, 
                                        predicate,
                                        priority,
                                        states,
                                        orders,
                                        guards,
                                        masterPredicate,
                                        masterRelation,
                                        masterGuards))->getId();
      }
    }

    return heuristic;
  }

  bool HeuristicsReader::readPriority(const TiXmlElement& configData, Priority& priority){
    const char* data = getTextChild(configData);
    priority = atof(data); // priority is a double!!
    return true;
  }
    
  bool HeuristicsReader::readPreference(const TiXmlElement& element, 
                                        VariableHeuristic::DomainOrder& dorder,
                                        std::list<double>& values) {
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Preference.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Preference tag.");
      if (strcmp("ValueOrder", tagName) == 0) {
        readValueOrder(*child, dorder);
        check_error(dorder != VariableHeuristic::VGENERATOR &&
                    dorder != VariableHeuristic::ENUMERATION, 
                    "Wrong type of value order, expected ascending or descending");
      }
      else if (strcmp("DomainOrder", tagName) == 0) {
        dorder = VariableHeuristic::ENUMERATION;
        readDomainOrder(*child, values);
      }
      else 
        checkError(ALWAYS_FAILS, "Unexpected stuff! Found " << tagName << " for preferences.");
    }

    return true;
  }

  void HeuristicsReader::readValueOrder(const TiXmlElement& element, VariableHeuristic::DomainOrder& order) {
    std::string dorder = getTextChild(element);
    switch (dorder[0]) {
    case 'a':
      order = VariableHeuristic::ASCENDING;
      break;
    case 'd':
      order = VariableHeuristic::DESCENDING;
      break;
    case 'e':
      order = VariableHeuristic::ENUMERATION;
      break;
    case 'v':
      order = VariableHeuristic::VGENERATOR;
      break;
    default:
      checkError(ALWAYS_FAILS, "Unknown Value Order:" << dorder);
      break;
    }
  }

  void HeuristicsReader::readDomainOrder(const TiXmlElement& element, std::list<double>& values) {
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected DomainOrder.");
      double value;
      readValue(*child, value);
      values.push_back(value);
    }
  }

  bool HeuristicsReader::readVariable(const TiXmlElement& configData, const LabelStr& predicate, LabelStr& varName){
    check_error(configData.FirstChildElement() != NULL);
    const char* data = getTextChild(*(configData.FirstChildElement()));
    double value;
    if(isNumber(data, value)){
      unsigned int index = (unsigned int) value;
      checkError(value == index, "Bad index " << data);
      varName = Schema::instance()->getNameFromIndex(predicate, index);
    }
    else
      varName = LabelStr(data);

    return true;
  }

  bool HeuristicsReader::readPredicateSpec(const TiXmlElement& configData, 
                                           LabelStr& predicate,
                                           std::vector< GuardEntry >& guards){
    
    for (TiXmlElement* child = configData.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Default Specification.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Default Specification tag.");

      if(strcmp(PREDICATE_NAME_TAG(), tagName) == 0)
        predicate = getTextChild(*child);
      else if(strcmp(PREDICATE_PARAMETERS_TAG(), tagName) == 0)
        readGuards(*child, predicate, guards);
      else
        checkError(ALWAYS_FAILS, 
                   "Expected " << PREDICATE_NAME_TAG() << " or " << PREDICATE_PARAMETERS_TAG() << " but " << tagName);

    }
    return true;
  }

  void HeuristicsReader::readGuards(const TiXmlElement& element, const LabelStr& predicate, 
                                    std::vector<GuardEntry>& guards) {

    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected PredicateParameters.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected PredicateParameters tag.");

      if (strcmp(PREDICATE_PARAMETER_TAG(), tagName) == 0) {
        int index = -1;
        double value;
        readParameter(*child, predicate, index, value);
        check_error(value != EMPTY_LABEL());
        check_error(index >= 0);
        guards.push_back(GuardEntry(index, value));
      }
      else 
        checkError(ALWAYS_FAILS, "Expected <" << PREDICATE_PARAMETERS_TAG() << "> but found <" << tagName << ">");
    }
  }

  void HeuristicsReader::readParameter(const TiXmlElement& element, const LabelStr& predicate, 
                                       int& index, double& value) {
    for (TiXmlElement* child = element.FirstChildElement(); child != NULL;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Parameter");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Parameter tag");
      if (strcmp("Index", tagName) == 0)
        readIndex(*child, index);
      else if (strcmp("Name", tagName) == 0){
        const char* varName = getTextChild(*child);
        index = Schema::instance()->getIndexFromName(predicate, varName);
      }
      else if (strcmp("Value", tagName) == 0)
        readValue(*child, value);
      else
        checkError(ALWAYS_FAILS, "Expected Index, Name or Value tag in Parameter." << tagName);
    }
  }

  void HeuristicsReader::readIndex(const TiXmlElement& element, int& index) {
    index = atoi(getTextChild(element));
  }

  /**
   * @note Treating bools as numbers
   */
  void HeuristicsReader::readValue(const TiXmlElement& element, double& value) {  
    const char* data = getTextChild(element);

    if(isNumber(data, value))
      return;

    if(strcmp(data, "true") == 0 || strcmp(data, "TRUE") == 0)
      value = 1;
    else if(strcmp(data, "false") == 0 || strcmp(data, "FALSE") == 0)
      value = 0;
    else {
      LabelStr lblStr(getTextChild(element));
      // Cast to a double
      value = (double) lblStr;
    }
  }

  bool HeuristicsReader::readMaster(const TiXmlElement& element, 
                                    MasterRelation& rel,
                                    LabelStr& masterPredicate,
                                    std::vector< GuardEntry >& guards ) {
    
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Master.");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Master tag.");
      if (strcmp("Relation", tagName) == 0)
        readRelation(*child,rel);
      else if (strcmp("PredicateSpec", tagName) == 0)
        readPredicateSpec(*child, masterPredicate, guards);
      else
        check_error(ALWAYS_FAILS, "Unexpected stuff!");
    }

    return true;
  }

  void HeuristicsReader::readRelation(const TiXmlElement& element, MasterRelation& rel) {
    std::string srel = getTextChild(element);
    switch (srel[0]) {
    case 'a':
      if (srel[1] == 'f')
        rel = Heuristic::AFTER;
      else if (srel[1] == 'n')
        rel = Heuristic::ANY;
      else
        check_error(ALWAYS_FAILS, "Unknown relation");
      break;
    case 'b':
      rel = Heuristic::BEFORE;
      break;
    case 'o':
      rel = Heuristic::OTHER;
      break;
    case 'n':
      rel = Heuristic::NONE;
      break;
    default:
      check_error(ALWAYS_FAILS, "Unknown relation");
      break;
    }
  }


  void HeuristicsReader::readDecisionPreference(const TiXmlElement& element, 
                                                std::vector<LabelStr>& states,
                                                std::vector<TokenHeuristic::CandidateOrder>& orders) {
    check_error(states.empty(), "States should be empty.");
    check_error(orders.empty(), "Orders should be empty.");

    for (TiXmlElement* child = element.FirstChildElement();
         child != NULL;
         child = child->NextSiblingElement()) {
      check_error(child != NULL, "Expected Decision Preference"); 
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected Decision Preference tag.");

      LabelStr state;
      TokenHeuristic::CandidateOrder order(TokenHeuristic::EARLY);

      if(strcmp("StateOrder", tagName) == 0)
        readStateOrder(*child, state, order);
      else {
        checkError(strcmp("State", tagName) == 0, "Expected State Decision Preference." << tagName);
        readState(*child, state);
      }

      states.push_back(state);
      orders.push_back(order);
    }
  }

  void HeuristicsReader::readStateOrder(const TiXmlElement& element, 
                                        LabelStr& state, 
                                        TokenHeuristic::CandidateOrder& order) {
    unsigned int tagCount = 0;
    for (TiXmlElement* child = element.FirstChildElement(); child;
         child = child->NextSiblingElement()) {
      check_error(tagCount < 2, "At most 2 child tags allowed.");
      check_error(child != NULL, "Expected State Order");
      const char* tagName = child->Value();
      check_error(tagName != NULL, "Expected State Order tag");

      if (strcmp("State", tagName) == 0)
        readState(*child, state);
      else {
        checkError(strcmp("Order", tagName) == 0, "Only alternative is Order." << tagName);
	
        std::string ostr = getTextChild(*child);
        switch(ostr[0]) {
        case 't':
          order = TokenHeuristic::TGENERATOR;
          break;
        case 'n': 
          if (ostr[1] == 'e')
            order = TokenHeuristic::NEAR;
          else if (ostr[1] == 'o')
            order = TokenHeuristic::NONE;
          else
            check_error(ALWAYS_FAILS, "Unknown order");
          break;
        case 'f':
          order = TokenHeuristic::FAR;
          break;
        case 'e':
          order = TokenHeuristic::EARLY;
          break;
        case 'l': 
          if (ostr[1] == 'a')
            order = TokenHeuristic::LATE;
          else if (ostr[1] == 'e')
            order = TokenHeuristic::LEAST_SPECIFIED; 
          else
            check_error(ALWAYS_FAILS, "Unknown order");
          break;
        case 'm':
          if (ostr[1] == 'a')
            order = TokenHeuristic::MAX_FLEXIBLE;
          else if (ostr[1] == 'i')
            order = TokenHeuristic::MIN_FLEXIBLE;
          else if (ostr[1] == 'o')
            order = TokenHeuristic::MOST_SPECIFIED;
          else
            check_error(ALWAYS_FAILS, "Unknown order");
          break;
        default:
          check_error(ALWAYS_FAILS, "Unknown order");
          break;
        }
      }

      tagCount++;
    }

    checkError(tagCount > 0, "Must have been at least one tag but there wasn't");
  }

  void HeuristicsReader::readState(const TiXmlElement& element, LabelStr& state) {
    std::string s = getTextChild(element);
    if (s == "activate")
      state = Token::ACTIVE;
    else if (s == "merge")
      state = Token::MERGED;
    else if (s == "reject")
      state = Token::REJECTED;
    else{
      assertTrue(state != EMPTY_LABEL(), 
                 "Expected one of activate, merge, or reject. Defer is not implemented yet.");
    }
  }
}
