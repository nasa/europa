#ifndef H_FlawHandler
#define H_FlawHandler

/**
 * @author Conor McGann
 */

#include "MatchingRule.hh"
#include "SolverDecisionPoint.hh"
#include "Constraint.hh"
#include <vector>

namespace EUROPA {
  namespace SOLVERS {
  
  
    typedef std::multimap<double, FlawHandlerId> FlawHandlerEntry;

    /**
     * @brief Factory for allocating DecisionPoint instances. Key extensions from a MatchingRule provide
     * behavior for allocation of a decision point from an entity and for storing initial configuration
     * data so that it can be accessed durng actual decision point allocation.
     * @see DecisionPoint
     */
    class FlawHandler: public MatchingRule {
    public:
      virtual ~FlawHandler();

      /**
       * @brief Main factory method
       */
      virtual DecisionPointId create(const DbClientId& client, const EntityId& flawedEntity, const LabelStr& explanation) const = 0;

      /**
       * @brief Get the prority
       */
      virtual Priority getPriority(const EntityId& entity = EntityId::noId());

      /**
       * @brief Retrieves a weight based on the number of criteria satisfied. Used to select the most specific
       * active flaw handler.
       */
      double getWeight() const;


      /**
       * @brief Tests for a match between this factory and the entity
       */
      virtual bool customStaticMatch(const EntityId& entity) const {return true;}

      /**
       * @brief True if there are any guards posted on this heuristic.
       */
      bool hasGuards() const;

      /**
       * @brief Evaluates dynamic matching - i.e. guards against variables
       */
      bool test(const std::vector<ConstrainedVariableId>& scope);

      /**
       * @brief Helper method to make the scope from guard data
       * @return true if successful in obtaining all the guards required.
       */
      bool makeConstraintScope(const EntityId& entity, std::vector<ConstrainedVariableId>& scope) const;

      /**
       * @brief Useful utility for eye-balling what the heuristic actually is.
       */
      virtual std::string toString() const;
 
      /**
       * @brief Handly default vector for no guards
       */
      static const std::vector< GuardEntry >& noGuards();

      /**
       * @brief Helper method to do value conversions for object domains. This one does the work.
       */
      static double convertValueIfNecessary(const PlanDatabaseId& db,
                                            const ConstrainedVariableId& guardVar,
                                            const double& testValue);

      static const unsigned int WEIGHT_BASE() {return 100000;} /*!< Used to weight active instances. 
                                                                 Establishes upper limit on priority values.*/


      friend class FlawManager;
      
      // This constraint notifies the FlawManager when a Guard on a FlawHandler is satisfied
      class VariableListener: public Constraint {
      public:
        /**
         * @brief Standard constraint constructor must be provided to facilitate
         * creation of a copy during merging.
         */
        VariableListener(const LabelStr& name,
                         const LabelStr& propagatorName,
                         const ConstraintEngineId& constraintEngine, 
                         const std::vector<ConstrainedVariableId>& variables);

        /**
         * @brief Specilized constructor also provided to create from the Heuristics Engine
         */
        VariableListener(const ConstraintEngineId& ce,
                         const EntityId& target,
                         const FlawManagerId& flawManager,
                         const FlawHandlerId& flawHandler,
                         const std::vector<ConstrainedVariableId>& scope);

        /**
         * @brief Standard constraint name
         */
        static const LabelStr& CONSTRAINT_NAME(){
          static const LabelStr sl_const("FlawListener");
          return sl_const;
        }

        /**
         * @brief Standard constraint name
         */
        static const LabelStr& PROPAGATOR_NAME(){
          static const LabelStr sl_const("Default");
          return sl_const;
        }
	
        const FlawHandlerId getHandler() const {return m_flawHandler;}
        const EntityId getTarget() const {return m_target;}
      private:
        void handleExecute();
        bool isApplied() const;
        void apply();
        void undo();

        const EntityId m_target;
        const FlawManagerId m_flawManager;
        const FlawHandlerId m_flawHandler;
        bool m_isApplied;
      };


    protected:

      /**
       * @brief Constructor
       * @param configData XML element of type <variable-heuristic>. May have child elements
       * particular to each derived class.
       */
      FlawHandler(const TiXmlElement& configData);

      TiXmlElement* m_configData;

      Priority m_priority; /*!< The priority for the flaw. Used in flaw ordering */
      double m_weight; /*!< An input to determine the weight of the flaw. Used to tie-break when priorities are equal */
      const std::vector< GuardEntry > m_guards; /*!< <index, value> tuples */
      const std::vector< GuardEntry > m_masterGuards; /*!< <index, value> tuples */

 

    private:
      /**
       * @brief Helper method to read the guards from XML element
       */
      static const std::vector<GuardEntry>& readGuards(const TiXmlElement& configData, bool forMaster);

      /**
       * @brief Helper method to get a double encoded value
       */
      static double readValue(const char* data);

      /**
       * @brief Helper method to stringify a guard
       */
      static std::string toString(const GuardEntry& entry);

      /**
       * @brief Helper method to test a guard value
       */
      bool matches(const ConstrainedVariableId& guardVar, const double& testValue);

      /**
       * @brief Helper to get a token, if possible, from an entity (token, or variable)
       */
      static TokenId getTokenFromEntity(const EntityId& entity);

      static TiXmlElement* makeConfigData(const TiXmlElement& configData);

      static TiXmlElement* s_element; /*!< Temporary holder for copied elements */

      const PlanDatabaseId& getPlanDatabase(const ConstrainedVariableId& tokenVar);

      PlanDatabaseId m_db;

    }; 

    /**
     * @brief Declares a Template class for concrete decision point factories
     */
    template<class CLASS>
    class ConcreteFlawHandler: public FlawHandler{
    public:
      ConcreteFlawHandler(const TiXmlElement& config)
        : FlawHandler(config){}

      DecisionPointId create(const DbClientId& client, const EntityId& flaw, const LabelStr& explanation) const {
        DecisionPoint* dp = new CLASS(client, flaw, *FlawHandler::m_configData, explanation);
        dp->setContext(m_context);
        return dp->getId();
      }

      /**
       * @brief Tests for a match between this factory and the entity
       */
      bool customStaticMatch(const EntityId& entity) const {
        return CLASS::customStaticMatch(entity);
      }

      /**
       * @brief Extends static filter count to permit a weight to be applied if custom filtering
       * is used. A future feature place holder!
       */
      unsigned int staticFilterCount() const {
        return CLASS::customStaticFilterCount() + FlawHandler::staticFilterCount();
      }
    };
  }
}

#define REGISTER_FLAW_HANDLER(CLASS, NAME)\
new EUROPA::SOLVERS::Component::ConcreteFactory< EUROPA::SOLVERS::ConcreteFlawHandler<CLASS> >(#NAME);

#define REGISTER_DECISION_FACTORY(CLASS, NAME) REGISTER_FLAW_HANDLER(CLASS, NAME)

#endif
