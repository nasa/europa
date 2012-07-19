#include "Solver.hh"
#include "Debug.hh"
#include "DbClient.hh"
#include "ConstraintEngine.hh"
#include "FlawManager.hh"
#include "SolverDecisionPoint.hh"
#include "Utils.hh"
#include "SolverUtils.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "FlawHandler.hh"
#include "Context.hh"
#include <bitset>

/**
 * @file Solver.cc
 * @brief Provides the implementation for the solver base class.
 * @author Conor McGann
 * @date May, 2005
 * @todo Generate notifications for Solver to publish (i.e. for the SearchListener).
 */

#define publish(m,dp) { \
    debugMsg("Solver:publish", "Publishing message " << #m);		\
  for(std::list<SearchListenerId>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it) { \
    (*it)->m(dp); \
  } \
}

namespace EUROPA {
  namespace SOLVERS {

    Solver::Solver(const PlanDatabaseId& db, const TiXmlElement& configData)
      : m_id(this), m_db(db),
        m_stepCountFloor(0), m_depthFloor(0), m_stepCount(0),
        m_noFlawsFound(false), m_exhausted(false), m_timedOut(false),
        m_maxSteps(PLUS_INFINITY), m_maxDepth(PLUS_INFINITY),
        m_masterFlawFilter(configData), m_ceListener(db->getConstraintEngine(), *this),
        m_dbListener(db, *this) {
      checkError(strcmp(configData.Value(), "Solver") == 0,
                 "Configuration file error. Expected element <Solver> but found " << configData.Value());

      // Extract the name of the Solver
      m_name = extractData(configData, "name");

      m_context = ((new Context(m_name.toString() + "Context"))->getId());
      // Initialize the common filter
      m_masterFlawFilter.initialize(configData, m_db, m_context);

      // Now load all the flaw managers
      for (TiXmlElement * child = configData.FirstChildElement();
           child != NULL;
           child = child->NextSiblingElement()) {
        const char* component = child->Attribute("component");

        if(strcmp(child->Value(), "FlawFilter") != 0){
          // If no component name is provided, register it with the tag name of configuration element
          // thus obtaining the default.
          if(component == NULL)
            child->SetAttribute("component", child->Value());

          // Now allocate the particular flaw manager using an abstract factory pattern.
          EngineId& engine = db->getEngine();
          ComponentFactoryMgr* cfm = (ComponentFactoryMgr*)engine->getComponent("ComponentFactoryMgr");
          FlawManagerId flawManager = cfm->createInstance(*child);
          debugMsg("Solver:Solver", "Created FlawManager with id " << flawManager);
          flawManager->initialize(*child, m_db, m_context, m_masterFlawFilter.getId());
          m_flawManagers.push_back(flawManager);
        }
      }
    }

    Solver::~Solver(){
      cleanupDecisions();
      EUROPA::cleanup(m_flawManagers);
      delete (Context*) m_context;
      m_id.remove();
    }

    void Solver::addListener(const SearchListenerId& sl) {
      m_listeners.push_back(sl);
    }

    void Solver::removeListener(const SearchListenerId& sl) {
      for(std::list<SOLVERS::SearchListenerId>::iterator it = m_listeners.begin();
          it != m_listeners.end(); ++it)
        if(*it == sl) {
          m_listeners.erase(it);
          return;
        }
    }

    bool Solver::solve(unsigned int maxSteps, unsigned int maxDepth){
      // Initialize the step count floor with the prior step count so we can apply limits
      m_stepCountFloor = getStepCount();
      m_depthFloor = getDepth();
      m_maxSteps = maxSteps;
      m_maxDepth = maxDepth;

      // Reset the flaw found flag for a new evaluation
      m_noFlawsFound = false;
      m_timedOut = false;

      while(!m_timedOut && !m_exhausted && !m_noFlawsFound) step();

      checkError(!m_exhausted || m_decisionStack.empty(),
                 "If we have exhausted all our options to recover, then we must have no further decision available." <<
                 " Stack size is " << m_decisionStack.size());

      debugMsg("Solver:solve", "Finished with " << m_stepCount << " steps and depth of " << m_decisionStack.size());

      return m_noFlawsFound;
    }

    const SolverId& Solver::getId() const{ return m_id;}

    const LabelStr& Solver::getName() const { return m_name;}

    unsigned int Solver::getDepth() const {return m_decisionStack.size();}

    unsigned int Solver::getStepCount() const {return m_stepCount;}

    std::string Solver::getLastExecutedDecision() const {return m_lastExecutedDecision;}

    bool Solver::noMoreFlaws() const{return m_noFlawsFound;}

    const DecisionStack& Solver::getDecisionStack() const {return m_decisionStack;}

    std::string Solver::getDecisionStackAsString() const
    {
    	std::ostringstream os;

    	for (unsigned int i=0; i<m_decisionStack.size(); i++) {
    		if (i>0)
    		    os << ",";
    	    os << m_decisionStack[i]->toShortString();
    	}

    	return os.str();
    }

    void Solver::setMaxSteps(const unsigned int steps) {m_maxSteps = steps;}

    void Solver::setMaxDepth(const unsigned int depth) {m_maxDepth = depth;}

    /**
     * @brief Provides baseline implementation for chosing the next flaw and allocating the next decision.
     *
     * This implementation has the following key points:
     * @li It will iterate over the flaw managers in the order they are defined in order to formulate the
     * next decision point.
     * @li It uses a priority of 0 to indicate a dead-end i.e. a flaw that cannot be resolved.
     * @li It uses a priority of ZERO_COMMITMENT_SCORE to indicate that a priority of this value or less is a zero commitment
     * score which means the flaw can be decided without making any unncessary decisions. As long as the cost of finding these
     * is not aggregious, we always want to search for them first, and we know we can quite when we find them.
     * @li It uses a priority of WORST_SCORE() as the worst possible priority
     * @li It will seek out the zero commitment flaws first.
     * @li If no zero commitment flaws are found, it will prefer flaws from flaw managers in the order in which they are
     * created, all else being equal.
     * @li It will seek flaws of best (i.e. lowest) priority.
     *
     * @note The 2 pass approach for finding the next decision will result in the worst case of 2 full passes over
     * the set of flaw candidates for each flaw manager. This can be costly in terms of the evaluation cost of dynamic
     * scoping rules and the evaluation of choices to obtain a priority. There would be scope to look at cached data across calls.
     * @see FlawManager, DecisionPoint
     */
    void Solver::allocateNewDecisionPoint(){
      checkError(!m_exhausted, "Cannot allocate a search node when retracting.");
      checkError(m_activeDecision.isNoId(), "There can be no active decision.");

      // First try for a zero commitment decision
      m_activeDecision = getZeroCommitmentDecision();

      // If we don't get a hit up front, search for best alternative
      if(m_activeDecision.isNoId()){
        Priority priority = getWorstCasePriority() + 1;
        // Initialize heuristic score to worst possible.
        for(FlawManagers::const_iterator it = m_flawManagers.begin(); it != m_flawManagers.end(); ++it){
          FlawManagerId fm = *it;
          // Give a lower bound for the score that exceeds that of Zero commitment to permit an early termination if
          // we arrive at such a score.
          DecisionPointId candidate = fm->next(priority);

          // If we get a decision back, it trumps our current best decisions so update the bestDecision
          if(!candidate.isNoId()){
            if(!m_activeDecision.isNoId())
              m_activeDecision->discard();

            m_activeDecision = candidate;
          }
        }
      }

      // If we have an active decision, initialize it. We do this at the end to avoid cost of populating choices
      // until we are sure we will be keeping the decision.
      if(m_activeDecision.isId()) {
        publish(notifyCreated,m_activeDecision);
        m_activeDecision->initialize();
      }
    }

    /**
     * @brief Uses same basic interaction with the flaw managers but assumes a very aggressive cut-off
     * to permit early termination in seeking out the flaws we want.
     */
    DecisionPointId Solver::getZeroCommitmentDecision(){
      checkError(!m_exhausted, "Cannot allocate a search node when retracting.");

      for(FlawManagers::const_iterator it = m_flawManagers.begin();
          it != m_flawManagers.end(); ++it){
        FlawManagerId fm = *it;
        DecisionPointId decision = fm->nextZeroCommitmentDecision();

        // If we get a decision back, return
        if(decision.isId())
          return decision;
      }

      return DecisionPointId::noId();
    }

    bool Solver::isExhausted() const {
      checkError(!m_exhausted || m_decisionStack.empty(),
                 "Cannot be left in an exhausted state if there are still decisions to evaluate.");
      return m_exhausted;
    }

    bool Solver::isTimedOut() const {
      return m_timedOut;
    }

    void Solver::step(){
        ConstraintEngineId ce = m_db->getConstraintEngine();
        bool autoPropagation = ce->getAutoPropagation();
        ce->setAutoPropagation(false);
        doStep();
        ce->setAutoPropagation(autoPropagation);
    }

    bool Solver::conflictLevelOk()
    {
        return m_db->getConstraintEngine()->getViolation() == m_baseConflictLevel;
    }

    /**
     * @brief Handles a single step in the search
     */
    void Solver::doStep(){
      static unsigned int sl_counter(0);
      sl_counter++;

      checkError(!m_exhausted, "Cannot be exhausted when about to commence a step." << sl_counter);

      m_baseConflictLevel = m_db->getConstraintEngine()->getViolation();
      m_db->getClient()->propagate();

      // If inconsistent, before doing anything, there is no solution.
      if(!conflictLevelOk()){
        m_exhausted = true;
        debugMsg("Solver:step", "No solution prior to stepping. Conflict before propagation: " << m_baseConflictLevel << ", after propagation:" << m_db->getConstraintEngine()->getViolation());
        publish(notifyExhausted,);
        return;
      }

      debugMsg("Solver:step",
               "OpenDecisions at [stepCnt=" << getStepCount() << ",depth=" << getDepth() << "]" << std::endl << printOpenDecisions());

      debugMsg("Solver:decisionStack",
               "DecisionStack at [stepCnt=" << getStepCount() << ",depth=" << getDepth() << "] " << getDecisionStackAsString());

      // Reset flag for flaws found
      m_noFlawsFound = false;

      // If we have no active decision to work on, we get one
      if(m_activeDecision.isNoId())
        allocateNewDecisionPoint();

      if(m_activeDecision.isNoId()){
        m_noFlawsFound = true;
        publish(notifyCompleted,);
        return;
      }

      if(m_maxSteps <= getStepCount() - m_stepCountFloor || m_maxDepth < getDepth() - m_depthFloor){
        debugMsg("Solver:step", "Timeout!  Max steps: " << m_maxSteps << " step (above floor) " << getStepCount() - m_stepCountFloor <<
                 " Max depth: " << m_maxDepth << " depth (above floor) " << getDepth() - m_depthFloor);

        publish(notifyTimedOut,);
        m_timedOut = true;
        return;
      }

      condDebugMsg(m_stepCount % 50 == 0, "Solver:heartbeat", std::endl << printOpenDecisions());

      if(!m_activeDecision->cut() && m_activeDecision->hasNext()){
        m_lastExecutedDecision = m_activeDecision->toString();
        m_activeDecision->execute();
	    m_db->getClient()->propagate();
        m_stepCount++;

        if(conflictLevelOk()){
          m_decisionStack.push_back(m_activeDecision);
          publish(notifyStepSucceeded,m_activeDecision);
          m_activeDecision = DecisionPointId::noId();
          debugMsg("Solver:printPlan:infrequent", std::endl << PlanDatabaseWriter::toString(m_db));
          return;
        }
        else {
          debugMsg("Solver:backtrack",
                   "Backtracking because of constraint inconsistency due to " << m_lastExecutedDecision);
        }
      }
      else {
        debugMsg("Solver:backtrack", "Backtracking because " << m_lastExecutedDecision << " has no further choices.");
      }

      // If we get here then we must have to backtrack. so do it!
      m_exhausted = backtrack();

      // If still left in a backtrack state, the deicion stack must be exhausted
      if(m_exhausted) {
        checkError(m_decisionStack.empty(), "Must be exhausted if we failed to backtrack out.");
        debugMsg("Solver:step", "Solver exhausted at step " << getStepCount());
        publish(notifyExhausted,);
      }
    }

    /**
     * @brief Will undo decisions for as long as necessary and as long as possible until
     * we arrive at a point from which we can resume.
     */
    bool Solver::backtrack(){
      debugMsg("Solver:backtrack", "Starting. Depth is:" << m_decisionStack.size());

      bool backtracking = true;

      while(backtracking && (m_activeDecision.isId() || !m_decisionStack.empty())){
        // If we have no active decision, source it from the decision stack
        if(m_activeDecision.isNoId() && !m_decisionStack.empty()){
          m_activeDecision = m_decisionStack.back();
          m_decisionStack.pop_back();
          debugMsg("Solver:backtrack", "Retrieving closed decision. Depth is:" << m_decisionStack.size());
        }

	// This debug message uses a toString call which requires the database to be propagated. Normally we do not propagate on relaxations as there may be many if
	// we pop the stack. To allow the debug, we will propgate inline. It should effect performance but not correctness, unless things are not working
	// as expected !
        debugMsg("Solver:backtrack", "Backtracking decision " << (m_db->getClient()->propagate() ? m_activeDecision->toString() : "No data"));

        // If the active decision is executed, undo it
        if(m_activeDecision->isExecuted()) {
          m_activeDecision->undo();
          publish(notifyUndone,m_activeDecision);
          //debugMsg("Solver:printPlan", std::endl << PlanDatabaseWriter::toString(m_db));
        }

        // If there are available choices to take, we can quit and resume normal search
        backtracking = m_activeDecision->cut() || !m_activeDecision->hasNext();

        // If still retracting, we must discard the active decision
        if(backtracking){
          publish(notifyRetractNotDone,m_activeDecision);
          publish(notifyDeleted,m_activeDecision);
          m_activeDecision->discard();
          m_activeDecision = DecisionPointId::noId();
        }
        else {
          publish(notifyRetractSucceeded,m_activeDecision);
        }
      }
      return backtracking;
    }

    void Solver::reset(){
      reset(m_decisionStack.size());
    }

    void Solver::reset(unsigned int depth){
      checkError(depth <= getDepth(), "Cannot reset past current depth: " << depth << " exceeds " << getDepth());

      if(m_activeDecision.isId()){
        if(m_activeDecision->canUndo()) {
          publish(notifyUndone,m_activeDecision);
          m_activeDecision->undo();
        }

        m_activeDecision->discard();
        m_activeDecision = DecisionPointId::noId();
      }

      while(depth > 0 && !m_decisionStack.empty()){
        DecisionPointId node = m_decisionStack.back();

        // We assume that the stack can be rolled back such that all stored decisions are
        // still valid ids and contain valid data. We also require
        // that the underlying flaw be executed. This seens reasonable but would have to be
        // relaxed in a scenario of non-chronological relaxations
        // on the database. Unde such circumstances, it would be best if a DecisionPoint had
        // the means to detect if the underlying data were still
        // retractable. This seems quite practical as long as we store the key within a
        // decision point of the underlying flaw.
        checkError(node.isValid(),
                   "Must have deleted the decision elsewhere." <<
                   " A bug in the Solver or FlawManager. A current assumption since we do not synchronize the stack.");

        m_decisionStack.pop_back();

        if(node->canUndo()) {
          publish(notifyUndone,node);
          node->undo();
        }

        publish(notifyDeleted,node);
        node->discard();
        depth--;
      }

      m_stepCount = 0;
      m_noFlawsFound = false;
      m_exhausted = false;
      m_timedOut = false;
    }

    bool Solver::backjump(unsigned int stepCount){
      checkError(stepCount > 0, "Should not be allowed to backjump 0 steps.");

      // If we have an active decision, then reset it
      if(m_activeDecision.isId()){
        if(m_activeDecision->canUndo()) {
          publish(notifyUndone,m_activeDecision);
          m_activeDecision->undo();
        }

        m_activeDecision->discard();
        m_activeDecision = DecisionPointId::noId();
        stepCount--;
      }

      // First reset all but the last step we want to get to
      checkError(getDepth() >= stepCount,
                 "Cannot retract " << stepCount << " steps. Current depth only " << getDepth());

      if(stepCount > 0)
        reset(stepCount-1);

      // Now backtrack the last choice
      m_exhausted = backtrack();
      m_stepCount = 0;
      m_noFlawsFound = false;
      m_timedOut = false;

      return m_exhausted;
    }

    void Solver::clear(){
      m_stepCount = 0;
      m_stepCountFloor = 0;
      m_depthFloor = 0;
      m_noFlawsFound = false;
      m_exhausted = false;
      m_timedOut = false;

      cleanupDecisions();
    }

    void Solver::cleanupDecisions(){
      if(m_activeDecision.isId()){
        m_activeDecision->discard();
        m_activeDecision = DecisionPointId::noId();
      }

      discardAll(m_decisionStack);
    }

    void Solver::cleanup(DecisionStack& decisionStack){
      for(DecisionStack::const_iterator it = decisionStack.begin(); it != decisionStack.end(); ++it){
        DecisionPointId node = *it;
        node->discard();
      }
      decisionStack.clear();
    }

    IteratorId Solver::createIterator() {
      return (new FlawIterator(m_flawManagers))->getId();
    }

    Solver::FlawIterator::FlawIterator(const FlawManagers& flawManagers) {
      for(FlawManagers::const_iterator it = flawManagers.begin(); it != flawManagers.end(); ++it)
        m_iterators.push_back((*it)->createIterator());
      m_it = m_iterators.begin();

      // move it to the first real one, so that done() will allways work consistently
      while (m_it != m_iterators.end()) {
          IteratorId it = (*m_it);
          if (it->done())
      	      ++m_it;
      	  else
      	      break;
      }
    }

    Solver::FlawIterator::~FlawIterator() {
      for(m_it = m_iterators.begin(); m_it != m_iterators.end(); ++m_it)
        delete (Iterator*) (*m_it);
    }

    bool Solver::FlawIterator::done() const
    {
    	return m_it == m_iterators.end();
    }

    const EntityId Solver::FlawIterator::next() {
      if(done())
        return EntityId::noId();

      IteratorId it = (*m_it);
      //if current iterator is done, move to the next one
      //and return that iterator's next
      if(it->done()) {
        ++m_it;
        return next();
      }

      EntityId retval = EntityId::noId();
      //get the next entity
      retval = it->next();

      //it's possible that it's returning a noId because
      //it's nearly done, so the previous call should bump it
      //into doneness.  call next to bump the meta-iterator forward.
      if(it->done())
        next();

      return retval;
    }

#define notify(m) { \
  m_masterFlawFilter.m;				\
  for(FlawManagers::const_iterator it = m_flawManagers.begin(); it != m_flawManagers.end(); ++it) { \
    debugMsg("Solver:notify", "Sending notification to " << (*it)); \
    (*it)->m; \
  } \
}

    bool Solver::isDecided(const EntityId& entity) {
      for(DecisionStack::const_iterator it = m_decisionStack.begin(); it != m_decisionStack.end(); ++it) {
        if((*it)->getFlawedEntityKey() == static_cast<unsigned int>(entity->getKey()))
          return true;
      }
      return false;
    }

    bool Solver::hasDecidedParameter(const TokenId& token) {
      for(std::vector<ConstrainedVariableId>::const_iterator it = token->parameters().begin(); it != token->parameters().end(); ++it) {
        if(isDecided(*it))
          return true;
      }
      return false;
    }

    void Solver::notifyRemoved(const ConstrainedVariableId& variable){
      checkError(!isDecided(variable),"Attempt to remove decided variable "<< variable->toString());
      notify(notifyRemoved(variable));
    }

    void Solver::notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType){

      switch(changeType){
      case DomainListener::UPPER_BOUND_DECREASED:
      case DomainListener::LOWER_BOUND_INCREASED:
      case DomainListener::VALUE_REMOVED:
      case DomainListener::EMPTIED:
        return;
      default:
        notify(notifyChanged(variable, changeType));
      }
    }

    void Solver::notifyAdded(const ConstraintId& constraint){
      notify(notifyAdded(constraint));
    }

    void Solver::notifyRemoved(const ConstraintId& constraint){
      //notify(notifyRemoved(constraint));
      m_masterFlawFilter.notifyRemoved(constraint);
      for(FlawManagers::const_iterator it = m_flawManagers.begin(); it != m_flawManagers.end(); ++it) {
        (*it)->notifyRemoved(constraint);
      }
    }

    void Solver::notifyAdded(const TokenId& token) {
      notify(notifyAdded(token));
    }

    void Solver::notifyRemoved(const TokenId& token) {
      checkError(!isDecided(token),"Attempt to remove decided token "<< token->toString());
      checkError(!hasDecidedParameter(token),"Attempt to remove token with decided parameters "<< token->toString());
      notify(notifyRemoved(token));
    }

    bool Solver::inScope(const EntityId& entity){
      for(FlawManagers::const_iterator it = m_flawManagers.begin();
          it != m_flawManagers.end(); ++it){
        FlawManagerId fm = *it;
        if(fm->inScope(entity))
          return true;
      }

      return false;
    }

    FlawHandlerId Solver::getFlawHandler(const EntityId entity){
      for(FlawManagers::const_iterator it = m_flawManagers.begin();
          it != m_flawManagers.end(); ++it){
        FlawManagerId fm = *it;
        if(fm->inScope(entity))
          return fm->getFlawHandler(entity);
      }

      checkError(!inScope(entity), "Solver configuration error. There is no flaw handler even though it is in scope.");

      return FlawHandlerId::noId();
    }

    bool Solver::isConstraintConsistent() const {
      return m_db->getConstraintEngine()->constraintConsistent();
    }

    std::multimap<Priority, std::string> Solver::getOpenDecisions() const
    {
      checkError(m_db->getConstraintEngine()->constraintConsistent(),
                 "Can only call this if variable changes have been propagated first.");

      std::multimap<Priority, std::string> priorityQueue; // Used to make output in priority order
      unsigned int multiplier = 1;
      for(FlawManagers::const_iterator it = m_flawManagers.begin();
          it != m_flawManagers.end(); ++it){
        FlawManagerId fm = *it;
        IteratorId flawIterator = fm->createIterator();
        double adjustment = EPSILON * multiplier++;
        while(!flawIterator->done()){
          std::stringstream os;
          EntityId flaw = flawIterator->next();
          const Priority priority = fm->getPriority(flaw);
          priorityQueue.insert(std::pair<Priority, std::string>(priority + adjustment, fm->toString(flaw)));
        }

        delete (Iterator*) flawIterator;
      }

      return priorityQueue;
    }

    /**
     * @brief Will print the open decisions in priority order
     */
    std::string Solver::printOpenDecisions() const {
      static const std::string sl_indentation("   ");

      std::stringstream os;
      os << " OpenDecisions:{ ";

      std::multimap<Priority, std::string> priorityQueue = getOpenDecisions();
      for(std::multimap<Priority, std::string>::const_iterator it=priorityQueue.begin();it!=priorityQueue.end(); ++it)
        os << std::endl << sl_indentation << it->second << " PRIORITY==" << it->first;

      os << std::endl << " }";

      return os.str();
    }

  bool Solver::isValid() const {
    // validate constraints.
    debugMsg("Solver:isValid","Entering Solver.isValid");
    ConstraintSet l_constraints = m_db->getConstraintEngine()->getConstraints();
    for(std::set<ConstraintId>::const_iterator it = l_constraints.begin(); it != l_constraints.end(); ++it) {
      // all guards on flaws must be present in a FlawManager
      if( Id<FlawHandler::VariableListener>::convertable(*it)) {
        bool l_present = false;
        for(FlawManagers::const_iterator fmit = m_flawManagers.begin(); fmit != m_flawManagers.end(); ++fmit) {
					std::multimap<unsigned int, ConstraintId> fhg = (*fmit)->getFlawHandlerGuards();
          for(std::multimap<unsigned int, ConstraintId>::const_iterator fhgit = fhg.begin();
              fhgit != fhg.end(); ++fhgit) {
            if(fhgit->second == (*it)) {
              l_present = true;
              break;
            }
          }
          if(l_present)
            break;
        }
        condDebugMsg(!l_present,"Solver:isValid",(*it)->toString() << " was not found in a FlawManager.");
				if(!l_present)
        	return false;
      }
    }
    debugMsg("Solver:isValid","Returning true");
    return true;
  }

    Solver::CeListener::CeListener(const ConstraintEngineId& ce, Solver& solver)
      : ConstraintEngineListener(ce), m_solver(solver) {}

    void Solver::CeListener::notifyRemoved(const ConstrainedVariableId& variable){
      m_solver.notifyRemoved(variable);
    }

    void Solver::CeListener::notifyChanged(const ConstrainedVariableId& variable,
                                           const DomainListener::ChangeType& changeType){
      m_solver.notifyChanged(variable, changeType);
    }

    void Solver::CeListener::notifyAdded(const ConstraintId& constraint){
      m_solver.notifyAdded(constraint);
    }

    void Solver::CeListener::notifyRemoved(const ConstraintId& constraint){
      m_solver.notifyRemoved(constraint);
    }

    Solver::DbListener::DbListener(const PlanDatabaseId& db, Solver& solver)
      : PlanDatabaseListener(db), m_solver(solver) {}

    void Solver::DbListener::notifyRemoved(const TokenId& token) {
      m_solver.notifyRemoved(token);
    }

    void Solver::DbListener::notifyAdded(const TokenId& token) {
      m_solver.notifyAdded(token);
    }
  }
}
