#ifndef _H_EventToken
#define _H_EventToken

#include "PlanDatabaseDefs.hh"
#include "IntervalToken.hh"

namespace Prototype {

  /**
   * @class EventToken 
   * @brief A Token with zero duration. May have temporal flexibility
   */
  class EventToken: public Token {
  public:
    EventToken(const PlanDatabaseId& planDatabase,
	       const LabelStr& predicateName,
	       const BooleanDomain& rejectabilityBaseDomain,
	       const IntervalIntDomain& timeBaseDomain,
	       const LabelStr& objectName = Token::noObject());

    EventToken(const TokenId& master,
	       const LabelStr& predicateName,
	       const BooleanDomain& rejectabilityBaseDomain,
	       const IntervalIntDomain& timeBaseDomain,
	       const LabelStr& objectName = Token::noObject());
  private:
    void commonInit();
  };

}

#endif
