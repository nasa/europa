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
      virtual std::string getExpression() const;

    protected:
      void setExpression(const std::string& expression);

    private:
      MatchingRule(const MatchingRule&); /*!< NO IMPL */
      std::string m_expression; /*!< Stores the matching expression as a string for display */
    };

    class ObjectMatchingRule: public MatchingRule {
    public:
      /**
       * @brief Construct a condition from an XML node
       */
      ObjectMatchingRule(const TiXmlElement& configData);

      /**
       * @brief Used to test of the given objectType is matched by this rule
       */
      bool matches(const LabelStr& objectType) const;

      /**
       * @brief True if the object is matched by the expression. Used as a point of extendibility.
       */
      virtual bool matches(const ObjectId& object) const {return true;}

    private:
      LabelStr m_objectType;
    };
  
    class TokenMatchingRule: public ObjectMatchingRule {
    public:
      /**
       * @brief Construct a condition from an XML node
       */
      TokenMatchingRule(const TiXmlElement& configData);

      /**
       * @brief Used to test a specific token for a match against the internal match expression.
       * @param objectType The object type of the parent token, rule instance or object, if it has such a parent.
       * @param predicate The predicate of token
       * @return true if a match has been found, otherwise false.
       */
      bool matches(const LabelStr& objectType, const LabelStr& predicate) const;

      /**
       * @brief A hook to add additional matching requirements in a derived class
       * @param token The token to be tested.
       */
      virtual bool matches(const TokenId& token) const {return true;}

      static std::string makeExpression(const TokenId& token);

      static void extractParts(const TokenId& token, LabelStr& objectType, LabelStr& predicate);

    private:
      LabelStr m_objectType;
      LabelStr m_predicate;
    };

    class VariableMatchingRule: public TokenMatchingRule {
    public:
      /**
       * @brief Construct a condition from an XML node
       */
      VariableMatchingRule(const TiXmlElement& configData);

      /**
       * @brief Used to test a specific variable for a match against the internal match expression.
       * @param varName The variable to be evaluated.
       * @param objectType The object type of the parent token, rule instance or object, if it has such a parent.
       * @param predicate The predicate of the parent token or rule instance, if it has suc a parent.
       * @return true if a match has been found, otherwose false.
       * @see extractParts
       */
      bool matches(const LabelStr& varName, const LabelStr& objectType, const LabelStr& predicate) const;

      /**
       * @brief Hook for additional macthing requirements to be added in a derived class.
       */
      virtual bool matches(const ConstrainedVariableId& var) const {return true;}

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

  }
}
#endif
