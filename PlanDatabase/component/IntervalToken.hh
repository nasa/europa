#ifndef _H_IntervalToken
#define _H_IntervalToken

#include "PlanDatabaseDefs.hh"
#include "Token.hh"

namespace Prototype {

  class IntervalToken: public Token {
  public:

    IntervalToken(const PlanDatabaseId& planDatabase, 
		  const LabelStr& predicateName, 
		  const BooleanDomain& rejectabilityBaseDomain = BooleanDomain(),
		  const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(),
		  const LabelStr& objectName = Token::noObject(),
		  bool closed = true);

    IntervalToken(const TokenId& master, 
		  const LabelStr& predicateName, 
		  const BooleanDomain& rejectabilityBaseDomain = BooleanDomain(),
		  const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(),
		  const LabelStr& objectName = Token::noObject(),
		  bool closed = true);
  private:
    void commonInit();
  };
}
#endif
