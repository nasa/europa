#ifndef H_MatchingRule
#define H_MatchingRule

/**
 * @brief Declares a base class and specific derived classes for providing rule matching capabilities against
 * the context of an object, token or variable.
 * @author Conor McGann
 */

#include "SolverDefs.hh"
#include "ComponentFactory.hh"

namespace EUROPA {
  namespace SOLVERS {

    class MatchingRule: public Component {
    public:
      /**
       * @brief Construct a condition
       */
      MatchingRule(const TiXmlElement& configData);

      virtual ~MatchingRule();

      /**
       * @brief Retrieves a string expression for the scope over which this filter is evaluated.
       */
      const std::string& getExpression() const;

    protected:
      void setExpression(const std::string& expression);

    private:
      MatchingRule(const MatchingRule&); /*!< NO IMPL */
      std::string m_expression; /*!< Stores the matching expression as a string for display */
    };

    class VariableMatchingRule: public MatchingRule {
    public:
      /**
       * @brief Construct a condition from an XML node
       */
      VariableMatchingRule(const TiXmlElement& configData);

      virtual ~VariableMatchingRule();

      /**
       * @brief Used to test a specific variable for a match against the internal match expression.
       * @param var The variable to be evaluated.
       * @param objectType The object type of the parent token, rule instance or object, if it has such a parent.
       * @param predicate The predicate of the parent token or rule instance, if it has suc a parent.
       * @return true if a match has been found, otherwose false.
       * @note The object type and predicate are passed redundantly to improve efficiency.
       * @see extractParts
       */
      virtual bool matches(const ConstrainedVariableId& var, const LabelStr& objectType, const LabelStr& predicate) const;

      /**
       * @brief Utility to extract the parts of the variable used for rule matching to expedite the process.
       * @param var The variable to extract from.
       * @param objectType An output of the object type that it is contained by. If global, then this will be EMPTY.
       * @param predicate An output of the unqualified predicate name.
       */
      static void extractParts(const ConstrainedVariableId& var, LabelStr& objectType, LabelStr& predicate);

      /**
       * @brief Utility to generate and expression for the variable which reflects the input to matching.
       * @param var The variable that is the source of the expression
       * @return An expression indicating the compositional path to the variable
       * @see extractParts
       */
      static std::string makeExpression(const ConstrainedVariableId& var);

    private:
      LabelStr m_objectType;
      LabelStr m_predicate;
      LabelStr m_var;
    };
  
    class TokenMatchingRule: public MatchingRule {
    public:
      /**
       * @brief Construct a condition from an XML node
       */
      TokenMatchingRule(const TiXmlElement& configData);

      virtual ~TokenMatchingRule();

      /**
       * @brief Test a specific token against this rule.
       * @param token The token to be tested.
       * @param objectType The objectType of the token.
       * @return true if matched, otherwise false.
       * @note The objectType is redundantly provided to improve efficiency
       * @see getObjectType
       */
      bool matches(const TokenId& token, const LabelStr& objectType) const;

      static const LabelStr& getObjectType(const TokenId& token);

    private:
      LabelStr m_objectType;
      LabelStr m_predicate;
    };

    class ObjectMatchingRule: public MatchingRule {
    public:
      /**
       * @brief Construct a condition from an XML node
       */
      ObjectMatchingRule(const TiXmlElement& configData);

      virtual ~ObjectMatchingRule();

      /**
       * @brief True if the object is matched by the expression.
       */
      bool matches(const ObjectId& object) const;

    private:
      LabelStr m_objectType;
    };
  }
}
#endif
