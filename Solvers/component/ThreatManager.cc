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

    bool ThreatManager::inScope(const EntityId& entity) {
      checkError(TokenId::convertable(entity), entity->toString());
      TokenId token = entity;
      return token->isActive() && FlawManager::inScope(entity);
    }

    IteratorId ThreatManager::createIterator(){
      return (new ThreatManager::FlawIterator(*this))->getId();
    }

    ThreatManager::FlawIterator::FlawIterator(ThreatManager& manager)
      : m_visited(0), m_timestamp(manager.m_db->getConstraintEngine()->cycleCount()),
	m_manager(manager), m_it(manager.m_db->getTokensToOrder().begin()), 
	m_end(manager.m_db->getTokensToOrder().end()) {

      // Must advance to the first available flaw in scope.
      while(!done()){
	TokenId tok = m_it->second.first;
	if(m_manager.inScope(tok))
	  break;
	else
	  ++m_it;
      }
    }

    bool ThreatManager::FlawIterator::done() const {
      return m_it == m_end;
    }

    const EntityId ThreatManager::FlawIterator::next() {
      check_error(m_manager.m_db->getConstraintEngine()->cycleCount() == m_timestamp,
		  "Error: potentially stale flaw iterator.");
      checkError(!done(), "Cannot be done when you call next.");

      // Pick up the flaw for the current position
      TokenId flaw = m_it->second.first;
      checkError(m_manager.inScope(flaw), "Not advancing correctly.");
      ++m_visited;

      // Advance till we get another hit
      ++m_it;
      while(!done()){
	TokenId tok = m_it->second.first;
	if(m_manager.inScope(tok))
	  break;
	else
	  ++m_it;
      }

      return flaw;
    }


    std::string ThreatManager::toString(const EntityId& entity) const {
      checkError(TokenId::convertable(entity), entity->toString());
      TokenId token = entity;
      std::stringstream os;
      os << "THREAT:" << token->toString();
      return os.str();
    }
  }
}
