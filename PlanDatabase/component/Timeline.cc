
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
#include "ObjectSet.hh"
#include "PlanDatabase.hh"

#include "../ConstraintEngine/ConstraintEngine.hh"
#include "../ConstraintEngine/Constraint.hh"
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"

#include <algorithm>

namespace Prototype {

  bool can_precede(const TokenId& first, const TokenId& second){
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

  Timeline::Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
    : Object(planDatabase, type, name){}

  Timeline::Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName)
    : Object(parent, type, localName){}

  Timeline::~Timeline(){
    // Clean up outstanding constraints
    for(std::list<ConstraintEntry*>::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it)
      delete *it;
  }


  void Timeline::getOrderingChoices(const TokenId& token, std::vector<TokenId>& results){
    check_error(results.empty());
    check_error(token.isValid());
    check_error(getPlanDatabase()->getConstraintEngine()->propagate()); // Must not be inconsistent
    check_error(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end());

    // Special cases, the sequence is empty.
    if(m_tokenSequence.empty()){
       results.push_back(TokenId::noId());
       return;
    }

    // Special case, the token could be placed at the end, which can't precede anything. This
    // results in an ordering choicxe of the noId() i.e. ordering w.r.t. no successor
    if(can_precede(m_tokenSequence.back(),token))
       results.push_back(TokenId::noId());

    // So now we can go through the sequence till we find something that we can precede.
    std::list<TokenId>::iterator current = m_tokenSequence.begin();

    // Move forward until we find a Token we can precede
    while (current != m_tokenSequence.end() && !can_precede(token, *current))
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
    check_error(results.empty());

    // Do propagation to update the information
    getPlanDatabase()->getConstraintEngine()->propagate();

    const std::set<TokenId>& tokensForThisObject = getTokens();
    for(std::set<TokenId>::const_iterator it = tokensForThisObject.begin(); it != tokensForThisObject.end(); ++it){
      TokenId token = *it;
      if(m_tokenIndex.find(token->getKey()) == m_tokenIndex.end() && token->isActive())
	results.push_back(token);
    }
  }


  const std::list<TokenId>& Timeline::getTokenSequence() const{return m_tokenSequence;}

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
  }

  void Timeline::remove(const TokenId& token){
    check_error(token.isValid());
    if(token->isActive())
      cleanupToken(token, true);
    else
      cleanupToken(token, false);
    Object::remove(token);
  }

  void Timeline::free(const TokenId& token){
    check_error(token.isValid());
    check_error(token->isActive());
    cleanupToken(token, true);
  }

  void Timeline::cleanupToken(const TokenId& token, bool deleteConstraints){
    check_error(token.isValid());
    check_error(isValid());
    check_error(token->isActive() == deleteConstraints); 
  
    // Remove Token's constraints if there are any
    std::list<ConstraintEntry*>::iterator it = m_constraints.begin();
    while(it != m_constraints.end()){
      ConstraintEntry* ce = *it;
      if (ce->m_first == token || ce->m_second == token){
	if(deleteConstraints){
	  check_error(ce->isValid());
	  delete (Constraint*) ce->m_constraint;
	}
	delete ce;
	it = m_constraints.erase(it);
      }
      else
	it++;
    }

    // Clean up TokenSequence and TokenIndex data structures
    std::map<int, std::list<TokenId>::iterator >::iterator token_it = m_tokenIndex.find(token->getKey());
    if(token_it != m_tokenIndex.end()){
      m_tokenSequence.erase(token_it->second);
      m_tokenIndex.erase(token_it);
    }

    check_error(isValid());
  }

  bool Timeline::isValid() const{
    return (m_tokenIndex.size() == m_tokenSequence.size());
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
