#include "IntervalToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "Domains.hh"
#include "ConstraintType.hh"

namespace EUROPA{

IntervalToken::IntervalToken(const PlanDatabaseId planDatabase,
                             const std::string& predicateName,
                             bool rejectable,
                             bool _isFact,
                             const IntervalIntDomain& startBaseDomain,
                             const IntervalIntDomain& endBaseDomain,
                             const IntervalIntDomain& durationBaseDomain,
                             const std::string& objectName,
                             bool closed)
:Token(planDatabase,
       predicateName,
       rejectable,
       _isFact,
       durationBaseDomain,
       objectName,
       false), m_start(), m_end(){
  commonInit(startBaseDomain, endBaseDomain, closed);
}

IntervalToken::IntervalToken(const TokenId _master,
                             const std::string& _relation,
                             const std::string& predicateName,
                             const IntervalIntDomain& startBaseDomain,
                             const IntervalIntDomain& endBaseDomain,
                             const IntervalIntDomain& durationBaseDomain,
                             const std::string& objectName,
                             bool closed)
    :Token(_master,
	   _relation,
	   predicateName,
	   durationBaseDomain,
	   objectName,
	   false), m_start(), m_end() {
  commonInit(startBaseDomain, endBaseDomain, closed);
}

  const TempVarId IntervalToken::start() const{
    checkError(m_start.isValid(), m_start);
    return m_start;}

  const TempVarId IntervalToken::end() const{
    checkError(m_end.isValid(), m_end);
    return m_end;
  }

  void IntervalToken::commonInit( const IntervalIntDomain& startBaseDomain,
				  const IntervalIntDomain& endBaseDomain,
				  bool closed){

    // Ensure non-zero duration for intervals. This is enforced by a base domain rather than a constraint
    check_error(m_duration->getBaseDomain().getLowerBound() > 0);


    m_start = (new TokenVariable<IntervalIntDomain>(m_id,
						    m_allVariables.size(),
						    m_planDatabase->getConstraintEngine(),
						    startBaseDomain,
						    false, // TODO: fixme
						    true,
						    "start"))->getId();
    m_allVariables.push_back(m_start);

    m_end = (new TokenVariable<IntervalIntDomain>(m_id,
						  m_allVariables.size(),
						  m_planDatabase->getConstraintEngine(),
						  endBaseDomain,
						  false, // TODO: fixme
						  true,
						  "end"))->getId();
    m_allVariables.push_back(m_end);

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(m_start);
    temp.push_back(m_duration);
    temp.push_back(m_end);

    ConstraintId temporalRelation =
        m_planDatabase->getConstraintEngine()->createConstraint("temporalDistance",temp);

    m_standardConstraints.insert(temporalRelation);

    if (closed)
      close();
  }
}
