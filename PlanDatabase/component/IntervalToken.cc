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
			       const LabelStr& objectName)
    :Token(planDatabase, 
	   predicateName, 
	   rejectabilityBaseDomain,
	   startBaseDomain,
	   endBaseDomain,
	   durationBaseDomain,
	   objectName){
    commonInit();
  }


  IntervalToken::IntervalToken(const TokenId& m_master, 
			       const LabelStr& predicateName, 
			       const BooleanDomain& rejectabilityBaseDomain,
			       const IntervalIntDomain& startBaseDomain,
			       const IntervalIntDomain& endBaseDomain,
			       const IntervalIntDomain& durationBaseDomain,
			       const LabelStr& objectName)
    :Token(m_master, 
	   predicateName, 
	   rejectabilityBaseDomain,
	   startBaseDomain,
	   endBaseDomain,
	   durationBaseDomain,
	   objectName){
    commonInit();
  }

  void IntervalToken::commonInit(){
    // Ensure non-zero duration for intervals.
    check_error(m_duration->getBaseDomain().getLowerBound() > 0);

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(m_start);
    temp.push_back(m_duration);
    temp.push_back(m_end);
    ConstraintId enforceTemporalRelation = 
      ConstraintLibrary::createConstraint(LabelStr("StartEndDurationRelation"), m_planDatabase->getConstraintEngine(), temp);

    m_localConstraints.insert(enforceTemporalRelation);
  }
}
