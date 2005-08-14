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
     * @brief Will filter a token based on the horizon.
     *
     * A number of policies are supported for applying the Horizon Tesy. They are:
     * @li PossiblyContained - for the token to be in the horizon, both the start and end times must 
     * intersect the horizon.
     * @li PartiallyContained - for the token to be in the horizon, there need only be some temporal overlap
     * between the temporal extent of the token and the horizon.
     * @li TotallyContained - for the token to be in the horizon, the temporal extend of the token must be a subset of
     * the horizon.
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

    /**
     * @brief Will compose the Horizon Filter already available for a token in order to ensure
     * that the token for the variable is actually in the horizon.
     */
    class HorizonVariableFilter: public VariableMatchingRule {
    public:
      HorizonVariableFilter(const TiXmlElement& configData);
      bool matches(const ConstrainedVariableId& var) const;
      std::string getExpression() const;

    private:
      HorizonFilter m_horizonFilter;
    };
  }
}
#endif
