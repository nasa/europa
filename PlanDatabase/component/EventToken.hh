#ifndef _H_EventToken
#define _H_EventToken

#include "PlanDatabaseDefs.hh"
#include "IntervalToken.hh"

namespace Prototype {

  /**
   * @class EventToken 
   * @brief A Token with zero duration. May have temporal flexibility
   */
  class EventToken: public IntervalToken {
  public:
    EventToken(const PlanDatabaseId& planDatabase,
	       const LabelStr& predicateName,
	       const BooleanDomain& rejectabilityBaseDomain,
	       const std::vector<ConstrainedVariableId> parameters,
	       const IntervalIntDomain& timeBaseDomain,
	       const LabelStr& objectName = Token::s_noObject);
  };

}

#endif
