#include "EventToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "./ConstraintEngine/IntervalIntDomain.hh"
#include "./ConstraintEngine/ConstraintLibrary.hh"

namespace Prototype{

  EventToken::EventToken(const PlanDatabaseId& planDatabase,
			 const LabelStr& predicateName,
			 const BooleanDomain& rejectabilityBaseDomain,
			 const IntervalIntDomain& timeBaseDomain,
			 const LabelStr& objectName)
    :Token(planDatabase, predicateName, 
	   rejectabilityBaseDomain,
	   timeBaseDomain, 
	   timeBaseDomain, 
	   IntervalIntDomain(0, 0), 
	   objectName){

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(m_start);
    temp.push_back(m_end);

    ConstraintId enforceEquality = 
      ConstraintLibrary::createConstraint(LabelStr("CoTemporal"), m_planDatabase->getConstraintEngine(), temp);

    m_localConstraints.insert(enforceEquality);
  }
}
