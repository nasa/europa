#ifndef H_EventToken
#define H_EventToken

#include "PlanDatabaseDefs.hh"
#include "IntervalToken.hh"

namespace EUROPA {

  /**
   * @class EventToken 
   * @brief A Token with zero duration. May have temporal flexibility
   */
  class EventToken: public Token {
  public:
    EventToken(const PlanDatabaseId planDatabase,
	       const std::string& predicateName,
	       bool rejectable,
	       bool isFact,
	       const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
	       const std::string& objectName = Token::noObject(),
	       bool closed = true);

    EventToken(const TokenId master,
	       const std::string& relation,
	       const std::string& predicateName,
	       const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
	       const std::string& objectName = Token::noObject(),
	       bool closed = true);

    const TempVarId start() const;
    const TempVarId end() const;
    const TempVarId getTime() const;
  private:
    void commonInit(const IntervalIntDomain& timeBaseDomain);
    TempVarId m_time;
  };

}

#endif
