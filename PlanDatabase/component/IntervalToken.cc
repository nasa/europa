#include "IntervalToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "IntervalIntDomain.hh"
#include "ConstraintLibrary.hh"

namespace EUROPA{

  IntervalToken::IntervalToken(const PlanDatabaseId& planDatabase, 
			       const LabelStr& predicateName,
			       bool rejectable,
			       const IntervalIntDomain& startBaseDomain,
			       const IntervalIntDomain& endBaseDomain,
			       const IntervalIntDomain& durationBaseDomain,
			       const LabelStr& objectName,
			       bool closed)
    :Token(planDatabase, 
	   predicateName, 
	   rejectable,
	   durationBaseDomain,
	   objectName,
	   false){
    commonInit(startBaseDomain, endBaseDomain, closed);
  }

  IntervalToken::IntervalToken(const TokenId& m_master, 
			       const LabelStr& m_relation,
			       const LabelStr& predicateName,
			       const IntervalIntDomain& startBaseDomain,
			       const IntervalIntDomain& endBaseDomain,
			       const IntervalIntDomain& durationBaseDomain,
			       const LabelStr& objectName,
			       bool closed)
    :Token(m_master, 
	   m_relation,
	   predicateName, 
	   durationBaseDomain,
	   objectName,
	   false){
    commonInit(startBaseDomain, endBaseDomain, closed);
  }

  const TempVarId& IntervalToken::getStart() const{
    check_error(m_start.isValid());
    return m_start;}

  const TempVarId& IntervalToken::getEnd() const{
    check_error(m_end.isValid());
    return m_end;}

  void IntervalToken::commonInit( const IntervalIntDomain& startBaseDomain,
				  const IntervalIntDomain& endBaseDomain,
				  bool closed){

    // Ensure non-zero duration for intervals. This is enforced by a base domain rather than a constraint
    check_error(m_duration->getBaseDomain().getLowerBound() > 0);


    m_start = (new TokenVariable<IntervalIntDomain>(m_id,
						    m_allVariables.size(),
						    m_planDatabase->getConstraintEngine(), 
						    startBaseDomain,
						    true,
						    LabelStr("start")))->getId();
    m_allVariables.push_back(m_start);

    m_end = (new TokenVariable<IntervalIntDomain>(m_id,
						  m_allVariables.size(),
						  m_planDatabase->getConstraintEngine(), 
						  endBaseDomain,
						  true,
						  LabelStr("end")))->getId();
    m_allVariables.push_back(m_end);

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(m_start);
    temp.push_back(m_duration);
    temp.push_back(m_end);
    ConstraintId temporalRelation = 
      ConstraintLibrary::createConstraint(LabelStr("StartEndDurationRelation"), m_planDatabase->getConstraintEngine(), temp);
    m_standardConstraints.insert(temporalRelation);
    if (closed)
      close();
  }
}
