#ifndef _H_IntervalToken
#define _H_IntervalToken

#include "PlanDatabaseDefs.hh"
#include "Token.hh"

namespace Prototype {

  class IntervalToken: public Token {
  public:

    IntervalToken(const PlanDatabaseId& planDatabase, 
		  const LabelStr& predicateName, 
		  bool rejectable,
		  const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, MAX_INT),
		  const LabelStr& objectName = Token::noObject(),
		  bool closed = true);

    IntervalToken(const TokenId& master, 
		  const LabelStr& predicateName, 
		  const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, MAX_INT),
		  const LabelStr& objectName = Token::noObject(),
		  bool closed = true);

    const TempVarId& getStart() const;
    const TempVarId& getEnd() const;

  private:
    void commonInit(const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    bool closed);
    TempVarId m_start;
    TempVarId m_end;
  };
}
#endif
