#include "HasAncestorConstraint.hh"
#include "Object.hh"

namespace EUROPA{

  HasAncestorConstraint::HasAncestorConstraint(const LabelStr& name,
						     const LabelStr& propagatorName,
						     const ConstraintEngineId& constraintEngine,
						     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_first(static_cast<ObjectDomain&>(getCurrentDomain(variables[0]))),
      m_restrictions(static_cast<ObjectDomain&>(getCurrentDomain(variables[1]))) {
    check_error(variables.size() == 2);
  }

  HasAncestorConstraint::~HasAncestorConstraint(){}

  void HasAncestorConstraint::handleExecute(){

    if(m_first.isOpen() || m_restrictions.isOpen())
      return;

    apply();
  }

  void HasAncestorConstraint::handleExecute(const ConstrainedVariableId& variable,
					       int argIndex,
					       const DomainListener::ChangeType& changeType){
    handleExecute();
  }

  bool HasAncestorConstraint::canIgnore(const ConstrainedVariableId& variable,
					   int argIndex,
					   const DomainListener::ChangeType& changeType){
    check_error(argIndex <= 1);

    if(m_first.isOpen() || m_restrictions.isOpen())
      return true;

    return (false);
  }

  void HasAncestorConstraint::apply() {
    
    std::list<edouble> firstValues;
    m_first.getValues(firstValues);

    // Get all the ancestors into a set we can use
    std::list<ObjectId> allAncestors;
    for(std::list<edouble>::const_iterator it = firstValues.begin(); it != firstValues.end(); ++it){
      std::list<ObjectId> candidatesAncestors;
      ObjectId candidate = m_first.getObject(*it);
      candidate->getAncestors(candidatesAncestors);
      allAncestors.merge(candidatesAncestors);
    }

    allAncestors.unique();
    ObjectId object = allAncestors.front();
    const DataTypeId& dt = m_constraintEngine->getCESchema()->getDataType(object->getRootType().c_str());
    ObjectDomain setOfAncestors(dt,allAncestors);
    // Prune this set for those elements in the set of restrictions imposed
    setOfAncestors.intersect(m_restrictions);

    // Iterate over each value in the possible non-singleton domain, and check if it has a common ancestor
    std::list<edouble> candidateValues;
    m_first.getValues(candidateValues);
    for(std::list<edouble>::const_iterator it = candidateValues.begin(); it != candidateValues.end(); ++it){
      std::list<ObjectId> candidatesAncestors;
      ObjectId candidate = m_first.getObject(*it);
      candidate->getAncestors(candidatesAncestors);
      bool removeCandidate = true;
      for(std::list<ObjectId>::const_iterator it1 = candidatesAncestors.begin(); it1 != candidatesAncestors.end(); ++it1){
	ObjectId object = *it1;
	if(setOfAncestors.isMember(object)){ // Found common ancestor
	  removeCandidate = false;
	  break;
	}
      }

      // This could possibly be optimized to batch the removals down the road. For example, intersect operator
      if(removeCandidate){
	m_first.remove(candidate);
	if(m_first.isEmpty())
	  return;
      }
    }
  }
}
