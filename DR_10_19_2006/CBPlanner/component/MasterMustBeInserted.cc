#include "MasterMustBeInserted.hh"
#include "Token.hh"
#include "PlanDatabase.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {

  MasterMustBeInserted::MasterMustBeInserted(const DecisionManagerId& dm) : Condition(dm){}

  MasterMustBeInserted::~MasterMustBeInserted() {}

  bool MasterMustBeInserted::test(const EntityId& entity) {
    check_error(entity.isValid());
    if(TokenId::convertable(entity))
      return executeTest(TokenId(entity));
    else if (ConstrainedVariableId::convertable(entity))
      return executeTest(ConstrainedVariableId(entity));
    else
      return true;
  }

  /**
   * If the token is INACTIVE and it has a master which has not yet been inserted
   * then we should exclude it. Otherwise allow it.
   */
  bool MasterMustBeInserted::executeTest(const TokenId& token) {
    if(token->isInactive() && token->getMaster().isId() && !token->getMaster()->isAssigned())
      return false;
    else
      return true;
  }

  /**
   * If the variable is on an inactive token then filter it
   */
  bool MasterMustBeInserted::executeTest(const ConstrainedVariableId& var) {
    if(var->getParent().isId() && TokenId::convertable(var->getParent())){
      TokenId token(var->getParent());
      return token->isAssigned();
    }
    else
      return true;
  }


}

