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
    class InfiniteDynamicFilter : public MatchingRule {
    public:
      InfiniteDynamicFilter(const TiXmlElement& configData);
      bool matches(const EntityId& entity) const;
    };

    /**
     * @brief Will filter a variable out until its derived domain becomes a singleton.
     */
    class SingletonFilter: public MatchingRule {
    public:
      SingletonFilter(const TiXmlElement& configData);
      bool matches(const EntityId& entity) const;
    };


    /**
     * @brief Will filter a token based on the horizon.
     *
     * A number of policies are supported for applying the Horizon Test. They are:
     * @li PossiblyContained - for the token to be in the horizon, both the start and end times must 
     * intersect the horizon.
     * @li PartiallyContained - for the token to be in the horizon, there need only be some temporal overlap
     * between the temporal extent of the token and the horizon.
     * @li TotallyContained - for the token to be in the horizon, the temporal extend of the token must be a subset of
     * the horizon.
     */
    class HorizonFilter: public MatchingRule {
    public:
      HorizonFilter(const TiXmlElement& configData);
      bool matches(const EntityId& entity) const;
      std::string toString() const;
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
    class HorizonVariableFilter: public MatchingRule {
    public:
      HorizonVariableFilter(const TiXmlElement& configData);
      bool matches(const EntityId& entity) const;
      std::string toString() const;

    private:
      HorizonFilter m_horizonFilter;
    };
  }
}
#endif
