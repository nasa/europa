#include "Heuristic.hh"
#include "HeuristicsEngine.hh"
#include "HeuristicInstance.hh"
#include "Schema.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "LabelStr.hh"
#include "Debug.hh"
#include "Object.hh"
#include "Schema.hh"

#include <sstream>
#include <math.h>
#include <map>

/**
 * @author Conor McGann
 */

namespace EUROPA {

  const MasterRelation Heuristic::DONT_CARE("dontcare");
  const MasterRelation Heuristic::NONE("none");
  const MasterRelation Heuristic::BEFORE("before");
  const MasterRelation Heuristic::AFTER("after");
  const MasterRelation Heuristic::ANY("any");
  const MasterRelation Heuristic::OTHER("other");
  const MasterRelation Heuristic::MEETS("meets");
  const MasterRelation Heuristic::MET_BY("met_by");

  Heuristic::Heuristic(const HeuristicsEngineId& heuristicsEngine,
		       const LabelStr& predicate,
		       const Priority& priority,
		       bool mustBeOrphan,
		       const std::vector< GuardEntry >& guards)
    : Entity(), m_id(this), m_heuristicsEngine(heuristicsEngine), 
      m_predicate(predicate), m_priority(priority), m_weight(m_priority), m_guards(guards),
      m_masterPredicate(EMPTY_LABEL()), 
      m_masterRelation(mustBeOrphan ? Heuristic::NONE : Heuristic::DONT_CARE) {
    commonInit();
  }

  Heuristic::Heuristic(const HeuristicsEngineId& heuristicsEngine,
		       const LabelStr& predicate, 
		       const Priority& priority,
		       const std::vector< GuardEntry >& guards,
		       const LabelStr& masterPredicate,
		       const MasterRelation& masterRelation,
		       const std::vector< GuardEntry >& masterGuards)
    : Entity(), m_id(this), m_heuristicsEngine(heuristicsEngine), 
      m_predicate(predicate), m_priority(priority), m_guards(guards),
      m_masterPredicate(masterPredicate), m_masterRelation(masterRelation), m_masterGuards(masterGuards) {
    checkError(Schema::instance()->isPredicate(m_masterPredicate) || m_masterRelation == Heuristic::ANY || m_masterRelation == Heuristic::NONE,
               m_masterPredicate.toString() << " is undefined.");
    checkError(m_masterRelation != Heuristic::DONT_CARE, "Must supply a master relation qualifier.");

    commonInit();
  }

  Heuristic::~Heuristic(){
    m_heuristicsEngine->remove(m_id);
    m_id.remove();
  }

  double Heuristic::computeWeight(unsigned int guardCount, const Priority& priority) const {
    // The base uses a number that exceeds the max absolute value priority allowed.
    // It also multiplies by a minimum of 1 to ensure that 0 guards are handled as low weights
    // in high and low interpretations of priority. Note that we make it 2 so that default compatibility heuristics 
    // can discount the weight without getting into the zero territory
    double weight = (2+guardCount) * WEIGHT_BASE;

    // If we prefer lower priorities, the weight should be negative
    if(m_heuristicsEngine->betterThan(0, 1))
      weight = -weight;

    debugMsg("Heuristic:computeWeight", "INPUTS(" << guardCount << ", " << priority << ")" <<
	     "WEIGHT=" << fabs(weight + priority));

    return fabs(weight + priority);
  }

  void Heuristic::commonInit(){
    checkError(fabs(m_priority) < WEIGHT_BASE, m_priority << " EXCEEDS " << WEIGHT_BASE);
    checkError(Schema::instance()->isPredicate(m_predicate), m_predicate.toString() << " is undefined.");

    // Compute the weight based on priority and guard critertia
    unsigned int guardCriteria(0);

    if(m_masterRelation!= Heuristic::DONT_CARE)
      guardCriteria++;

    guardCriteria = guardCriteria + m_guards.size() + m_masterGuards.size();

    // Now we can set the weight
    m_weight = computeWeight(guardCriteria, m_priority);

    m_heuristicsEngine->add(m_id, m_predicate);
  }

  const HeuristicId& Heuristic::getId() const {return m_id;}

  const LabelStr& Heuristic::getPredicate() const {return m_predicate;}

  const MasterRelation& Heuristic::getMasterRelation() const {return m_masterRelation;}

  const Priority& Heuristic::getPriority() const {
    return m_priority;
  }

  double Heuristic::getWeight() const {return m_weight;}

  std::string Heuristic::toString() const{
    std::stringstream sstr;

    sstr << "Heuristic (" << getKey() << ")" << " ON " << m_predicate.toString();

    if(m_masterRelation != DONT_CARE && m_masterRelation != NONE)
      sstr << " WITH MASTER " << m_masterPredicate.toString() << " RELATED BY " << m_masterRelation.toString();
    if(m_masterRelation == NONE)
      sstr << " WITH NO MASTER ";

    // if we have gaurds
    if(!m_guards.empty() || !m_masterGuards.empty()){
      static const std::string sl_indent("       ");
      sstr << " WHEN  { " << std::endl;

      for(unsigned int index = 0; index < m_guards.size(); index++){
	sstr << sl_indent;

	LabelStr varName = Schema::instance()->getNameFromIndex(m_predicate, m_guards[index].first);
	sstr << varName.toString() << " == ";

	if(LabelStr::isString(m_guards[index].second))
	  sstr << LabelStr(m_guards[index].second).toString();
	else
	  sstr << m_guards[index].second;
	sstr << std::endl;
      }

      for(unsigned int index = 0; index < m_masterGuards.size(); index++){
	sstr << sl_indent;

	LabelStr varName = Schema::instance()->getNameFromIndex(m_masterPredicate, m_masterGuards[index].first);
	sstr << "MASTER." << varName.toString() << " == ";
	if(LabelStr::isString(m_masterGuards[index].second))
	  sstr << LabelStr(m_masterGuards[index].second).toString();
	else
	  sstr << m_masterGuards[index].second;
	sstr << std::endl;
      }

      sstr << "}";
    }

    sstr << std::endl;

    sstr << "SET PRIORITY == " << m_priority << std::endl;

    sstr << "SET WEIGHT == " << m_weight << std::endl;
    return sstr.str();
  }

  bool Heuristic::canMatch(const TokenId& token) const {
    checkError(!token->isRejected() && !token->isMerged(),
	       "Should not be calling this if the token is merged or rejected.");

    debugMsg("Heuristic:canMatch", "Evaluating match for " << token->toString() << std::endl << toString());

    TokenId master = token->getMaster();
    TokenId slave = token;
    MasterRelation masterRelation = Heuristic::NONE; // Set as the dfault

    check_error(master.isNoId() || master.isValid());

    if(master.isId()){
      masterRelation = slave->getRelation();
      checkError(masterRelation != Heuristic::NONE, "Cannot have a master relation of 'NONE' if it has a master");
    }

    if(!canMatch(masterRelation, m_masterRelation)){
      debugMsg("Heuristic:canMatch", 
	       "Failed. " << masterRelation.toString() << " cannot match " << m_masterRelation.toString());
      return false;
    }

    // If there is a master and we discriminate a master in the heuristic, enforce a match.
    if(master.isId() && 
       m_masterRelation != Heuristic::DONT_CARE && 
       m_masterPredicate != LabelStr("NO_PREDICATE") &&
       master->getPredicateName() != m_masterPredicate) {
      debugMsg("Heuristic:canMatch",
               "Failed. " << master->getPredicateName().toString() << " cannot match " << m_masterPredicate.toString());
      return false;
    }

    // If we expect a master to be present and it isn't we can't match
    if((m_masterPredicate != EMPTY_LABEL() && m_masterRelation != Heuristic::NONE) && !master.isId()){
      debugMsg("Heuristic:canMatch",  "Failed. No master when one is required.");
      return false;
    }

    // Since the same predicate can be overloaded with different parameters, we must make sure the guard
    // indexes can be matched. This is a bit dangerous since we could have different types in each signature and
    // we do not test for that. Instead we will require that the value is in the base domain which 
    // gets us part of the way there. We do this for guards on master and slave
    if(!m_guards.empty()){
      const std::vector<ConstrainedVariableId>& parameters = token->getParameters();
      unsigned int paramCount = parameters.size();
      for (std::vector< GuardEntry >::const_iterator it = m_guards.begin(); it != m_guards.end();  ++it){
        unsigned int guardIndex = it->first;
        if(guardIndex >= paramCount){
          debugMsg("Heuristic:canMatch", 
                   "Failed. Guard index of " << guardIndex << " exceeds param count for " << token->toString());
          return false;
        }

        double guardValue = convertValueIfNecessary(m_heuristicsEngine->getPlanDatabase(), 
                                                    parameters[guardIndex], it->second);
        if(!parameters[guardIndex]->baseDomain().isMember(guardValue)){
          debugMsg("Heuristic:canMatch", 
                   "Failed. Guard value not in base domain for " << parameters[guardIndex]->toString());
          return false;
        }
      }
    }

    if(!m_masterGuards.empty()){
      const std::vector<ConstrainedVariableId>& parameters = master->getParameters();
      unsigned int paramCount = parameters.size();
      for (std::vector< GuardEntry >::const_iterator it = m_masterGuards.begin(); it != m_masterGuards.end(); ++it){
        unsigned int guardIndex = it->first;
        if(guardIndex >= paramCount){
          debugMsg("Heuristic:canMatch", 
                   "Failed. Master guard index of " << guardIndex << " exceeds param count for " << token->toString());
          return false;
        }

        if(!parameters[guardIndex]->lastDomain().isMember(it->second)){
          debugMsg("Heuristic:canMatch",
                   "Failed. Master guard value not in derived domain for " << parameters[guardIndex]->toString());
          return false;
        }
      }
    }

    // Finally, we test the predicate. This is last since it usually will already have been pruned, but it
    // is worth keeping in for completeness. It is really only in cases of unit testing where we might call
    // this function where this is not already a given.
    if(token->getPredicateName() != m_predicate){
      debugMsg("Heuristic:canMatch", 
               "Failed to match " << token->getPredicateName().toString() << " with " << m_predicate.toString());
      return false;
    }

    debugMsg("Heuristic:canMatch", "MATCHED!");
    return true;
  }

  bool Heuristic::canMatch(const MasterRelation& tokenRelation, const MasterRelation& guardRelation){
    // If they match, trivially we are done.
    if(tokenRelation == guardRelation)
      return true;

    // If we have a master then we can only match with ANY or DONT_CARE
    if( (tokenRelation != Heuristic::NONE) &&
	(guardRelation == Heuristic::ANY || guardRelation == Heuristic::DONT_CARE))
      return true;

    // If we have no master, then we can only match with NONE && DONT_CARE
    if(tokenRelation == Heuristic::NONE && 
       (guardRelation == Heuristic::NONE || guardRelation == Heuristic::DONT_CARE))
      return true;

    // Guards on other exclude before and after
    if(guardRelation == Heuristic::OTHER &&
       tokenRelation != Heuristic::NONE &&
       tokenRelation != Heuristic::BEFORE &&
       tokenRelation != Heuristic::AFTER)
      return true;

    // MEETS is a case of BEFORE
    if(guardRelation == Heuristic::BEFORE && tokenRelation == Heuristic::MEETS)
      return true;

    // MET_BY is a case of AFTER
    if(guardRelation == Heuristic::AFTER && tokenRelation == Heuristic::MET_BY)
      return true;

    return false;
  }

  bool Heuristic::hasGuards() const {return !m_guards.empty() || !m_masterGuards.empty();}

  const std::vector< GuardEntry >& Heuristic::noGuards(){
    static const std::vector< GuardEntry > sl_noGuards;
    return sl_noGuards;
  }

  void Heuristic::makeConstraintScope(const TokenId& token, std::vector<ConstrainedVariableId>& scope) const {
    if(!m_guards.empty()){
      const std::vector<ConstrainedVariableId>& parameters = token->getParameters();
      for (std::vector< GuardEntry >::const_iterator it = m_guards.begin(); it != m_guards.end(); ++it){
	unsigned int guardIndex = it->first;
	checkError(guardIndex >= 0 && guardIndex < parameters.size(),
		   "Invalid guard index of " << guardIndex << " for " <<  token->toString());
	ConstrainedVariableId guard = parameters[guardIndex];
	scope.push_back(guard);
      }
    }

    if(!m_masterGuards.empty()){
      TokenId master = token->getMaster();

      checkError(master.isValid(), 
		 "No master for " << token->toString() << " but it expects guards." << toString());

      const std::vector<ConstrainedVariableId>& parameters = master->getParameters();
      for (std::vector< GuardEntry >::const_iterator it = m_masterGuards.begin(); it != m_masterGuards.end(); ++it){
	unsigned int guardIndex = it->first;
	checkError(guardIndex >= 0 && guardIndex < parameters.size(),
		   "Invalid guard index of " << guardIndex << " for " <<  token->getMaster()->toString());
	ConstrainedVariableId guard = parameters[guardIndex];
	scope.push_back(guard);
      }
    }
  }

  bool Heuristic::test(const std::vector<ConstrainedVariableId>& scope) const {
    debugMsg("Heuristic:test", std::endl << toString());

    checkError(scope.size() == m_guards.size() + m_masterGuards.size(),
	       scope.size() << "!=" << m_guards.size() << " + " << m_masterGuards.size());

    unsigned int scopeIndex = 0;

    for(unsigned int index = 0; index <  m_guards.size(); index++){
      if(!matches(scope[scopeIndex], m_guards[index].second))
	return false;
      scopeIndex++;
    }

    for(unsigned int index = 0; index < m_masterGuards.size(); index++){
      if(!scope[scopeIndex]->lastDomain().isSingleton() || 
	 !matches(scope[scopeIndex], m_masterGuards[index].second))
	return false;
      scopeIndex++;
    }

    return true;
  }

  /* Have to convert if it is an object variable */
  bool Heuristic::matches(const ConstrainedVariableId& guardVar, const double& testValue) const{
    if(!guardVar->lastDomain().isSingleton())
      return false;

    double convertedValue = convertValueIfNecessary(m_heuristicsEngine->getPlanDatabase(), guardVar, testValue);

    condDebugMsg(Schema::instance()->isObjectType(guardVar->baseDomain().getTypeName()),"Heuristic::matches",
		"Comparing " << guardVar->lastDomain() << " with " << LabelStr(testValue).toString());
    condDebugMsg(LabelStr::isString(testValue), "Heuristic::matches", 
		 "Comparing " << guardVar->lastDomain() << " with " << LabelStr(testValue).toString());
    condDebugMsg(!LabelStr::isString(testValue), "Heuristic::matches", 
		 "Comparing " << guardVar->lastDomain() << " with " << testValue);

    return guardVar->lastDomain().getSingletonValue() == convertedValue;
  }

  double Heuristic::convertValueIfNecessary(const PlanDatabaseId& db,
					    const ConstrainedVariableId& guardVar,
					    const double& testValue){
    // Convert if an object variable. Make it the object id.
    if(Schema::instance()->isObjectType(guardVar->baseDomain().getTypeName())){
      checkError(LabelStr::isString(testValue), "Should be a declared string since it must be the object name.");
      return db->getObject(LabelStr(testValue));
    }

    return testValue;
  }

  /**
   * VARIABLE HEURISTIC IMPLEMENTATION
   */
  HeuristicInstanceId VariableHeuristic::createInstance(const TokenId& token) const {
    ConstrainedVariableId variableTarget = token->getVariable(m_variableTarget);
    checkError(variableTarget.isValid(), 
	       "No valid target variable for " << m_variableTarget.toString() << " in " << token->toString());

    HeuristicInstanceId instance = (new HeuristicInstance(token,
							  variableTarget->getKey(), 
							  m_id, m_heuristicsEngine))->getId();
    return instance;
  }

  VariableHeuristic::VariableHeuristic(const HeuristicsEngineId& heuristicsEngine,
				       const LabelStr& predicate, 
				       const LabelStr& variableTarget,
				       const Priority& priority,
				       bool mustBeOrphan,
				       const std::vector< GuardEntry >& guards)
    : Heuristic(heuristicsEngine, predicate, priority, mustBeOrphan, guards),
      m_variableTarget(variableTarget), 
      m_values(VariableHeuristic::noValues()), 
      m_domainOrder(heuristicsEngine->getDefaultDomainOrder()){
    commonInit();
  }

  VariableHeuristic::VariableHeuristic(const HeuristicsEngineId& heuristicsEngine,
				       const LabelStr& predicate, 
				       const LabelStr& variableTarget,
				       const Priority& priority,
				       const std::list<double>& values,
				       const DomainOrder& domainOrder,
				       bool mustBeOrphan,
				       const std::vector< GuardEntry >& guards)
    : Heuristic(heuristicsEngine,predicate, priority, mustBeOrphan, guards),
      m_variableTarget(variableTarget), m_values(values), m_domainOrder(domainOrder){
    commonInit();
  }

  VariableHeuristic::VariableHeuristic(const HeuristicsEngineId& heuristicsEngine,
				       const LabelStr& predicate, 
				       const LabelStr& variableTarget,
				       const Priority& priority,
				       const std::list<double>& values,
				       const DomainOrder& domainOrder,
				       const std::vector< GuardEntry >& guards,
				       const LabelStr& masterPredicate,
				       const MasterRelation& masterRelation,
				       const std::vector< GuardEntry >& masterGuards)
    : Heuristic(heuristicsEngine,predicate, priority, guards, masterPredicate, masterRelation, masterGuards),
      m_variableTarget(variableTarget), m_values(values), m_domainOrder(domainOrder){
    commonInit();
  }

  void VariableHeuristic::commonInit(){
    checkError(m_domainOrder == ENUMERATION || m_values.empty(), "Can only provide values for enumerated orderings.");

    checkError(Schema::instance()->hasMember(m_predicate, m_variableTarget),
	       "Invalid target of " << m_variableTarget.toString() << " for " << m_predicate.toString());

    debugMsg("VariableHeuristic:commonInit", std::endl << toString());
  }

  const std::list<double>& VariableHeuristic::getValues() const {return m_values;}

  const VariableHeuristic::DomainOrder& VariableHeuristic::getDomainOrder() const {return m_domainOrder;}

  std::string VariableHeuristic::toString() const{
    std::stringstream sstr;

    sstr << Heuristic::toString();

    // Now output the Domain Order.
    sstr << "FOR VARIABLE = '" << m_variableTarget.toString() << "' SET PREFERENCE = ";
    if(getDomainOrder() == ENUMERATION){
      sstr << "{";
      for (std::list<double>::const_iterator it = getValues().begin(); it != getValues().end(); ++it){
	double value = *it;
	if(LabelStr::isString(value))
	  sstr << LabelStr(value).toString() << " ";
	else
	  sstr << value << " ";
      }
      sstr << "}";
    }
    else
      sstr << domainOrderStrings()[getDomainOrder()];

    sstr << std:: endl;

    return sstr.str();
  }

  const std::vector<std::string>& VariableHeuristic::domainOrderStrings(){
    static std::vector<std::string> sl_instance;
    static bool sl_initialized(false);

    if(!sl_initialized){
      sl_instance.push_back("VGENERATOR");
      sl_instance.push_back("ASCENDING");
      sl_instance.push_back("DESCENDING");
      sl_instance.push_back("ENUMERATION");
      sl_initialized = true;
    }

    return sl_instance;
  }

  const std::list<double>& VariableHeuristic::noValues(){
    static const std::list<double> sl_instance;
    return sl_instance;
  }

  void VariableHeuristic::orderChoices(const PlanDatabaseId& db,
				       const ConstrainedVariableId& var,
				       const std::list<double>& values, 
				       const DomainOrder& domainOrder,
				       std::list<double>& orderedChoices){
    checkError(domainOrder == ENUMERATION || values.empty(), "Should not have values passed in with " << domainOrder);
    checkError(var->lastDomain().isFinite(), "Must be an enumeration to process values:" << var->toString());
    const AbstractDomain& allowedChoices = var->lastDomain(); // Prune agains this.

    bool isObjectVariable = Schema::instance()->isObjectType(var->baseDomain().getTypeName());

    if(values.empty()){ // Store in the right order
      std::map<double, double> orderedValues;
      for(std::list<double>::const_iterator it = orderedChoices.begin(); it != orderedChoices.end(); ++it){
	double value = *it;

	// No need to convert values since we are given the actual values up front. However, if it is an
	// object variable, then we will want to order according to key, rather than leave to the memory address
	// whici is volatile!
	if(isObjectVariable){
	  ObjectId object = value;
	  orderedValues.insert(std::pair<double, double>(object->getKey(), object));
	}
	else
	  orderedValues.insert(std::pair<double, double>(value, value));
      }

      // Now clear initial list and fillin order
      orderedChoices.clear();
      for(std::map<double, double>::const_iterator it = orderedValues.begin(); it != orderedValues.end(); ++it){
	double value = it->second;
	if(domainOrder == VariableHeuristic::ASCENDING)
	  orderedChoices.push_back(value);
	else
	  orderedChoices.push_front(value);
      }
    }
    else{
      orderedChoices.clear();
      for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it){
	double value = *it;
	value = convertValueIfNecessary(db, var, value);

	if(allowedChoices.isMember(value))
	  orderedChoices.push_back(value);
      }
    }

    debugMsg("VariableHeuristic:orderChoices", 
	     "Choices for " << var->toString() << " are " << toString(var, orderedChoices));
  }

  std::string VariableHeuristic::toString(const ConstrainedVariableId& var, const std::list<double>& choices) {
    if(Schema::instance()->isObjectType(var->baseDomain().getTypeName()))
      return ObjectDomain(ObjectDomain::makeObjectList(choices), var->baseDomain().getTypeName().c_str()).toString();

    EnumeratedDomain domain(choices, var->baseDomain().isNumeric(), "ChoiceList");
    return domain.toString();
  }

  /**
   * TOKEN HEURISTIC IMPLEMENTATION
   */

  HeuristicInstanceId TokenHeuristic::createInstance(const TokenId& token) const {
    HeuristicInstanceId instance = (new HeuristicInstance(token, 
							  token->getKey(), 
							  m_id, m_heuristicsEngine))->getId();
    return instance;
  }

  TokenHeuristic::TokenHeuristic(const HeuristicsEngineId& heuristicsEngine,
				 const LabelStr& predicate, 
				 const Priority& priority,
				 bool mustBeOrphan,
				 const std::vector< GuardEntry >& guards)
    :Heuristic(heuristicsEngine,predicate, priority, mustBeOrphan, guards),
     m_states(noStates()), m_orders(noOrders()){
    commonInit();
  }

  TokenHeuristic::TokenHeuristic(const HeuristicsEngineId& heuristicsEngine,
				 const LabelStr& predicate,
				 const Priority& priority,
				 const std::vector<LabelStr>& states,
				 const std::vector<CandidateOrder>& orders,
				 bool mustBeOrphan,
				 const std::vector< GuardEntry >& guards)
    : Heuristic(heuristicsEngine,predicate, priority, mustBeOrphan, guards),
      m_states(states), m_orders(orders){
    commonInit();
  }

  TokenHeuristic::TokenHeuristic(const HeuristicsEngineId& heuristicsEngine,
				 const LabelStr& predicate,
				 const Priority& priority,
				 const std::vector<LabelStr>& states,
				 const std::vector<CandidateOrder>& orders,
				 const std::vector< GuardEntry >& guards,
				 const LabelStr& masterPredicate,
				 const MasterRelation& masterRelation,
				 const std::vector< GuardEntry >& masterGuards)
    : Heuristic(heuristicsEngine, predicate, 
		priority, guards, masterPredicate, masterRelation, masterGuards),
      m_states(states), m_orders(orders){
    commonInit();
  } 

  void TokenHeuristic::commonInit(){
    checkError(m_states.size() == m_orders.size(), 
	       "Both must be equal since they are really tuples" << m_states.size() << "!=" << m_orders.size());

    debugMsg("TokenHeuristic:commonInit", std::endl << toString());
  }

  const std::vector<LabelStr>& TokenHeuristic::getStates() const {return m_states;}

  const std::vector<TokenHeuristic::CandidateOrder>& TokenHeuristic::getOrders() const {return m_orders;}

  std::string TokenHeuristic::toString() const {
    std::stringstream sstr;

    sstr << Heuristic::toString();

    if(!m_states.empty()){
      unsigned int choiceCount = m_states.size();

      sstr << "SET PREFERENCE = {";
      for(unsigned int index = 0; index < choiceCount; index++){
	LabelStr state = m_states[index];
	CandidateOrder order = m_orders[index];
	sstr << "<" << state.toString() << ", " << candidateOrderStrings()[order] << "> ";
      }

      sstr<< "}" << std:: endl;
    }

    return sstr.str();
  }

  const std::vector<std::string>& TokenHeuristic::candidateOrderStrings(){
    static std::vector<std::string> sl_instance;
    static bool sl_initialized(false);

    if(!sl_initialized){
      sl_instance.push_back("TGENERATOR");
      sl_instance.push_back("NEAR");
      sl_instance.push_back("FAR");
      sl_instance.push_back("EARLY");
      sl_instance.push_back("LATE");
      sl_instance.push_back("MAX_FLEXIBLE");
      sl_instance.push_back("MIN_FLEXIBLE");
      sl_instance.push_back("LEAST_SPECIFIED");
      sl_instance.push_back("MOST_SPECIFIED");
      sl_instance.push_back("NONE");
      sl_instance.push_back("UNKNOWN");
      sl_initialized = true;
    }

    return sl_instance;
  }

  const std::vector< LabelStr >& TokenHeuristic::noStates(){
    static const std::vector< LabelStr > sl_instance;
    return sl_instance; 
  }

  const std::vector< TokenHeuristic::CandidateOrder >& TokenHeuristic::noOrders(){
    static const std::vector< TokenHeuristic::CandidateOrder > sl_instance;
    return sl_instance;
  }


  void TokenHeuristic::orderTokens(std::vector<TokenId>& choicesToOrder, 
				   const TokenId& referenceToken,
				   const TokenHeuristic::CandidateOrder& order){
    std::multimap<int, TokenId> orderedChoices;
    unsigned int tokenCount = choicesToOrder.size();
    for(unsigned int i = 0; i < tokenCount; i++){
      TokenId choice = choicesToOrder[i];
      check_error(choice.isValid());

      int weight(PLUS_INFINITY);
      switch(order){
      case NEAR:
        debugMsg("TokenHeuristic:orderTokens", "For token " << referenceToken->getKey() << " Ordering by NEAR");
	weight = absoluteDistance(referenceToken, choice);
	break;
      case FAR:
        debugMsg("TokenHeuristic:orderTokens", "Ordering by FAR");
	weight = -absoluteDistance(referenceToken, choice);
	break;
      case LATE:
        debugMsg("TokenHeuristic:orderTokens", "Ordering by LATE");
	weight = (int) -(choice->getStart()->lastDomain().getLowerBound());
	break;
      case NONE:
      case EARLY:
        debugMsg("TokenHeuristic:orderTokens", "Ordering by NONE/EARLY");
	weight = (int) (choice->getStart()->lastDomain().getLowerBound());
	break;
      default:
	checkError(ALWAYS_FAILS, "Should have this case excluded in the reader or the engine:" << order);
      }

      // Now insert in weighted order (ascending)
      orderedChoices.insert(std::pair<int, TokenId>(weight, choice));
    }

    choicesToOrder.clear();

    // Finally, repopulate the choices in ascending weighted order
    for(std::multimap<int, TokenId>::const_iterator it = orderedChoices.begin(); it != orderedChoices.end(); ++it){
      TokenId choice = it->second;
      choicesToOrder.push_back(choice);
    }


    debugMsg("TokenHeuristic:orderTokens", "Merge candidates are " << toString(choicesToOrder));
  }

  std::string TokenHeuristic::toString(const std::vector<TokenId>& choices){
    std::stringstream sstr;
    sstr << "{";
    for(std::vector<TokenId>::const_iterator it = choices.begin(); it != choices.end(); ++it){
      TokenId token = *it;
      sstr << "(" << token->getKey() << ") ";
    }
    sstr << "}";
    return sstr.str();
  }

  /**
   * @note This is almos the same as the prior function for just a vectr, but we have a differnt input.
   */
  void TokenHeuristic::orderTokens(std::vector< OrderingChoice >& choicesToOrder,
				   const TokenId& referenceToken,
				   const TokenHeuristic::CandidateOrder& order){
    std::multimap<int, OrderingChoice > orderedChoices;
    unsigned int tokenCount = choicesToOrder.size();
    for(unsigned int i = 0; i < tokenCount; i++){
      std::pair< ObjectId, std::pair<TokenId, TokenId> > choice = choicesToOrder[i];
      TokenId successor = choice.second.second;
      check_error(successor.isValid());

      int weight(PLUS_INFINITY);
      switch(order){
      case NEAR:
	weight = absoluteDistance(referenceToken, successor);
	break;
      case FAR:
	weight = -absoluteDistance(referenceToken, successor);
	break;
      case LATE:
	weight = (int) -(successor->getStart()->lastDomain().getLowerBound());
	break;
      case NONE:
      case EARLY:
	weight = (int) (successor->getStart()->lastDomain().getLowerBound());
	break;
      default:
	checkError(ALWAYS_FAILS, "Should have this case excluded in the reader or the engine:" << order);
      }

      // Now insert in weighted order (ascending)
      orderedChoices.insert(std::pair<int,  OrderingChoice >(weight, choice));
    }

    choicesToOrder.clear();

    // Finally, repopulate the choices in ascending weighted order
    for(std::multimap<int, OrderingChoice >::const_iterator it = orderedChoices.begin(); 
	it != orderedChoices.end(); ++it){
      OrderingChoice choice = it->second;
      choicesToOrder.push_back(choice);
    }

    debugMsg("TokenHeuristic:orderTokens", "Candidate orderings are " << toString(choicesToOrder));
  }


  std::string TokenHeuristic::toString(const std::vector<OrderingChoice>& choices){
    std::stringstream sstr;
    sstr << "{";
    for(std::vector< OrderingChoice >::const_iterator it = choices.begin(); it != choices.end(); ++it){
      TokenId predecessor = it->second.first;
      TokenId successor = it->second.second;
      sstr << "(" << predecessor->getKey() << "<" << successor->getKey() << ") ";
    }
    sstr << "}";
    return sstr.str();
  }

  int TokenHeuristic::absoluteDistance(const TokenId& a, const TokenId& b){
    return abs(midpoint(a) - midpoint(b));
  }

  int TokenHeuristic::midpoint(const TokenId& token){
    int maxTemporalExtent = (int)( token->getEnd()->lastDomain().getUpperBound() - 
      token->getStart()->lastDomain().getLowerBound() );

    int midpoint = (int) token->getStart()->lastDomain().getLowerBound() + maxTemporalExtent / 2;

    return midpoint;
  }

  /**
   * DEFAULT COMPATIBILITY IMPLEMENTATION
   */

  /**
   * @brief Set up very similar to a normal heuristic. No relation speciifcation and do not
   * require it to be an orphan.
   */
  DefaultCompatibilityHeuristic::DefaultCompatibilityHeuristic(const HeuristicsEngineId& heuristicsEngine,
							       const LabelStr& masterPredicate, 
							       const Priority& priority,
							       const std::vector< GuardEntry >& masterGuards)
    : Heuristic(heuristicsEngine, masterPredicate, priority, false, masterGuards) {

    // At this point, the weight is normalized so that higher values are dominant. We therefore just reduce the weight.
    m_weight = m_weight - WEIGHT_BASE;

    // In addition to adding this heuristic in the usual way from the base class, 
    // we also add it with no predicate qualifier
    m_heuristicsEngine->add(m_id);

    debugMsg("DefaultCompatibilityHeuristic:commonInit", std::endl << toString());
  }


  /**
   * @brief Allocate an instance based on if the token is the master or the slave. Primarily impacts
   * the target set.
   */
  HeuristicInstanceId DefaultCompatibilityHeuristic::createInstance(const TokenId& token) const {
    std::vector<int> targets;

    // The guard source may be the master, but we start by assumin its not
    TokenId guardSource = token;

    // Either way, the token is a target
    targets.push_back(token->getKey());

    // If the token predicate matches the master, we also add the variables. Will add all parameter variables
    // since that is what the spec says. We could add all the variables. This would be more useful probably.
    if(m_predicate == token->getPredicateName()){
      const std::vector<ConstrainedVariableId>& parameters = token->getParameters();
      for(std::vector<ConstrainedVariableId>::const_iterator it = parameters.begin(); 
	  it != parameters.end(); ++it){
	ConstrainedVariableId var = *it;
	targets.push_back(var->getKey());
      }
    }
    else{
      checkError(token->getMaster().isValid(), "Must have a master token for this to happen!" << token->toString());
      guardSource = token->getMaster();
    }

    // Now we can allocate the instance with the collected targets
    HeuristicInstanceId instance = (new HeuristicInstance(token,
							  targets, 
							  guardSource,
							  m_id, m_heuristicsEngine))->getId();

    return instance;
  }

  /**
   * @brief The given token may be matched as a master or slave.
   */
  bool DefaultCompatibilityHeuristic::canMatch(const TokenId& token) const{
    // If the token predicate does not equal the heuristic predicate, and it has a master, then return the base class
    // evaluation against the master
    if (m_predicate != token->getPredicateName() && token->getMaster().isId())
      return Heuristic::canMatch(token->getMaster());
    return Heuristic::canMatch(token);
  }

  std::string DefaultCompatibilityHeuristic::toString() const{
    std::stringstream sstr;
    sstr << "DEFAULT COMPATIBILITY" << std::endl << Heuristic::toString();
    return sstr.str();
  }

}
