#ifndef _H_TokenDecisionPoint
#define _H_TokenDecisionPoint

#include "DecisionPoint.hh"
#include "Token.hh"
#include <vector>

namespace EUROPA {

  class TokenDecisionPoint : public DecisionPoint {
  public:
    virtual ~TokenDecisionPoint();

    const bool assign();
    const bool retract();
    const bool hasRemainingChoices();
    const TokenId& getToken() const;

    std::vector<LabelStr>& getChoices();

    unsigned int getCurrentChoice() const;

    void print(std::ostream& os) const;
  protected:
    TokenDecisionPoint(const DbClientId& dbClient, const TokenId&, const OpenDecisionManagerId& odm);
  private:
    friend class OpenDecisionManager;
    friend class DefaultOpenDecisionManager;
    friend class HSTSOpenDecisionManager;

    void initializeChoices();

    TokenId m_tok; /**< cached for efficiency */
    std::vector<LabelStr> m_choices;
    unsigned int m_choiceIndex; /*< keeps a pointer to current choice */
    std::vector<TokenId> m_compatibleTokens; 
    unsigned int m_mergeIndex; /*< keeps a pointer to current compatible token */
    OpenDecisionManagerId m_odm; /*< to reference the algorithm for initializing choices */
  };

std::ostream& operator <<(std::ostream& os, const Id<TokenDecisionPoint>&);

}
#endif
