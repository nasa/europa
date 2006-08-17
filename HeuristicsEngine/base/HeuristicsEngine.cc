#include "HeuristicsEngine.hh"
#include "Heuristic.hh"
#include "HeuristicInstance.hh"
#include "PlanDatabase.hh"
#include "ConstraintEngine.hh"
#include "Token.hh"
#include "Debug.hh"
#include "Utils.hh"

#include <iomanip>

/**
 * @author Conor McGann
 */

namespace EUROPA {

  HeuristicsEngine::DbListener::DbListener(const PlanDatabaseId& db, const HeuristicsEngineId& he)
    : PlanDatabaseListener(db), m_he(he) {
  }

  void HeuristicsEngine::DbListener::notifyAdded(const TokenId& token){ m_he->notifyAdded(token);}

  void HeuristicsEngine::DbListener::notifyRemoved(const TokenId& token){ m_he->notifyRemoved(token);}

  HeuristicsEngine::HeuristicsEngine(const PlanDatabaseId& planDatabase)
    : m_id(this), m_planDb(planDatabase), m_deleted(false), m_initialized(false),
      m_preferLowPriority(true), m_preferNewOverOld(true),
      m_defaultTokenPriority(Heuristic::WEIGHT_BASE-1),
      m_defaultVariablePriority(Heuristic::WEIGHT_BASE-1),
      m_defaultDomainOrder(VariableHeuristic::ASCENDING){
    // Add default States
    m_defaultStates.push_back(Token::MERGED);
    m_defaultStates.push_back(Token::ACTIVE);
    m_defaultStates.push_back(Token::REJECTED);

    // Add correspondnig default orders
    m_defaultOrders.push_back(TokenHeuristic::EARLY);
    m_defaultOrders.push_back(TokenHeuristic::EARLY);
    m_defaultOrders.push_back(TokenHeuristic::EARLY);

    // Plug into the database
    m_planDbListener = (new DbListener(m_planDb, m_id))->getId();
  }

  HeuristicsEngine::~HeuristicsEngine() {
    m_deleted = true;

    for(std::multimap<int, HeuristicInstanceId>::const_iterator it = m_heuristicInstancesByToken.begin();
	it != m_heuristicInstancesByToken.end();
	++it){
      HeuristicInstanceId heuristic = it->second;
      killIt(heuristic);
    }

    std::set<HeuristicId> heuristicsToDelete;
    for(std::multimap<double, HeuristicId>::const_iterator it = m_heuristicsByPredicateKey.begin();
	it != m_heuristicsByPredicateKey.end();
	++it){
      HeuristicId heuristic = it->second;
      heuristicsToDelete.insert(heuristic);
    }

    cleanup(heuristicsToDelete);

    delete (PlanDatabaseListener* ) m_planDbListener;
    m_id.remove();
  }

  const HeuristicsEngineId& HeuristicsEngine::getId() const {return m_id;}

  const PlanDatabaseId& HeuristicsEngine::getPlanDatabase() const {return m_planDb;}

  ConstraintEngineId HeuristicsEngine::getConstraintEngine() const {return m_planDb->getConstraintEngine();}

  const std::multimap<double, HeuristicId>& HeuristicsEngine::getHeuristics() const {
    return m_heuristicsByPredicateKey;
  }

  const std::multimap<int, HeuristicInstanceId>& HeuristicsEngine::getHeuristicInstances() const {
    checkError(getConstraintEngine()->constraintConsistent(), 
	       "Must be propagated to consistency to get valid data.");
    return m_heuristicInstancesByToken;
  }

  bool HeuristicsEngine::isInitialized() const {return m_initialized;}

  /**
   * @brief Computes the set of heuristics for available tokens
   * @note No instances may be allocated yet.
   */
  void HeuristicsEngine::initialize() {
    checkError(!m_initialized, "Cannot initialize if already initialized.");
    checkError(m_heuristicInstancesByToken.empty(), "Cannot initialize if already allocated instances.");
    m_initialized = true;

    // @todo Change so it only processes system flaw candidates.
    for (TokenSet::const_iterator it = m_planDb->getTokens().begin(); it != m_planDb->getTokens().end(); ++it){
      TokenId token = *it;
      handleAddition(token, token->getPredicateName());
    }
  }

  /**
   * On execution of a heuristic instance, we add it to the collection of fired instances.
   */
  void HeuristicsEngine::notifyExecuted(const HeuristicInstanceId& instance){
    checkError(instance.isValid(), instance);
    checkError(isValid(), "Data structure integrity failure.");
    
    const std::vector<int>& targets = instance->getTargets();
    for(std::vector<int>::const_iterator it = targets.begin(); it != targets.end(); ++it)
      addHeuristicForTarget(*it, instance);
  }

  void HeuristicsEngine::addHeuristicForTarget(int targetKey, const HeuristicInstanceId& instance){
    std::map<int, HeuristicEntry >::iterator it = m_firedHeuristics.find(targetKey);

    // If no entries yet, add an empty one.
    if(it == m_firedHeuristics.end()){
      static HeuristicEntry sl_emptyEntry;
      m_firedHeuristics.insert(std::pair<int, HeuristicEntry>(targetKey, sl_emptyEntry));
      it = m_firedHeuristics.find(targetKey);
    }

    // Now we should have a place for insertion
    HeuristicEntry& entry = it->second;
    double weight = instance->getWeight();
    entry.insert(std::pair<double, HeuristicInstanceId>(weight, instance));

    debugMsg("HeuristicsEngine:addHeuristicForTarget", 
	     std::endl << "Target (" << targetKey << ") has " << entry.size() << " ACTIVE ENTRIES.");
  }

  /**
   * When undone, remove the entry in fired heuristics if still present
   */
  void HeuristicsEngine::notifyUndone(const HeuristicInstanceId& instance){
    checkError(isValid(), "Data structure integrity failure.");
    checkError(instance.isValid(), "invalid instance argument");

    const std::vector<int>& targets = instance->getTargets();
    for(std::vector<int>::const_iterator it = targets.begin(); it != targets.end(); ++it)
      removeHeuristicForTarget(*it, instance);
  }

  void HeuristicsEngine::removeHeuristicForTarget(int targetKey, const HeuristicInstanceId& instance){
    debugMsg("HeuristicsEngine:removeHeuristicForTarget", "Target key " << targetKey );
    std::map<int, HeuristicEntry >::iterator it = m_firedHeuristics.find(targetKey);
    checkError(it != m_firedHeuristics.end(), "Should be getting a hit.");
    HeuristicEntry& entry = it->second;

    double weight = instance->getWeight();
    HeuristicEntry::iterator entryIt = entry.find(weight);
    // Locate the instance
    while(entryIt != entry.end() && entryIt->second != instance) {++entryIt;}

    checkError(entryIt != entry.end(), "Should be getting a hit.");

    entry.erase(entryIt);

    if(entry.empty())
      m_firedHeuristics.erase(it);
  }

  HeuristicInstanceId HeuristicsEngine::getActiveHeuristic(const EntityId& target) const{
    checkError(isValid(), "Data structure integrity failure.");
    check_error(target.isValid());
    checkError(getConstraintEngine()->constraintConsistent(), 
	       "Must be propagated to consistency to get valid data.");

    HeuristicInstanceId instance;
    std::map<int, HeuristicEntry >::const_iterator it = m_firedHeuristics.find(target->getKey());
    if(it != m_firedHeuristics.end()){
      HeuristicEntry entry = it->second;
      HeuristicEntry::const_iterator entryIt = entry.end();
      instance = (--entryIt)->second;
      checkError(instance->isExecuted(), "Must be fired to be found here. Bug in synchronization.");
    }
    condDebugMsg(instance.isId(), "HeuristicsEngine:getActiveHeuristic", "returning instance " << instance
                 << " with priority " << instance->getPriority() << " for key " << target->getKey());
    condDebugMsg(!instance.isId(), "HeuristicsEngine:getActiveHeuristic", "returning noId for key " << target->getKey());
    return instance;
  }


  const Priority& HeuristicsEngine::bestCasePriority() const {
    if(betterThan(CONST_MIN_PRIORITY(), CONST_MAX_PRIORITY()))
      return CONST_MIN_PRIORITY();
    else
      return CONST_MAX_PRIORITY();
  }

  const Priority& HeuristicsEngine::worstCasePriority() const {
    if(betterThan(CONST_MIN_PRIORITY(), CONST_MAX_PRIORITY()))
      return CONST_MAX_PRIORITY();
    else
      return CONST_MIN_PRIORITY();
  }

  bool HeuristicsEngine::betterThan(const Priority p1, const Priority p2) const {
    debugMsg("HeuristicsEngine:betterThan", std::setprecision(20) << "betterThan(" << p1 << ", " << p2 << "), preferLow: " << 
             m_preferLowPriority << " (p2 - p1): " << (p2 - p1) << " (p1 - p2): " << (p1 - p2) << " EPSILON: " << EPSILON << 
             " (p2 - p1) >= EPSILON: " << ((p2 - p1) >= EPSILON) << " (p1 - p2) > EPSILON: " << ((p1 - p2) >= EPSILON));
    if (m_preferLowPriority)
      return  (p2 - p1) >= EPSILON; //(p1 < p2);
    return (p1 - p2) >= EPSILON; //(p2 < p1);
  }

  Priority HeuristicsEngine::getPriority(const EntityId& entity) const {
    HeuristicInstanceId instance = getActiveHeuristic(entity);
    if (instance.isId())
      return(instance->getPriority());
    if (ConstrainedVariableId::convertable(entity))
      return(getDefaultVariablePriority());
    checkError(TokenId::convertable(entity), entity->toString() << " must be a token or variable.");
    return(getDefaultTokenPriority());
  }

  Priority HeuristicsEngine::getDefaultVariablePriority() const {
    return(m_defaultVariablePriority);
  }

  Priority HeuristicsEngine::getDefaultTokenPriority() const {
    return(m_defaultTokenPriority);
  }

  const bool& HeuristicsEngine::preferNewerEntities() const {
    return(m_preferNewOverOld);
  }

  const VariableHeuristic::DomainOrder& HeuristicsEngine::getDefaultDomainOrder() const {
    return(m_defaultDomainOrder);
  }

  void HeuristicsEngine::orderChoices(const ConstrainedVariableId& var, std::list<double>& choicesToOrder) const {
    HeuristicInstanceId instance = getActiveHeuristic(var);
    check_error(instance.isNoId() || instance.isValid());

    if (instance.isId() && VariableHeuristicId::convertable(instance->getHeuristic())) {
      VariableHeuristicId heuristic = (VariableHeuristicId) instance->getHeuristic();
      const std::list<double>& values = heuristic->getValues();
      const VariableHeuristic::DomainOrder& order = heuristic->getDomainOrder();
      VariableHeuristic::orderChoices(getPlanDatabase(), var, values, order, choicesToOrder);
      debugMsg("HeuristicsEngine:orderChoices", "HEURISTIC " << heuristic->toString());
      return;
    }

    static const std::list<double> sl_emptyList;
    VariableHeuristic::orderChoices(getPlanDatabase(), var, sl_emptyList, m_defaultDomainOrder, choicesToOrder);
    condDebugMsg(instance.isNoId(), "HeuristicsEngine:orderChoices", "NO HEURISTIC");
    condDebugMsg(instance.isId(), "HeuristicsEngine:orderChoices", "NO VARIABLE HEURISTIC " << instance->toString());
  }

  void HeuristicsEngine::orderChoices(const TokenId& token, 
				      std::vector<LabelStr>& statesToOrder, 
				      std::vector<TokenId>& choicesToOrder) const {
    // Set up the default order in case we don't get a hit
    TokenHeuristic::CandidateOrder order = getOrderForState(Token::MERGED, m_defaultStates, m_defaultOrders);

    // First, insert states into a set. This will be used to test for membership
    std::set<LabelStr> allowedStates;
    for(std::vector<LabelStr>::const_iterator it = statesToOrder.begin(); it != statesToOrder.end(); ++it)
      allowedStates.insert(*it);

    // clear the states to order. In returning, it could be pruned!
    statesToOrder.clear();

    // Now try to use a heuristic to ppulate in order, with intersection
    HeuristicInstanceId instance = getActiveHeuristic(token);
    check_error(instance.isNoId() || instance.isValid());

    if(instance.isId() && TokenHeuristicId::convertable(instance->getHeuristic()) ){
      TokenHeuristicId heuristic = (TokenHeuristicId) instance->getHeuristic();
      if(!heuristic->getStates().empty()){
	for(unsigned int i = 0; i < heuristic->getStates().size(); i++){
	  LabelStr state = heuristic->getStates()[i];
	  if(allowedStates.find(state) != allowedStates.end())
	    statesToOrder.push_back(heuristic->getStates()[i]);
	}
	order = getOrderForState(Token::MERGED, heuristic->getStates(), heuristic->getOrders());
	debugMsg("HeuristicsEngine:orderChoices", "HEURISTIC " << heuristic->toString());
	return;
      }
    }

    // Otherwise we set defaults    
    for(unsigned int i = 0; i < m_defaultStates.size(); i++){
      LabelStr state = m_defaultStates[i];
      if(allowedStates.find(state) != allowedStates.end())
	statesToOrder.push_back(state);

      debugMsg("HeuristicsEngine:orderChoices", "NO HEURISTIC - Using Default State Order");
    }
  }

  void HeuristicsEngine::orderChoices(const TokenId& token, std::vector< OrderingChoice >& choicesToOrder) const {
    // Set up the default order in case we don't get a hit
    TokenHeuristic::CandidateOrder order = getOrderForState(Token::ACTIVE, m_defaultStates, m_defaultOrders);

    bool hit(false);
    HeuristicInstanceId instance = getActiveHeuristic(token);
    if(instance.isId() && TokenHeuristicId::convertable(instance->getHeuristic())){
      TokenHeuristicId heuristic = (TokenHeuristicId) instance->getHeuristic();
      if(!heuristic->getStates().empty()){
	order = getOrderForState(Token::ACTIVE, heuristic->getStates(), heuristic->getOrders());
	hit = true;
	debugMsg("HeuristicsEngine:orderChoices", "HEURISTIC " << heuristic->toString());
      }
    }

    condDebugMsg(!hit, "HeuristicsEngine:orderChoices", "NO HEURISTIC - Using Default Token Order");

    // Select the reference token
    TokenId referenceToken = (token->getMaster().isId() ? token->getMaster() : token);
    TokenHeuristic::orderTokens(choicesToOrder, referenceToken, order);
  }

  TokenHeuristic::CandidateOrder HeuristicsEngine::getOrderForState(const LabelStr& state,
								    const std::vector<LabelStr>& states,
								    const std::vector<TokenHeuristic::CandidateOrder>& orders){
    unsigned int index = 0;
    for(std::vector<LabelStr>::const_iterator it = states.begin(); it != states.end(); ++it){
      if(state == *it)
	return orders[index];
      index++;
    }

    return TokenHeuristic::NONE;
  }

  /**
   * On addition of a token, we find all the heuristics that match with the predicate and
   * we test of they can match the token. If the do, we allocate instances.
   */
  void HeuristicsEngine::notifyAdded(const TokenId& token){
    check_error(token.isValid());
    checkError(m_initialized, "Cannot add tokens if not initialized.");
    handleAddition(token, token->getPredicateName());
    handleAddition(token, ANY_PREDICATE());
  }

  void HeuristicsEngine::handleAddition(const TokenId& token, const LabelStr& predicate){
    check_error(token.isValid());
    checkError(Schema::instance()->isPredicate(predicate) || predicate == ANY_PREDICATE(), predicate.toString());

    const double dblKey = predicate.getKey();
    std::multimap<double, HeuristicId>::const_iterator it = m_heuristicsByPredicateKey.find(dblKey);

    debugMsg("HeuristicsEngine:handleAddition", "Evaluating possible heuristics for " << token->toString() << "(" << predicate.toString() << ")");
    
    while(it != m_heuristicsByPredicateKey.end() && it->first == dblKey){
      static unsigned int sl_counter(0);
      sl_counter++;
      
      const HeuristicId& heuristic = it->second;
      if(heuristic->canMatch(token) && !alreadyAllocated(token, heuristic)){
        HeuristicInstanceId heuristicInstance = heuristic->createInstance(token);
        checkError(heuristicInstance.isValid(), "Invalid heuristic for " << token->toString());
        debugMsg("HeuristicsEngine:handleAddition", heuristicInstance->toString());
        
        m_heuristicInstancesByToken.insert(std::pair<int, HeuristicInstanceId>(token->getKey(), heuristicInstance));
      }

      ++it;
    }

    // If the predicate is defined on the parent class, then
    // call this function recursively.
    // @todo Test case for this
    if(predicate != ANY_PREDICATE() && Schema::instance()->hasParent(predicate))
      handleAddition(token, Schema::instance()->getParent(predicate));

    debugMsg("HeuristicsEngine:handleAddition", "Completed.  Found " << m_heuristicInstancesByToken.count(token->getKey()) << " heuristics.");
  }

  /**
   * @note No need to address removal of proxy dependent information since normal unwind rules will
   * guarantee that the merged token for which it is a proxy has already been split, and it is handled there.
   */
  void HeuristicsEngine::notifyRemoved(const TokenId& token){
    check_error(token.isValid());
    std::multimap<int, HeuristicInstanceId>::iterator it = m_heuristicInstancesByToken.find(token->getKey());
    while(it != m_heuristicInstancesByToken.end() && it->first == token->getKey()){
      HeuristicInstanceId heuristicInstance = it->second;
      m_heuristicInstancesByToken.erase(it++);
      killIt(heuristicInstance);
    }
  }

  void HeuristicsEngine::add(const HeuristicId& heuristic, const LabelStr& predicate){
    check_error(heuristic.isValid());
    check_error(m_heuristicInstancesByToken.empty());
    std::pair<double, HeuristicId> element(predicate.getKey(), heuristic);
    debugMsg("HeuristicsEngine:add", "Adding heuristic for predicate " << predicate.toString() << std::endl << heuristic->toString());
    m_heuristicsByPredicateKey.insert(element);
  }

  void HeuristicsEngine::remove(const HeuristicId& heuristic){
    check_error(heuristic.isValid());
    if(!m_deleted){
      const double predicateKey = heuristic->getPredicate().getKey();
      std::multimap<double, HeuristicId>::iterator it = m_heuristicsByPredicateKey.find(predicateKey);
      checkError(it != m_heuristicsByPredicateKey.end(), "Bad synchronization.");
      while(it != m_heuristicsByPredicateKey.end() && it->first == predicateKey){
	if(it->second == heuristic)
	  m_heuristicsByPredicateKey.erase(it++);
	else
	  ++it;
      }
    }
  }

  void HeuristicsEngine::setDefaultCreationPreference(bool preferNewer) {
    m_preferNewOverOld = preferNewer;
  }

  void HeuristicsEngine::setDefaultPriorityPreference(bool preferLowPriority) {
    m_preferLowPriority = preferLowPriority;
  }

  void HeuristicsEngine::setDefaultPriorityForToken(const Priority p, 
						    const LabelStr& pred, 
						    const std::vector< GuardEntry >& guards){
    check_error(!m_initialized);

    // Allocate the token heuristic for this
    new TokenHeuristic(m_id, pred, p, TokenHeuristic::noStates(),
		       TokenHeuristic::noOrders(), false, guards);
  }

  void HeuristicsEngine::setDefaultPriorityForToken(const Priority p){
    check_error(!m_initialized);
    m_defaultTokenPriority = p;
  }

  void HeuristicsEngine::setDefaultPriorityForConstrainedVariable(const Priority p){
    check_error(!m_initialized);
    m_defaultVariablePriority = p;
  }

  void HeuristicsEngine::setDefaultPreferenceForToken(const std::vector<LabelStr>& states, 
							 const std::vector<TokenHeuristic::CandidateOrder>& orders){
    check_error(!m_initialized);
    check_error(states.size() == orders.size());
    if(states.empty() || orders.empty())
      return;

    m_defaultStates = states;
    m_defaultOrders = orders;
  }

  void HeuristicsEngine::setDefaultPreferenceForConstrainedVariable(const VariableHeuristic::DomainOrder order){
    check_error(!m_initialized);
    m_defaultDomainOrder = order;
  }

  void HeuristicsEngine::killIt(const HeuristicInstanceId& heuristicInstance) {
    check_error(heuristicInstance.isValid());
    debugMsg("HeuristicsEngine::killIt", heuristicInstance->getKey());
    delete (HeuristicInstance*) heuristicInstance;
  }

  bool HeuristicsEngine::alreadyAllocated(const TokenId& token, const HeuristicId& heuristic) const{
    // Be default, return value is the end.
    int tokenKey = token->getKey();
    std::multimap<int, HeuristicInstanceId>::const_iterator it = m_heuristicInstancesByToken.find(tokenKey);
    while(it != m_heuristicInstancesByToken.end() && it->first == tokenKey){
      HeuristicInstanceId l_instance = it->second;
      if(l_instance->getHeuristic() == heuristic)
	return true;

      ++it;
    }

    return false;
  }

  /**
   * Validate:
   * @li <target/heuristic> tuples should be unique.
   * @li All id's are valid
   * @li All instances that are not on the fired list are not executed.
   * @li All instances that are on the fired list are executed.
   */ 
  bool HeuristicsEngine::isValid() const {
    // Verify the tuple<token/heuristic> is unique.
    std::set<int> uniqueKeys;
    std::multimap<int, HeuristicInstanceId>::const_iterator it = m_heuristicInstancesByToken.begin();
    while(it != m_heuristicInstancesByToken.begin()){
      HeuristicInstanceId instance = it->second;
      checkError(instance.isValid(), instance);
      HeuristicId heuristic = instance->getHeuristic();
      checkError(heuristic.isValid(), heuristic);

      int tokenKey = it->first;

      // Assume no more than 100000000 entities
      int uniqueKey = heuristic->getKey() * 100000000  + tokenKey;
      checkError(uniqueKeys.find(uniqueKey) == uniqueKeys.end(), 
		 "Dupicate entry for token (" << tokenKey << ") and " << heuristic->toString())

      uniqueKeys.insert(uniqueKey);
      ++it;
    }

    return true;
  }
}
