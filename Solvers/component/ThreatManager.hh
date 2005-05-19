#ifndef H_ThreatManager
#define H_ThreatManager


#include "SolverDefs.hh"
#include "FlawManager.hh"
#include "ThreatDecisionPoint.hh"

/**
 * @author Conor McGann
 * @date May, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Responsible for handling object flaws i.e. induced ordering constraints arising from
     * assignment or possible assignment to an object.
     */
    class ThreatManager: public FlawManager {
    public:
      ThreatManager(const TiXmlElement& configData);

      virtual ~ThreatManager();

      bool inScope(const TokenId& token) const;
    private:
      virtual DecisionPointId next(unsigned int priorityLowerBound, unsigned int& bestPriority);
      DecisionPointId allocateDecisionPoint(const TokenId& tokenToOrder);
      void handleInitialize();


      /**
       * @brief Helper method to iterate over the rules to match
       */
      bool matches(const TokenId& token, const std::list<TokenMatchingRuleId>& rules) const;

      /**
       * @brief Helper method to obtain the most restrictive decision point factory
       */
      ThreatDecisionPointFactoryId matchFactory(const TokenId& token) const;

      std::list<TokenMatchingRuleId> m_staticMatchingRules;
      std::list<TokenMatchingRuleId> m_dynamicMatchingRules;
      std::list<ThreatDecisionPointFactoryId> m_factories;
    };

  }
}
#endif
