#ifndef H_Filters
#define H_Filters

#include "FlawFilter.hh"
#include "Domains.hh"

/**
 * @file Provides Declarations for useful flaw filter components
 * @author Conor McGann
 * @date May, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Filters out parameters
     */
    class ParameterFilter : public FlawFilter {
    public:
      ParameterFilter(const TiXmlElement& configData);
      virtual bool test(const EntityId& entity);
    };

    /**
     * @brief Filters out "local variables" (non-parameters)
     */
    class LocalVariableFilter : public FlawFilter{
    public:
      LocalVariableFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
    };

    /**
     * @brief Filters out guards
     */
    class GuardFilter : public FlawFilter {
    public:
      GuardFilter(const TiXmlElement& configData);
      virtual bool test(const EntityId& entity);
    };

    /**
     * @brief Filters out non-guards
     */
    class NotGuardFilter : public GuardFilter {
    public:
      NotGuardFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
    };

    /**
     * @brief Will filter a variable out until its derived domain is closed
     * and finite.
     */
    class InfiniteDynamicFilter : public FlawFilter {
    public:
      InfiniteDynamicFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
    };

    /**
     * @brief Will filter a variable out until its derived domain becomes a singleton.
     */
    class SingletonFilter: public FlawFilter {
    public:
      SingletonFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
    };

    /**
     * @brief Will filter a variable out until its parent token is assigned.
     */
    class TokenMustBeAssignedFilter: public FlawFilter {
    public:
      TokenMustBeAssignedFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
    };


    /**
     * @brief Will filter a token out until it's parent token is assigned.
     */
    class MasterMustBeAssignedFilter : public FlawFilter {
    public:
      MasterMustBeAssignedFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
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
    class HorizonFilter: public FlawFilter {
    public:
      HorizonFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
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
    class HorizonVariableFilter: public FlawFilter {
    public:
      HorizonVariableFilter(const TiXmlElement& configData);
      bool test(const EntityId& entity);
      std::string toString() const;

    private:
      HorizonFilter m_horizonFilter;
    };
  }
}
#endif
