
/**
 * @file Timeline.cc
 * @brief Implementation for a Timeline - key use cases only.
 * @todo Could consider propagating when we get assigned a singleton. If a Token has no candidate successors then we could
 * empty the object variable.
 * @todo Figure out how to integrate the zig zag checks in getOrderingChoices
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

#include <algorithm>

namespace PLASMA {

  Timeline::Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
    : Object(planDatabase, type, name, true){ if (!open) close();}

  Timeline::Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open)
    : Object(parent, type, localName, true){ if (!open) close();}

  Timeline::~Timeline(){
  }


  void Timeline::getOrderingChoices(const TokenId& token, std::vector<TokenId>& results){
    check_error(results.empty());
    check_error(token.isValid());

    // Force propagation and return if inconsistent - leads to no ordering choices.
    if(!getPlanDatabase()->getConstraintEngine()->propagate())
      return;

    check_error(isValid());

    // It makes no sense to query for ordering choices for a Token that has already been inserted. Conseqeuntly,
    // we trap that as an error. We could just return indicating no choices but that might lead caller to conclude
    // there is simply an inconsistency rather than force them to write code to ensure this does not happen.
    check_error(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end());

    // Special cases, the sequence is empty.
    if(m_tokenSequence.empty()){
       results.push_back(TokenId::noId());
       return;
    }

    // So now we can go through the sequence till we find something that we can precede.
    std::list<TokenId>::iterator current = m_tokenSequence.begin();

    // Move forward until we find a Token we can precede
    while (current != m_tokenSequence.end()) {
      if (getPlanDatabase()->getTemporalAdvisor()->canPrecede(token, *current))
	break;

      current++;
    }

     // the token can precede every token in m_tokenSequence.
    if (current == m_tokenSequence.end()) {
      // std::cout << " Reached last token " << std::endl;
      // Special case, the token could be placed at the end, which can't precede anything. This
      // results in an ordering choice of the noId() i.e. ordering w.r.t. no successor
      //std::cout << "Testing if token can go at the end " << std::endl;

      if(getPlanDatabase()->getTemporalAdvisor()->canPrecede(m_tokenSequence.back(),token))
	results.push_back(TokenId::noId());

      return; // No additional choices
    }

    // TBW: if it can precede the first one, it can precede every other
    // token in the sequence, thus, we should push it back and move on to
    // see whether it can fit between the following tokens.

     if(current == m_tokenSequence.begin()){ // Can add current and increment
       results.push_back(*current);
       //       return;
       current++;
     }

    std::list<TokenId>::iterator previous = --current; // step back for predecessor
    ++current; // step forward again to current

    // Stopping criteria: At the end or at a point where the token cannot come after the current token
    while (current != m_tokenSequence.end()){
      // Prune if the token cannot fit between tokens
      TokenId predecessor = *previous;
      TokenId successor = *current;

      //      std::cout << " Checking between tokens " << predecessor->getKey() << " and " << successor->getKey() << std::endl;

      // we still need to check that the predecessor can precede the token,
      // otherwise we'll return bogus successors (see PlanDatabse::module-tests::testNoChoicesThatFit
      if (!getPlanDatabase()->getTemporalAdvisor()->canPrecede(predecessor,token))
	  break;

      if(getPlanDatabase()->getTemporalAdvisor()->canFitBetween(token, predecessor, successor)) {
	//	std::cout << "can fit between returned true " << std::endl;
	results.push_back(successor);
      }

      previous = current++;
    }

    // Special case, the token could be placed at the end, which can't precede anything. This
    // results in an ordering choice of the noId() i.e. ordering w.r.t. no successor

    if(getPlanDatabase()->getTemporalAdvisor()->canPrecede(m_tokenSequence.back(),token))
      results.push_back(TokenId::noId());

  }

  void Timeline::getTokensToOrder(std::vector<TokenId>& results){
    check_error(results.empty());

    // Do propagation to update the information
    bool isOk = getPlanDatabase()->getConstraintEngine()->propagate();
    check_error(isOk);
    check_error(isValid());

    // Should only progress if we are consistent
    check_error(isOk);

    const TokenSet& tokensForThisObject = getTokens();
    for(TokenSet::const_iterator it = tokensForThisObject.begin(); it != tokensForThisObject.end(); ++it){
      TokenId token = *it;
      if(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end() && token->isActive())
	results.push_back(token); // We should now have an active token that has not been constrained
    }
  }

  bool Timeline::hasTokensToOrder() const {
    const TokenSet& tokensForThisObject = getTokens();

    for(TokenSet::const_iterator it = tokensForThisObject.begin(); it != tokensForThisObject.end(); ++it){
      TokenId token = *it;
      if(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end() && token->isActive())
	return true;
    }

    return false;
  }

  const std::list<TokenId>& Timeline::getTokenSequence() const{
    return m_tokenSequence;
  }

  void Timeline::constrain(const TokenId& token, const TokenId& successor){
    check_error(token.isValid());
    check_error(token->isActive());
    check_error(successor.isNoId() || (successor.isValid() && successor->isActive()));
    check_error(token != successor);

    if(m_tokenSequence.empty()){
      check_error(successor.isNoId());
      m_tokenSequence.push_back(token);
      m_tokenIndex.insert(std::make_pair(token->getKey(), m_tokenSequence.begin()));
      Object::constrain(token); // No successor given, so will just ensure object variable is constrained
      return;
    }

    // We are potentially considering 3 tokens during insertion in a token sequence.
    TokenId first;
    TokenId second;
    TokenId third;

    std::list<TokenId>::iterator pos;

    // If no successor given - place at the end, and set token as taking place after last token
    if(successor.isNoId()){
      second = token;
      pos = m_tokenSequence.end();
    }
    else {
      pos = std::find(m_tokenSequence.begin(), m_tokenSequence.end(), successor);
      second = token;
      third = successor;
    }

    // Apply the constraint for second and third, even if third is a noId. We must apply at least one constraint!
    check_error(second != third);
    check_error(second.isValid());

    // Conduct insertion for token sequence and index
    pos = m_tokenSequence.insert(pos, token);
    m_tokenIndex.insert(std::make_pair(token->getKey(), pos));
    check_error(*pos == token);

    // Must ensure the token has been inserted before delegating to the parent
    Object::constrain(second,third);

    // If the token is sandwiched bteween first and third, then also constraint against first, should
    // it exist.
    if(token == second && pos != m_tokenSequence.begin()){
      --pos;
      first = *pos;
      Object::constrain(first, second);
    }
  }

  void Timeline::remove(const TokenId& token){
    check_error(token.isValid());
    cleanup(token);
    Object::remove(token);
  }

  void Timeline::free(const TokenId& token){
    check_error(token.isValid());
    check_error(token->isActive());
    cleanup(token);
  }

  /**
   * Note that we take special care for active vs. inactive tokens. It is not intuitive
   * that we may clean up for a token that is now inactive, since only active tokens
   * can be assigned to objects. However, due to propagation of the object variable to a
   * singleton, or during deletion of a token - when it gets deactivated automatically - such cleanup
   * is required.
   */
  void Timeline::cleanup(const TokenId& token){
    check_error(token.isValid());
    check_error(isValid(CLEANING_UP));

    // If we are freeing a token which is between 2 other tokens, then there may need
    // to be a new constraint inserted between its successor and predecessor in order to maintain
    // the temporal relationship. However, such a constraint may not be required, since it may already be present.
    std::map<int, std::list<TokenId>::iterator >::iterator token_it = m_tokenIndex.find(token->getKey());
    if(token_it != m_tokenIndex.end()){

      std::list<TokenId>::iterator token_pos = token_it->second;
      TokenId predecessor;
      TokenId successor;

      if(token_pos != m_tokenSequence.begin()){
	--token_pos;
	predecessor = *token_pos;
	++token_pos;

	// Move on to successor
	++token_pos;
	if(token_pos != m_tokenSequence.end())
	  successor = *token_pos;
      }

      // Clean up the data structures
      m_tokenSequence.erase(token_it->second);
      m_tokenIndex.erase(token_it);
      Object::free(token);

      // Now, if we have anything to fix up, do so.
      if(!successor.isNoId() && !predecessor.isNoId() && ! areConstrained(predecessor, successor))
	Object::constrain(predecessor, successor);
    }
    check_error(isValid(CLEANING_UP));
  }

  bool Timeline::isValid(bool cleaningUp) const{
    check_error(m_tokenIndex.size() == m_tokenSequence.size());
    int prior_earlist_start = MINUS_INFINITY - 1;
    int prior_earliest_end = MINUS_INFINITY-1;
    int prior_latest_end = MINUS_INFINITY;
    TokenId predecessor;

    std::set<TokenId> allTokens;
    for(std::list<TokenId>::const_iterator it = m_tokenSequence.begin(); it != m_tokenSequence.end(); ++it){
      TokenId token = *it;
      allTokens.insert(token);
      check_error(m_tokenIndex.find(token->getKey()) != m_tokenIndex.end());
      check_error(cleaningUp || token->isActive());

      // Validate that earliset start times are monotonically increasing, as long as we are constraint consistent at any rate!
      // Also ensure x.end <= (x+1).start.
      if(!cleaningUp && getPlanDatabase()->getConstraintEngine()->constraintConsistent()){
	if(!predecessor.isNoId())
	  check_error(isConstrainedToPrecede(predecessor, token));
	int earliest_start = (int) token->getStart()->lastDomain().getLowerBound();
	int latest_start = (int) token->getStart()->lastDomain().getUpperBound();
	check_error(earliest_start == MINUS_INFINITY || earliest_start == PLUS_INFINITY || earliest_start > prior_earlist_start);
	check_error(prior_earliest_end <= earliest_start);
	check_error(prior_latest_end <= latest_start);
	prior_earliest_end = (int) token->getEnd()->lastDomain().getLowerBound();
	prior_latest_end = (int) token->getEnd()->lastDomain().getLowerBound();
	prior_earlist_start = earliest_start;
      }

      predecessor = token;
    }
    check_error(allTokens.size() == m_tokenSequence.size()); // No duplicates.
    return true;
  }

}
