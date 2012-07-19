#include "ResourceMatching.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Resource.hh"
#include "Schema.hh"

namespace EUROPA {
  namespace SOLVERS {

    template<>
    void MatchingEngine::getMatches(const SAVH::InstantId& inst,
				    std::vector<MatchingRuleId>& results) {
      m_cycleCount++;
      results = m_unfilteredRules;
      getMatchesInternal(inst, results);
    }

    template<>
    void MatchingEngine::getMatchesInternal(const SAVH::InstantId& inst,
					    std::vector<MatchingRuleId>& results) {
      debugMsg("MatchingEngine:getMatchesInternal",
	       "Triggering matches for object types (" <<
	       inst->getProfile()->getResource()->getType().toString() << ")");
      trigger(Schema::instance()->getAllObjectTypes(inst->getProfile()->getResource()->getType()), m_rulesByObjectType, results);
    }

    void InstantMatchFinder::getMatches(const MatchingEngineId& engine, const EntityId& entity,
					std::vector<MatchingRuleId>& results) {
      engine->getMatches(SAVH::InstantId(entity), results);
    }

    class ResourceMatchingLocalStatic {
    public:
      ResourceMatchingLocalStatic() {
	static bool sl_registerMatcher = false;
	checkError(!sl_registerMatcher, "Should only be called once.");
	MatchingEngine::addMatchFinder(SAVH::Instant::entityTypeName(),
				       (new InstantMatchFinder())->getId());
      }
    };

    ResourceMatchingLocalStatic sl_resourceMatchingLocalStatic;
  }
}
