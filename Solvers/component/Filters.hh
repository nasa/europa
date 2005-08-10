#ifndef H_Filters
#define H_Filters

#include "MatchingRule.hh"
#include "IntervalIntDomain.hh"

/**
 * @file Provides Declarations for useful flaw filter components
 * @author Conor McGann
 * @date May, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Will filter a variable out until its derived domain is closed
     * and finite.
     */
    class InfiniteDynamicFilter : public VariableMatchingRule {
    public:
      InfiniteDynamicFilter(const TiXmlElement& configData);
      bool matches(const ConstrainedVariableId& var) const;
      std::string getExpression() const;
    };

    /**
     * @brief Will filter a variable out until its derived domain becomes a singleton.
     */
    class SingletonFilter: public VariableMatchingRule {
    public:
      SingletonFilter(const TiXmlElement& configData);
      bool matches(const ConstrainedVariableId& var) const;
    };


    /**
     * @brief Will filter a token based on the horizon
     */
    class HorizonFilter: public TokenMatchingRule {
    public:
      HorizonFilter(const TiXmlElement& configData);
      bool matches(const TokenId& token) const;
      std::string getExpression() const;
      /**
       * @brief Allowed policy strings for customization
       */
      static const LabelStr& policies(){
	static const LabelStr sl_policies("PossiblyContained:PartiallyContained:TotallyContained:");
	return sl_policies;
      }

      /**
       * @brief uses a singleton horizon that is regularly adjusted.
       */
      static IntervalIntDomain& getHorizon();

    private:

      LabelStr m_policy;
    };
  }
}
#endif
