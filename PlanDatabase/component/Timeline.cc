
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

#include "../ConstraintEngine/ConstraintEngine.hh"
#include "../ConstraintEngine/Constraint.hh"
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"

#include <algorithm>

namespace Prototype {

  bool canPrecede(const TokenId& first, const TokenId& second){
    int earliest_end = (int) first->getEnd()->getDerivedDomain().getLowerBound();
    int latest_start = (int) second->getStart()->getDerivedDomain().getUpperBound();
    return (earliest_end <= latest_start);
  }

  bool canFitBetween(const TokenId& token, const TokenId& predecessor, const TokenId& successor){
      check_error(successor != predecessor);
      check_error(token != successor);
      check_error(token != predecessor);

      int latest_start = (int) successor->getStart()->getDerivedDomain().getUpperBound();
      int earliest_end = (int) predecessor->getEnd()->getDerivedDomain().getLowerBound();
      int min_duration = latest_start - earliest_end;

      if(min_duration >= token->getDuration()->getDerivedDomain().getLowerBound())
	return true;
      else
	return false;
  }

  Timeline::Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
    : Object(planDatabase, type, name, true){ if (!open) close();}

  Timeline::Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open)
    : Object(parent, type, localName, true){ if (!open) close();}

  Timeline::~Timeline(){
    // Clean up outstanding constraints
    for(std::list<ConstraintEntry*>::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it)
      delete *it;
  }


  void Timeline::getOrderingChoices(const TokenId& token, std::vector<TokenId>& results){
    check_error(isValid());
    check_error(results.empty());
    check_error(token.isValid());

    // Force propagation and return if inconsistent - leads to no ordering choices.
    if(!getPlanDatabase()->getConstraintEngine()->propagate())
      return;

    // It makes no sense to query for ordering choices for a Token that has already been inserted. Conseqeuntly,
    // we trap that as an error. We could just return indicating no choices but that might lead caller to conclude
    // there is simply an inconsistency rather than force them to write code to ensure this does not happen.
    check_error(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end());

    // Special cases, the sequence is empty.
    if(m_tokenSequence.empty()){
       results.push_back(TokenId::noId());
       return;
    }

    // Special case, the token could be placed at the end, which can't precede anything. This
    // results in an ordering choicxe of the noId() i.e. ordering w.r.t. no successor
    if(canPrecede(m_tokenSequence.back(),token))
       results.push_back(TokenId::noId());

    // So now we can go through the sequence till we find something that we can precede.
    std::list<TokenId>::iterator current = m_tokenSequence.begin();

    // Move forward until we find a Token we can precede
    while (current != m_tokenSequence.end() && !canPrecede(token, *current))
      current++;

    if (current == m_tokenSequence.end())
      return; // No additional choices

    if(current == m_tokenSequence.begin()){ // Can add current and return - only one token sequenced.
      results.push_back(*current);
      return;
    }

    std::list<TokenId>::iterator previous = --current; // step back for predecessor
    ++current; // step forward again to current

    // Stopping criteria: At the end or at a point where the token cannot come after the current token
    while (current != m_tokenSequence.end()){
      // Prune if the token cannot fit between tokens
      TokenId predecessor = *previous;
      TokenId successor = *current;

      if(canFitBetween(token, predecessor, successor))
	results.push_back(successor);

      previous = current++;
    }
  }

  void Timeline::getTokensToOrder(std::vector<TokenId>& results){
    check_error(isValid());
    check_error(results.empty());

    // Do propagation to update the information
    bool isOk = getPlanDatabase()->getConstraintEngine()->propagate();

    // Should only progress if we are consistent
    check_error(isOk);

    const std::set<TokenId>& tokensForThisObject = getTokens();
    for(std::set<TokenId>::const_iterator it = tokensForThisObject.begin(); it != tokensForThisObject.end(); ++it){
      TokenId token = *it;
      if(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end() && token->isActive())
	results.push_back(token); // We should now have an active token that has not been constrained
    }
  }

  bool Timeline::hasTokensToOrder() const {
    const std::set<TokenId>& tokensForThisObject = getTokens();

    for(std::set<TokenId>::const_iterator it = tokensForThisObject.begin(); it != tokensForThisObject.end(); ++it){
      TokenId token = *it;
      if(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end() && token->isActive())
	return true;
    }

    return false;
  }

  const std::list<TokenId>& Timeline::getTokenSequence() const{
    check_error(isValid());
    return m_tokenSequence;
  }

  void Timeline::constrain(const TokenId& token, const TokenId& successor){
    check_error(token.isValid());
    check_error(isValid());
    check_error(token->isActive());
    check_error(successor.isNoId() || (successor.isValid() && successor->isActive()));
    check_error(token != successor);

    if(getTokens().count(token) == 0)
      Object::add(token);

    // Constrain the object variable to a singleton of this object
    std::list<ObjectId> singleton;
    singleton.push_back(getId());
    ObjectSet base = token->getObject()->getBaseDomain();
    base.set(getId());
    constrainToSingleton(token, base, token->getObject());

    if(m_tokenSequence.empty()){
      check_error(successor.isNoId());
      m_tokenSequence.push_back(token);
      m_tokenIndex.insert(std::make_pair(token->getKey(), m_tokenSequence.begin()));
      Object::constrain(token,successor);
      return;
    }

    TokenId first;
    TokenId second;
    std::list<TokenId>::iterator pos;

    // If no successor given - place at the end, and set token as taking place after last token
    if(successor.isNoId()){ 
      first = m_tokenSequence.back();
      second = token;
      pos = m_tokenSequence.end();
    }
    else {
      first = token;
      second = successor;
      pos = std::find(m_tokenSequence.begin(), m_tokenSequence.end(), successor);
    }

    // Conduct insertion for token sequence and index
    pos = m_tokenSequence.insert(pos, token);
    m_tokenIndex.insert(std::make_pair(token->getKey(), pos));

    check_error(first != second);
    check_error(*pos == token);
    check_error(first.isValid());
    check_error(second.isValid());

    // Conduct insertion for constraint
    std::vector<ConstrainedVariableId> vars;
    vars.push_back(first->getEnd());
    vars.push_back(second->getStart());
    ConstraintId constraint =  ConstraintLibrary::createConstraint(LabelStr("Before"),
								   getPlanDatabase()->getConstraintEngine(),
								   vars);
    m_constraints.push_back(new ConstraintEntry(constraint, first, second));
    check_error(isValid());
    Object::constrain(token,successor);
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
    Object::free(token);
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

    std::vector<ConstraintId> constraintsToDelete;

    // Remove Token's constraints if there are any
    std::list<ConstraintEntry*>::iterator it = m_constraints.begin();
    while(it != m_constraints.end()){
      ConstraintEntry* ce = *it;
      if (ce->m_first == token || ce->m_second == token){
	if(token->isActive()){ // Then we want to delete the constraints
	  check_error(ce->isValid());
	  constraintsToDelete.push_back(ce->m_constraint);
	}
	it = m_constraints.erase(it);
	delete ce;
      }
      else
	it++;
    }

    for(std::vector<ConstraintId>::const_iterator it = constraintsToDelete.begin(); it != constraintsToDelete.end(); ++it){
      ConstraintId constraintToDelete = *it;
      check_error(constraintToDelete.isValid());
      delete (Constraint*) constraintToDelete;
    }

    // Clean up TokenSequence and TokenIndex data structures
    std::map<int, std::list<TokenId>::iterator >::iterator token_it = m_tokenIndex.find(token->getKey());
    if(token_it != m_tokenIndex.end()){
      m_tokenSequence.erase(token_it->second);
      m_tokenIndex.erase(token_it);
    }

    check_error(isValid(CLEANING_UP));
  }

  bool Timeline::isValid(bool cleaningUp) const{
    check_error(m_tokenIndex.size() == m_tokenSequence.size());
    std::set<TokenId> allTokens;
    for(std::list<TokenId>::const_iterator it = m_tokenSequence.begin(); it != m_tokenSequence.end(); ++it){
      TokenId token = *it;
      allTokens.insert(token);
      check_error(m_tokenIndex.find(token->getKey()) != m_tokenIndex.end());
      check_error(cleaningUp || token->isActive());
    }
    check_error(allTokens.size() == m_tokenSequence.size()); // No duplicates.
    return true;
  }

  void Timeline::constrainToSingleton(const TokenId& token, const AbstractDomain& domain, const ConstrainedVariableId& var){
    ConstraintId constraint = ConstraintLibrary::createConstraint(LabelStr("Singleton"),
								  getPlanDatabase()->getConstraintEngine(),
								  var,
								  domain);
    m_constraints.push_back(new ConstraintEntry(constraint, token));
  }

  Timeline::ConstraintEntry::ConstraintEntry(const ConstraintId& constraint, const TokenId& first, const TokenId& second)
    :m_constraint(constraint), m_first(first), m_second(second){}

  bool Timeline::ConstraintEntry::isValid() const{
    return (m_constraint.isValid() &&
	    m_first.isValid() &&
	    (m_second.isNoId() || m_second.isValid()));
  }
}
