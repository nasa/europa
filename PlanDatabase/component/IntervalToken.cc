#include "IntervalToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "./ConstraintEngine/IntervalIntDomain.hh"
#include "./ConstraintEngine/ConstraintLibrary.hh"

namespace Prototype{

  IntervalToken::IntervalToken(const PlanDatabaseId& planDatabase, 
		const LabelStr& predicateName, 
		const BooleanDomain& rejectabilityBaseDomain,
		const IntervalIntDomain& startBaseDomain,
		const IntervalIntDomain& endBaseDomain,
		const IntervalIntDomain& durationBaseDomain,
		const std::vector<ConstrainedVariableId> parameters,
		const LabelStr& objectName)
    :Token(planDatabase, predicateName, parameters, objectName), m_parameters(parameters){

    check_error(durationBaseDomain.getLowerBound() > 0); // Prohibit non-zero durations at this level

    m_rejectability = (new TokenVariable<IntervalIntDomain>(m_id, 
							    m_planDatabase->getConstraintEngine(), 
							    rejectabilityBaseDomain))->getId();
    m_variables.push_back(m_rejectability);

    m_start = (new TokenVariable<IntervalIntDomain>(m_id, 
						    m_planDatabase->getConstraintEngine(), 
						    startBaseDomain))->getId();
    m_variables.push_back(m_start);

    m_end = (new TokenVariable<IntervalIntDomain>(m_id, 
						  m_planDatabase->getConstraintEngine(), 
						  endBaseDomain))->getId();
    m_variables.push_back(m_end);

    m_duration = (new TokenVariable<IntervalIntDomain>(m_id, 
						       m_planDatabase->getConstraintEngine(), 
						       durationBaseDomain))->getId();
    m_variables.push_back(m_duration);

    {
      std::vector<ConstrainedVariableId> temp;
      temp.push_back(m_start);
      temp.push_back(m_duration);
      temp.push_back(m_end);
      ConstraintId enforceTemporalRelation = 
	ConstraintLibrary::createConstraint(LabelStr("StartEndDurationRelation"), m_planDatabase->getConstraintEngine(), temp);

      m_localConstraints.insert(enforceTemporalRelation);
    }
  }

  IntervalToken::IntervalToken(const PlanDatabaseId& planDatabase, 
			       const LabelStr& predicateName, 
			       const std::vector<ConstrainedVariableId> parameters,
			       const LabelStr& objectName)
    :Token(planDatabase, predicateName, parameters, objectName), m_parameters(parameters){}


  IntervalToken::~IntervalToken(){}

  const BoolVarId& IntervalToken::getRejectability() const{return m_rejectability;}
  const TempVarId& IntervalToken::getStart() const{return m_start;}
  const TempVarId& IntervalToken::getEnd() const{return m_end;}
  const TempVarId& IntervalToken::getDuration() const{return m_duration;}
  const std::vector<ConstrainedVariableId>& IntervalToken::getParameters() const {return m_parameters;}
}
