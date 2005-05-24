/**
 * @file Timeline.cc
 * @brief Implementation for a Timeline - key use cases only.
 * @author Conor McGann
 * @date December, 2003
 */

#include "Timeline.hh"
#include "Token.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "TemporalAdvisor.hh"
#include "ConstraintEngine.hh"
#include "Constraint.hh"
#include "ConstraintLibrary.hh"
#include "IntervalIntDomain.hh"
#include "Utils.hh"
#include "Debug.hh"

#include <algorithm>

namespace EUROPA {

  /**
   * @brief Maintains a cache of candidate ordering choices for a given active token.
   */
  class OrderingChoicesCache {
  public:

    /**
     * @brief Stores pertinent information for caching previously computed information regarding
     * available ordering choices.
     */
    class Entry {
    public:
      Entry(const TokenId& tokenToOrder, const std::list<TokenId>::iterator& first, const std::list<TokenId>::iterator& last, unsigned int timestamp)
	: m_tokenToOrder(tokenToOrder), m_tokenToOrderKey(tokenToOrder->getKey()), m_first(first), m_last(last), m_timestamp(timestamp), m_id(this) {}

      ~Entry(){m_id.remove();}

      const Id<Entry>& getId() const {return m_id;}

      const TokenId m_tokenToOrder; /*!< The token for which the ordering choices are cached */
      const unsigned int m_tokenToOrderKey; /*!< Used for cleanup and validation */
      std::list<TokenId>::iterator m_first; /*!< Pointer to first candidate token insertion position */
      std::list<TokenId>::iterator m_last; /*!< Pointer to last candidate token insertion position */
      std::set<TokenId> m_excludedTokens; /*!< Stores prior exclusions */
      unsigned int m_timestamp; /*!< Use to assess if a reporop has occured and need to reset. */
    private:
      Id<Entry> m_id;
    };

    OrderingChoicesCache(std::list<TokenId>& tokenSequence, const TemporalAdvisorId& temporalAdvisor)
      :m_id(this), m_tokenSequence(tokenSequence), m_temporalAdvisor(temporalAdvisor){}

    ~OrderingChoicesCache();

    const Id<OrderingChoicesCache>& getId() const {return m_id;}

    /**
     * @brief Uses cached results as the starting point or allocates a new cache entry
     * based on the full sequence and prunes from there.
     */
    void getOrderingChoices(const TokenId& token, 
			    std::vector< std::pair<TokenId, TokenId> >& results,
			    unsigned int limit);

    /** Synchronization Methods **/

    /**
     * @brief token no longer requires ordering. Applies when the Token has been ordered or when the
     * token has been deactivated.
     */
    void removeCacheEntry(const TokenId& token);

    /**
     * @brief When a token is no longer available for ordering, since it was freed from the timeline, deactivated
     * etc.
     */
    void removeInsertedToken(const TokenId& token);

    /**
     * @brief Retrieve the cache entry for this token.
     */
    Id<OrderingChoicesCache::Entry> getCacheEntry(const TokenId& tokenToOrder);

  private:
    bool isValid() const;

    Id<OrderingChoicesCache> m_id;
    std::list<TokenId>& m_tokenSequence;
    TemporalAdvisorId m_temporalAdvisor;

    std::map<TokenId, Id<OrderingChoicesCache::Entry> > m_orderingChoices;
  };

  OrderingChoicesCache::~OrderingChoicesCache(){
    cleanup(m_orderingChoices);
    m_id.remove();
  }

  bool OrderingChoicesCache::isValid() const {
    for(std::map<TokenId, Id<OrderingChoicesCache::Entry> >::const_iterator it = m_orderingChoices.begin(); it != m_orderingChoices.end(); ++it){
      Id<OrderingChoicesCache::Entry> entry = it->second;
      check_error(entry.isValid());

      // Exit if it is a stale entry
      if(Entity::getEntity(entry->m_tokenToOrderKey).isNoId())
	return true;

      checkError(entry->m_tokenToOrder.isValid(), entry->m_tokenToOrder);
      check_error(entry->m_first == m_tokenSequence.end() || (*entry->m_first).isValid());
      checkError(entry->m_first == m_tokenSequence.end() || (*entry->m_first)->isActive(), "Synchronization bug. " << (*entry->m_first)->toString() << " is not active.");
      check_error(entry->m_last == m_tokenSequence.end() || (*entry->m_last).isValid());
      checkError(entry->m_last == m_tokenSequence.end() || (*entry->m_last)->isActive(), "Synchronization bug. " << (*entry->m_last)->toString() << " is not active.");
    }
    return true;
  }

  Id<OrderingChoicesCache::Entry> OrderingChoicesCache::getCacheEntry(const TokenId& tokenToOrder){
    unsigned int currentTimeStamp = tokenToOrder->getPlanDatabase()->getConstraintEngine()->cycleCount();
    Id<OrderingChoicesCache::Entry> entry;


    std::map<TokenId, Id<OrderingChoicesCache::Entry> >::const_iterator it = m_orderingChoices.find(tokenToOrder);
    // If no hit, must allocate
    if(it == m_orderingChoices.end()){
      entry = (new OrderingChoicesCache::Entry(tokenToOrder, m_tokenSequence.begin(), m_tokenSequence.end(), currentTimeStamp))->getId();
      m_orderingChoices.insert(std::pair<TokenId, Id<OrderingChoicesCache::Entry> >(tokenToOrder, entry));
    }
    else
      entry = it->second;

    if(entry->m_timestamp < m_temporalAdvisor->mostRecentRepropagation()){
      removeCacheEntry(tokenToOrder);
      entry = getCacheEntry(tokenToOrder);
    }

    checkError(entry.isValid(), "Must have a valid entry by now for token " << tokenToOrder->toString());

    return entry;
  }

  void OrderingChoicesCache::getOrderingChoices(const TokenId& token, 
						std::vector< std::pair<TokenId, TokenId> >& results,
						unsigned int limit){
    check_error(isValid());
    check_error(results.empty());
    check_error(token.isValid());
    check_error(limit > 0, "Cannot set limit to less than 1.");

    unsigned int choiceCount = 0;

    // Obtain the cache entry
    Id<OrderingChoicesCache::Entry> entry = getCacheEntry(token);

    // Alternatively, we can go through the sequence till we find something that we can precede.
    std::list<TokenId>::iterator& current = entry->m_first;
    std::list<TokenId>::iterator& last = entry->m_last;
    std::set<TokenId>& excludedTokens = entry->m_excludedTokens;

    // Move forward until we find a Token we can precede
    while (current != last) {
      if (m_temporalAdvisor->canPrecede(token, *current)) {
	debugMsg("Timeline:getOrderingChoices:canPrecede"," at first position: " << token->toString() << " precedes " << (*current)->toString());
        break;
      }
      else {
	debugMsg("Timeline:getOrderingChoices:canPrecede"," at first position: " << token->toString() << " cannot precede " << (*current)->toString());
      }
      ++current;
    }

    // Update cache entry iterator with possibly new first position
    entry->m_first = current;

    // If it can precede the first one, we do not have to test for fitting between
    // token in the sequence, thus, we should push it back and move on.
    if (current == m_tokenSequence.begin() && choiceCount < limit) {
      debugMsg("Timeline:getOrderingChoices:canPrecede", " precedes the beginning token ");
      results.push_back(std::make_pair(token, *current));
      current++;
      choiceCount++;
    }

    // Now we have to consider the distance between tokens, so need the previous position also
    std::list<TokenId>::iterator previous = --current; // step back for predecessor
    ++current; // step forward again to current

    // Stopping criteria: At the end or at a point where the token cannot come after the current token
    while (current != last && choiceCount < limit) {
      // Prune if the token cannot fit between tokens
      TokenId predecessor = *previous;
      TokenId successor = *current;
      check_error(predecessor.isValid() && predecessor->isActive());
      check_error(successor.isValid() && successor->isActive());

      // Only worth considering this insertion poistion if the successor has not already
      // been excluded
      if(excludedTokens.find(successor) == excludedTokens.end()){
	// we still need to check that the predecessor can precede the token,
	// otherwise we'll return bogus successors (see PlanDatabse::module-tests::testNoChoicesThatFit
	if (!m_temporalAdvisor->canPrecede(predecessor,token)) {
	  debugMsg("Timeline:getOrderingChoices:canPrecede",predecessor->toString() << " cannot precede " << token->toString());
	  last = current; // Update end-point if we reach it
	}
	else {
	  if (m_temporalAdvisor->canFitBetween(token, predecessor, successor)){
	    debugMsg("Timeline:getOrderingChoices:canPrecede",
		     token->toString() << "can be inserted between " << predecessor->toString() << " and " << successor->toString()); 
	    results.push_back(std::make_pair(token, successor));
	    choiceCount++;
	  }
	  else {
	    debugMsg("Timeline:getOrderingChoices:canPrecede",
		     token->toString() << "cannot be inserted between " << predecessor->toString() << " and " << successor->toString());
	    excludedTokens.insert(successor);
	  }
	  previous = current++;
	}
      }
    }

    // Special case, the token could be placed at the end, which can't precede anything. This
    // results in an ordering choice w.r.t. oneself
    if (choiceCount < limit && m_temporalAdvisor->canPrecede(m_tokenSequence.back(),token)) {
      debugMsg("Timeline:getOrderingChoices:canPrecede",
	       "last entry " << m_tokenSequence.back()->toString() << " precedes " << token->toString());
      results.push_back(std::make_pair(m_tokenSequence.back(), token));
    }

    removeCacheEntry(token);

    debugMsg("Timeline:getOrderingChoices:canPrecede", "exiting");
  }


  void OrderingChoicesCache::removeCacheEntry(const TokenId& token){
    std::map<TokenId, Id<OrderingChoicesCache::Entry> >::iterator it = m_orderingChoices.find(token);
    if(it != m_orderingChoices.end()){
      Id<OrderingChoicesCache::Entry> entry = it->second;
      m_orderingChoices.erase(it);
      delete (OrderingChoicesCache::Entry*) entry;
    }
  }

  /** TIMELINE IMPLEMENTATION **/

  Timeline::Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
    : Object(planDatabase, type, name, true) {commonInit(open);}

  Timeline::Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open)
    : Object(parent, type, localName, true) {commonInit(open);}

  Timeline::~Timeline(){
    delete (OrderingChoicesCache*) m_cache;
  }

  void Timeline::commonInit(bool open){
    if (!open)
      close();
    m_cache = (new OrderingChoicesCache(m_tokenSequence, getPlanDatabase()->getTemporalAdvisor()))->getId();
  }

  void Timeline::getOrderingChoices(const TokenId& token, 
				    std::vector< std::pair<TokenId, TokenId> >& results,
				    unsigned int limit){
    check_error(results.empty());
    check_error(token.isValid());
    check_error(limit > 0, "Cannot set limit to less than 1.");

    debugMsg("Timeline:getOrderingChoices", "getting ordering choices for token (" << token->getKey() << ") " << token->getName().c_str());

    // Force propagation and return if inconsistent - leads to no ordering choices.
    if (!getPlanDatabase()->getConstraintEngine()->propagate())
      return;

    check_error(isValid());

    // It makes no sense to query for ordering choices for a Token that has already been inserted. Conseqeuntly,
    // we trap that as an error. We could just return indicating no choices but that might lead caller to conclude
    // there is simply an inconsistency rather than force them to write code to ensure this does not happen.
    check_error(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end(),
                "Attempted to query for choices to constrain token " + token->getPredicateName().toString() + 
                " which has already been constrained.");

    // If the sequence is empty, add the case where both elements of the pair are the given token.
    if (m_tokenSequence.empty()) {
      debugMsg("Timeline:getOrderingChoices:canPrecede"," token sequence is empty, returning a single pair with same token");
      results.push_back(std::make_pair(token, token));
      return;
    }

    m_cache->getOrderingChoices(token, results, limit);
  }

  void Timeline::getTokensToOrder(std::vector<TokenId>& results) {
    check_error(results.empty());

    // Do propagation to update the information
    getPlanDatabase()->getConstraintEngine()->propagate();

    checkError(getPlanDatabase()->getConstraintEngine()->constraintConsistent(),
	       "Should be consistent to continue here. Should have checked before you called the method in the first place.");
    check_error(isValid());

    const TokenSet& tokensForThisObject = getTokens();
    for (TokenSet::const_iterator it = tokensForThisObject.begin(); it != tokensForThisObject.end(); ++it) {
      TokenId token = *it;
      if (m_tokenIndex.find(token->getKey()) == m_tokenIndex.end() && token->isActive())
        results.push_back(token); // We should now have an active token that has not been constrained
    }
  }

  bool Timeline::hasTokensToOrder() const {
    const TokenSet& tokensForThisObject = getTokens();

    // Quits on the first token that is active and not yet sequenced
    for (TokenSet::const_iterator it = tokensForThisObject.begin(); it != tokensForThisObject.end(); ++it) {
      TokenId token = *it;
      if (token->isActive() && m_tokenIndex.find(token->getKey()) == m_tokenIndex.end())
        return(true);
    }
    return(false);
  }

  const std::list<TokenId>& Timeline::getTokenSequence() const {
    return(m_tokenSequence);
  }

  /**
   * Constraining tokens on a timeline results in an ordered list of tokens.
   * @todo Consider deactivating redundant constraints. If we do this, we would also have to
   * activate them when freeing.
   */
  void Timeline::constrain(const TokenId& predecessor, const TokenId& successor) {
    // Delegate to base class.
    Object::constrain(predecessor, successor, true);

    // Notify the PlanDatabase that an ordering is no longer required for either of these, if they were previously required
    if(orderingRequired(predecessor))
      notifyOrderingNoLongerRequired(predecessor);

    if(predecessor != successor && orderingRequired(successor))
      notifyOrderingNoLongerRequired(successor);


    // Additional tests for a Timeline
    check_error(m_tokenIndex.find(predecessor->getKey()) == m_tokenIndex.end() ||
                m_tokenIndex.find(successor->getKey()) == m_tokenIndex.end(),
                "At least one of predecessor and successor should not be sequenced yet.");

    check_error(m_tokenSequence.empty() || 
                m_tokenIndex.find(predecessor->getKey()) != m_tokenIndex.end() ||
                m_tokenIndex.find(successor->getKey()) != m_tokenIndex.end(),
                "At least one of predecessor or successor should be already sequenced in a non empty sequence.");

    check_error(m_tokenSequence.empty() || predecessor != successor,
                "Can only constrain with respect to yourself on an empty timeline.");

    // CASE 0: No tokens are sequenced yet
    if (m_tokenSequence.empty()) {
      // Insert the successor and store its position
      m_tokenSequence.push_back(successor);
      insertToIndex(successor, m_tokenSequence.begin());

      if (predecessor != successor) { // Insert the predecessor and store its position
        m_tokenSequence.push_front(predecessor);
        insertToIndex(predecessor, m_tokenSequence.begin());
      }
      return;
    }

    // If the successor is sequenced we must insert the predecessor before it in the list, and additionally
    // constrain the predecessor.
    std::map<int, std::list<TokenId>::iterator >::const_iterator indexPos = m_tokenIndex.find(successor->getKey());

    // CASE 1: Successor already sequenced, so insert predecessor before it
    if (indexPos != m_tokenIndex.end()) {
      std::list<TokenId>::iterator sequencePos = indexPos->second;

      // Insert into sequence and index
      sequencePos = m_tokenSequence.insert(sequencePos, predecessor);
      insertToIndex(predecessor, sequencePos);

      // If we are not at the beginning of the list, constrain prdecessor to succeed prior predecessor
      if (sequencePos != m_tokenSequence.begin()) {
        sequencePos--;
        TokenId oldPredecessor = *sequencePos;
        Object::constrain(oldPredecessor, predecessor, false);
      }
    } else {  // CASE 2: Predecessor already sequenced, so insert successor after it. Thus is a given if we get here.
      indexPos = m_tokenIndex.find(predecessor->getKey());
      std::list<TokenId>::iterator sequencePos = indexPos->second;

      // Insert into sequence and index
      sequencePos = m_tokenSequence.insert(++sequencePos, successor);
      insertToIndex(successor, sequencePos);

      // If we are not at the end of the list, constrain successor to precede prior successor
      if (++sequencePos != m_tokenSequence.end()) {
        TokenId oldSuccessor = *sequencePos;
        Object::constrain(successor, oldSuccessor, false);
      }
    }
  }

  void Timeline::add(const TokenId& token){
    Object::add(token);
    notifyOrderingRequired(token);
  }

  void Timeline::remove(const TokenId& token) {
    check_error(token.isValid());
    check_error(isValid(CLEANING_UP));

    if(orderingRequired(token))
      notifyOrderingNoLongerRequired(token);

    // CASE 0: It is not sequenced, so can ignore it
    std::map<int, std::list<TokenId>::iterator >::iterator token_it = m_tokenIndex.find(token->getKey());
    if (token_it == m_tokenIndex.end()) {
      Object::remove(token);
      return;
    }

    TokenId earlier;
    TokenId later;
    std::list<TokenId>::iterator token_pos = token_it->second;

    if (token_pos != m_tokenSequence.begin()) {
      --token_pos;
      earlier = *token_pos;
      ++token_pos;
    }

    ++token_pos;
    if (token_pos != m_tokenSequence.end())
      later = *token_pos;
    --token_pos;

    // May have to post a constraint between earlier and later if none exists already in the case
    // where the token is surrounded
    if (!earlier.isNoId() && !later.isNoId() && getPrecedenceConstraint(earlier, later).isNoId())
      constrain(earlier, later);

    std::map<int, std::list<TokenId>::iterator >::iterator it = m_tokenIndex.find(token->getKey());
    m_tokenSequence.erase(it->second);
    m_tokenIndex.erase(it);

    Object::remove(token);

    check_error(isValid(CLEANING_UP));
  }

  /**
   * The main complexity in implementing this procedure is determining
   * what tokens to remove from the sequence, and what constraints should correctly
   * be removed.
   */
  void Timeline::free(const TokenId& predecessor, const TokenId& successor) {
    check_error(m_tokenIndex.find(predecessor->getKey()) != m_tokenIndex.end() &&
                m_tokenIndex.find(successor->getKey()) != m_tokenIndex.end(),
                "Predecessor and successor must both be sequenced if they are to be freed");

    check_error(isValid());

    // Remove mark as explicit in all cases. We do this prior to calling free in the base class
    // as the latter also deletes the constraint.
    m_explicitConstraints.erase(toString(predecessor, successor));

    // Tokens are supported if they participate in any explict constraints for the sequence
    bool predecessorRequired = hasExplicitConstraint(predecessor);

    bool successorRequired = hasExplicitConstraint(successor);

    // If both are still required, return without removing necessary constraint, although it is no longer
    // an explicit constraint
    if (predecessorRequired && successorRequired)
      return;

    // We free the constraint now, and adjust as necessary
    Object::free(predecessor, successor, false); // Since no longer explict

    // In the event we are freeing based on a single token, unlink it and get out
    if (predecessor == successor) {
      unlink(predecessor);
      return;
    }

    TokenId startTok(predecessor);
    TokenId endTok(successor);

    // If successor is not required, remove it. Update the end token.
    if (!successorRequired)
      endTok = removeSuccessor(successor);

    // If predecessor is not required, remove it and update the start token
    if (!predecessorRequired)
      startTok = removePredecessor(predecessor);

    // If valid start and end tokens, currently unconstrained, and adjacemt
    if (adjacent(startTok, endTok) && !isConstrainedToPrecede(startTok, endTok))
      Object::constrain(startTok, endTok, false); // Add an implied constraint
 
    check_error(isValid());
  }

  bool Timeline::isValid(bool cleaningUp) const {
#ifdef EUROPA_FAST
    assertTrue(ALWAYS_FAIL, "Timeline::isValid() should never be called when compiling #define EUROPA_FAST");
#else
    check_error(Object::isValid());
    check_error(m_tokenIndex.size() == m_tokenSequence.size());
    int prior_earliest_start = MINUS_INFINITY - 1;
    int prior_earliest_end = MINUS_INFINITY - 1;
    int prior_latest_end = MINUS_INFINITY;
    TokenId predecessor;

    std::set<TokenId> allTokens;
    for (std::list<TokenId>::const_iterator it = m_tokenSequence.begin(); it != m_tokenSequence.end(); ++it) {
      TokenId token = *it;
      allTokens.insert(token);
      check_error(m_tokenIndex.find(token->getKey()) != m_tokenIndex.end());
      check_error(cleaningUp || token->isActive());

      // Validate that earliest start times are monotonically increasing, as long as we are constraint consistent at any rate!
      // Also ensure x.end <= (x+1).start.
      if (!cleaningUp && getPlanDatabase()->getConstraintEngine()->constraintConsistent()) {
        check_error(predecessor.isNoId() || isConstrainedToPrecede(predecessor, token));
        int earliest_start = (int) token->getStart()->lastDomain().getLowerBound();
        int latest_start = (int) token->getStart()->lastDomain().getUpperBound();
        check_error(earliest_start == MINUS_INFINITY || earliest_start == PLUS_INFINITY || earliest_start > prior_earliest_start);
        check_error(prior_earliest_end <= earliest_start);
        check_error(prior_latest_end <= latest_start);
        prior_earliest_end = (int) token->getEnd()->lastDomain().getLowerBound();
        prior_latest_end = (int) token->getEnd()->lastDomain().getLowerBound();
        prior_earliest_start = earliest_start;
      }
      predecessor = token;
    }
    check_error(allTokens.size() == m_tokenSequence.size()); // No duplicates.
#endif
    return(true);
  }

  bool Timeline::atStart(const TokenId& token) const {
    return(!m_tokenSequence.empty() && token == m_tokenSequence.front());
  }

  bool Timeline::atEnd(const TokenId& token) const {
    return(!m_tokenSequence.empty() && token == m_tokenSequence.back());
  }

  TokenId Timeline::removeSuccessor(const TokenId& token) {
    freeImplicitConstraints(token);
    std::list<TokenId>::iterator pos = m_tokenIndex.find(token->getKey())->second;
    removeFromIndex(token);

    if (m_tokenIndex.empty()) {
      m_tokenSequence.clear();
      return TokenId::noId();
    }

    if (atEnd(token)) {
      m_tokenSequence.pop_back();
      return m_tokenSequence.back();
    }

    m_tokenSequence.erase(pos++);
    return *pos;
  }

  TokenId Timeline::removePredecessor(const TokenId& token) {
    freeImplicitConstraints(token);
    std::list<TokenId>::iterator pos = m_tokenIndex.find(token->getKey())->second;
    removeFromIndex(token);

    if (m_tokenIndex.empty()) {
      m_tokenSequence.clear();
      return TokenId::noId();
    }

    if (atStart(token)) {
      m_tokenSequence.pop_front();
      return m_tokenSequence.front();
    }

    m_tokenSequence.erase(pos--);
    return(*pos);
  }

  bool Timeline::adjacent(const TokenId& x, const TokenId& y) const {
    if (x.isNoId() || y.isNoId())
      return(false);
    std::list<TokenId>::const_iterator pos = m_tokenIndex.find(x->getKey())->second;
    ++pos;
    return(pos != m_tokenSequence.end() && y == *pos);
  }

  void Timeline::unlink(const TokenId& token) {
    freeImplicitConstraints(token);
    std::list<TokenId>::iterator pos = m_tokenIndex.find(token->getKey())->second;
    removeFromIndex(token);
    if (atStart(token))
      m_tokenSequence.pop_front();
    else
      if (atEnd(token))
        m_tokenSequence.pop_back();
      else { // Have to handle removal in the middle and re-linking if necessary
        m_tokenSequence.erase(pos--);
        TokenId startTok = *pos;
        ++pos;
        TokenId endTok = *pos;
        if (!isConstrainedToPrecede(startTok, endTok))
          Object::constrain(startTok, endTok, false);
      }
    return;
  }

  void Timeline::insertToIndex(const TokenId& token, const std::list<TokenId>::iterator& position){
    // Remove the cache entry for this token as it is now inserted
    m_cache->removeCacheEntry(token);
    m_tokenIndex.insert(std::pair<int, std::list<TokenId>::iterator>(token->getKey(), position));
  }

  void Timeline::removeFromIndex(const TokenId& token){
    m_tokenIndex.erase(token->getKey());
    notifyOrderingRequired(token);
  }

  bool Timeline::orderingRequired(const TokenId& token){
    return (m_tokenIndex.find(token->getKey()) == m_tokenIndex.end());
  }

}
