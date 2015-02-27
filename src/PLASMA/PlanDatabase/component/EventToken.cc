#include "EventToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "Domains.hh"
#include "ConstraintType.hh"

namespace EUROPA{

EventToken::EventToken(const PlanDatabaseId planDatabase,
                       const std::string& predicateName,
                       bool rejectable,
                       bool _isFact,
                       const IntervalIntDomain& timeBaseDomain,
                       const std::string& objectName,
                       bool closed)
    :Token(planDatabase, predicateName,
	   rejectable,
	   _isFact,
	   IntervalIntDomain(0, 0),
	   objectName,
	   closed), m_time() {
  commonInit(timeBaseDomain);
}

  EventToken::EventToken(const TokenId _master,
			 const std::string& relation,
			 const std::string& predicateName,
			 const IntervalIntDomain& timeBaseDomain,
			 const std::string& objectName,
			 bool closed)
    :Token(_master, relation, predicateName,
	   IntervalIntDomain(0, 0),
	   objectName,
	   closed), m_time() {
    commonInit(timeBaseDomain);
  }

  const TempVarId EventToken::start() const{return m_time;}
  const TempVarId EventToken::end() const{return m_time;}
  const TempVarId EventToken::getTime() const{return m_time;}

  void EventToken::commonInit(const IntervalIntDomain& timeBaseDomain){
    m_time = (new TokenVariable<IntervalIntDomain>(m_id,
						   m_allVariables.size(),
						   m_planDatabase->getConstraintEngine(),
						   timeBaseDomain,
						   false, // TODO: fixme
						   true,
						   "time"))->getId();
    m_allVariables.push_back(m_time);
  }
}
