#ifndef _H_IntervalToken
#define _H_IntervalToken

#include "PlanDatabaseDefs.hh"
#include "Token.hh"

namespace Prototype {

  class IntervalToken: public Token {
  public:

    IntervalToken(const PlanDatabaseId& planDatabase, 
		  const LabelStr& predicateName, 
		  const BooleanDomain& rejectabilityBaseDomain,
		  const IntervalIntDomain& startBaseDomain,
		  const IntervalIntDomain& endBaseDomain,
		  const IntervalIntDomain& durationBaseDomain,
		  const LabelStr& objectName = Token::noObject(),
		  bool closed = true);

    IntervalToken(const TokenId& master, 
		  const LabelStr& predicateName, 
		  const BooleanDomain& rejectabilityBaseDomain,
		  const IntervalIntDomain& startBaseDomain,
		  const IntervalIntDomain& endBaseDomain,
		  const IntervalIntDomain& durationBaseDomain,
		  const LabelStr& objectName = Token::noObject(),
		  bool closed = true);
  private:
    void commonInit();
  };
}
#endif
