#include "EventToken.hh"
#include "TokenVariable.hh"
#include "PlanDatabase.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "../ConstraintEngine/ConstraintLibrary.hh"

namespace Prototype{

  EventToken::EventToken(const PlanDatabaseId& planDatabase,
			 const LabelStr& predicateName,
			 const BooleanDomain& rejectabilityBaseDomain,
			 const IntervalIntDomain& timeBaseDomain,
			 const LabelStr& objectName,
			 bool closed)
    :Token(planDatabase, predicateName, 
	   rejectabilityBaseDomain,
	   IntervalIntDomain(0, 0), 
	   objectName,
	   closed){
    commonInit(timeBaseDomain);
  }

  EventToken::EventToken(const TokenId& master,
			 const LabelStr& predicateName,
			 const BooleanDomain& rejectabilityBaseDomain,
			 const IntervalIntDomain& timeBaseDomain,
			 const LabelStr& objectName,
			 bool closed)
    :Token(master, predicateName, 
	   rejectabilityBaseDomain,
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
						    timeBaseDomain))->getId();
    m_allVariables.push_back(m_time);
  }
}
