#ifndef H_FlawManager
#define H_FlawManager

/**
 * @file FlawManager.hh
 * @author Conor McGann
 * @date April, 2005
 * @brief Provides the declaration for FlawManager
 */

#include "SolverDefs.hh"
#include "EntityIterator.hh"
#include "SolverDecisionPoint.hh"
#include "DomainListener.hh"
#include "MatchingEngine.hh"
#include "FlawHandler.hh"
#include "hash_map.hh"

namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Provides access to a set of flaws in priority order.
     *
     * Derived classes will maintain access to the set of flaws under its control and
     * provide a selection mechanism to obtain the next best decision available. The Flaw
     * Manager will only return uninitialized decisions. If the client decides to use them
     * they should be initialized at that time.
     *
     * @note Extends Entity to take advantage of keys and the key based ordering.
     */
    class FlawManager: public Component {
    public:

      virtual ~FlawManager();

      const std::multimap<eint, ConstraintId> getFlawHandlerGuards() const {return m_flawHandlerGuards;}

      const PlanDatabaseId& getPlanDatabase() const {return m_db;}
      /**
       * @brief Initialize the constructed FlawManager.
       * @see handleInitialize for extension point.
       */
      void initialize(const TiXmlElement& configData, const PlanDatabaseId& db, const ContextId& ctx = ContextId::noId(), const FlawManagerId& parent = FlawManagerId::noId());


      /**
       * @brief True if the given entity is in scope. This is expensive.
       */
      bool inScope(const EntityId& entity);

      /**
       * @brief True if the filter criteria are matched
       * @see inScope, staticMatch, dynamicMatch
       */
      bool matches(const EntityId& entity);

      /**
       * @brief Obtain the priority for the given entity
       */
      Priority getPriority(const EntityId& entity);

      /**
       * @brief Obtains the first avaialble flaw that is forced i.e. a unit decision or a dead-end
       * @return An uninitialized DecisionPoint for a zero commitment flaw if available, otherwise a noId. 
       * @see next
       */
      virtual DecisionPointId nextZeroCommitmentDecision();

      /**
       * @brief Primary service provided to the planner. Seeks out the next best decision
       * that is a higher priority than the current best priority.
       * @param bestPriority The current best priority. May be reduced if a better decision found.
       * @return If a better decision found, it will be returned (uninitialized), otherwise return a noId.
       * @see nextZeroCommitmentDecision
       */
      virtual DecisionPointId next(Priority& bestPriority);

      /**
       * @brief Get an iterator for the set of Flaws
       * @return A Flaw iterator.  
       */
      virtual IteratorId createIterator();

      virtual void notifyAdded(const ConstraintId& constraint){}
      virtual void notifyRemoved(const ConstraintId& constraint);
      virtual void notifyRemoved(const ConstrainedVariableId& var);
      virtual void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType){}
      virtual void notifyAdded(const TokenId& token) {}
      virtual void notifyRemoved(const TokenId& token);

      /**
       * @brief Indicates that a flaw handler is now active, passing its guards
       */
      void notifyActivated(const EntityId& target, const FlawHandlerId& flawHandler);

      /**
       * @brief Indicates that a flaw handelr is no longer active
       */
      void notifyDeactivated(const EntityId& target, const FlawHandlerId& flawhandler);

      /**
       * @brief Retrieve the Flaw Handler for a given entity.
       */
      FlawHandlerId getFlawHandler(const EntityId& entity);

      /**
       * @brief Generates a flaw manager specific string description for an entity contained by the flaw.
       */
      virtual std::string toString(const EntityId& entity) const;

      /**
       * @brief test if the timestamp is current
       */
      bool inSynch() const;

      /**
       * @brief Update the timestamp
       */
      void synchronize();

      /**
       * Helper method to evaluate entity w.r.t static conditions only. Encapslates condition access.
       */
      virtual bool staticMatch(const EntityId& entity);

      /**
       * Helper method to evaluate entity w.r.t dynamic conditions only. Encapslates condition access.
       */
      virtual bool dynamicMatch(const EntityId& entity);

      ContextId getContext() const {return m_context;}

    protected:

      FlawManager(const TiXmlElement& configData);

      /**
       * @brief Factory method to allocate instance for selected decision point
       */
      DecisionPointId allocateDecisionPoint(const EntityId& entity, const LabelStr& explanation);      

      /**
       * @brief Subclass to implement in order to trigger database dependent data
       */
      virtual void handleInitialize() = 0;

      virtual bool betterThan(const EntityId& a, const EntityId& b, LabelStr& explanation);

      PlanDatabaseId m_db;

    private:
      bool staticallyExcluded(const EntityId& entity) const;
      bool isValid() const;

      FlawManagerId m_parent;
      MatchingEngineId m_flawFilters;
      MatchingEngineId m_flawHandlers;
      std::map<eint, bool> m_staticFiltersByKey; /*!< Summary of static filter outcome for the entity */
      /*std::map<unsigned int, std::vector<FlawFilterId> > m_dynamicFiltersByKey;*/ /*!< Dynamic conditions for the entity */
      __gnu_cxx::hash_map<eint, std::vector<FlawFilterId> > m_dynamicFiltersByKey;
      std::multimap<eint, ConstraintId> m_flawHandlerGuards; /*!< Flaw Handler Guard constraints by Entity Key */
      std::map<eint, FlawHandlerEntry> m_activeFlawHandlersByKey; /*!< Applicable Flaw Handlers for each entity */
      unsigned int m_timestamp; /*!< Used for testing for stale iterators */
      ContextId m_context;
      //static const Priority BEST_CASE_PRIORITY = 0;
    };

    /**
     * @brief Declares an abstract base class for handling flaw iteration
     */
    class FlawIterator : public Iterator {
    public:
      FlawIterator(FlawManager& manager);
      bool done() const;
      const EntityId next();
      unsigned int visited() const {return m_visited;}
    protected:
      void advance();
      virtual const EntityId nextCandidate() = 0;
    private:
      unsigned int m_visited;
      bool m_done;
      FlawManager& m_manager;
      EntityId m_flaw;
    };
  }
}

#define REGISTER_FLAW_MANAGER(MGR,CLASS, NAME) REGISTER_COMPONENT_FACTORY(MGR,CLASS, NAME)

#endif
