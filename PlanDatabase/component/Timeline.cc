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
    return (first->getEnd()->getDerivedDomain().getLowerBound() <= second->getStart()->getDerivedDomain().getUpperBound());
  }

  Timeline::Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)
    : Object(planDatabase, type, name){}

  Timeline::Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName)
    : Object(parent, type, localName){}

  Timeline::~Timeline(){
    // Clean up outstanding constraints
    for(std::multimap<double, ConstraintId>::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it)
      delete (Constraint*) it->second;
  }

  void Timeline::constrain(const TokenId& token, const TokenId& successor){
    check_error(token.isValid());
    check_error(m_constraints.find((double)token) == m_constraints.end());
    check_error(isValid());
    check_error(token->isActive());

    if(getTokens().count(token) == 0)
      Object::add(token);

    // Constrain the object variable to a singleton of this object
    std::list<ObjectId> singleton;
    singleton.push_back(getId());
    constrainToSingleton(token, ObjectSet(singleton), token->getObject());
    constrainToSingleton(token, BooleanDomain(1, 1), token->getRejectability());

    if(m_tokenSequence.empty()){
      check_error(successor.isNoId());
      m_tokenSequence.push_back(token);
      m_tokenIndex.insert(std::make_pair(token, m_tokenSequence.begin()));
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
    m_tokenIndex.insert(std::make_pair(token, pos));

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
    m_constraints.insert(std::make_pair(token, constraint));
    check_error(isValid());
  }

  void Timeline::remove(const TokenId& token){
    Object::remove(token);
    free(token);
  }

  void Timeline::free(const TokenId& token){
    check_error(token.isValid());
    check_error(isValid());
  
    // Remove Token's constraints if there are any
    std::multimap<double, ConstraintId>::const_iterator constraint_it = m_constraints.find(token);
    while (constraint_it != m_constraints.end() && constraint_it->first == token){
      delete (Constraint*) constraint_it->second;
      ++constraint_it;
    }
    // Now remove from the map
    m_constraints.erase(token);

    // Clean up TokenSequence and TokenIndex data structures
    std::map<double, std::list<TokenId>::iterator >::iterator token_it = m_tokenIndex.find(token);
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
    m_constraints.insert(std::make_pair(token, constraint));
  }
}
