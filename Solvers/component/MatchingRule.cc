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

    const std::string& MatchingRule::getExpression() const {return m_expression;}

    /** VARIABLE MATCHING RULE **/

    VariableMatchingRule::VariableMatchingRule(const TiXmlElement& configData)
      : MatchingRule(configData){
      const char* argData = NULL;
      m_objectType = ((argData = configData.Attribute("class-match")) == NULL ? WILD_CARD() : LabelStr(argData));
      m_predicate = ((argData = configData.Attribute("predicate-match")) == NULL ? WILD_CARD() : LabelStr(argData));
      m_var = ((argData = configData.Attribute("var-match")) == NULL ? WILD_CARD() : LabelStr(argData));

      setExpression(m_objectType.toString() + "." + m_predicate.toString() + "." + m_var.toString());
    }

    VariableMatchingRule::~VariableMatchingRule(){}


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

      objectType = token->getObject()->baseDomain().getTypeName();

      // Strip qualifiers of necessary.
      if(token->getPredicateName().countElements(".") == 2)
	predicate = token->getPredicateName().getElement(1, ".");
      else
	predicate = token->getPredicateName();
    }

    bool VariableMatchingRule::matches(const ConstrainedVariableId& var, const LabelStr& objectType, const LabelStr& predicate) const {
      SchemaId schema = Schema::instance();

      if(m_objectType != WILD_CARD() && (objectType == EMPTY_LABEL() || !schema->isA(objectType, m_objectType)))
	 return false;

      if(m_predicate != WILD_CARD() && (predicate == EMPTY_LABEL() || predicate != m_predicate))
	return false;

      // Finally, if we care about the name
      if(m_var != WILD_CARD() && m_var != var->getName())
	return false;

      // If we get here, we have a match.
      return true;
    }

    std::string VariableMatchingRule::makeExpression(const ConstrainedVariableId& var){
      LabelStr objectType, predicate;
      extractParts(var, objectType, predicate);
      return(objectType.toString() + "." + predicate.toString() + "." + var->getName().toString());
    }
  }
}
