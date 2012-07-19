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

      bool staticMatch(const EntityId& entity);

      bool dynamicMatch(const EntityId& entity);

      IteratorId createIterator();

      std::string toString(const EntityId& entity) const;

    private:
      friend class ThreatIterator;
      void handleInitialize();
    };

  }
}
#endif
