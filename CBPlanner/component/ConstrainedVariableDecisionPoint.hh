#ifndef _H_ConstrainedVariableDecisionPoint
#define _H_ConstrainedVariableDecisionPoint

#include "DecisionPoint.hh"

namespace PLASMA {

  class ConstrainedVariableDecisionPoint : public DecisionPoint {
  public:
    virtual ~ConstrainedVariableDecisionPoint();

    const bool assign(const ChoiceId&);
    const bool retract();
    std::list<ChoiceId>& getChoices();
    const ConstrainedVariableId& getVariable() const;

    void print(std::ostream& os) const;
  private:
    friend class OpenDecisionManager;

    ConstrainedVariableDecisionPoint(const DbClientId& dbClient, const ConstrainedVariableId&);
    const bool testIfExhausted();
    ConstrainedVariableId m_var;
  };

std::ostream& operator <<(std::ostream& os, const Id<ConstrainedVariableDecisionPoint>&);

}
#endif
