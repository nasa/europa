#include "HSTSHeuristics.hh"
#include "PlanDatabaseDefs.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Schema.hh"
#include "Utils.hh"
#include <sstream>

namespace PLASMA {

  // todo: check bounds on priority, if inserting, make sure it wasn't
  // there

  TokenType::TokenType(const LabelStr& predicateName, 
		       const std::vector<std::pair<LabelStr, LabelStr> >& domainSpecs) 
    : m_predicateName(predicateName), m_domainSpecs(domainSpecs), m_id(this) { 
    //    std::cout << "Constructing a token type with predicate " << predicateName.c_str() << " and number of variables " << m_domainSpecs.size() << std::endl;
  }

  TokenType::~TokenType() { check_error(m_id.isValid()); m_id.remove(); }

  const TokenTypeId& TokenType::getId() const {
    return m_id;
  }

  const LabelStr& TokenType::getPredicate() const {
    return m_predicateName;
  }

  const std::vector<std::pair<LabelStr,LabelStr> >& TokenType::getDomainSpecs() const {
    //    std::cout << " Get DomainSpecs for predicate " << m_predicateName.c_str() << " with size = " << m_domainSpecs.size() << std::endl;
    return m_domainSpecs;
  }

  void TokenType::getIndexKey(const TokenTypeId& tt, LabelStr& indexKey) {
    check_error(tt.isValid());
    std::stringstream key;
    check_error(LabelStr::isString(tt->getPredicate()));
    key << tt->getPredicate().c_str();
    std::vector<std::pair<LabelStr,LabelStr> > ds = tt->getDomainSpecs();
    //    std::cout << " getIndexKey for token type with  predicate = " << tt->getPredicate().c_str() << " and domain spec size = " << ds.size() << std::endl;
    for (unsigned int i=0; i < ds.size(); ++i) {
      key << DELIMITER;
      key << ds[i].first.c_str() << "|" << ds[i].second.c_str();
    }
    indexKey = key.str();
  }

  void TokenType::split(const std::string& str, const char& delim, std::vector<std::string>& strings) {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delim, 0);
    // Find first "non-delimiter".
    std::string::size_type pos = str.find_first_of(delim, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
      // Found a token, add it to the vector.
      strings.push_back(str.substr(lastPos, pos - lastPos));
      // Skip delim
      lastPos = str.find_first_not_of(delim, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delim, lastPos);
    }
  }

  // todo: get base domain?  Alternatively, could cache it once it is
  // computed for the first time on a variable instance of the same type
  // todo: if domain is too large, create generator.
  void TokenType::getTokenType(const LabelStr& indexKey, TokenTypeId& tt) {
    std::vector<std::string> strings;
    split(indexKey.toString(),DELIMITER,strings);
    check_error(strings.size() >= 1);
    std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
    for (unsigned int i = 1; i < strings.size();  i++) {
      std::vector<std::string> domstr;
      split(strings[i],'|',domstr);
      check_error(domstr.size() == 2);
      domainSpec.push_back(std::make_pair<LabelStr,LabelStr>(domstr[0],domstr[1]));
    }
    tt = (new TokenType(LabelStr(strings[0]),domainSpec))->getId();
  }

  bool TokenType::matches(const TokenTypeId& tt) {
    LabelStr myIndexKey(NO_STRING);
    TokenType::getIndexKey(getId(),myIndexKey);
    LabelStr indexKey(NO_STRING);
    TokenType::getIndexKey(tt, indexKey);
    return (myIndexKey.getKey() == indexKey.getKey());
  }

  bool TokenType::conflicts(const TokenTypeId& tt) {
    return !matches(tt);
  }

  HSTSHeuristics::TokenEntry::TokenEntry() {}

  // todo: create generators from names
  HSTSHeuristics::TokenEntry::TokenEntry(const Priority p, 
					 const std::vector<LabelStr>& states, 
					 const std::vector<CandidateOrder>& orders)
    : m_priority(p), m_states(states), m_orders(orders) { 
    // create generators from names
  }

  HSTSHeuristics::TokenEntry::~TokenEntry() {}

  void HSTSHeuristics::TokenEntry::setPriority(const Priority p) { 
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    m_priority = p; 
  }

  const HSTSHeuristics::Priority HSTSHeuristics::TokenEntry::getPriority() const { return m_priority; }

  const std::vector<LabelStr>& HSTSHeuristics::TokenEntry::getStates() const {
    return m_states;
  }

  const std::vector<HSTSHeuristics::CandidateOrder>& HSTSHeuristics::TokenEntry::getOrders() const {
    return m_orders;
  }
  
  HSTSHeuristics::VariableEntry::VariableEntry() {}

  HSTSHeuristics::VariableEntry::VariableEntry( const std::set<LabelStr>& domain,
						const Priority p, 
						const DomainOrder order,
						const LabelStr& generatorName,
						const std::list<LabelStr>& enumeration)
    : m_priority(p) {
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    if (!enumeration.empty())
      m_domain = enumeration;
    switch (order) {
    case VGENERATOR:
      // create generator from name
      //      std::cout << " Generators such as " << generatorName.c_str() << " are not yet supported" << std::endl;
      break;
    case ASCENDING:
      if (m_domain.empty()) {
	for (std::set<LabelStr>::const_iterator it(domain.begin()); it != domain.end(); ++it)
	  m_domain.push_back(*it);
	m_domain.sort();
	/*
	std::cout << " ASCENDING domain =";
	for (std::list<LabelStr>::const_iterator it(m_domain.begin()); it != m_domain.end(); ++it) {
	  std::cout << " "; 
	  std::cout << "value[//]"; //(*it).c_str();
	}
	std::cout << std::endl;
	*/
      }
      break;
    case DESCENDING:
      if (m_domain.empty()) {
	for (std::set<LabelStr>::const_iterator it(domain.begin()); it != domain.end(); ++it)
	  m_domain.push_back(*it);
	m_domain.sort();
	/*
	std::cout << " DESCENDING domain =";
	for (std::list<LabelStr>::const_iterator it(m_domain.begin()); it != m_domain.end(); ++it) {
	  std::cout << " ";
	  std::cout << "value[\\]"; //<< (*it).c_str();
	}
	std::cout << std::endl;
	*/
      }
      m_domain.reverse();
      break;
    case ENUMERATION: 
      /*
      std::cout << "Enumeration" << std::endl;
      for (std::list<LabelStr>::const_iterator it(m_domain.begin()); it != m_domain.end(); ++it) {
	std::cout << " ";
	std::cout << (*it).c_str();
      }
      std::cout << std::endl;
      */
      check_error(!enumeration.empty());
      check_error(!m_domain.empty());
      check_error(m_domain.size() == enumeration.size());
      break;
    default:
      check_error(ALWAYS_FAILS);
    }
  }

  HSTSHeuristics::VariableEntry::~VariableEntry() {}

  void HSTSHeuristics::VariableEntry::setDomain(const std::list<LabelStr>& domain) {
    m_domain = domain;
  }

  const HSTSHeuristics::Priority HSTSHeuristics::VariableEntry::getPriority() const { return m_priority; }
  
  const std::list<LabelStr>& HSTSHeuristics::VariableEntry::getDomain() const { return m_domain; }

  const HSTSHeuristics::DomainOrder& HSTSHeuristics::VariableEntry::getDomainOrder() const { return m_order; }
  
  const GeneratorId& HSTSHeuristics::VariableEntry::getGenerator() const { return m_generator; }

  HSTSHeuristics::HSTSHeuristics(const PlanDatabaseId& planDatabase) : m_id(this), m_pdb(planDatabase) {
    m_defaultPriorityPreference = LOW;
    m_defaultTokenPriority = 0.0;
    m_defaultVariablePriority = 0.0;
    m_defaultDomainOrder = ASCENDING;
  }

  HSTSHeuristics::~HSTSHeuristics() {
    check_error(m_id.isValid());
    m_id.remove();
  }

  const HSTSHeuristicsId& HSTSHeuristics::getId() const { return m_id; }
  HSTSHeuristicsId& HSTSHeuristics::getNonConstId() { return m_id; }

  void HSTSHeuristics::setDefaultPriorityPreference(const PriorityPref pp) {
    check_error(m_defaultPriorityPreference <= 1);
    check_error(m_defaultPriorityPreference >= 0);
    m_defaultPriorityPreference = pp;
  }

  const HSTSHeuristics::PriorityPref HSTSHeuristics::getDefaultPriorityPreference() const {
    return m_defaultPriorityPreference;
  }

  void HSTSHeuristics::setDefaultPriorityForTokenDPsWithParent(const Priority p, const TokenTypeId& tt) {
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    LabelStr indexKey(NO_STRING);
    TokenType::getIndexKey(tt,indexKey);
    m_defaultCompatibilityPriority.insert(std::make_pair<LabelStr,Priority>(indexKey, p));
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getDefaultPriorityForTokenDPsWithParent(const TokenTypeId& tt) const {
    check_error(false);
    return 0.0;
  }

  void HSTSHeuristics::setDefaultPriorityForTokenDPs(const Priority p) {
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    m_defaultTokenPriority = p;
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getDefaultPriorityForTokenDPs() const {
    return m_defaultTokenPriority;
  }

  void HSTSHeuristics::setDefaultPriorityForConstrainedVariableDPs(const Priority p) {
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    m_defaultVariablePriority = p;
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getDefaultPriorityForConstrainedVariableDPs() const {
    return m_defaultVariablePriority;
  }

  void HSTSHeuristics::setDefaultPreferenceForTokenDPs(const std::vector<LabelStr>& states, 
						       const std::vector<CandidateOrder>& orders) {
    check_error(states.size() == orders.size());
    m_defaultTokenStates = states;
    m_defaultCandidateOrders = orders;
  }

  void HSTSHeuristics::setDefaultPreferenceForConstrainedVariableDPs(const DomainOrder order) {
    m_defaultDomainOrder = order;
  }

  const HSTSHeuristics::DomainOrder HSTSHeuristics::getDefaultPreferenceForConstrainedVariableDPs() const { return m_defaultDomainOrder; }

  void HSTSHeuristics::setHeuristicsForConstrainedVariableDP(const Priority p, 
							     const LabelStr variableName, 
							     const TokenTypeId& tt, 
							     const DomainOrder order,
							     const LabelStr& generatorName, 
							     const std::list<LabelStr>& enumeration) {
    LabelStr key(HSTSHeuristics::getIndexKey(variableName,tt));
    check_error(tt.isValid());
    LabelStr pred(tt->getPredicate());
    unsigned int index = Schema::instance()->getIndexFromName(pred, variableName);
    LabelStr varType(Schema::instance()->getParameterType(pred, index));

    std::set<LabelStr> values; // we instantiate values lazily upon evaluation
    if (generatorName == NO_STRING) {
      VariableEntry entry(values, p, order, NO_STRING, enumeration);
      m_variableHeuristics.insert(std::make_pair<LabelStr,VariableEntry>(key, entry));
    }
    else  {
      VariableEntry entry(values, p, order, generatorName, enumeration);
      m_variableHeuristics.insert(std::make_pair<LabelStr,VariableEntry>(key, entry));
    }
  }

  void HSTSHeuristics::setHeuristicsForTokenDP(const Priority p,
					       const TokenTypeId& tt, 
					       const Relationship rel, 
					       const TokenTypeId& mastertt, 
					       const std::vector<LabelStr>& states, 
					       const std::vector<CandidateOrder>& orders) {
    check_error(states.size() == orders.size());
    Origin orig;
    if (mastertt.isNoId())
      orig = INITIAL;
    else
      orig = SUBGOAL;
    LabelStr key = HSTSHeuristics::getIndexKey(tt, rel, mastertt, orig);
    TokenEntry entry(p, states, orders);
    m_tokenHeuristics.insert(std::make_pair<LabelStr, TokenEntry>(key, entry));
  }

  void HSTSHeuristics::setHeuristicsForTokenDPsWithParent(const Priority p, const TokenTypeId& tt) {
  }

  void HSTSHeuristics::addSuccTokenGenerator(const GeneratorId& generator) {
    check_error(generator.isValid());
    check_error(m_generatorsByName.find(generator->getName()) == m_generatorsByName.end());
    m_succTokenGenerators.insert(generator);
    m_generatorsByName.insert(std::pair<LabelStr,GeneratorId>(generator->getName(), generator));
  }

  void HSTSHeuristics::addVariableGenerator(const GeneratorId& generator) {
    check_error(generator.isValid());
    check_error(m_generatorsByName.find(generator->getName()) == m_generatorsByName.end());
    m_variableGenerators.insert(generator);
    m_generatorsByName.insert(std::pair<LabelStr,GeneratorId>(generator->getName(), generator));
  }

  const GeneratorId& HSTSHeuristics::getGeneratorByName(const LabelStr& name) const {
    std::map<LabelStr,GeneratorId>::const_iterator pos = m_generatorsByName.find(name);
    if (pos == m_generatorsByName.end())
      return GeneratorId::noId();
    else
      return (pos->second);
  }

  void TokenType::createTokenType(const TokenId& token, TokenTypeId& tt) {
    if (token.isNoId()) {
      tt = TokenTypeId::noId();
      return;
    }
    std::vector<std::pair<LabelStr,LabelStr> > domains;
    const std::vector<ConstrainedVariableId>& variables(token->getParameters());
    for (unsigned int i = 0; i < variables.size(); ++i) {
      if (variables[i]->lastDomain().isSingleton()) {
	double val = variables[i]->lastDomain().getSingletonValue();
	LabelStr name = variables[i]->getName();
	if (variables[i]->lastDomain().isNumeric()) {
	  domains.push_back(std::make_pair<LabelStr,LabelStr>(name,val)); 
	} 
	else {
	  if (LabelStr::isString(val))
	    domains.push_back(std::make_pair<LabelStr,LabelStr>(name,val));
	  else {
	    EntityId entity(val);
	    domains.push_back(std::make_pair<LabelStr,LabelStr>(name,entity->getName().toString()));
	  }
	}
      }
    }
    tt = (new TokenType(token->getName(), domains))->getId();
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getPriorityForTokenDP(const TokenDecisionPointId& tokDec) {
    //    std::cout << "In getPriorityForTokenDP" << std::endl;
    check_error(tokDec.isValid());
    return getInternalPriorityForTokenDP(tokDec->getToken());
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getPriorityForObjectDP(const ObjectDecisionPointId& objDec) {
    check_error(objDec.isValid());
    return getInternalPriorityForTokenDP(objDec->getToken());
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getPriorityForConstrainedVariableDP(const ConstrainedVariableDecisionPointId& varDec) {
    check_error(varDec.isValid());
    const ConstrainedVariableId& var = varDec->getVariable();
    const EntityId& parent = var->getParent();
    if (parent.isNoId() || !TokenId::convertable(parent)) 
      return m_defaultVariablePriority;
    TokenTypeId tt;
    TokenType::createTokenType(parent,tt);
    Priority p = getInternalPriorityForConstrainedVariableDP(var->getName(), tt);
    if (!tt.isNoId()) tt.remove(); // we really don't need it
    return p;
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getInternalPriorityForConstrainedVariableDP(const LabelStr variableName, 
									     const TokenTypeId& tt) {
    LabelStr key = HSTSHeuristics::getIndexKey(variableName,tt);
    std::map<LabelStr, VariableEntry>::iterator pos = m_variableHeuristics.find(key);
    if (pos != m_variableHeuristics.end())
      return pos->second.getPriority();
    return m_defaultVariablePriority;
  }

  const LabelStr HSTSHeuristics::getIndexKeyForToken(const TokenId& tok) {
    TokenTypeId tt;
    TokenType::createTokenType(tok,tt);
    TokenTypeId mastertt;
    TokenType::createTokenType(tok->getMaster(),mastertt);
    LabelStr relName = tok->getRelation();
    Relationship rel;
    if (relName == LabelStr("before")) 
      rel = HSTSHeuristics::BEFORE;
    else if (relName == LabelStr("after"))
      rel = HSTSHeuristics::AFTER;
    else if (relName == LabelStr("any"))
      rel = HSTSHeuristics::ANY;
    else rel = HSTSHeuristics::OTHER;
    
    Origin orig;
    if (mastertt.isNoId())
      orig = INITIAL;
    else
      orig = SUBGOAL;

    check_error(tt.isValid());

    //    std::cout << "In getInternalPriorityForTokenDP" << std::endl;

    LabelStr key = HSTSHeuristics::getIndexKey(tt, rel, mastertt, orig);

    if (!tt.isNoId()) tt.remove();
    if (!mastertt.isNoId()) mastertt.remove();

    return key;
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getInternalPriorityForTokenDP(const TokenId& tok) {
    LabelStr key(getIndexKeyForToken(tok));

    //    std::cout << " got key for token = " << key.c_str() << std::endl;

    std::map<LabelStr, TokenEntry>::iterator pos = m_tokenHeuristics.find(key);
    Priority p;
    if (pos != m_tokenHeuristics.end())
      p = pos->second.getPriority();
    else if (!tok->getMaster().isNoId()) {
      TokenTypeId mastertt;
      TokenType::createTokenType(tok->getMaster(),mastertt);
      LabelStr key2(NO_STRING);
      TokenType::getIndexKey(mastertt,key2);
      std::map<LabelStr, Priority>::iterator pos2 =  m_defaultCompatibilityPriority.find(key2);
      if (pos2 != m_defaultCompatibilityPriority.end())
	p = pos2->second;
      mastertt.remove(); // we no longer need it.
    }
    else p = m_defaultTokenPriority;
    return p;
  }

  void HSTSHeuristics::getOrderedDomainForConstrainedVariableDP(const ConstrainedVariableDecisionPointId& varDec, std::list<double>& domain) {
    check_error(domain.empty());
    check_error(varDec.isValid());
    const ConstrainedVariableId& var = varDec->getVariable();
    const EntityId& parent = var->getParent();
    if (!parent.isNoId() && TokenId::convertable(parent)) {
      TokenTypeId tt;
      TokenType::createTokenType(parent,tt);
      LabelStr key = HSTSHeuristics::getIndexKey(var->getName(),tt);
      std::map<LabelStr, VariableEntry>::iterator pos = m_variableHeuristics.find(key);
      if (pos != m_variableHeuristics.end()) {
	std::list<LabelStr> values(pos->second.getDomain()); 
	for (std::list<LabelStr>::const_iterator it(values.begin()); it != values.end(); ++it) {
	  if (var->baseDomain().isNumeric()) {
	    std::string str((*it).c_str());
	    //	    domain.push_back(strtod(str, str.end()));
	    check_error(ALWAYS_FAIL, "Numeric domain not yet handled");
	  }
	  else if (var->baseDomain().isEnumerated()) {
	    if (Schema::instance()->isObjectType(var->baseDomain().getTypeName())) {
	      ObjectId obj(m_pdb->getObject(*it));
	      std::stringstream msg;
	      msg << "Object with label " << (*it).c_str() << " not found.";
	      check_error(obj.isValid(), msg.str());
	      domain.push_back(obj);
	    }
	    else if(LabelStr::isString(*it))
	      domain.push_back((*it).getKey());
	    else check_error(ALWAYS_FAIL, "Expected enumerated domain to be of either object or string/symbol type");
	  }
	}	  
	//	std::cout << "\nFound domain for variable decision point with variable " << var->getName().c_str() << " and token type " << key.c_str() << std::endl;
      }
      else {
	//	std::cout << "DIDN'T find domain for variable decision point with variable " << var->getName().c_str() << " and token type " << key.c_str() << std::endl;
	var->baseDomain().getValues(domain);
	domain.sort();
	if (var->baseDomain().isEnumerated())
	  domain.reverse(); // hack because enumerated domains are backwards
      }
      check_error(!domain.empty());
      tt.remove();
    }
  }

  void HSTSHeuristics::getOrderedStatesForTokenDP(const TokenDecisionPointId& tokDec, std::list<LabelStr>& states, CandidateOrder& order){
    check_error(tokDec.isValid());
    check_error(states.empty());
    LabelStr key(getIndexKeyForToken(tokDec->getToken()));

    std::map<LabelStr, TokenEntry>::iterator pos = m_tokenHeuristics.find(key);
    if (pos != m_tokenHeuristics.end()) {
      //      std::cout << "Found token entry for token decision point with token type " << key.c_str() << std::endl;
      std::vector<LabelStr> vs(pos->second.getStates());
      for (unsigned int i=0; i < vs.size(); ++i) {
	//	std::cout << " considering state " << vs[i].c_str() << std::endl;
	states.push_back(vs[i]);
	if (vs[i] == Token::MERGED)
	  order = pos->second.getOrders()[i];
      }
      if (vs.empty()) {
	//	std::cout << "No states specified, falling back on defaults" << std::endl;
	states.push_back(Token::MERGED);
	states.push_back(Token::ACTIVE);
	states.push_back(Token::REJECTED);
	order = EARLY;
      }
    }
    else { // defaults
      //      std::cout << "DIDN'T find entry, falling back on defaults for token entry for token decision point with token type " << key.c_str() << std::endl;
      states.push_back(Token::MERGED);
      states.push_back(Token::ACTIVE);
      states.push_back(Token::REJECTED);
      order = EARLY;
    }
  }

  void HSTSHeuristics::getOrderForObjectDP(const ObjectDecisionPointId& objDec, CandidateOrder& order) {
    check_error(objDec.isValid());
    LabelStr key(getIndexKeyForToken(objDec->getToken()));

    std::map<LabelStr, TokenEntry>::iterator pos = m_tokenHeuristics.find(key);
    if (pos != m_tokenHeuristics.end()) {
      //      std::cout << "Found token entry for object decision point with token type " << key.c_str() << std::endl;
      std::vector<LabelStr> states = pos->second.getStates();
      for (unsigned int i=0; i < states.size(); ++i) {
	if (states[i] == Token::ACTIVE) {
	  order = pos->second.getOrders()[i];
	  break;
	}
      }
    }
    else {
      //      std::cout << "DIDN'T find token entry for object decision point with token type " << key.c_str() << std::endl;
      order = EARLY; // default
    }
  }

  const LabelStr HSTSHeuristics::getIndexKey(const LabelStr& variableName, 
					     const TokenTypeId& tt) {
    check_error(tt.isValid());
    std::stringstream key;
    LabelStr indexKey(NO_STRING);
    TokenType::getIndexKey(tt,indexKey);
    key << variableName.c_str() << DELIMITER << indexKey.c_str();
    return key.str();
  }

  const LabelStr HSTSHeuristics::getIndexKey(const TokenTypeId& tt, 
					     const Relationship rel, 
					     const TokenTypeId& mastertt, 
					     const Origin o) {
    check_error(tt.isValid());
    check_error(mastertt.isValid() || mastertt.isNoId());
    std::stringstream key;
    if (o != FREE) {
      LabelStr str(NO_STRING);
      HSTSHeuristics::originToString(o,str);
      key << str.toString() << DELIMITER;
    }
    LabelStr indexKey(NO_STRING);
    TokenType::getIndexKey(tt,indexKey);
    key << indexKey.c_str();
    if (!mastertt.isNoId()) {
      LabelStr str(NO_STRING);
      HSTSHeuristics::relationshipToString(rel,str);
      key << DELIMITER << str.toString();
      LabelStr iKey(NO_STRING);
      TokenType::getIndexKey(mastertt,iKey);
      key << DELIMITER << iKey.c_str();
    }
    return key.str();
  }

  void HSTSHeuristics::relationshipToString(const Relationship& rel, LabelStr& str) {
    switch(rel) {
    case 0: str = LabelStr("BEFORE"); break;
    case 1: str = LabelStr("AFTER"); break;
    case 2: str = LabelStr("OTHER"); break;
    case 3: str = LabelStr("ANY"); break;
    default: check_error(false, "Relationship not recognized.");
    }
  }

  void HSTSHeuristics::originToString(const Origin& orig, LabelStr& str) { 
    switch(orig) {
    case 0: str = LabelStr("FREE"); break;
    case 1: str = LabelStr("INITIAL"); break;
    case 2: str = LabelStr("SUBGOAL"); break;
    default: check_error(false, "Origin not recognized.");
    }
  }

  void HSTSHeuristics::candidateOrderToString(const CandidateOrder& order, LabelStr& str) {
    switch(order) {
    case 0: str = LabelStr("TGENERATOR"); break;
    case 1: str = LabelStr("NEAR"); break;
    case 2: str = LabelStr("FAR"); break;
    case 3: str = LabelStr("EARLY"); break;
    case 4: str = LabelStr("LATE"); break;
    case 5: str = LabelStr("MAX_FLEXIBLE"); break;
    case 6: str = LabelStr("MIN_FLEXIBLE"); break;
    case 7: str = LabelStr("LEAST_SPECIFIED"); break;
    case 8: str = LabelStr("MOST_SPECIFIED"); break;
    case 9: str = LabelStr("NONE"); break;
    default: check_error(false, "Candidate Order value not recognized.");
    }
  }

  void HSTSHeuristics::domainOrderToString(const DomainOrder dorder, LabelStr& str) {
    switch(dorder) {
    case 0: str = LabelStr("VGENERATOR"); break;
    case 1: str = LabelStr("ASCENDING"); break;
    case 2: str = LabelStr("DESCENDING"); break;
    case 3: str = LabelStr("ENUMERATION"); break;
    default: check_error(false, "Domain Order value not recognized.");
    }
  }

  void HSTSHeuristics::write(std::ostream& os) {
    os << "HSTSHeuristics: " << std::endl;
    os << " Default Priority Preference: ";
    const PriorityPref pp = getDefaultPriorityPreference();
    if (pp == 0) 
      os << "LOW" << std::endl;
    else
      os << "HIGH" << std::endl;
    os << " Default Variable Priority: " << getDefaultPriorityForConstrainedVariableDPs() << std::endl;
    const DomainOrder dorder = getDefaultPreferenceForConstrainedVariableDPs();
    LabelStr str(NO_STRING);
    domainOrderToString(dorder, str);
    os << " Default Variable Value Order: " << str.c_str() << std::endl;
    os << " Default Token Priority: " << getDefaultPriorityForTokenDPs() << std::endl;
    os << " Default Token States/Orders: " << std::endl;
    std::vector<LabelStr>::const_iterator its(m_defaultTokenStates.begin());
    std::vector<CandidateOrder>::const_iterator ito(m_defaultCandidateOrders.begin());
    for (; its != m_defaultTokenStates.end(); ++its, ++ito) {
      os << "   ";
      os << (*its).c_str();
      os << "/";
      HSTSHeuristics::candidateOrderToString(*ito,str);
      os << str.c_str();
      os << std::endl;
    }
    os << " Default Compatibility Priority: " << std::endl;
    std::map<LabelStr, Priority>::const_iterator itc(m_defaultCompatibilityPriority.begin());
    for (; itc != m_defaultCompatibilityPriority.end(); ++itc) {
      const LabelStr key = itc->first; 
      const Priority p = itc->second;
      os << "   " << key.c_str() << " " << p << std::endl;
    }
    os << " Token Heuristics: " << std::endl;
    std::map<LabelStr, TokenEntry>::const_iterator itt(m_tokenHeuristics.begin());
    for (; itt != m_tokenHeuristics.end(); ++itt) {
      os << " Token: "; 
      const LabelStr key = itt->first;
      os << "   " << key.c_str() << " " << itt->second.getPriority();
      const std::vector<LabelStr> states(itt->second.getStates());
      const std::vector<CandidateOrder> orders(itt->second.getOrders());
      std::vector<LabelStr>::const_iterator itts(states.begin());
      std::vector<CandidateOrder>::const_iterator itto(orders.begin());
      for (; itts != states.end(); ++itts, ++itto) {
	os << " ";
	HSTSHeuristics::candidateOrderToString(*itto,str);
	os << str.c_str();
	os << "/" << (*itts).c_str();
      }
      os << std::endl;
    }
    os << " Variable Heuristics: " << std::endl;
    std::map<LabelStr, VariableEntry>::const_iterator itv(m_variableHeuristics.begin());
    for (; itv != m_variableHeuristics.end(); ++itv) {
      os << " Variable: ";
      const LabelStr key(itv->first);
      VariableEntry entry(itv->second);
      os << "   " << key.c_str() << " " << entry.getPriority();
      if (!entry.getGenerator().isNoId()) 
	os << " " << entry.getGenerator()->getName().c_str();
      os << " try values:";
      const std::list<LabelStr> domain = entry.getDomain();
      if (domain.empty())
	os << " to be computed lazily ";
      else {
	std::list<LabelStr>::const_iterator itvd(domain.begin());
	for (; itvd != domain.end(); ++itvd)
	  os << " " << (*itvd).c_str();
      }
      os << std::endl;
    }
  }

}
