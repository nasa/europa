#include "FlawManager.hh"
#include "SolverUtils.hh"
#include "Utils.hh"
#include "FlawFilter.hh"
#include "FlawHandler.hh"
#include "Token.hh"
#include "Debug.hh"
#include "SAVH_Instant.hh"

/**
 * @file FlawManager.cc
 * @author Conor McGann
 * @date April, 2005
 * @brief Provides implementation for FlawManager
 */

namespace EUROPA {
  namespace SOLVERS {

    FlawManager::FlawManager(const TiXmlElement& configData)
      : Component(configData), 
        m_flawFilters(configData, "FlawFilter"), 
        m_flawHandlers(configData, "FlawHandler"),
        m_timestamp(0){
    }

    FlawManager::~FlawManager(){

    }

    bool FlawManager::isValid() const {
      for(std::map<unsigned int, bool>::const_iterator it = m_staticFiltersByKey.begin(); it != m_staticFiltersByKey.end(); ++it) {
        unsigned int key = it->first;
        EntityId entity = Entity::getEntity(key);
        condDebugMsg(!entity.isValid(), "FlawManager:isValid", getId() << " Invalid id in m_staticFiltersByKey. Entity key: " << it->first);        
      }
      for(__gnu_cxx::hash_map<unsigned int, std::vector<FlawFilterId> >::const_iterator it = m_dynamicFiltersByKey.begin();
          it != m_dynamicFiltersByKey.end(); ++it) {
        EntityId entity = Entity::getEntity(it->first);
        condDebugMsg(!entity.isValid(), "FlawManager:isValid", getId() << " Invalid id in m_dynamicFiltersByKey. Entity key: " << it->first);
        const std::vector<FlawFilterId>& filters(it->second);
        for(std::vector<FlawFilterId>::const_iterator subIt = filters.begin(); subIt != filters.end(); ++subIt) {
          condDebugMsg(!(*subIt).isValid(), "FlawManager:isValid", getId() << " Invalid flaw filter id for entity " << it->first <<
                       " in m_dynamicFiltersByKey");
        }
      }
      for(std::multimap<unsigned int, ConstraintId>::const_iterator it = m_flawHandlerGuards.begin(); it != m_flawHandlerGuards.end();
          ++it) {
        EntityId entity = Entity::getEntity(it->first);
        condDebugMsg(!entity.isValid(), "FlawManager:isValid", getId() << " Invalid id in m_flawHandlerGuards.  Entity key:" << it->first);
        condDebugMsg(!it->second.isValid(), "FlawManager:isValid", getId() << " Invalid constraint id in m_flawHandlerGuards.  Entity key: " << it->first);
        FlawHandler::VariableListener* listener = (FlawHandler::VariableListener*) (it->second);
        condDebugMsg(!listener->getTarget().isValid(), "FlawManager:isValid", getId() << " Invalid target id in m_flawHandlerGuards.  Entity key: " << it->first);
        if(m_activeFlawHandlersByKey.find(it->first) == m_activeFlawHandlersByKey.end()) {
          debugMsg("FlawManager:isValid", getId() << "Target '" << listener->getTarget()->toString() << "' has no active flaw handlers.");
          std::stringstream scope;
          for(std::vector<ConstrainedVariableId>::const_iterator scopeIt = listener->getScope().begin(); scopeIt != listener->getScope().end(); 
              ++scopeIt)
            scope << (*scopeIt)->toString() << " ";
          debugMsg("FlawManager:isValid", "Variable listener scope: " << scope.str());
        }
      }
      debugMsg("FlawManager:isValid", "If zero, it's crazy time: " <<m_activeFlawHandlersByKey.size());
      for(std::map<unsigned int, FlawHandlerEntry>::const_iterator it = m_activeFlawHandlersByKey.begin(); it != m_activeFlawHandlersByKey.end();
          ++it) {
        EntityId entity = Entity::getEntity(it->first);
        condDebugMsg(!entity.isValid(), "FlawManager:isValid", getId() << " Invalid id in m_activeFlawHandlersByKey.  Entity key: " << it->first);
        const FlawHandlerEntry& entry(it->second);
        for(FlawHandlerEntry::const_iterator it = entry.begin(); it != entry.end(); ++it)
          condDebugMsg(!it->second.isValid(), "FlawManager:isValid", getId() << " Invalid flaw handler id in m_activeFlawHandlersByKey.  Entity key: " << it->first);
      }
      return true;
    }

    void FlawManager::initialize(const PlanDatabaseId& db, const ContextId& ctx, const FlawManagerId& parent){
      checkError(m_db.isNoId(), "Can only be initialized once.");
      m_db = db;
      m_parent = parent;
      m_context = ctx;
      for(std::set<MatchingRuleId>::iterator it = m_flawFilters.getRules().begin(); it != m_flawFilters.getRules().end(); ++it) {
        MatchingRuleId rule = *it;
        check_error(rule.isValid());
        rule->setContext(m_context);
      }
      for(std::set<MatchingRuleId>::iterator it = m_flawHandlers.getRules().begin(); it != m_flawHandlers.getRules().end(); ++it) {
        MatchingRuleId rule = *it;
        check_error(rule.isValid());
        rule->setContext(m_context);
      }
      handleInitialize();
      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
    }

    IteratorId FlawManager::createIterator(){ return IteratorId::noId();}

    void FlawManager::notifyRemoved(const ConstraintId& constraint) {
     /*
      if(Id<FlawHandler::VariableListener>::convertable(constraint)) {
        debugMsg("FlawManager:notifyRemoved", getId() << " Removing " << constraint->toString() << " from m_flawHandlerGuards");

        Id<FlawHandler::VariableListener> listener = constraint;
        int targetKey = listener->getTarget()->getKey();
        double weight = listener->getHandler()->getWeight();
        m_flawHandlerGuards.erase(targetKey);

        std::map<unsigned int, FlawHandlerEntry>::iterator activeFlawHandlerEntry = m_activeFlawHandlersByKey.find(targetKey);
        if(activeFlawHandlerEntry == m_activeFlawHandlersByKey.end()) {
          debugMsg("FlawManager:notifyRemoved", "Badges, we don't need no stinking badges.");
          return;
        }
        FlawHandlerEntry& entry = m_activeFlawHandlersByKey.find(targetKey)->second;
        FlawHandlerEntry::iterator it = entry.find(weight);
        while(it != entry.end() && it->first == weight) {
          if(it->second == listener->getHandler()) {
            debugMsg("FlawManager:notifyRemoved", getId() << " Removing " << it->second->toString() << " from flaw handler entry for " << targetKey);
            entry.erase(it);
            break;
          }
          ++it;
        }
        //check_error(it != entry.end());
        //check_error(it->second == listener->getHandler());
      }
      */
    }

    /**
     * Remove flaw handler guard constraints for this variable, and if it is a state
     * variable also reomve for the token
     */
    void FlawManager::notifyRemoved(const ConstrainedVariableId& var){
      debugMsg("FlawManager:notifyRemoved", getId() << " Removing active flaw handlers and guards for " << var->toString());
      // TODO: recomment when this breaks something
      // ERROR: The error is here
      for(std::multimap<unsigned int, ConstraintId>::iterator it = m_flawHandlerGuards.find(var->getKey());
          it != m_flawHandlerGuards.end() && it->first == (unsigned int) var->getKey(); ++it) {
        it->second->discard();
      }
      unsigned int mapsize = m_flawHandlerGuards.size();
      m_flawHandlerGuards.erase(var->getKey());
      debugMsg("FlawManager:notifyRemoved", getId() << " removed " << var->getKey() << " from m_flawHandlerGuards (" <<
               mapsize << ", " << m_flawHandlerGuards.size() << ")");

      mapsize = m_activeFlawHandlersByKey.size();
      m_activeFlawHandlersByKey.erase(var->getKey());
      debugMsg("FlawManager:notifyRemoved", getId() << " removed " << var->getKey() << " from m_activeFlawHandlersByKey (" <<
               mapsize << ", " << m_activeFlawHandlersByKey.size() << ")");

      mapsize = m_staticFiltersByKey.size();
      m_staticFiltersByKey.erase(var->getKey());
      debugMsg("FlawManager:notifyRemoved", getId() << " removed " << var->getKey() << " from m_staticFiltersByKey (" <<
               mapsize << ", " << m_staticFiltersByKey.size() << ")");

      mapsize = m_dynamicFiltersByKey.size();
      m_dynamicFiltersByKey.erase(var->getKey());
      debugMsg("FlawManager:notifyRemoved", getId() << " removed " << var->getKey() << " from m_dynamicFiltersByKey (" <<
               mapsize << ", " << m_dynamicFiltersByKey.size() << ")");

      for(std::multimap<unsigned int, ConstraintId>::iterator it = m_flawHandlerGuards.begin(); it != m_flawHandlerGuards.end(); ++it) {
        if(std::find(it->second->getScope().begin(),it->second->getScope().end(),var) != it->second->getScope().end()) {

          Id<FlawHandler::VariableListener> listener = it->second;
          int targetKey = listener->getTarget()->getKey();
          double weight = listener->getHandler()->getWeight();

          std::map<unsigned int, FlawHandlerEntry>::iterator activeFlawHandlerEntry = m_activeFlawHandlersByKey.find(targetKey);
          if(activeFlawHandlerEntry != m_activeFlawHandlersByKey.end()) {
            FlawHandlerEntry& entry = m_activeFlawHandlersByKey.find(targetKey)->second;
            FlawHandlerEntry::iterator eit = entry.find(weight);
            while(eit != entry.end() && eit->first == weight) {
              if(eit->second == listener->getHandler()) {
                debugMsg("FlawManager:notifyRemoved", getId() << " Removing " << eit->second->toString() << " from flaw handler entry for " << targetKey);
                entry.erase(eit);
                break;
              }
              ++eit;
            }
          }

          std::multimap<unsigned int, ConstraintId>::iterator temp = it;
          --it;
          m_flawHandlerGuards.erase(temp);
        }
      }

      if(Token::isStateVariable(var)){
        debugMsg("FlawManager:notifyRemoved", 
                 getId() << " Variable " << var->toString() << " is a state variable.  Removing flaw handlers and guards for " <<
                 ((Token*)var->getParent())->getPredicateName().toString());
        // TODO: recomment when this breaks something
        for(std::multimap<unsigned int, ConstraintId>::iterator it = m_flawHandlerGuards.find(var->getParent()->getKey());
            it != m_flawHandlerGuards.end() && it->first == (unsigned int)var->getParent()->getKey(); ++it) {
          it->second->discard();
        }
        m_flawHandlerGuards.erase(var->getParent()->getKey());
        m_activeFlawHandlersByKey.erase(var->getParent()->getKey());
        m_staticFiltersByKey.erase(var->getParent()->getKey());
        m_dynamicFiltersByKey.erase(var->getParent()->getKey());
        check_error(m_flawHandlerGuards.find(var->getParent()->getKey()) == m_flawHandlerGuards.end());
        check_error(m_activeFlawHandlersByKey.find(var->getParent()->getKey()) == m_activeFlawHandlersByKey.end());
        check_error(m_staticFiltersByKey.find(var->getParent()->getKey()) == m_staticFiltersByKey.end());
        check_error(m_dynamicFiltersByKey.find(var->getParent()->getKey()) == m_dynamicFiltersByKey.end());
      }
      check_error(m_flawHandlerGuards.find(var->getKey()) == m_flawHandlerGuards.end());
      check_error(m_activeFlawHandlersByKey.find(var->getKey()) == m_activeFlawHandlersByKey.end());
      check_error(m_staticFiltersByKey.find(var->getKey()) == m_staticFiltersByKey.end());
      check_error(m_dynamicFiltersByKey.find(var->getKey()) == m_dynamicFiltersByKey.end());
      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
    }

    void FlawManager::notifyRemoved(const TokenId& token) {
      debugMsg("FlawManager:notifyRemoved", getId() << " Removing active flaw handlers and guards for " << token->getPredicateName().toString() <<
               "(" << token->getKey() << ")");
      // TODO: recomment when this breaks something
      for(std::multimap<unsigned int, ConstraintId>::iterator it = m_flawHandlerGuards.find(token->getKey());
          it != m_flawHandlerGuards.end() && it->first == (unsigned int) token->getKey(); ++it)
        it->second->discard();
      m_flawHandlerGuards.erase(token->getKey());
      m_activeFlawHandlersByKey.erase(token->getKey());
      m_staticFiltersByKey.erase(token->getKey());
      m_dynamicFiltersByKey.erase(token->getKey());
      check_error(m_flawHandlerGuards.find(token->getKey()) == m_flawHandlerGuards.end());
      check_error(m_activeFlawHandlersByKey.find(token->getKey()) == m_activeFlawHandlersByKey.end());
      check_error(m_staticFiltersByKey.find(token->getKey()) == m_staticFiltersByKey.end());
      check_error(m_dynamicFiltersByKey.find(token->getKey()) == m_dynamicFiltersByKey.end());

      condDebugMsg(!isValid(), "FlawManager:isValid", getId() << " Invalid datastructures in flaw manager.");
    }

    DecisionPointId FlawManager::nextZeroCommitmentDecision(){
      return DecisionPointId::noId();
    }

    DecisionPointId FlawManager::next(Priority& bestPriority){
      EntityId flawToResolve;

      // Cannot do better if we already have a best case priority
      if(bestPriority == getBestCasePriority())
        return DecisionPointId::noId();

      // Initialize the prority to beat
      Priority bestP =  bestPriority - (2 * EPSILON);
      IteratorId it = createIterator();

      LabelStr explanation = "unknown";
      // Now go through the candidates
      while(!it->done()){

        // Get the next flaw candidate
        const EntityId candidate = it->next();
        checkError(candidate.isValid(), "Iterator bug returning a noId");
        checkError(!dynamicMatch(candidate), "Iterator bug allowing " << candidate->toString());

        debugMsg("FlawManager:next", "Evaluating " << candidate->toString() << " to beat " << bestP);

        Priority priority = getPriority(candidate);

        // >= +EPSILON if priority < bestP
        // <= -EPSILON if priority > bestP
        // > -EPSILON && < EPSILON if priority == bestP
        Priority priorityDiff = bestP - priority; 

        debugMsg("FlawManager:next", "Got priority " << priority);
        // If we have a better candidate

        //is this a bug?
        if(priorityDiff >= EPSILON){
          debugMsg("FlawManager:next", "Updating because priority " << priority << 
                   " is better than old best (" << bestP << ")");          
          flawToResolve = candidate;
          bestP = priority;
          explanation = "priority";
          debugMsg("FlawManager:next", "Updating flaw to resolve " << candidate->toString());          
          if(bestP == getBestCasePriority())
            break;
        }
        else if((fabs(priorityDiff) < EPSILON && betterThan(candidate, flawToResolve, explanation))){
          debugMsg("FlawManager:next",
                   "Updating because candidate is judged better than old candidate.");
          flawToResolve = candidate;
          bestP = priority;
          //explanation = "preference";
          debugMsg("FlawManager:next", "Updating flaw to resolve " << candidate->toString());          
          if(bestP == getBestCasePriority())
            break;
        }
        
//         condDebugMsg(priorityDiff <= -EPSILON || fabs(priorityDiff) < EPSILON, "FlawManager:next", "Priority for " << candidate->toString() << " (" << 
//                      priority << ") is not better than the current best (" << bestP << ")");
//         condDebugMsg(fabs(priorityDiff) < EPSILON && !betterThan(candidate, flawToResolve), "FlawManager:next", "Candidate " <<
//                      candidate->toString() << " isn't better than the current best flaw (" << 
//                      flawToResolve->toString() << ")");
      }

      delete (Iterator*) it;

      DecisionPointId decision;
      if(flawToResolve.isId()){
        bestPriority = bestP;
        decision = allocateDecisionPoint(flawToResolve, explanation);
      }

      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      return decision;
    }

    bool FlawManager::inScope(const EntityId& entity) {
      checkError(m_db->getConstraintEngine()->constraintConsistent(), 
                 "Assumes the database is constraint consistent but it is not.");
      IteratorId it = createIterator();
      bool result = false;
      while(!it->done()){
        if(it->next() == entity){
          result = true;
          break;
        }
      }

      delete (Iterator*) it;
      return result;
    }

    bool FlawManager::matches(const EntityId& entity){
      return staticMatch(entity) || dynamicMatch(entity);
    }

    bool FlawManager::staticMatch(const EntityId& entity) {
      if(m_parent.isId() && m_parent->staticMatch(entity)) {
        debugMsg("FlawManager:staticMatch", "Excluding " << entity->getKey() << " because parent is excluded.");
        return true;
      }

      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      /// If found in the static filters, then return the value
      std::map<unsigned int, bool>::const_iterator it = m_staticFiltersByKey.find(entity->getKey());
      if(it != m_staticFiltersByKey.end()) {
        debugMsg("FlawManager:staticMatch", 
                 (it->second ? "Excluding " : "Including ") << entity->getKey() << " because it was previously " <<
                 (it->second ? "excluded." : "included."));
        return it->second;
      }

      // Load filters
      std::vector<MatchingRuleId> filters;
      if(TokenId::convertable(entity))
        m_flawFilters.getTokenMatches(entity, filters);
      else
        m_flawFilters.getVariableMatches(entity, filters);

      std::vector<FlawFilterId> dynamicFilters;
      for(std::vector<MatchingRuleId>::const_iterator it = filters.begin(); it!= filters.end(); ++it){
        FlawFilterId flawFilter = (FlawFilterId) *it;
        if(flawFilter->isDynamic())
          dynamicFilters.push_back(flawFilter);
        else { // Static so prune and quit
          debugMsg("FlawManager:staticMatch", getId() << " Sticking " << entity->getKey() << " into static filters.");
          m_staticFiltersByKey.insert(std::pair<unsigned int, bool>(entity->getKey(), true));
          debugMsg("FlawManager:staticMatch", 
                   "Excluding " << entity->getKey() << ".  Matched " << flawFilter->toString() << ".");
          return true;
        }
      }

      // If we get here, it is not statically filtered and it we can add the entries, if any, for dynamic filtering
      debugMsg("FlawManager:staticMatch", getId() << " Sticking " << entity->getKey() << " into static filters.");
      m_staticFiltersByKey.insert(std::pair<unsigned int, bool>(entity->getKey(), false));
      debugMsg("FlawManager:staticMatch", getId() << " Sticking " << entity->getKey() << " into dynamic filters.");
      m_dynamicFiltersByKey.insert(std::pair<unsigned int, std::vector<FlawFilterId> >(entity->getKey(), dynamicFilters));
      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      return false;
    }

    bool FlawManager::staticallyExcluded(const EntityId& entity) const {
      if(m_parent.isId() && m_parent->staticallyExcluded(entity)) {
        debugMsg("FlawManager:staticallyExcluded", "Excluding " << entity->getKey() << " because its parent was statically excluded.");
        return true;
      }

      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      std::map<unsigned int, bool>::const_iterator it = m_staticFiltersByKey.find(entity->getKey());
      checkError(it != m_staticFiltersByKey.end(), "Filters must not have been queried yet for " << entity->getKey());

      debugMsg("FlawManager:staticallyExcluded", (it->second ? "Excluding " : "Including ") << entity->getKey());
      return it->second;
    }

    bool FlawManager::dynamicMatch(const EntityId& entity) {
      checkError(!staticallyExcluded(entity), "Canot call dynamic match if statically excluded");
      if(m_parent.isId() && m_parent->dynamicMatch(entity))
        return true;

      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      __gnu_cxx::hash_map<unsigned int, std::vector<FlawFilterId> >::const_iterator it = 
        m_dynamicFiltersByKey.find(entity->getKey());

      if(it != m_dynamicFiltersByKey.end()){
        const std::vector<FlawFilterId>& dynamicFilters = it->second;
        for(std::vector<FlawFilterId>::const_iterator it_a = dynamicFilters.begin(); it_a != dynamicFilters.end(); ++it_a){
          FlawFilterId dynamicFilter = *it_a;
          checkError(dynamicFilter->isDynamic(), "Must be a bug in construction code.");
          if(dynamicFilter->test(entity)){
            debugMsg("FlawManager:dynamicMatch",
                     "Excluding " << entity->toString() << " with " << dynamicFilter->toString());
            condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
            return true;
          }
        }
      }
      return false;
    }

    Priority FlawManager::getPriority(const EntityId& entity){
      debugMsg("FlawManager:getPriority", "Getting priority for " << entity->getKey());
      FlawHandlerId flawHandler = getFlawHandler(entity);
      checkError(flawHandler.isValid(), "No flawHandler for " << entity->toString());
      debugMsg("FlawManager:getPriority", "Returning priority " << flawHandler->getPriority(entity));
      return flawHandler->getPriority(entity);
    }

    /**
     * @brief Now we conduct a simple match where we select based on first avalaible.
     */
    DecisionPointId FlawManager::allocateDecisionPoint(const EntityId& entity, const LabelStr& explanation){
      static unsigned int sl_counter(0); // Helpful for debugging
      sl_counter++;

      FlawHandlerId flawHandler = getFlawHandler(entity);
      checkError(flawHandler.isValid(), "On " << sl_counter << ": No flawHandler for " << entity->toString());
      return flawHandler->create(m_db->getClient(), entity, explanation);
    }

    FlawHandlerId FlawManager::getFlawHandler(const EntityId& entity){
      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      static unsigned int sl_counter(0); // Helpful for debugging
      sl_counter++;

      // First, try to find if there is one available already
      checkError(entity.isValid(), entity);
      checkError(m_db->getConstraintEngine()->constraintConsistent(), "Must be propagated and consistent.");

      std::map<unsigned int, FlawHandlerEntry >::const_iterator it = m_activeFlawHandlersByKey.find(entity->getKey());
      if(it != m_activeFlawHandlersByKey.end()){
        FlawHandlerEntry entry = it->second;
        FlawHandlerEntry::const_iterator entryIt = entry.end();
        FlawHandlerId flawHandler = (--entryIt)->second;

        debugMsg("FlawManager:getFlawHandler", "returning handler " << flawHandler->toString() <<
                 " with priority " << flawHandler->getPriority() << " for key " << entity->getKey());
        return flawHandler;
      }
      else { // Have to load heuristics
        debugMsg("FlawManager:getFlawHandler", "Loading heuristics.");
        std::vector<MatchingRuleId> candidates;
        if(TokenId::convertable(entity))
          m_flawHandlers.getTokenMatches(entity, candidates);
        else if(ConstrainedVariableId::convertable(entity))
          m_flawHandlers.getVariableMatches(entity, candidates);
        else if(SAVH::InstantId::convertable(entity))
          m_flawHandlers.getInstantMatches(entity, candidates);
        else {
          checkError(ALWAYS_FAIL, "No way to match entities of type " << entity->toString());
        }

        FlawHandlerEntry entry;
        bool requiresPropagation = false;
        for(std::vector<MatchingRuleId>::const_iterator it = candidates.begin(); it!= candidates.end(); ++it){
          FlawHandlerId candidate = *it;
          if(candidate->hasGuards()){
            std::vector<ConstrainedVariableId> guards;
            // Make the scope. If cant match up, then ignore this handler
            if(!candidate->makeConstraintScope(entity, guards) || !candidate->customStaticMatch(entity))
              continue;

            ConstraintId guardListener = (new FlawHandler::VariableListener(m_db->getConstraintEngine(),
                                                                            entity,
                                                                            getId(),
                                                                            candidate,
                                                                            guards))->getId();
            requiresPropagation = true;
            debugMsg("FlawManager:getFlawManager", getId() << " Sticking " << entity->getKey() <<
                     " into flaw handler guards (Guard listener: " << guardListener->getKey() <<").");
            m_flawHandlerGuards.insert(std::pair<unsigned int, ConstraintId>(entity->getKey(), guardListener));
            // If we are not yet ready to move on.
            if(!candidate->test(guards))
              continue;
          }    
          entry.insert(std::pair<double, FlawHandlerId>(candidate->getWeight(), candidate));
        }
        debugMsg("FlawManager:getFlawManager", "Found " << entry.size() << " possible heuristics.");
        checkError(!entry.empty(), "No Heuristics for " << entity->toString());

        debugMsg("FlawManager:getFlawManager", getId() << " Sticking " << entity->getKey() << " into active flaw handlers.");
        m_activeFlawHandlersByKey.insert(std::pair<unsigned int, FlawHandlerEntry>(entity->getKey(), entry));

        // Propagate if necessary to process any constraints that have been added. This is required
        // so that we get the most up to date flaw handler
        if(requiresPropagation){
          m_db->getConstraintEngine()->propagate();
          synchronize();
        }

        // Now call recursively, but should get a hit this time
        debugMsg("FlawManager:getFlawHandler", "Making recursive call.");
        condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
        return getFlawHandler(entity);
      }
    }

    void FlawManager::notifyActivated(const EntityId& target, const FlawHandlerId& flawHandler){
      check_error(target.isValid());
      check_error(!target->isDiscarded());
      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      std::map<unsigned int, FlawHandlerEntry>::iterator it = m_activeFlawHandlersByKey.find(target->getKey());
      checkError(it != m_activeFlawHandlersByKey.end(), 
                 "We should have at least one entry for a standard handler for entity " << target->getKey() << " handler " << flawHandler->toString());
      FlawHandlerEntry& entry = it->second;
      entry.insert(std::pair<double, FlawHandlerId>(flawHandler->getWeight(),flawHandler ));
      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
    }

    void FlawManager::notifyDeactivated(const EntityId& target, const FlawHandlerId& flawHandler){
      condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
      std::map<unsigned int, FlawHandlerEntry>::iterator it = m_activeFlawHandlersByKey.find(target->getKey());
      checkError(it != m_activeFlawHandlersByKey.end(), 
                 "We should have at least one entry for a standard handler for " << target->getKey() << " handler " << flawHandler->toString());

      FlawHandlerEntry& entry = it->second;

      for(FlawHandlerEntry::iterator handlerIt = entry.begin(); handlerIt != entry.end(); ++handlerIt){
        if(handlerIt->second == flawHandler){
          entry.erase(handlerIt);
          condDebugMsg(!isValid(), "FlawManager:isValid", "Invalid datastructures in flaw manger.");
          return;
        }
      }
    }

    bool FlawManager::betterThan(const EntityId& a, const EntityId& b, LabelStr& explanation){
      if(a.isId() && b.isId()) {
        explanation = "higherKey";
        return (a->getKey() > b->getKey());
      }
      else {
        explanation = "b.isNoId";
        checkError(b.isNoId(), b);
        return true;
      }
    }

    std::string FlawManager::toString(const EntityId& entity) const {
      return entity->toString();
    }

    /**
     * @brief test if the timestamp is current
     */
    bool FlawManager::inSynch() const {
      return m_timestamp == m_db->getConstraintEngine()->cycleCount();
    }

    /**
     * @brief Update the timestamp
     */
    void FlawManager::synchronize(){
      m_timestamp =  m_db->getConstraintEngine()->cycleCount();
    }

    /** FLAW ITERATOR IMPLEMENTION **/

    FlawIterator::FlawIterator(FlawManager& manager)
      : m_visited(0), 
        m_done(false),
        m_manager(manager){

      // Force the manager to synch up with the cycle count of the constraint engine
      manager.synchronize();
    }

    void FlawIterator::advance(){
      EntityId candidate = nextCandidate();
      while(candidate.isId()){
        if(m_manager.dynamicMatch(candidate))
          candidate = nextCandidate();
        else {
          m_flaw = candidate;
          return;
        }
      }

      m_done = true;
      m_flaw = TokenId::noId();
    }

    bool FlawIterator::done() const {
      return m_done;
    }

    const EntityId FlawIterator::next() {
      check_error(m_manager.inSynch(), "Error: stale flaw iterator.");
      checkError(!done(), "Cannot be done when you call next.");

      EntityId flaw = m_flaw;
      advance();
      ++m_visited;
      return flaw;
    }
  }
}
