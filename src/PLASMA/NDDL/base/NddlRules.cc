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
   * @brief Strip out values delimited in the string. Return by ref to avoid
   * unnecessary copying.
   */
  const std::list<edouble>& listFromString(const std::string& str, bool isNumeric){
    static const std::string sl_delimiter("$");
    static std::list<edouble> sl_values;
    sl_values.clear();
    std::vector<std::string> tokens;
    tokenize(str, tokens, sl_delimiter);
    for(std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      if(isNumeric)
	sl_values.push_back(toValue<edouble>(*it));
      else
	sl_values.push_back(LabelStr(*it));
    }
    return sl_values;
  }

  TokenId allocateOnSameObject(const TokenId& master, const LabelStr& predicateSuffix, const LabelStr& relationToMaster){
    std::string tokenType = master->getBaseObjectType().toString() + "." + predicateSuffix.toString();
    TokenId slave = master->getPlanDatabase()->createSlaveToken(master, tokenType, relationToMaster);
    return slave;
  }
}
