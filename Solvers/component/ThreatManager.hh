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

      bool inScope(const EntityId& entity) const;
    private:
      virtual DecisionPointId next(unsigned int priorityLowerBound, unsigned int& bestPriority);
      void handleInitialize();
    };

  }
}
#endif
