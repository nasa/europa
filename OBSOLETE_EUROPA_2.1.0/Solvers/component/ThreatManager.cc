#include "ThreatManager.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include "Debug.hh"
#include "Utils.hh"

/**
 * @author Conor McGann
 * @date May, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    ThreatManager::ThreatManager(const TiXmlElement& configData)
      : FlawManager(configData){}

    ThreatManager::~ThreatManager(){}

    void ThreatManager::handleInitialize() {}

    /**
     * Filter out if not a token
     */
    bool ThreatManager::staticMatch(const EntityId& entity){
      return !TokenId::convertable(entity) || FlawManager::staticMatch(entity);
    }

    /**
     * Because we do not process threat candidates until we query the database, we have not yet statically matched
     * them. Thus we are able to use the dynamic match case in flaw iteration for all types of flaw managers.
     */
    bool ThreatManager::dynamicMatch(const EntityId& entity){
      return staticMatch(entity) || FlawManager::dynamicMatch(entity);
    }

    std::string ThreatManager::toString(const EntityId& entity) const {
      checkError(TokenId::convertable(entity), entity->toString());
      TokenId token = entity;
      std::stringstream os;
      os << "THREAT:" << token->toString();
      return os.str();
    }

    class ThreatIterator : public FlawIterator {
    public:
      ThreatIterator(ThreatManager& manager)
	: FlawIterator(manager), 
	  m_it(manager.getPlanDatabase()->getTokensToOrder().begin()), 
	  m_end(manager.getPlanDatabase()->getTokensToOrder().end()){
	advance();
      }

    private:
      const EntityId nextCandidate() {
	EntityId candidate;
	if(m_it != m_end){
	  candidate = m_it->second.first;
	  ++m_it;
	}
	return candidate;
      }

      std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator m_it;
      std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator m_end;
    };

    IteratorId ThreatManager::createIterator() {
      return (new ThreatIterator(*this))->getId();
    }
  }
}
