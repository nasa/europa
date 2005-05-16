#include "MatchingRule.hh"
#include "Object.hh"
#include "Token.hh"
#include "ConstrainedVariable.hh"
#include "SolverUtils.hh"
#include "Debug.hh"

/**
 * @author Conor McGann
 * @date May, 2005
 * @file Provides implementation for core rule matching classes.
 * @todo Enforce expected arguments for different XML structures
 */
namespace EUROPA {
  namespace SOLVERS {

    MatchingRule::MatchingRule(const TiXmlElement& configData): Component(configData), m_expression("NO_EXPRESSION"){}

    MatchingRule::~MatchingRule(){}

    void MatchingRule::setExpression(const std::string& expression){
      m_expression = expression;
      debugMsg("MatchingRule:setExpression", expression);
    }

    std::string MatchingRule::getExpression() const {return m_expression;}

    /** OBJECT MATCHING RULE **/

    ObjectMatchingRule::ObjectMatchingRule(const TiXmlElement& configData)
      : MatchingRule(configData){
      const char* argData = NULL;
      m_objectType = ((argData = configData.Attribute("class-match")) == NULL ? WILD_CARD() : LabelStr(argData));

      setExpression(m_objectType.toString());
    }

    bool ObjectMatchingRule::matches(const LabelStr& objectType) const {
      SchemaId schema = Schema::instance();

      if(m_objectType != WILD_CARD() && (objectType == EMPTY_LABEL() || !schema->isA(objectType, m_objectType)))
	 return false;

      return true;
    }

    /** TOKEN MATCHING RULE **/

    TokenMatchingRule::TokenMatchingRule(const TiXmlElement& configData)
      : ObjectMatchingRule(configData){
      const char* argData = NULL;
      m_objectType = ((argData = configData.Attribute("class-match")) == NULL ? WILD_CARD() : LabelStr(argData));
      m_predicate = ((argData = configData.Attribute("predicate-match")) == NULL ? WILD_CARD() : LabelStr(argData));

      setExpression(m_objectType.toString() + "." + m_predicate.toString());
    }

    bool TokenMatchingRule::matches(const LabelStr& objectType, const LabelStr& predicate) const {
      if(m_predicate != WILD_CARD() && (predicate == EMPTY_LABEL() || predicate != m_predicate))
	return false;

      return ObjectMatchingRule::matches(objectType);
    }

    void TokenMatchingRule::extractParts(const TokenId& token, LabelStr& objectType, LabelStr& predicate){
      objectType = token->getBaseObjectType();
      predicate = token->getPredicateName().getElement(1, ".");
    }

    std::string TokenMatchingRule::makeExpression(const TokenId& token){
      return(token->getPredicateName().toString());
    }

    /** VARIABLE MATCHING RULE **/

    VariableMatchingRule::VariableMatchingRule(const TiXmlElement& configData)
      : TokenMatchingRule(configData){
      const char* argData = NULL;
      m_var = ((argData = configData.Attribute("var-match")) == NULL ? WILD_CARD() : LabelStr(argData));

      setExpression(m_objectType.toString() + "." + m_predicate.toString() + "." + m_var.toString());
    }

    void VariableMatchingRule::extractParts(const ConstrainedVariableId& var, LabelStr& objectType, LabelStr& predicate){
      objectType = EMPTY_LABEL();
      predicate = EMPTY_LABEL();

      if(var->getParent().isNoId())
	return;

      if(ObjectId::convertable(var->getParent())){
	objectType = ObjectId(var->getParent())->getType();
	return;
      }

      TokenId token;
      if(TokenId::convertable(var->getParent()))
	token = var->getParent();
      else
	token = RuleInstanceId(var->getParent())->getToken();

      objectType = token->getBaseObjectType();

      // Strip qualifiers of necessary.
      if(token->getPredicateName().countElements(".") == 2)
	predicate = token->getPredicateName().getElement(1, ".");
      else
	predicate = token->getPredicateName();
    }

    bool VariableMatchingRule::matches(const LabelStr& varName, const LabelStr& objectType, const LabelStr& predicate) const {

      // Finally, if we care about the name
      if(m_var != WILD_CARD() && m_var != varName)
	return false;

      return TokenMatchingRule::matches(objectType, predicate);
    }

    std::string VariableMatchingRule::makeExpression(const ConstrainedVariableId& var){
      LabelStr objectType, predicate;
      extractParts(var, objectType, predicate);
      return(objectType.toString() + "." + predicate.toString() + "." + var->getName().toString());
    }
  }
}
