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

      bool inScope(const EntityId& entity);

      IteratorId createIterator();

      std::string toString(const EntityId& entity) const;

    private:
      void handleInitialize();

      class FlawIterator : public Iterator {
      public:
	FlawIterator(ThreatManager& manager);
	bool done() const;
	const EntityId next();
	unsigned int visited() const {return m_visited;}
      protected:
      private:
	unsigned int m_visited;
	unsigned int m_timestamp;
	ThreatManager& m_manager;
	std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator m_it;
	std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator m_end;
      };
    };

  }
}
#endif
