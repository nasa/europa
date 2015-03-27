#ifndef H_IntervalToken
#define H_IntervalToken

#include "PlanDatabaseDefs.hh"
#include "Token.hh"

namespace EUROPA {
 /**
   * @class IntervalToken 
   * @brief to be written
   */
  class IntervalToken: public Token {
  public:

    IntervalToken(const PlanDatabaseId planDatabase, 
		  const std::string& predicateName, 
		  bool rejectable,
		  bool isFact,
		  const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		  const std::string& objectName = Token::noObject(),
		  bool closed = true);

    IntervalToken(const TokenId master, 
		  const std::string& relation,
		  const std::string& predicateName, 
		  const IntervalIntDomain& startBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& endBaseDomain = IntervalIntDomain(),
		  const IntervalIntDomain& durationBaseDomain = IntervalIntDomain(1, PLUS_INFINITY),
		  const std::string& objectName = Token::noObject(),
		  bool closed = true);

    const TempVarId start() const;
    const TempVarId end() const;

  private:
    void commonInit(const IntervalIntDomain& startBaseDomain,
		    const IntervalIntDomain& endBaseDomain,
		    bool closed);
    TempVarId m_start;
    TempVarId m_end;
  };
}
#endif
