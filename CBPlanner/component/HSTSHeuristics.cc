#include "HSTSHeuristics.hh"
#include "PlanDatabaseDefs.hh"
#include "Token.hh"
#include "TokenDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include <sstream>

namespace PLASMA {

  // todo: check bounds on priority, if inserting, make sure it wasn't
  // there

  TokenType::TokenType(const LabelStr& predicateName, 
				       const std::vector<std::pair<LabelStr, LabelStr> >& domainSpecs) 
    : m_predicateName(predicateName), m_domainSpecs(domainSpecs), m_id(this) { }

  TokenType::~TokenType() { check_error(m_id.isValid()); m_id.remove(); }

  const TokenTypeId& TokenType::getId() const {
    return m_id;
  }

  const LabelStr& TokenType::getPredicate() const {
    return m_predicateName;
  }

  const std::vector<std::pair<LabelStr,LabelStr> >& TokenType::getDomainSpecs() const {
    return m_domainSpecs;
  }

  const LabelStr TokenType::getIndexKey(const TokenTypeId& tt) {
    check_error(tt.isValid());
    std::stringstream key;
    check_error(LabelStr::isString(tt->getPredicate()));
    key << tt->getPredicate().getKey(); 
    std::vector<std::pair<LabelStr,LabelStr> > ds = tt->getDomainSpecs();
    for (unsigned int i=0; i < ds.size(); ++i) {
      key << DELIMITER;
      key << ds[i].first << "|" << ds[i].second;
    }
    key << std::endl;
    return key.str();
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
  const TokenTypeId TokenType::getTokenType(const LabelStr& indexKey) {
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
    return (new TokenType(LabelStr(strings[0]),domainSpec))->getId();
  }

  bool TokenType::matches(const TokenTypeId& tt) {
    LabelStr myIndexKey(TokenType::getIndexKey(getId()));
    LabelStr indexKey(TokenType::getIndexKey(tt));
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

  // construct the generator according to the generator name
  HSTSHeuristics::VariableEntry::VariableEntry(const std::list<LabelStr>& domain,
					       const Priority p, 
					       const DomainOrder order,
					       const LabelStr& generatorName) 
    : m_domain(domain), m_priority(p) {
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    switch (order) {
    case VGENERATOR:
      // create generator from name
      break;
    case ASCENDING:
      break;
    case DESCENDING:
      m_domain.reverse();
      break;
    default:
      check_error(ALWAYS_FAILS);
    }
  }

  HSTSHeuristics::VariableEntry::VariableEntry(const std::list<LabelStr>& domain,
					       const Priority p, 
					       const DomainOrder order,
					       const std::list<LabelStr>& enumeration)
    : m_domain(domain), m_priority(p) {
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    switch (order) {
    case VGENERATOR:
      // create generator from name
      break;
    case ASCENDING:
      break;
    case DESCENDING:
      m_domain.reverse();
      break;
    default:
      check_error(ALWAYS_FAILS);
    }
  }

  HSTSHeuristics::VariableEntry::~VariableEntry() {}

  const HSTSHeuristics::Priority HSTSHeuristics::VariableEntry::getPriority() const { return m_priority; }
  
  const std::list<LabelStr>& HSTSHeuristics::VariableEntry::getDomain() const { return m_domain; }
  
  const GeneratorId& HSTSHeuristics::VariableEntry::getGenerator() const { return m_generator; }

  HSTSHeuristics::HSTSHeuristics() {
    m_defaultPriorityPreference = LOW;
    m_defaultTokenPriority = 0.0;
    m_defaultVariablePriority = 0.0;
    m_defaultDomainOrder = ASCENDING;
  }

  HSTSHeuristics::~HSTSHeuristics() {
    m_defaultCompatibilityPriority.clear();
    m_defaultTokenStates.clear();
    m_defaultCandidateOrders.clear();
    //    std::map<LabelStr,TokenEntry>::iterator it = m_tokenHeuristics.begin();
    m_tokenHeuristics.clear();
    //    std::map<LabelStr,VariableEntry>::iterator it2 = m_variableHeuristics.begin();
    m_variableHeuristics.clear();
  }

  void HSTSHeuristics::setDefaultPriorityPreference(const PriorityPref pp) {
    m_defaultPriorityPreference = pp;
  }

  const HSTSHeuristics::PriorityPref HSTSHeuristics::getDefaultPriorityPreference() const {
    return m_defaultPriorityPreference;
  }

  void HSTSHeuristics::setDefaultPriorityForTokenDPsWithParent(const Priority p, const TokenTypeId& tt) {
    check_error(MIN_PRIORITY <= p);
    check_error(MAX_PRIORITY >= p);
    m_defaultCompatibilityPriority.insert(std::make_pair<LabelStr,Priority>(TokenType::getIndexKey(tt), p));
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

  void HSTSHeuristics::setHeuristicsForConstrainedVariableDP(const Priority p, 
							     const LabelStr variableName, 
							     const TokenTypeId& tt, 
							     const DomainOrder order,
							     const LabelStr& generatorName, 
							     const std::vector<LabelStr>& enumeration) {
    LabelStr key("");
    if (!tt.isNoId())
      key = HSTSHeuristics::getIndexKey(variableName,tt);
    else
      key = variableName;
    // get varId from variableName and tt
    // get baseDomain
    // if (generatorName == NO_STRING) {
    //  VariableEntry entry(baseDomain.getValues(), p, order, enumeration);
    // }
    // else  {
    //   VariableEntry entry(baseDomain.getValues(), p, order, generatorName);
    // }
    //m_variableHeuristics.insert(std::make_pair<LabelStr,VariableEntry>(key, entry));
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

  const TokenTypeId TokenType::createTokenType(const TokenId& token) {
    if (token.isNoId()) return TokenTypeId::noId();
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
    return (new TokenType(token->getName(), domains))->getId();
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getPriorityForTokenDP(const TokenDecisionPointId& tokDec) {
    TokenId tok = tokDec->getToken();
    TokenTypeId tt = TokenType::createTokenType(tok);
    TokenTypeId mastertt = TokenType::createTokenType(tok->getMaster());
    //    Relationship rel = tok->getRelationship(); // once it is implemented on tokens.
    Relationship rel = BEFORE;
    Origin orig;
    if (mastertt.isNoId())
      orig = INITIAL;
    else
      orig = SUBGOAL;
    return getInternalPriorityForTokenDP(tt, rel, mastertt, orig);
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getPriorityForObjectDP(const ObjectDecisionPointId& objDec) {
    return getPriorityForTokenDP(objDec->getToken());
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getPriorityForConstrainedVariableDP(const ConstrainedVariableDecisionPointId& varDec) {
    const ConstrainedVariableId& var = varDec->getVariable();
    const EntityId& parent = var->getParent();
    if (parent.isNoId() || !TokenId::convertable(parent)) 
      return m_defaultVariablePriority;
    TokenTypeId tt = TokenType::createTokenType(parent);
    return getInternalPriorityForConstrainedVariableDP(var->getName(), tt);
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getInternalPriorityForConstrainedVariableDP(const LabelStr variableName, 
									     const TokenTypeId& tt) {
    LabelStr key = HSTSHeuristics::getIndexKey(variableName,tt);
    std::map<LabelStr, VariableEntry>::iterator pos = m_variableHeuristics.find(key);
    if (pos != m_variableHeuristics.end())
      return pos->second.getPriority();
    return m_defaultVariablePriority;
  }

  const HSTSHeuristics::Priority HSTSHeuristics::getInternalPriorityForTokenDP(const TokenTypeId& tt, 
							       const Relationship rel, 
							       const TokenTypeId& mastertt, 
							       const Origin o) {
    check_error(tt.isValid());
    check_error(mastertt.isValid());
    LabelStr key = HSTSHeuristics::getIndexKey(tt, rel, mastertt, o);
    std::map<LabelStr, TokenEntry>::iterator pos = m_tokenHeuristics.find(key);
    if (pos != m_tokenHeuristics.end())
      return pos->second.getPriority();
    LabelStr key2 = TokenType::getIndexKey(mastertt);
    std::map<LabelStr, Priority>::iterator pos2 =  m_defaultCompatibilityPriority.find(key2);
    if (pos2 != m_defaultCompatibilityPriority.end())
      return pos2->second;
    return m_defaultTokenPriority;
  }

  const LabelStr HSTSHeuristics::getIndexKey(const LabelStr& variableName, 
					     const TokenTypeId& tt) {
    check_error(tt.isValid());
    std::stringstream key;
    key << variableName << DELIMITER << TokenType::getIndexKey(tt);
    key << std::endl;
    return key.str();
  }

  const LabelStr HSTSHeuristics::getIndexKey(const TokenTypeId& tt, 
					     const Relationship rel, 
					     const TokenTypeId& mastertt, 
					     const Origin o) {
    check_error(tt.isValid());
    check_error(mastertt.isValid() || mastertt.isNoId());
    std::stringstream key;
    if (o != FREE)
      key << o << DELIMITER;
    key << TokenType::getIndexKey(tt);
    if (!mastertt.isNoId()) {
      key << DELIMITER << TokenType::getIndexKey(mastertt);
      key << DELIMITER << rel;
    }
    key << std::endl;
    return key.str();
  }

  void HSTSHeuristics::write(std::ostream& os) {
    os << "HSTSHeuristics: " << std::endl;
    os << " Default Priority Preference: " << getDefaultPriorityPreference() << std::endl;
    os << " Default Token Priority: " << getDefaultPriorityForTokenDPs() << std::endl;
    os << " Default Variable Priority: " << getDefaultPriorityForConstrainedVariableDPs() << std::endl;
    os << " Default Token Orders/States: " << std::endl;
    std::vector<LabelStr>::const_iterator its(m_defaultTokenStates.begin());
    std::vector<CandidateOrder>::const_iterator ito(m_defaultCandidateOrders.begin());
    for (; its != m_defaultTokenStates.end(); ++its, ++ito) {
      os << "   " << *ito << "/" << (*its).c_str() << std::endl;
    }
    os << " Default Compatibility Priority: " << std::endl;
    std::map<LabelStr, Priority>::const_iterator itc(m_defaultCompatibilityPriority.begin());
    for (; itc != m_defaultCompatibilityPriority.end(); ++itc) {
      const LabelStr key = itc->first; 
      const Priority p = itc->second;
      std::cout << "   " << key.c_str() << " " << p << std::endl;
    }
    std::map<LabelStr, TokenEntry>::const_iterator itt(m_tokenHeuristics.begin());
    for (; itt != m_tokenHeuristics.end(); ++itt) {
      const LabelStr key = itt->first;
      std::cout << "   " << key.c_str() << " " << itt->second.getPriority();
      const std::vector<LabelStr> states(itt->second.getStates());
      const std::vector<CandidateOrder> orders(itt->second.getOrders());
      std::vector<LabelStr>::const_iterator itts(states.begin());
      std::vector<CandidateOrder>::const_iterator itto(orders.begin());
      for (; itts != states.end(); ++itts, ++itto) {
	std::cout << " " << *ito << "/" << (*its).c_str();
      }
    }
    std::map<LabelStr, VariableEntry>::const_iterator itv(m_variableHeuristics.begin());
    for (; itv != m_variableHeuristics.end(); ++itv) {
      const LabelStr key = itv->first;
      std::cout << "   " << key.c_str() << " " << itv->second.getPriority();
      const std::list<LabelStr> domain = itv->second.getDomain();
      std::list<LabelStr>::const_iterator itvd(domain.begin());
      for (; itvd != domain.end(); ++itvd)
	std::cout << " " << (*itvd).c_str();
      std::cout << " " << itv->second.getGenerator()->getName().c_str() << std::endl;
    }
  }

}
