#include "EventToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "./ConstraintEngine/IntervalIntDomain.hh"
#include "./ConstraintEngine/ConstraintLibrary.hh"

namespace Prototype{

  EventToken::EventToken(const PlanDatabaseId& planDatabase,
			 const LabelStr& predicateName,
			 const BooleanDomain& rejectabilityBaseDomain,
			 const std::vector<ConstrainedVariableId> parameters,
			 const IntervalIntDomain& timeBaseDomain,
			 const LabelStr& objectName)
    :IntervalToken(planDatabase, predicateName, parameters, objectName){

    m_rejectability = (new TokenVariable<IntervalIntDomain>(m_id, 
							    m_planDatabase->getConstraintEngine(), 
							    rejectabilityBaseDomain))->getId();
    m_variables.push_back(m_rejectability);

    m_start = (new TokenVariable<IntervalIntDomain>(m_id, 
						    m_planDatabase->getConstraintEngine(), 
						    timeBaseDomain))->getId();
    m_variables.push_back(m_start);

    m_end = (new TokenVariable<IntervalIntDomain>(m_id, 
						  m_planDatabase->getConstraintEngine(), 
						  timeBaseDomain))->getId();
    m_variables.push_back(m_end);

    m_duration = (new TokenVariable<IntervalIntDomain>(m_id, 
						       m_planDatabase->getConstraintEngine(), 
						       IntervalIntDomain(0, 0)))->getId();
    m_variables.push_back(m_duration);

    {
      std::vector<ConstrainedVariableId> temp;
      temp.push_back(m_start);
      temp.push_back(m_end);
      ConstraintId enforceEquality = 
	ConstraintLibrary::createConstraint(LabelStr("CoTemporal"), m_planDatabase->getConstraintEngine(), temp);

      m_localConstraints.insert(enforceEquality);
    }
  }


}
