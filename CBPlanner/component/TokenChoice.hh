#ifndef _H_TokenChoice
#define _H_TokenChoice

#include "CBPlannerDefs.hh"
#include "Choice.hh"

namespace Prototype {

  class TokenChoice : public Choice {
  public:
    virtual ~TokenChoice();
    
    const TokenId& getToken() const;

    bool operator==(const Choice& choice) const;

    void print(std::ostream& os) const;

    double getValue() const;
  private:
    friend class Choice;

    TokenChoice(const DecisionPointId&, const TokenId&);

    TokenId m_token;
  };

}
#endif 
