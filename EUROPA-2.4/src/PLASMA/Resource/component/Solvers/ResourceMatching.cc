#include "ResourceMatching.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "Instant.hh"
#include "Profile.hh"
#include "Resource.hh"

namespace EUROPA {
  namespace SOLVERS {

    template<>
    void MatchingEngine::getMatches(const InstantId& inst,
				    std::vector<MatchingRuleId>& results) {
      m_cycleCount++;
      results = m_unfilteredRules;
      getMatchesInternal(inst, results);
    }

    template<>
    void MatchingEngine::getMatchesInternal(const InstantId& inst,
					    std::vector<MatchingRuleId>& results)
    {
      const ResourceId& res = inst->getProfile()->getResource();
      debugMsg("MatchingEngine:getMatchesInternal",
	       "Triggering matches for object types (" <<
	       res->getType().toString() << ")");
      const SchemaId& schema = res->getPlanDatabase()->getSchema();
      trigger(schema->getAllObjectTypes(res->getType()), m_rulesByObjectType, results);
    }

    void InstantMatchFinder::getMatches(const MatchingEngineId& engine, const EntityId& entity,
					std::vector<MatchingRuleId>& results) {
      engine->getMatches(InstantId(entity), results);
    }
  }
}
