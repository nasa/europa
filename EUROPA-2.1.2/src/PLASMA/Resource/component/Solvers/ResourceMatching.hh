#ifndef _H_ResourceMatching
#define _H_ResourceMatching

#include "MatchingEngine.hh"
#include "SAVH_ResourceDefs.hh"

namespace EUROPA {
  namespace SOLVERS {
    /**
     * @brief Retrieves all relevant matching rules for the given instant.
     */
    template<>
    void MatchingEngine::getMatches(const SAVH::InstantId& inst,
				    std::vector<MatchingRuleId>& results);

    template<>
    void MatchingEngine::getMatchesInternal(const SAVH::InstantId& inst,
					    std::vector<MatchingRuleId>& results);

    class InstantMatchFinder : public MatchFinder {
    public:
      void getMatches(const MatchingEngineId& engine, const EntityId& entity,
		      std::vector<MatchingRuleId>& results);
    };
  }
}

#endif
