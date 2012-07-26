#ifndef H_UnboundVariableManager
#define H_UnboundVariableManager

#include "FlawManager.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "EntityIterator.hh"

/**
 * @brief Provides class declaration for handling variable flaws.
 * @author Conor McGann
 */
namespace EUROPA {
  namespace SOLVERS {

    class UnboundVariableManager: public FlawManager {
    public:
      UnboundVariableManager(const TiXmlElement& configData);

      DecisionPointId nextZeroCommitmentDecision();

      bool staticMatch(const EntityId& entity);
      bool dynamicMatch(const EntityId& entity);

      IteratorId createIterator();

      std::string toString(const EntityId& entity) const;

    private:
      friend class UnboundVariableIterator;

      void notifyRemoved(const ConstrainedVariableId& var);
      void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);
      void handleInitialize();
      void addFlaw(const ConstrainedVariableId& var);
      void removeFlaw(const ConstrainedVariableId& var);
      void updateFlaw(const ConstrainedVariableId& var);
      bool betterThan(const EntityId& a, const EntityId& b, LabelStr& explanation);

      /**
       * @brief Utility to test if the given variable is part of a token that is merged, rejected or inactive.
       */
      static bool variableOfNonActiveToken(const ConstrainedVariableId& var);


      ConstrainedVariableSet m_flawCandidates; /*!< All variables that have passed the static filter */
    };
  }
}
#endif
