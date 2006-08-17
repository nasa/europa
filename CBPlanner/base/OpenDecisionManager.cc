#include "ConstrainedVariable.hh"
#include "OpenDecisionManager.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include "TokenDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Condition.hh"
#include "Debug.hh"
#include "RuleVariableListener.hh"
#include "HeuristicsEngine.hh"
#include "Schema.hh"
#include "Entity.hh"

#include <iomanip>

/**
 * Migration Notes:
 * - This will become one or more FlawManagers
 * - Will deprecate attachment and detachment of conditions. It will all be done via configuration
 * - 
 */
namespace EUROPA {

  OpenDecisionManager::OpenDecisionManager(const PlanDatabaseId& db) 
    : m_db(db),
      m_id(this),
      m_dbListener(NULL),
      m_ceListener(NULL),
      m_initialized(false),
      m_createdHE(true){
    m_dbListener = new OpenDecisionManager::DbListener(m_db, *this);
    m_ceListener = new OpenDecisionManager::CeListener(m_db->getConstraintEngine(), *this);
    m_he = (new HeuristicsEngine(db))->getId();
    m_he->initialize();
  }

  OpenDecisionManager::OpenDecisionManager(const PlanDatabaseId& db, const HeuristicsEngineId& he) 
    : m_db(db),
      m_id(this),
      m_dbListener(NULL),
      m_ceListener(NULL),
      m_initialized(false),
      m_he(he), m_createdHE(false){
    m_dbListener = new OpenDecisionManager::DbListener(m_db, *this);
    m_ceListener = new OpenDecisionManager::CeListener(m_db->getConstraintEngine(), *this);
  }

  OpenDecisionManager::~OpenDecisionManager() {
    check_error(m_ceListener != NULL);
    delete m_ceListener;

    check_error(m_dbListener != NULL);
    delete m_dbListener;

    if(m_createdHE){
      check_error(m_he.isValid());
      delete (HeuristicsEngine*) m_he;
    }

    check_error(m_id.isValid());
    m_id.remove();
  }

  const OpenDecisionManagerId& OpenDecisionManager::getId() const {return m_id;}

  bool OpenDecisionManager::preferHigherKeys() const {
    return(m_he->preferNewerEntities());
  }

  void OpenDecisionManager::attach(const ConditionId& cond) {
    checkError(!m_initialized, "Set of conditions should be fixed if we have initialized.");

    if(cond->isDynamic())
      m_dynamicConditions.insert(cond);
    else
      m_staticConditions.insert(cond);

    debugMsg("OpenDecisionManager:attach", "Condition Added " << cond); 
  }

  void OpenDecisionManager::detach(const ConditionId& cond) {
    if(cond->isDynamic())
      m_dynamicConditions.erase(cond);
    else
      m_staticConditions.erase(cond);
  }

  void OpenDecisionManager::initializeIfNeeded() {
    if(!m_initialized){
      populateFlawCandidates();
      m_initialized = true;
    }
  }

  std::vector<ConditionId> OpenDecisionManager::getConditions() const {
    std::vector<ConditionId> allConditions;

    for(ConditionSet::const_iterator it = m_staticConditions.begin(); it != m_staticConditions.end(); ++it){
      const ConditionId cond = *it;
      allConditions.push_back(cond);
    }

    for(ConditionSet::const_iterator it = m_dynamicConditions.begin(); it != m_dynamicConditions.end(); ++it){
      const ConditionId cond = *it;
      allConditions.push_back(cond);
    }

    return allConditions;
  }

  ConditionId OpenDecisionManager::getCondition(const int pos) const {
    std::vector<ConditionId> allConditions = getConditions();
    return allConditions[pos];
  }

  /**
   * The total number of decisions includes:
   * @li All Unbound Variables
   * @li All Open Conditions
   * @li All Threats
   * Where each of the above is in scope. This method will be slow and is primarily provided for
   * testing purposes.
   */
  unsigned int OpenDecisionManager::getNumberOfDecisions() const {
    checkError(m_initialized, "Must initialize the open decision manager before use.");

    checkError(m_db->getConstraintEngine()->constraintConsistent(),
               "Can only call this iif variable changes have been propagated first.");

    checkError(variableCachesAreValid(), "Variable buffers are invalid.");

    unsigned int flawCount(0);

    // Units are a subset of the more general variable flaws, so just iterate over them
    for(ConstrainedVariableSet::const_iterator it = m_unitVariableFlawCandidates.begin(); 
        it != m_unitVariableFlawCandidates.end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid());
      if(isVariableDecision(var))
        flawCount++;
    }

    // Units are a subset of the more general variable flaws, so just iterate over them
    for(ConstrainedVariableSet::const_iterator it = m_variableFlawCandidates.begin(); 
        it != m_variableFlawCandidates.end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid());
      if(isVariableDecision(var))
        flawCount++;
    }

    // Now for open conditions (token decision points)
    for(TokenSet::const_iterator it = m_tokenFlawCandidates.begin(); 
        it != m_tokenFlawCandidates.end(); ++it){
      TokenId token = *it;
      check_error(token.isValid());
      if(isTokenDecision(token))
        flawCount++;
    }

    // Now for threats
    const std::map<int, std::pair<TokenId, ObjectSet> >& m_tokensToOrder = m_db->getTokensToOrder();
    for(std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator it = m_tokensToOrder.begin(); 
        it != m_tokensToOrder.end(); ++it){
      TokenId token = it->second.first;
      check_error(token.isValid());
      if(isObjectDecision(token))
        flawCount++;
    }

    return flawCount;
  }


  const ConstrainedVariableSet& OpenDecisionManager::getUnitVariableFlawCandidates() const {
    return m_unitVariableFlawCandidates;
  }

  const ConstrainedVariableSet& OpenDecisionManager::getVariableFlawCandidates() const {
    return m_variableFlawCandidates;
  }

  const TokenSet& OpenDecisionManager::getTokenFlawCandidates() const {
    return m_tokenFlawCandidates;
  }

  bool OpenDecisionManager::passesStaticConditions(const EntityId& entity) const{
    return !isTemporalVariable(entity) &&  passesConditions(m_staticConditions, entity);
  }

  bool OpenDecisionManager::passesDynamicConditions(const EntityId& entity) const{
    debugMsg("OpenDecisionManager:passesDynamicConditions", "Evaluating " << entity->toString());
    return passesConditions(m_dynamicConditions, entity);
  }

  bool OpenDecisionManager::isVariableDecision(const ConstrainedVariableId& var) const{
    checkError(m_db->getConstraintEngine()->constraintConsistent(),"Must be propagated to consistency.");

    if(!isBuffered(var)){
      debugMsg("OpenDecisionManager:isVariableDecision", var->toString() << " is not decidable.");
      return false;
    }

    if(!isCompatGuard(var) && var->lastDomain().isSingleton()){
      debugMsg("OpenDecisionManager:isVariableDecision", var->toString() << " is a singleton but not a guard.");
      return false;
    }

    if(!passesStaticConditions(var) || !passesDynamicConditions(var))
      return false;

    return true;
  }

  bool OpenDecisionManager::isTokenDecision(const TokenId& token) const{
    checkError(m_db->getConstraintEngine()->constraintConsistent(),"Must be propagated to consistency.");
    if(!token->isInactive() ||
       !passesStaticConditions( token) ||
       !passesDynamicConditions( token))
      return false;
    else
      return true;
  }

  bool OpenDecisionManager::isObjectDecision(const TokenId& token) const{
    checkError(m_db->getConstraintEngine()->constraintConsistent(),"Must be propagated to consistency.");
    if(!token->isActive() || 
       m_db->getTokensToOrder().find(token->getKey()) == m_db->getTokensToOrder().end() ||
       !passesStaticConditions( token) ||
       !passesDynamicConditions( token))
      return false;
    else
      return true;
  }

  bool OpenDecisionManager::passesConditions(const ConditionSet& conditions, const EntityId& entity) {

    for(ConditionSet::const_iterator it = conditions.begin() ; it != conditions.end(); it++){
      const ConditionId curCond = (*it);
      if (!curCond->test(entity)) {
        debugMsg("OpenDecisionManager:passesCondition", entity->toString() << " failed " << curCond);
        return false;
      }
    }
    debugMsg("OpenDecisionManager:passesCondition", entity->toString() << " passes filter conditions.");
    return true;
  }

  void OpenDecisionManager::print(std::ostream& os) {
    std::vector<ConditionId> conditions = getConditions();
    for (std::vector<ConditionId>::const_iterator it = conditions.begin(); it != conditions.end(); ++it){
      const ConditionId condition = *it;
      os << condition << std::endl;
    }
  }

  /**
   * @brief Updates the membership of a variable in the flaw candidate buffers.
   *
   * We must consider cases of:
   * 1. Token Variables - should only be flaws when the token is active
   * 2. Object Variables - should always be specified to singletons and never be flaws
   * 3. Local Rule Variables - should only be flaws when they are guards
   * 4. Global Variables - designated by the variable having no parent
   */
  void OpenDecisionManager::updateFlaw(const ConstrainedVariableId& var){
    debugMsg("OpenDecisionManager:updateFlaw", var->toString());

    // For simpicity, remove from both, and add back as necessary
    m_variableFlawCandidates.erase(var);
    m_unitVariableFlawCandidates.erase(var);

    if(!isDecidable(var) || !passesStaticConditions( var)){
      debugMsg("OpenDecisionManager:updateFlaw", "Excluding: " << var->toString());
      return;
    }

    if(var->lastDomain().isSingleton()){
      debugMsg("OpenDecisionManager:updateFlaw", "Adding as a unit: " << var->toString());
      m_unitVariableFlawCandidates.insert(var);
    }
    else{
      debugMsg("OpenDecisionManager:updateFlaw", "Adding as a non-unit: " << var->toString());
      m_variableFlawCandidates.insert(var);
    }
  }

  void OpenDecisionManager::removeFlaw(const ConstrainedVariableId& var){
    condDebugMsg(m_variableFlawCandidates.find(var) != m_variableFlawCandidates.end(), 
                 "OpenDecisionManager:removeFlaw", "Removing " << var->getKey() << " as a flaw.");
    m_variableFlawCandidates.erase(var);
    m_unitVariableFlawCandidates.erase(var);
  }

  bool OpenDecisionManager::variableCachesAreValid() const {
    static unsigned int sl_counter(0);
    sl_counter++;
    for(ConstrainedVariableSet::const_iterator it = m_unitVariableFlawCandidates.begin(); 
        it != m_unitVariableFlawCandidates.end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid());
      checkError(var->lastDomain().isSingleton(), var->toString() << ":" << sl_counter);
      if(!var->lastDomain().isSingleton())
        return false;
    }

    for(ConstrainedVariableSet::const_iterator it = m_variableFlawCandidates.begin(); 
        it != m_variableFlawCandidates.end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid());
      checkError(!var->lastDomain().isSingleton(), var->toString() << ":" << sl_counter);
      if(var->lastDomain().isSingleton())
        return false;
    }

    return true;
  }

  void OpenDecisionManager::addGuard(const ConstrainedVariableId& var){
    std::map<ConstrainedVariableId, unsigned int>::iterator it = m_guardCache.find(var);
    unsigned int refCount = 1;
    // If already guarded just increment the ref count
    if(it != m_guardCache.end()){
      refCount = it->second;
      refCount++;
      it->second = refCount;
    }
    else // Insert a new pair
      m_guardCache.insert(std::pair<ConstrainedVariableId, unsigned int>(var, 1));

    debugMsg("OpenDecisionManager:addGuard", 
             "GUARDS=" << refCount << " for " << var->getName().toString() << "(" << var->getKey() << ")");
  }

  void OpenDecisionManager::removeGuard(const ConstrainedVariableId& var){
    std::map<ConstrainedVariableId, unsigned int>::iterator it = m_guardCache.find(var);

    if(it == m_guardCache.end())
      return;

    unsigned int refCount = it->second;
    if(refCount == 1)
      m_guardCache.erase(it);
    else
      it->second = --refCount;
  }

  void OpenDecisionManager::handleConstraintAddition(const ConstraintId& constraint){
    if(constraint->getName() == RuleVariableListener::CONSTRAINT_NAME()){
      const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
      for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
        ConstrainedVariableId guard = *it;
        addGuard(guard);
      }
    }
  }

  void OpenDecisionManager::handleConstraintRemoval(const ConstraintId& constraint){
    if(constraint->getName() == RuleVariableListener::CONSTRAINT_NAME()){
      const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
      for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
        ConstrainedVariableId guard = *it;
        removeGuard(guard);
        
      }
    }
  }

  bool OpenDecisionManager::variableOfNonActiveToken(const ConstrainedVariableId& var){
    // If var parent is a token and the state is active, then false.
    if(TokenId::convertable(var->getParent())){
      TokenId token(var->getParent());
      return !token->isActive();
    }

    // Otherwise false
    return false;
  }

  bool OpenDecisionManager::isDecidable(const ConstrainedVariableId& var){
    return var->canBeSpecified() && !var->isSpecified() && !variableOfNonActiveToken(var);
  }

  bool OpenDecisionManager::isBuffered(const ConstrainedVariableId& var) const {
    if( m_unitVariableFlawCandidates.find(var) == m_unitVariableFlawCandidates.end() &&
        m_variableFlawCandidates.find(var) == m_variableFlawCandidates.end()){

      checkError((var->lastDomain().isOpen() && var->getParent().isNoId()) || 
                 !isDecidable(var) || 
                 !passesStaticConditions(var), 
                 "Must be a gloabal open variable, !decideable or it was filtered by a condition: " << var->toString());

      return false;
    }

    return true;
  }

  bool OpenDecisionManager::isCompatGuard(const ConstrainedVariableId& var) const{
    return m_guardCache.find(var) != m_guardCache.end();
  }

  void OpenDecisionManager::addFlaw(const TokenId& token){
    if(token->isInactive() && passesStaticConditions( token)){
      m_tokenFlawCandidates.insert(token);
      debugMsg("OpenDecisionManager:addFlaw",
               "Added " << token->toString() << " as a candidate flaw. There are now " <<
               m_tokenFlawCandidates.size() << " candidates.");
    }
  }

  void OpenDecisionManager::removeFlaw(const TokenId& token){
    m_tokenFlawCandidates.erase(token);
    debugMsg("OpenDecisionManager:removeFlaw",
             "Removeded " << token->toString() << " as a candidate flaw. There are now " << 
             m_tokenFlawCandidates.size() << " candidates.");
  }

  OpenDecisionManager::CeListener::CeListener(const ConstraintEngineId& ce,
                                              OpenDecisionManager& odm)
    : ConstraintEngineListener(ce), m_odm(odm){}

  void OpenDecisionManager::CeListener::notifyRemoved(const ConstrainedVariableId& variable){
    m_odm.removeFlaw(variable);
  }

  void OpenDecisionManager::CeListener::notifyChanged(const ConstrainedVariableId& variable, 
                                                      const DomainListener::ChangeType& changeType){

    // In the event it is bound to a singleton, we remove it altogether as a flaw.
    if(changeType == DomainListener::SET_TO_SINGLETON){
      m_odm.removeFlaw(variable);
      return;
    }

    // Now listen for all the other events of interest. We can ignore other cases of restriction since
    // the event set below is sufficient to capture all the meaningful changes without incurring
    // all the evaluation costs on every propagation.
    if(changeType == DomainListener::RESET || 
       changeType == DomainListener::CLOSED ||
       changeType == DomainListener::RELAXED ||
       changeType == DomainListener::RESTRICT_TO_SINGLETON)
      m_odm.updateFlaw(variable);
  }

  void OpenDecisionManager::CeListener::notifyAdded(const ConstraintId& constraint){
    m_odm.handleConstraintAddition(constraint);
  }

  void OpenDecisionManager::CeListener::notifyRemoved(const ConstraintId& constraint){
    m_odm.handleConstraintRemoval(constraint);
  }

  OpenDecisionManager::DbListener::DbListener(const PlanDatabaseId& db,
                                              OpenDecisionManager& dm)
    : PlanDatabaseListener(db), m_odm(dm){}

  void OpenDecisionManager::DbListener::notifyAdded(const TokenId& token){
    m_odm.addFlaw(token);
  }

  void OpenDecisionManager::DbListener::notifyRemoved(const TokenId& token){
    m_odm.removeFlaw(token);
  }

  void OpenDecisionManager::DbListener::notifyActivated(const TokenId& token){
    const std::vector<ConstrainedVariableId>& variables = token->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
      ConstrainedVariableId var = *it;
      m_odm.updateFlaw(var);
    }

    m_odm.removeFlaw(token);
  }

  void OpenDecisionManager::DbListener::notifyDeactivated(const TokenId& token){
    const std::vector<ConstrainedVariableId>& variables = token->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
      ConstrainedVariableId var = *it;
      m_odm.removeFlaw(var);
    }
    m_odm.addFlaw(token);
  }

  void OpenDecisionManager::DbListener::notifyMerged(const TokenId& token){
    m_odm.removeFlaw(token);
  }

  void OpenDecisionManager::DbListener::notifySplit(const TokenId& token){
    m_odm.addFlaw(token);
  }

  void OpenDecisionManager::DbListener::notifyRejected(const TokenId& token){
    m_odm.removeFlaw(token);
  }

  void OpenDecisionManager::DbListener::notifyReinstated(const TokenId& token){
    m_odm.addFlaw(token);
  }

  DecisionPointId OpenDecisionManager::getNextDecision() {
    checkError(m_initialized, "Must initialize the open decision manager before use.");

    // Force propagation
    m_db->getConstraintEngine()->propagate();

    debugMsg("OpenDecisionManager:getNextDecision", std::endl << printOpenDecisions());

    checkError(m_db->getConstraintEngine()->constraintConsistent(),
               "Database must be fully propagated and consistent to make this query.");

    //prefer zero commitment decisions over everything. So just try to get one.
    DecisionPointId bestDecision = getZeroCommitmentDecision();

    // Initially we start with the absolute worst case so we will not filter
    // anything out falsely. This should be a priority that is unattainably bad.
    // In order to handle cases of high or low interpretations, we multiply by the
    // worstCase valid priority.
    Priority bestp = m_he->worstCasePriority() * 2;

    // Take the unit if it is available, otherwise, delve deeper
    if(bestDecision.isNoId()){
      DecisionPointId candidateDecision;

      // Threats
      TokenSet threats;
      const std::map<int, std::pair<TokenId, ObjectSet> >& tokensToOrder = m_db->getTokensToOrder();
      for(std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator it = tokensToOrder.begin(); 
          it != tokensToOrder.end(); ++it)
        threats.insert(it->second.first);

      debugMsg("OpenDecisionManager:getNextDecision", "Getting best threat decision...");
      candidateDecision = getBestTokenDecision(bestp, threats);
      updateDecisionPoints(candidateDecision, bestDecision);
      if(m_he->bestCasePriority() == bestp)
        goto handleNewDecision;
    
      // Now try token decisions
      debugMsg("OpenDecisionManager:getNextDecision", "Getting best open condition decision...");
      candidateDecision = getBestTokenDecision(bestp, getTokenFlawCandidates());
      updateDecisionPoints(candidateDecision, bestDecision);
      if(m_he->bestCasePriority() == bestp)
        goto handleNewDecision;


      // Process variables and extract guards rom non-guars
      // @todo - maintain in separate buffers
      const ConstrainedVariableSet& vars = getVariableFlawCandidates();
      std::vector<ConstrainedVariableId> compats;
      std::vector<ConstrainedVariableId> nonCompats;
      for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it){
        const ConstrainedVariableId var = *it;
        if(isCompatGuard(var))
          compats.push_back(var);
        else
          nonCompats.push_back(var);
      }

      // Non Unit compats are next
      debugMsg("OpenDecisionManager:getNextDecision", "Getting best guard variable decision...");
      candidateDecision = getBestVariableDecision(bestp, compats);
      updateDecisionPoints(candidateDecision, bestDecision);
      if(m_he->bestCasePriority() == bestp)
        goto handleNewDecision;

      // Finally, if we are desparate :-) General variables
      debugMsg("OpenDecisionManager:getNextDecision", "Getting best variable decision...");
      candidateDecision = getBestVariableDecision(bestp, nonCompats);
      updateDecisionPoints(candidateDecision, bestDecision);
    }
    
  handleNewDecision: // GOT LANDING POINT: HANDLE NEW DECISION

    // Now if we have a decision point, we should initialize its choices.
    if(bestDecision.isId())
      initializeChoices(bestDecision);

    debugMsg("OpenDecisionManager:getNextDecision", "Best Dec = [" << bestp << "] " << bestDecision);
    return bestDecision;
  }

  void OpenDecisionManager::updateDecisionPoints(DecisionPointId& candidate, DecisionPointId& best){
    condDebugMsg(candidate.isNoId() && best.isId(), "OpenDecisionManager:updateDecisionPoints", "Not updating best decision " << best->toString());
    if(candidate.isId()){
      condDebugMsg(best.isId(), "OpenDecisionManager:updateDecisionPoints", "Updating best decision from " << best->toString() << " to " << candidate->toString());
      condDebugMsg(best.isNoId(), "OpenDecisionManager:updateDecisionPoints", "Updating best decision from noId to " << candidate->toString());
      if(best.isId())
        delete (DecisionPoint*) best;
      best = candidate;
      candidate = DecisionPointId::noId();
    }
  }

  /**
   * While we could do more, we will restrict ourselves to unit variable decisions, and they must be compat guards
   */
  DecisionPointId OpenDecisionManager::getZeroCommitmentDecision(){
    for(ConstrainedVariableSet::const_iterator it = m_unitVariableFlawCandidates.begin();
        it != m_unitVariableFlawCandidates.end(); ++it){
      const ConstrainedVariableId candidate = *it;
      checkError(candidate->lastDomain().isSingleton(), "Not singleton:" << candidate->toString());
      if(isCompatGuard(candidate) && passesDynamicConditions( candidate))
        return createConstrainedVariableDecisionPoint(candidate);
    }

    return DecisionPointId::noId();
  }

  bool OpenDecisionManager::isUnitDecision(const ConstrainedVariableId& var) const{
    checkError(m_db->getConstraintEngine()->constraintConsistent(),"Must be propagated to consistency.");
    return (getUnitVariableFlawCandidates().find(var) != getUnitVariableFlawCandidates().end() &&
            isCompatGuard(var) && passesDynamicConditions(var));
  }

  class TokenEvaluator : public OpenDecisionManager::Evaluator {
  public:
    TokenEvaluator(bool preferHigherKeys): Evaluator(preferHigherKeys){}

    unsigned int countChoices(const EntityId& candidate) const {
      checkError(TokenId::convertable(candidate), candidate);
      TokenId token = candidate;
      if(token->isActive()){
        static const unsigned int sl_upperLimit(2); /*!< Only values of 1 and 2 will be considered useful */
        return token->getPlanDatabase()->countOrderingChoices(candidate, sl_upperLimit);
      }
      else {
        unsigned int choiceCount = 0;
        if(token->getState()->lastDomain().isMember(Token::MERGED) &&
           token->getPlanDatabase()->hasCompatibleTokens(token))
          choiceCount++;

        if (token->getState()->lastDomain().isMember(Token::ACTIVE) &&
            token->getPlanDatabase()->hasOrderingChoice(token)) 
          choiceCount++;

        if (token->getState()->lastDomain().isMember(Token::REJECTED))
          choiceCount++;
        return choiceCount;
      }
    }
  };

  /**
   * Iterate over the set of candidates. If it gets a better priority than the current best priority.
   */
  DecisionPointId OpenDecisionManager::getBestTokenDecision(Priority& p, const TokenSet& candidates) {
    // Now go through in key order to get the best choice
    EntityIterator< TokenSet::const_iterator > tokenIterator(candidates.begin(), candidates.end());
    TokenEvaluator evaluator(preferHigherKeys());
    return getBestCandidate(p, tokenIterator, evaluator);
  }

  class VariableEvaluator : public OpenDecisionManager::Evaluator {
  public:
    VariableEvaluator(): Evaluator(false){}
    unsigned int countChoices(const EntityId& candidate) const {
      checkError(ConstrainedVariableId::convertable(candidate), candidate);
      ConstrainedVariableId var = candidate;
      checkError(!var->isSpecified(), var->toString());

      unsigned int domainSize = (var->lastDomain().isFinite() ? 
                                 var->lastDomain().getSize() : 
                                 PLUS_INFINITY);

      return domainSize;
    }
  };

  DecisionPointId OpenDecisionManager::getBestVariableDecision(Priority& p,
                                                               const std::vector<ConstrainedVariableId>& candidates) {

    // Now go through in key order to get the best choice
    EntityIterator< std::vector<ConstrainedVariableId>::const_iterator > variableIterator(candidates.begin(), 
                                                                                          candidates.end());

    VariableEvaluator evaluator;
    return getBestCandidate(p, variableIterator, evaluator);
  }

  void OpenDecisionManager::printOpenDecisions(std::ostream& os) {
    os << printOpenDecisions();
  }

  /**
   * @brief Will print the open decisions in priority order
   */
  std::string OpenDecisionManager::printOpenDecisions() const {
    static const std::string sl_indentation("   ");

    checkError(m_db->getConstraintEngine()->constraintConsistent(),
               "Can only call this if variable changes have been propagated first.");

    std::multimap<Priority, std::string> priorityQueue; // Used to make output in priority order

    // Use a multiplier to flip the order based on high or low preference. Low priority should give
    // positive adjustment.
    int multiplier = (m_he->betterThan(0, 1) ? 1 : -1 );

    // This is how overall orderings are defined in the event of a tiebreak
    const double threatAdjustment = multiplier * EPSILON;
    const double openConditionAdjustment = multiplier * 2 * EPSILON;
    const double guardAdjustment = multiplier * 3 * EPSILON;
    const double nonUnitVarAdjustment = multiplier * 4 * EPSILON;

    // First units
    for(ConstrainedVariableSet::const_iterator it = m_unitVariableFlawCandidates.begin(); 
        it != m_unitVariableFlawCandidates.end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid());
      std::string compatStr = (isCompatGuard(var) ? " GUARD" : "");
      if(isVariableDecision(var)){
        std::stringstream os;
        os << sl_indentation << "VAR:   " << var->toString() << " UNIT"  << compatStr << std::endl;
        priorityQueue.insert(std::pair<Priority, std::string>(m_he->bestCasePriority(), os.str()));
      }
    }

    // Now for threats
    const std::map<int, std::pair<TokenId, ObjectSet> >& m_tokensToOrder = m_db->getTokensToOrder();
    for(std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator it = m_tokensToOrder.begin(); 
        it != m_tokensToOrder.end(); ++it){
      TokenId token = it->second.first;
      check_error(token.isValid());
      if(isObjectDecision(token)){
        std::stringstream os;
        Priority priority = m_he->getPriority(token);
        os << sl_indentation << "THREAT:" << token->toString() << " PRIORITY==" << priority << std::endl;
        priorityQueue.insert(std::pair<Priority, std::string>(priority + threatAdjustment, os.str()));
      }
    }

    for(ConstrainedVariableSet::const_iterator it = m_variableFlawCandidates.begin(); 
        it != m_variableFlawCandidates.end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid());
      if(isVariableDecision(var)){
        std::string compatStr = "";
        double adjustment = nonUnitVarAdjustment;
        if(isCompatGuard(var)){
          compatStr = "GUARD";
          adjustment = guardAdjustment;
        }

        std::stringstream os;
        Priority priority = m_he->getPriority(var);
        os << sl_indentation << "VAR:   " << var->toString() << compatStr << " PRIORITY==" << 
          priority << (isUnitDecision(var) ? " UNIT" : "" ) << std::endl;
        priorityQueue.insert(std::pair<Priority, std::string>(priority + adjustment, os.str()));
      }
    }

    // Now for open conditions (token decision points)
    for(TokenSet::const_iterator it = m_tokenFlawCandidates.begin(); 
        it != m_tokenFlawCandidates.end(); ++it){
      TokenId token = *it;
      check_error(token.isValid());
      if(isTokenDecision(token)){
        std::stringstream os;
        Priority priority = m_he->getPriority(token);
        os << sl_indentation << "TOKEN: " << token->toString() << " PRIORITY==" << priority << std::endl;
        priorityQueue.insert(std::pair<Priority, std::string>(priority + openConditionAdjustment, os.str()));
      }
    }

    std::stringstream os;
    os << "OpenDecisions:{ " << std::endl;

    // Order based on semantics of priority
    if(m_he->betterThan(0, 1)){
      for(std::multimap<Priority, std::string>::const_iterator it=priorityQueue.begin();it!=priorityQueue.end(); ++it){
        const std::string& str = it->second;
        os << str;
      }
    }
    else {
      std::multimap<Priority, std::string>::const_iterator it=priorityQueue.end();
      while(it!=priorityQueue.begin()){
        --it;
        const std::string& str = it->second;
        os << str;
      }
    }

    os << " }" << std::endl;

    return os.str();
  }

  void OpenDecisionManager::initializeChoices(const DecisionPointId& dec){
    check_error(dec.isValid());
    if(ConstrainedVariableDecisionPointId::convertable(dec))
      initializeVariableChoices(dec);
    else if(TokenDecisionPointId::convertable(dec))
      initializeTokenChoices(dec);
    else
      initializeObjectChoices(dec);
  }

  DecisionPointId OpenDecisionManager::createDecisionPoint(const EntityId& flaw){
    if(ConstrainedVariableId::convertable(flaw))
      return createConstrainedVariableDecisionPoint(flaw);

    checkError(TokenId::convertable(flaw), flaw);

    TokenId token = flaw;
    if(token->isInactive())
      return createTokenDecisionPoint(token);

    return createObjectDecisionPoint(token);
  }

  void OpenDecisionManager::initializeTokenChoicesInternal(const TokenDecisionPointId& tdp) {
    const StateDomain stateDomain(tdp->getToken()->getState()->lastDomain());
    TokenId tok(tdp->getToken());
    if(stateDomain.isMember(Token::MERGED)) {
      tok->getPlanDatabase()->getCompatibleTokens(tok, tdp->m_compatibleTokens, PLUS_INFINITY, true);
      debugMsg("OpenDecisionManager:initializeTokenChoices", "Found " << tdp->m_compatibleTokens.size() << " compatible tokens for " << tok->getKey());
      if(tdp->m_compatibleTokens.size() > 0) {
        debugMsg("OpenDecisionManager:initializeTokenChoices", "Pushing token:merged m_choices");
        tdp->m_choices.push_back(Token::MERGED);
      }
    }
    if(stateDomain.isMember(Token::ACTIVE) && tok->getPlanDatabase()->hasOrderingChoice(tok))
      tdp->m_choices.push_back(Token::ACTIVE);
    if(stateDomain.isMember(Token::REJECTED))
      tdp->m_choices.push_back(Token::REJECTED);
  }

  void OpenDecisionManager::initializeObjectChoicesInternal(const ObjectDecisionPointId& odp) {
    std::list<double> values;
    TokenId tok(odp->getToken());
    tok->getObject()->getLastDomain().getValues(values);
    EntityComparator<EntityId> cmp;
    values.sort(cmp);
    std::list<double>::iterator it = values.begin();
    for ( ; it != values.end(); ++it) {
      ObjectId obj = *it;
      check_error(obj.isValid());
      std::vector<std::pair<TokenId, TokenId> > tuples;
      obj->getOrderingChoices(tok, tuples);
      std::vector<std::pair<TokenId, TokenId> >::iterator it = tuples.begin();
      debugMsg("ObjectDecisionPoint:getChoices", "Choices constraining (" << tok->getKey() << ")");
      for (; it != tuples.end(); it++) {
        TokenId predecessor = it->first;
        TokenId successor = it->second;
        check_error(predecessor.isValid());
        check_error(successor.isValid());
        odp->m_choices.push_back(std::make_pair<ObjectId,std::pair<TokenId,TokenId> > (obj, *it));
        debugMsg("ObjectDecisionPoint:getChoices", "  constrain(" << predecessor->getKey() << ", " << successor->getKey() << ") on Object ( " << obj->getKey() << ")");
      }
    }
  }

  void OpenDecisionManager::initializeTokenChoices(const TokenDecisionPointId& tdp) {
    check_error(tdp.isValid());
    check_error(tdp->m_choices.empty());
    initializeTokenChoicesInternal(tdp);

    if (tdp->m_choices.empty()) return;

    debugMsg("OpenDecisionManager:initializeTokenChoices", "Allowed states " <<tdp->getToken()->getState()->toString());
    m_he->orderChoices(tdp->getToken(), tdp->m_choices, tdp->m_compatibleTokens);
  }

  void OpenDecisionManager::initializeVariableChoices(const ConstrainedVariableDecisionPointId& vdp) {
    check_error(vdp.isValid());
    check_error(vdp->getVariable()->lastDomain().isFinite());
    check_error(vdp->m_choices.empty());

    if (vdp->m_var->lastDomain().isNumeric() && vdp->m_var->lastDomain().getSize() > 50) {
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getLowerBound());
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getUpperBound()); // we'll keep the initial lb and ub for ref.
    }
    else {
      std::list<double> values;
      vdp->m_var->lastDomain().getValues(values);


      // Only apply a heuristic if it is not a singleton
      if(!vdp->m_var->lastDomain().isSingleton())
        m_he->orderChoices(vdp->m_var, values);

      for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it)
        vdp->m_choices.push_back(*it);
    }
  }

  void OpenDecisionManager::initializeObjectChoices(const ObjectDecisionPointId& odp) {
    check_error(odp.isValid());
    check_error(odp->m_choices.empty());
    initializeObjectChoicesInternal(odp);

    if (odp->m_choices.empty()) return;

    // Invoke heuristics engine to order choices according to whatever heuristics it has
    m_he->orderChoices(odp->getToken(), odp->m_choices);

  }

  void OpenDecisionManager::populateFlawCandidates(){
    checkError(!m_initialized, "Should callat most once.");
    m_unitVariableFlawCandidates.clear();
    m_variableFlawCandidates.clear();
    m_tokenFlawCandidates.clear();

    // FILL UP VARIABLES
    const ConstrainedVariableSet& allVars = m_db->getConstraintEngine()->getVariables();
    for(ConstrainedVariableSet::const_iterator it = allVars.begin(); it != allVars.end(); ++it){
      ConstrainedVariableId var = *it;
      updateFlaw(var);
    }


    // PROCESS CONSTRAINTS TO INITIALIZE GUARDS. We are looking for RuleVariableListener constraints since they 
    // determine if a variable is guarded or not.
    const ConstraintSet& allConstraints = m_db->getConstraintEngine()->getConstraints();
    for(ConstraintSet::const_iterator it = allConstraints.begin(); it != allConstraints.end(); ++it){ 
      ConstraintId constraint = *it;
      handleConstraintAddition(constraint);
    }

    // FILL UP ALL TOKEN FLAWS
    const TokenSet& allTokens = m_db->getTokens();
    for(TokenSet::const_iterator it = allTokens.begin(); it != allTokens.end(); ++it){
      TokenId token = *it;
      addFlaw(token);
    }
  }

  ObjectDecisionPointId OpenDecisionManager::createObjectDecisionPoint(const TokenId& token) {
    return (new ObjectDecisionPoint(m_db->getClient(), token, m_id) )->getId();
  }

  TokenDecisionPointId OpenDecisionManager::createTokenDecisionPoint(const TokenId& token) {
    return (new TokenDecisionPoint(m_db->getClient(), token, m_id))->getId();
  }

  ConstrainedVariableDecisionPointId OpenDecisionManager::createConstrainedVariableDecisionPoint(const ConstrainedVariableId& variable) {
    return (new ConstrainedVariableDecisionPoint(m_db->getClient(), variable, m_id))->getId();
  }

  bool OpenDecisionManager::isTemporalVariable(const EntityId& entity){
    static const LabelStr sl_start("start");
    static const LabelStr sl_end("end");
    static const LabelStr sl_duration("duration");
    static const LabelStr sl_time("time");

    if(!ConstrainedVariableId::convertable(entity))
      return false;

    ConstrainedVariableId var(entity);
    
    if(var->getParent().isNoId() || !TokenId::convertable(var->getParent()))
      return false;
    
    if(var->getName() == sl_start ||
       var->getName() == sl_end ||
       var->getName() == sl_duration ||
       var->getName() == sl_time)
      return true;

    return false;
  }

  bool OpenDecisionManager::isFlaw(const EntityId& candidate) const{
    checkError(TokenId::convertable(candidate) || ConstrainedVariableId::convertable(candidate), candidate);
    if(TokenId::convertable(candidate) && TokenId(candidate)->isActive())
      return isObjectDecision(candidate);
    else
      return passesDynamicConditions(candidate);
  }

  /**
   * Iterate over the set of candidates. If it gets a better priority than the current best priority.
   */
  DecisionPointId OpenDecisionManager::getBestCandidate(Priority& p, 
                                                        Iterator& it, 
                                                        const Evaluator& evaluator) {
    // Establish a local decision priority so we will not replace exiting decisions if
    // we only meet their priority
    Priority bestp = (m_he->betterThan(0,1) ? p - (2 * EPSILON) : p + (2 * EPSILON));

    debugMsg("OpenDecisionManager:getBestCandidate", "Working with current best priority " << std::setprecision(20) << bestp);
    // Now go through in key order to get the best choice
    EntityId flawToResolve;
    unsigned int minChoiceCount = PLUS_INFINITY; // Used for tie breaking
    int bestKey = 0;
    condDebugMsg(evaluator.preferHigherKeys(), "OpenDecisionManager:getBestCandidate", "Preferring higher keys.");
    while(!it.done()){
      const EntityId candidate = it.next();

      // If we already have the best achievable, quit looking.
      if(!m_he->betterThan(m_he->bestCasePriority(), bestp)) {
        debugMsg("OpenDecisionManager:getBestCandidate", "Already have best possible priority.");
        break;
      }

      // Since it is a decision, obtain its priority
      Priority priority = m_he->getPriority(candidate);

      // If we cannot meet or beat current best, or it should be filtered out, then skip
      if(m_he->betterThan(bestp, priority)) {
        debugMsg("OpenDecisionManager:getBestCandidate", std::setprecision(20) << "Skipping " << candidate->getKey() << " ( priority " <<
                 priority << " not better than current best " << bestp << ")");
        continue;
      }

      debugMsg("OpenDecisionManager:getBestCandidate", std::setprecision(20) << "Priority for " << candidate->getKey() << "(" << priority << 
               ") is apparently better than or equal to current best (" << bestp << ")");

      // Try cheap check first
      //if(priority == bestp && evaluator.preferHigherKeys() && bestKey > candidate->getKey()) {
      if(fabs(priority - bestp) < EPSILON && evaluator.preferHigherKeys() && bestKey > candidate->getKey()) {
        debugMsg("OpenDecisionManager:getBestCandidate", "Skipping " << candidate->getKey() << " ( key " <<
                 candidate->getKey() << " < " << bestKey << ")");
        continue;
      }

      // Must be a valid flaw - test it here if we dont prefer higher keys
      // since choice counting is expensive so we prune the flaw firts
      if(!evaluator.preferHigherKeys() && !isFlaw(candidate)) {
        debugMsg("OpenDecisionManager:getBestCandidate", "Skipping " << candidate->getKey() << 
                 " ( preferHigherKeys == false && not a flaw)");
        continue;
      }

      // If we don't have a preference for higher keys, test choice count. This can be expensive!
      unsigned int choiceCount(0);
      if(!evaluator.preferHigherKeys()){
        choiceCount = evaluator.countChoices(candidate);
        //if(priority == bestp && choiceCount >= minChoiceCount) {
        if(fabs(priority - bestp) < EPSILON && choiceCount >= minChoiceCount) {
          debugMsg("OpenDecisionManager:getBestCandidate", "Skipping " << candidate->getKey() <<
                   " ( choice count " << choiceCount << " >= " << minChoiceCount << ")");
          continue;
        }
      }


      // We leave test as a flaw till here if we prefer higher keys, since other tests will be
      // a good bit faster
      if(evaluator.preferHigherKeys() && !isFlaw(candidate)) {
        debugMsg("OpenDecisionManager:getBestCandidate", "Skipping " << candidate->getKey() << 
                 " ( preferHigherKeys == true && not a flaw)");
        continue;
      }

      debugMsg("OpenDecisionManager:getBestCandidate", candidate->toString() << " is a better decision.");
      debugMsg("OpenDecisionManager:getBestCandidate", std::setprecision(20) << "Updating minChoiceCount (" << minChoiceCount << " -> " << choiceCount << "), bestp (" <<
               bestp << " -> " << priority << "), bestKey (" << bestKey << " -> " << candidate->getKey() << ")");

      // Otherwise we update our data for this to be the best
      minChoiceCount = choiceCount;
      flawToResolve = candidate;
      bestp = priority;
      bestKey = candidate->getKey();
    }

    DecisionPointId decision;
    if(flawToResolve.isId()){
      p = bestp;
      decision = createDecisionPoint(flawToResolve);
    }

    return decision;
  }

}
