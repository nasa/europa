#ifndef _H_TokenDecisionPoint
#define _H_TokenDecisionPoint

#include "DecisionPoint.hh"

namespace PLASMA {

  class TokenDecisionPoint : public DecisionPoint {
  public:
    enum State {
      INCOMPLETE = 0,
      INACTIVE,
      ACTIVE,
      MERGED,
      REJECTED
    };

    virtual ~TokenDecisionPoint();

    const bool assign(const ChoiceId&);
    const bool retract();
    std::list<ChoiceId>& getChoices();
    const TokenId& getToken() const;

    void print(std::ostream& os) const;
  private:
    friend class OpenDecisionManager;
    friend class Choice;

    TokenDecisionPoint(const DbClientId& dbClient, const TokenId&);
    TokenId m_tok; /**< cached for efficiency */
  };

std::ostream& operator <<(std::ostream& os, const Id<TokenDecisionPoint>&);

}
#endif
