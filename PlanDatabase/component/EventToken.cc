#include "EventToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "IntervalIntDomain.hh"
#include "ConstraintLibrary.hh"

namespace PLASMA{

  EventToken::EventToken(const PlanDatabaseId& planDatabase,
			 const LabelStr& predicateName,
			 bool rejectable,
			 const IntervalIntDomain& timeBaseDomain,
			 const LabelStr& objectName,
			 bool closed)
    :Token(planDatabase, predicateName, 
	   rejectable,
	   IntervalIntDomain(0, 0), 
	   objectName,
	   closed){
    commonInit(timeBaseDomain);
  }

  EventToken::EventToken(const TokenId& master,
			 const LabelStr& predicateName,
			 const IntervalIntDomain& timeBaseDomain,
			 const LabelStr& objectName,
			 bool closed)
    :Token(master, predicateName, 
	   IntervalIntDomain(0, 0), 
	   objectName,
	   closed){
    commonInit(timeBaseDomain);
  }

  const TempVarId& EventToken::getStart() const{return m_time;}
  const TempVarId& EventToken::getEnd() const{return m_time;}
  const TempVarId& EventToken::getTime() const{return m_time;}

  void EventToken::commonInit(const IntervalIntDomain& timeBaseDomain){
    m_time = (new TokenVariable<IntervalIntDomain>(m_id,
						   m_allVariables.size(),
						   m_planDatabase->getConstraintEngine(), 
						   timeBaseDomain,
						   true,
						   LabelStr("Time")))->getId();
    m_allVariables.push_back(m_time);
  }
}
