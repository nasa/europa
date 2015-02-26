#include "NddlRules.hh"
#include "Utils.hh"
#include "Token.hh"

/**
 * @file Provides implementations for utilities employed in integration with
 * the rules engine.
 * @author Conor McGann
 * @date December, 2004
 */
using namespace EUROPA;
namespace NDDL {


TokenId allocateOnSameObject(const TokenId master, const std::string& predicateSuffix,
                             const std::string& relationToMaster){
    std::string tokenType = master->getBaseObjectType() + "." + predicateSuffix;
    TokenId slave = master->getPlanDatabase()->createSlaveToken(master, tokenType, relationToMaster);
    return slave;
  }
}
