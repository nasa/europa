#ifndef _H_TokenDecisionPoint
#define _H_TokenDecisionPoint

#include "DecisionPoint.hh"
#include "Token.hh"
#include <vector>

namespace EUROPA {

  class TokenDecisionPoint : public DecisionPoint {
  public:
    virtual ~TokenDecisionPoint();

    bool assign();
    bool retract();
    bool hasRemainingChoices();
    const TokenId& getToken() const;

    std::vector<LabelStr>& getChoices();

    unsigned int getCurrentChoice() const;

    void print(std::ostream& os) const;

    TokenDecisionPoint(const DbClientId& dbClient, const TokenId&, const OpenDecisionManagerId& odm);
  protected:
    TokenId m_tok; /**< cached for efficiency */
  private:
    friend class OpenDecisionManager;
    friend class HSTSOpenDecisionManager;


    std::vector<LabelStr> m_choices;
    unsigned int m_choiceIndex; /*< keeps a pointer to current choice */
    std::vector<TokenId> m_compatibleTokens; 
    unsigned int m_mergeIndex; /*< keeps a pointer to current compatible token */
    OpenDecisionManagerId m_odm; /*< to reference the algorithm for initializing choices */
    bool m_initialized;
  };

std::ostream& operator <<(std::ostream& os, const Id<TokenDecisionPoint>&);

}
#endif
