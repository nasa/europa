#ifndef _H_ValueChoice
#define _H_ValueChoice

#include "CBPlannerDefs.hh"
#include "Choice.hh"

namespace PLASMA {

  class ValueChoice : public Choice {
  public:
    virtual ~ValueChoice();
    
    double getValue() const;
    const TokenId& getToken() const;

    bool operator==(const Choice& choice) const;

    void print(std::ostream& os) const;

  private:
    friend class TokenDecisionPoint;
    friend class ConstrainedVariableDecisionPoint;

    static void makeChoices(const DecisionPointId&, const AbstractDomain&, std::list<ChoiceId>&);

    ValueChoice(const DecisionPointId&, const double);
    ValueChoice(const DecisionPointId&, const double, const TokenId&);

    void printValue(std::ostream& os) const;

    double m_value;		/**< One value of a domain. */
    TokenId m_token;
  };

}
#endif 
