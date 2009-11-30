#include "MergeMemento.hh"
#include "Token.hh"
#include "PlanDatabase.hh"
#include "ConstraintType.hh"
#include "TokenVariable.hh"
#include "Utils.hh"
#include <map>

namespace EUROPA{

  /**
   * @brief Helper method to lookup a replacement for a variable or use the one u r given.
   *
   * The need for this arises out of Token merging where we want to migrate any constraints on
   * a Token being merged to its active token. Thus, any variables of the Token being merged which
   * participate in a constraint shall be replaced with the equivalent variable in the active
   * token.
   * @param lookup A lookup table for variable in the merged token to corresponding variable in the active Token.
   * The key is a double, which is the encoding of the variable Id.
   * @param var The variable to be replaced, if found. It may not be in the map, indicating it is not a variable of the
   * merged token.
   * @return Variable Id Entry in lookup table if the given token is part of the token being merged,
   * otherwise returns var.
   * @todo The algorithm for splitting is very sub-optimal in the non-chronological case. Consider in the future
   * an algorithm to migrate consequenecs rather than force additional splits. This is similar to deactivation
   * of a token in its simple, robust, and potentially inefficient treatment of such non-chronological retractions.
   */
  ConstrainedVariableId checkForReplacement(const std::map<edouble, ConstrainedVariableId>& lookup, const ConstrainedVariableId& var){
    std::map<edouble, ConstrainedVariableId>::const_iterator it = lookup.find((edouble) var);
    if(it == lookup.end())
      return var;
    else
      return (it->second);
  }

  MergeMemento::MergeMemento(const TokenId& inactiveToken, const TokenId& activeToken)
    :m_inactiveToken(inactiveToken), m_activeToken(activeToken), m_undoing(false){

    checkError(inactiveToken.isValid(), inactiveToken);
    checkError(activeToken.isValid(), activeToken);

    // Iterate over all variables and impose unaries on spec domain and the corresponding variable in
    // Active Token.
    const std::vector<ConstrainedVariableId>& inactiveVariables = inactiveToken->getVariables();
    const std::vector<ConstrainedVariableId>& activeVariables = activeToken->getVariables();

    check_error(inactiveVariables.size() == activeVariables.size());

    std::map<edouble, ConstrainedVariableId> varMap;
    std::set<ConstraintId> deactivatedConstraints;

    //Exclude this for the state variable, which will necessarily conflict with the target active token
    for(unsigned int i=1; i<inactiveVariables.size(); i++){
      check_error(varMap.find((edouble) inactiveVariables[i]) == varMap.end());
      // Add to the map to support lookup and store all constraints on any variables
      varMap.insert(std::make_pair((edouble) inactiveVariables[i], activeVariables[i]));
      inactiveVariables[i]->constraints(deactivatedConstraints);
      // i.e. not a state variable

      // Post restrictions arising from the base domain
      activeVariables[i]->handleBase(inactiveVariables[i]->baseDomain());

      // if the variable is specified, then post its value to active variable
      if(inactiveVariables[i]->isSpecified()){
	checkError(inactiveVariables[i]->lastDomain().isSingleton(),
		   inactiveVariables[i]->toString() << " is specified but not a singleton. Pas possible!");
	activeVariables[i]->handleSpecified(inactiveVariables[i]->lastDomain().getSingletonValue());
      }

      // Deactivate variable
      inactiveVariables[i]->deactivate();
    }

    // Iterate over all constraints and deactivate them, as well as create and store new ones where necessary
    for(std::set<ConstraintId>::const_iterator it = deactivatedConstraints.begin(); it != deactivatedConstraints.end(); ++it){
      ConstraintId constraint = *it;
      // Standard constraints will not be migrated as they will be built in to the target already
      if(!m_inactiveToken->isStandardConstraint(constraint))
	migrateConstraint(constraint);
    }
  }

  MergeMemento::~MergeMemento() {}

  void MergeMemento::undo(bool activeTokenDeleted){
    checkError(activeTokenDeleted || m_activeToken.isValid(), m_activeToken);

    if(m_inactiveToken->isTerminated())
      return;

    // Start by removing all the new constraints that were created. To avoid a call back
    // into this method for synching data structures, we set a flag for undoing
    m_undoing = true;

    // If the active token is committed, and the merged token is being terminated, then we leave the new constraints.
    // othwreiwse we can nuke them
    discardAll(m_newConstraints);

    // Clear the deactivated ocnstraints
    m_deactivatedConstraints.clear();


    // Iterate over all active token variables and trigger a reset of the domain
    if(!activeTokenDeleted){
      const std::vector<ConstrainedVariableId>& activeVariables = m_activeToken->getVariables();
      for(unsigned int i=1;i<activeVariables.size();i++) // We skip the State Variable
	activeVariables[i]->handleReset();
    }

    // Iterate over all variables in this token and trigger a reset of the domain to force
    // re-evaluation
    const std::vector<ConstrainedVariableId>& inactiveVariables = m_inactiveToken->getVariables();
    for(unsigned int i=1;i<inactiveVariables.size();i++){ // We skip the State Variable
      inactiveVariables[i]->undoDeactivation();
      inactiveVariables[i]->handleReset();
    }
    m_undoing = false;
  }

  void MergeMemento::handleAdditionOfInactiveConstraint(const ConstraintId& constraint){
    debugMsg("europa:merging:handleAdditionOfInactiveConstraint", constraint->toString());
    migrateConstraint(constraint);
  }

  void MergeMemento::handleRemovalOfInactiveConstraint(const ConstraintId& constraint){
    check_error(m_deactivatedConstraints.size() + m_inactiveToken->getVariables().size() >= m_newConstraints.size());
    checkError(!constraint->isActive(), constraint->toString());

    if(m_undoing)
      return;

    // Iterate through the lists and delete when found
    std::list<ConstraintId>::iterator it_1 = m_deactivatedConstraints.begin();
    std::list<ConstraintId>::iterator it_2 = m_newConstraints.begin();

    while(it_1 != m_deactivatedConstraints.end()){
      if((*it_1) == constraint){
	ConstraintId newConstraint = *it_2;

	// Ensure that if a constraint was migrated, it has the same scope length at least.
	checkError(newConstraint.isNoId() || (newConstraint->getScope().size() == constraint->getScope().size()),
		   newConstraint->toString() << " does not match " << constraint->toString());

	// Remove from both lists at this location.
	m_deactivatedConstraints.erase(it_1);
	m_newConstraints.erase(it_2);

	// Now delete the new constraint which arose from migration, if it was migrated
	if(!newConstraint.isNoId())
	  newConstraint->discard();

	return;
      }

      it_1++;
      it_2++;
    }
  }

  void MergeMemento::migrateConstraint(const ConstraintId& constraint){
    checkError(constraint.isValid(), constraint);
    checkError(m_activeToken.isValid(), m_activeToken);
    checkError(m_activeToken->isActive(), m_activeToken->toString());


    // If it is not a standard constraint, then we need to create a surrogate as the target active token
    // may not have it already.
    if(!m_inactiveToken->isStandardConstraint(constraint)){
      debugMsg("europa:merging:migrateConstraint", "Replacing scope for " << constraint->toString());

      // Begin by constructing the scope
      const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
      std::vector<ConstrainedVariableId> newScope;
      for(unsigned int i=0;i<variables.size();i++){
	ConstrainedVariableId var = variables[i];

	// If not a variable of the source token, add directly. Note that we now include the state variable, wheras
	// we used to ignore constraints that included a reference to the state variable of an active token. This is because
	// the state variable is a very useful point from which events can be triggered and to prohibit such constraints
	// cuts off alot of functionality. However, this also means that constraints expresed to pin the state variable (e.g. state == MERGED)
	// should not be permitted since they will now be applied to the active token also. This is reasonable since constraining a token
	// to be merged is a bogus hack, which encodes a search control rule in the model.
	if(var->parent() != m_inactiveToken)
	  newScope.push_back(var);
	else {
	  ConstrainedVariableId newVar = m_activeToken->getVariables()[var->getIndex()];

	  checkError(newVar.isValid(), newVar << " is invalid for index " << var->getIndex() << " in " << m_activeToken->toString() <<
		     " from parent " << var->parent()->toString() << " and constraint " << constraint->toString());

	  newScope.push_back(newVar);
	}
      }


      debugMsg("europa:merging:migrateConstraint", "Creating replacement for " << constraint->toString());
      ConstraintId newConstraint;
      newConstraint = m_activeToken->getPlanDatabase()->getConstraintEngine()->createConstraint(constraint->getName(),newScope);

      // Now set the source on the new constraint to give opportunity to pass data
      newConstraint->setSource(constraint);
      m_newConstraints.push_back(newConstraint);
    }

    m_deactivatedConstraints.push_back(constraint);
  }
}
