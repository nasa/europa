#include "NddlRules.hh"
#include "Utils.hh"
#include "TokenFactory.hh"
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
  const std::list<double>& listFromString(const std::string& str, bool isNumeric){
    static const std::string sl_delimiter("$");
    static std::list<double> sl_values;
    sl_values.clear();
    std::vector<std::string> tokens;
    tokenize(str, tokens, sl_delimiter);
    for(std::vector<std::string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      const char* value = it->c_str();
      if(isNumeric)
	sl_values.push_back(atof(value));
      else
	sl_values.push_back(LabelStr(value));
    }
    return sl_values;
  }

  TokenId allocateOnSameObject(const TokenId& master, const LabelStr& predicateSuffix, const LabelStr& relationToMaster){
    std::string tokenType = master->getBaseObjectType().toString() + "." + predicateSuffix.toString();
    TokenId slave = TokenFactory::createInstance(master, tokenType, relationToMaster);
    return slave;
  }
}
