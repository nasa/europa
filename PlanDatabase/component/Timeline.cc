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

#include <sstream>
#include <algorithm>

namespace EUROPA {

  /** TIMELINE IMPLEMENTATION **/

  Timeline::Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
    : Object(planDatabase, type, name, true) {commonInit(open);}

  Timeline::Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open)
    : Object(parent, type, localName, true) {commonInit(open);}

  Timeline::~Timeline(){
    discard(false);
  }

  void Timeline::commonInit(bool open){
    if (!open)
      close();
  }

  bool Timeline::hasToken(const TokenId& token) const{
    return m_tokenIndex.find(token->getKey()) != m_tokenIndex.end();
  }

  void Timeline::getOrderingChoices(const TokenId& token, 
				    std::vector< std::pair<TokenId, TokenId> >& results,
				    unsigned int limit){
    check_error(results.empty());
    check_error(token.isValid());
    check_error(limit > 0, "Cannot set limit to less than 1.");

    debugMsg("Timeline:getOrderingChoices", 
	     "Getting ordering choices for token (" << token->getKey() << ") " << token->getName().c_str());

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
      debugMsg("Timeline:getOrderingChoices:canPrecede",
	       "Token sequence is empty, returning a single pair with same token");
      results.push_back(std::make_pair(token, token));
      return;
    }

    unsigned int choiceCount = 0;

    TemporalAdvisorId temporalAdvisor = getPlanDatabase()->getTemporalAdvisor();

    // Alternatively, we can go through the sequence till we find something that we can precede.
    std::list<TokenId>::iterator current = m_tokenSequence.begin(); // Start at first token in the sequence
    const std::list<TokenId>::iterator& last = m_tokenSequence.end(); // For termination criteria

    // Move forward until we find a Token we can precede
    while (current != last) {
      TokenId successor = *current;
      if (temporalAdvisor->canPrecede(token, successor)) {
	debugMsg("Timeline:getOrderingChoices:canPrecede",
		 "At first position: " << token->toString() << " precedes " << (*current)->toString());
        break;
      }
      else {
	debugMsg("Timeline:getOrderingChoices:canPrecede"," at first position: " << token->toString() << " cannot precede " << (*current)->toString());      }
      ++current;
    }

    // If it can precede the first one, we do not have to test for fitting between
    // token in the sequence, thus, we should push it back and move on.
    if (current == m_tokenSequence.begin()) {
      debugMsg("Timeline:getOrderingChoices:canPrecede", " precedes the beginning token ");
      results.push_back(std::make_pair(token, *current));
      current++;
      choiceCount++;
    }

    // Stopping criteria: At the end or at a point where the token cannot come after the current token
    TokenId lastToken = m_tokenSequence.back();
    bool foundLastPredecessor = false;
    bool foundLastToken = (current == last);

    // Now we have to consider the distance between tokens, so need the previous position also.
    // Step back to start the predecessor and current at the right locations
    --current;

    while (!foundLastToken && !foundLastPredecessor && choiceCount < limit) {
      // Prune if the token cannot fit between tokens
      TokenId predecessor = *(current++);
      TokenId successor = *current;
      check_error(predecessor.isValid() && predecessor->isActive());
      check_error(successor.isValid() && successor->isActive());

      // we still need to check that the predecessor can precede the token,
      // otherwise we'll return bogus successors (see PlanDatabse::module-tests::testNoChoicesThatFit
      if (!temporalAdvisor->canPrecede(predecessor,token)) {
	debugMsg("Timeline:getOrderingChoices:canPrecede",predecessor->toString() << " cannot precede " << token->toString());
	foundLastPredecessor = true;
      }
      else {
	if (temporalAdvisor->canFitBetween(token, predecessor, successor)){
	  debugMsg("Timeline:getOrderingChoices:canPrecede",
		   token->toString() << "can be inserted between " << predecessor->toString() << " and " << successor->toString()); 
	  results.push_back(std::make_pair(token, successor));
	  choiceCount++;
	}
	else {
	  debugMsg("Timeline:getOrderingChoices:canPrecede",
		   token->toString() << "cannot be inserted between " << predecessor->toString() << " and " << successor->toString());
	}
      }

      foundLastToken = (successor == lastToken);
    }

    // Special case, the token could be placed at the end, which can't precede anything. This
    // results in an ordering choice w.r.t. oneself. For this to be possible, we cannot have already
    // found the last predecessor of the token, but rather we must have come to the end
    if (choiceCount < limit && !foundLastPredecessor){
      if(temporalAdvisor->canPrecede(m_tokenSequence.back(),token)) {
	debugMsg("Timeline:getOrderingChoices:canPrecede",
		 "last entry " << m_tokenSequence.back()->toString() << " precedes " << token->toString());
	results.push_back(std::make_pair(m_tokenSequence.back(), token));
      }
      else{
	debugMsg("Timeline:getOrderingChoices:canPrecede",
		 "last entry " << m_tokenSequence.back()->toString() << " cannot precede " << token->toString());
      }
    }
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
    checkError(getPrecedenceConstraint(predecessor, successor).isNoId(),
	       "At least one of predecessor and successor should not be sequenced yet." <<
	       predecessor->toString() << " before " << successor->toString());

    // Delegate to base class.
    Object::constrain(predecessor, successor, true);

    // Notify the PlanDatabase that an ordering is no longer required for either of these, if they were previously required
    if(orderingRequired(predecessor))
      notifyOrderingNoLongerRequired(predecessor);

    if(predecessor != successor && orderingRequired(successor))
      notifyOrderingNoLongerRequired(successor);

    checkError(m_tokenSequence.empty() || 
	       m_tokenIndex.find(predecessor->getKey()) != m_tokenIndex.end() ||
	       m_tokenIndex.find(successor->getKey()) != m_tokenIndex.end(),
	       "At least one of predecessor or successor should be already sequenced in a non empty sequence." <<
	       predecessor->toString() << " before " << successor->toString());

    checkError(m_tokenSequence.empty() || predecessor != successor,
	       "Can only constrain with respect to yourself on an empty timeline." <<
	       predecessor->toString() << " before " << successor->toString());

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

    // Obtain the successor and predecessor index positions
    std::map<int, std::list<TokenId>::iterator >::const_iterator predecessorIndexPos = 
      m_tokenIndex.find(predecessor->getKey());
    std::map<int, std::list<TokenId>::iterator >::const_iterator successorIndexPos = 
      m_tokenIndex.find(successor->getKey());

    // CASE 1: Only successor so so insert predecessor before it
    if (predecessorIndexPos == m_tokenIndex.end()) {
      std::list<TokenId>::iterator sequencePos = successorIndexPos->second;

      // Insert into sequence and index
      sequencePos = m_tokenSequence.insert(sequencePos, predecessor);
      insertToIndex(predecessor, sequencePos);

      // If we are not at the beginning of the list, constrain prdecessor to succeed prior predecessor
      if (sequencePos != m_tokenSequence.begin()) {
        sequencePos--;
        TokenId oldPredecessor = *sequencePos;
        Object::constrain(oldPredecessor, predecessor, false);
      }
    } 
    // CASE 2: Predecessor already sequenced, so insert successor after it. Thus is a given if we get here. 
    else if(successorIndexPos == m_tokenIndex.end()){
      std::list<TokenId>::iterator sequencePos = predecessorIndexPos->second;

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

    // Go backwards to obtain a predecessor
    if (token_pos != m_tokenSequence.begin()) {
      --token_pos;
      earlier = *token_pos;
      ++token_pos;
    }

    // Go forwards to obtain a successor
    ++token_pos;
    if (token_pos != m_tokenSequence.end())
      later = *token_pos;
    --token_pos;

    // Erase the current token from the sequence and index
    m_tokenSequence.erase(token_it->second);
    m_tokenIndex.erase(token_it);

    // May have to post a constraint between earlier and later if none exists already in the case
    // where the token is surrounded
    if (!earlier.isNoId() && !later.isNoId() && 
	getPrecedenceConstraint(earlier, later).isNoId())
      constrain(earlier, later);

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
    ConstraintId constraint = getPrecedenceConstraint(predecessor, successor);

    // Clear markers for explicit constraints for this token pair
    if(constraint.isId())
      m_explicitConstraints.erase(constraint->getKey());
    if(predecessor == successor)
      m_explicitConstraints.erase(predecessor->getKey());

    // Tokens are supported if they participate in any further explict constraints for the sequence
    bool predecessorRequired = hasExplicitConstraint(predecessor);

    bool successorRequired = hasExplicitConstraint(successor);

    // If both are still required, return without removing necessary constraint, although it is no longer
    // an explicit constraint
    if (predecessorRequired && successorRequired)
      return;

    // We free the constraint now, and adjust as necessary. Already taken care of markers
    // for the explicit constraint
    Object::free(predecessor, successor, false);

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
    checkError(m_tokenIndex.size() == m_tokenSequence.size(), 
	       m_tokenIndex.size() << " != " << m_tokenSequence.size());

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
    m_tokenIndex.insert(std::pair<int, std::list<TokenId>::iterator>(token->getKey(), position));
  }

  void Timeline::removeFromIndex(const TokenId& token){
    m_tokenIndex.erase(token->getKey());
    notifyOrderingRequired(token);
  }

  bool Timeline::orderingRequired(const TokenId& token){
    return (!token->isDeleted() && m_tokenIndex.find(token->getKey()) == m_tokenIndex.end());
  }

  void Timeline::notifyMerged(const TokenId& token){}
  void Timeline::notifyRejected(const TokenId& token) {}
  void Timeline::notifyDeleted(const TokenId& token){
    remove(token);
  }
}
