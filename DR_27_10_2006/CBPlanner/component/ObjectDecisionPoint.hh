#ifndef _H_ObjectDecisionPoint
#define _H_ObjectDecisionPoint

#include "CBPlannerDefs.hh"
#include "DecisionPoint.hh"
#include "Token.hh"
#include "Object.hh"
#include <vector>

namespace EUROPA {

  class ObjectDecisionPoint : public DecisionPoint {
  public:
    virtual ~ObjectDecisionPoint();

    bool assign();
    bool retract();
    bool hasRemainingChoices();

    const TokenId& getToken() const;

    void print(std::ostream& os) const;

    ObjectDecisionPoint(const DbClientId& dbClient, const TokenId&, const OpenDecisionManagerId& odm);
  private:
    friend class OpenDecisionManager;
    friend class HSTSOpenDecisionManager;

    std::vector< std::pair< ObjectId, std::pair<TokenId, TokenId> > > m_choices;
    unsigned int m_choiceIndex;

    TokenId m_token;
    OpenDecisionManagerId m_odm;
  };

  std::ostream& operator <<(std::ostream& os, const Id<ObjectDecisionPoint>&);

}
#endif
