#include "IntervalToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "../ConstraintEngine/ConstraintLibrary.hh"

namespace Prototype{

  IntervalToken::IntervalToken(const PlanDatabaseId& planDatabase, 
			       const LabelStr& predicateName, 
			       const BooleanDomain& rejectabilityBaseDomain,
			       const IntervalIntDomain& startBaseDomain,
			       const IntervalIntDomain& endBaseDomain,
			       const IntervalIntDomain& durationBaseDomain,
			       const LabelStr& objectName,
			       bool closed)
    :Token(planDatabase, 
	   predicateName, 
	   rejectabilityBaseDomain,
	   durationBaseDomain,
	   objectName,
	   closed){
    commonInit(startBaseDomain, endBaseDomain);
  }

  IntervalToken::IntervalToken(const TokenId& m_master, 
			       const LabelStr& predicateName, 
			       const BooleanDomain& rejectabilityBaseDomain,
			       const IntervalIntDomain& startBaseDomain,
			       const IntervalIntDomain& endBaseDomain,
			       const IntervalIntDomain& durationBaseDomain,
			       const LabelStr& objectName,
			       bool closed)
    :Token(m_master, 
	   predicateName, 
	   rejectabilityBaseDomain,
	   durationBaseDomain,
	   objectName,
	   closed){
    commonInit(startBaseDomain, endBaseDomain);
  }

  const TempVarId& IntervalToken::getStart() const{return m_start;}

  const TempVarId& IntervalToken::getEnd() const{return m_end;}

  void IntervalToken::commonInit( const IntervalIntDomain& startBaseDomain,
				  const IntervalIntDomain& endBaseDomain){

    // Ensure non-zero duration for intervals. This is enforced by a base domain rather than a constraint
    check_error(m_duration->getBaseDomain().getLowerBound() > 0);


    m_start = (new TokenVariable<IntervalIntDomain>(m_id,
						    m_allVariables.size(),
						    m_planDatabase->getConstraintEngine(), 
						    startBaseDomain))->getId();
    m_allVariables.push_back(m_start);

    m_end = (new TokenVariable<IntervalIntDomain>(m_id,
						  m_allVariables.size(),
						  m_planDatabase->getConstraintEngine(), 
						  endBaseDomain))->getId();
    m_allVariables.push_back(m_end);

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(m_start);
    temp.push_back(m_duration);
    temp.push_back(m_end);
    ConstraintId temporalRelation = 
      ConstraintLibrary::createConstraint(LabelStr("StartEndDurationRelation"), m_planDatabase->getConstraintEngine(), temp);
    m_standardConstraints.insert(temporalRelation);
  }
}
