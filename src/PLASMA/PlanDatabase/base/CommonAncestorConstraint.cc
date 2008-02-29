#include "CommonAncestorConstraint.hh"
#include "Object.hh"

namespace EUROPA{  

  CommonAncestorConstraint::CommonAncestorConstraint(const LabelStr& name,
						     const LabelStr& propagatorName,
						     const ConstraintEngineId& constraintEngine,
						     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_first(static_cast<ObjectDomain&>(getCurrentDomain(variables[0]))),
      m_second(static_cast<ObjectDomain&>(getCurrentDomain(variables[1]))),
      m_restrictions(static_cast<ObjectDomain&>(getCurrentDomain(variables[2]))) {
    check_error(variables.size() == 3);
  }

  CommonAncestorConstraint::~CommonAncestorConstraint(){}

  void CommonAncestorConstraint::handleExecute(){
    check_error(!m_second.isEmpty() && !m_second.isEmpty() && !m_restrictions.isEmpty());

    // If neither one is not a singleton, just ignore. Could do more, perhaps!
    if(!m_first.isSingleton() && !m_second.isSingleton())
      return;

    // Pick the singleton and apply filtering as neceesary.
    if(m_first.isSingleton())
      apply(m_first, m_second);
    else
      apply(m_second, m_first);
  }

  void CommonAncestorConstraint::handleExecute(const ConstrainedVariableId& variable, 
					       int argIndex, 
					       const DomainListener::ChangeType& changeType){
    handleExecute();
  }

  bool CommonAncestorConstraint::canIgnore(const ConstrainedVariableId& variable, 
					   int argIndex,
					   const DomainListener::ChangeType& changeType){
    check_error(argIndex <= 2);
    return (!m_first.isSingleton() && !m_second.isSingleton());
  }

  /**
   * Works by first generating the intersection of the restrictions and the ancestors of the singleton
   * value. Then it iterates over each ancestor of the other objects, and removes any that do not have an
   * encestor relation to at least one element of setOfAncesotrs.
   */
  void CommonAncestorConstraint::apply(ObjectDomain& singleton, ObjectDomain& other){
    static int counter(0);
    counter++;
    check_error(singleton.isSingleton());
    ObjectId singletonObject = singleton.getSingletonValue();
    //std::cout << counter << ":" << std::endl << singleton << ", " << other;

    // Get all singleton ancestors into a set we can use
    std::list<ObjectId> singletonAncestors;
    singletonObject->getAncestors(singletonAncestors);

    singletonAncestors.push_back(singletonObject); // Has at least one, itself

    ObjectDomain setOfAncestors(singletonAncestors, singletonObject->getRootType().c_str());
    //std::cout << ", " << setOfAncestors ;
    //std::cout << ", " << m_restrictions << std::endl;

    // Prune this set for those elements in the set of restrictions imposed
    setOfAncestors.intersect(m_restrictions);
    //std::cout << "Revised set of ancestors " << setOfAncestors << std::endl;
    if(setOfAncestors.isEmpty()){
      other.empty();
      return;
    }

    // Iterate over each value in the possible non-singleton domain, and check if it has a common ancestor
    std::list<double> candidateValues;
    other.getValues(candidateValues);
    for(std::list<double>::const_iterator it = candidateValues.begin(); it != candidateValues.end(); ++it){
      std::list<ObjectId> candidatesAncestors;
      ObjectId candidate = *it;
      check_error(candidate.isValid());
      candidate->getAncestors(candidatesAncestors);
      candidatesAncestors.push_back(candidate);
      ObjectDomain od(candidatesAncestors, candidate->getRootType().c_str());
      //std::cout << "Candidate " << candidate->getName().toString() << " has ancestors " << od << std::endl;
      od.intersect(setOfAncestors);
      if(od.isEmpty()){
	other.remove(candidate);
	//std::cout << "Removed " << candidate->getName().toString() << ". Result = " << other << std::endl;
	if(other.isEmpty()){
	  //std::cout << other << " is empty" << std::endl;
	  return;
	}
      }
    }
  }
}
