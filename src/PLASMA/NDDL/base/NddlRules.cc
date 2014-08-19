#include "NddlRules.hh"
#include "Utils.hh"
#include "Token.hh"

/**
 * @file Provides implementations for utilities employed in integration with
 * the rules engine.
 * @author Conor McGann
 * @date December, 2004
 */
namespace NDDL {

  /**
   * @brief Strip out values delimited in the string.
   */
  std::list<edouble> listFromString(const std::string& str, bool isNumeric){
    static const std::string sl_delimiter("$");
    std::list<edouble> values;
    std::vector<std::string> tokens;
    tokenize(str, tokens, sl_delimiter);
    for(std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      if(isNumeric)
	values.push_back(toValue<edouble>(*it));
      else
	values.push_back(LabelStr(*it));
    }
    return values;
  }

  TokenId allocateOnSameObject(const TokenId& master, const LabelStr& predicateSuffix, const LabelStr& relationToMaster){
    std::string tokenType = master->getBaseObjectType().toString() + "." + predicateSuffix.toString();
    TokenId slave = master->getPlanDatabase()->createSlaveToken(master, tokenType, relationToMaster);
    return slave;
  }
}
