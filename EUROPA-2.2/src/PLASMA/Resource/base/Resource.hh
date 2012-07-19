#ifndef _H_Resource
#define _H_Resource

/**
 * @file Resource.hh
 * @author Michael Iatauro
 * @brief Defines the base class for the Resource object model.
 * @date   March, 2006
 * @ingroup Resource
*/

#include "ResourceDefs.hh"
#include "Object.hh"
#include "Constraint.hh"
#include "PlanDatabaseDefs.hh"
#include "PSResource.hh"

namespace EUROPA {

// Labels to associate with enum values:
  const static char* problemLabels[] = { "ProductionRateExceeded",
		  "ConsumptionRateExceeded",
		  "ProductionSumExceeded",
		  "ConsumptionSumExceeded",
		  "LevelTooHigh",
		  "LevelTooLow"
  };


  /**
     * @class Resource
     * @brief The base class for different Resource implementations.
     *
     * The Resource is provided as a base class from which a variety of implementations for similar resources or
     * a variety of resources can be derived.
     */
    class Resource : public Object, public virtual PSResource {
    public:

      /**
       * @brief Constructor.  This is primarily a convenience constructor for resource interactions outside of a model.
       * @param planDatabase @see Object
       * @param type @see Object
       * @param name @see Object
       * @param initCapacityLb The lower bound of the Resource's initial level. (For example, a battery may be only half charged initially.)
       * @param initCapacityUb The upper bound of the Resource's initial level. (For example, a battery may be only half charged initially.)
       * @param lowerLimit The lower capacity limit. (For example, a battery can't have less than no energy stored, or it might be considered unsafe to allow
       * the capacity to get below 25%.)
       * @param upperLimit The upper capacity limit. (For example, a battery has a maximum charge, or it might be unsafe to fill a beaker more than three-quarters full.)
       * @param maxInstProduction The maximum amount of production possible at an instant. (For example, a power bus might only allow 2.5A to be drawn at one instant.)
       * @param maxInstConsumption The maximum amount of consumption possible at an instant. (For example, a power bus might only allow 2.5A to be drawn at one instant.)
       * @param maxProduction The maximum amount of production possible on this resource.
       * @param maxConsumption The maximum amount of consumption possible on this resource.
       */
      Resource(const PlanDatabaseId& planDatabase,
               const LabelStr& type,
               const LabelStr& name,
               const LabelStr& detectorName,
               const LabelStr& profileName,
	           double initCapacityLb = 0,
	           double initCapacityUb = 0,
	           double lowerLimit = MINUS_INFINITY,
	           double upperLimit = PLUS_INFINITY,
	           double maxInstProduction = PLUS_INFINITY,
	           double maxInstConsumption = PLUS_INFINITY,
	           double maxProduction = PLUS_INFINITY,
	           double maxConsumption = PLUS_INFINITY);


      /** Only LevelTooHigh and LevelTooLow are currently used by Flaw,
                     Violation uses all types */
      enum ProblemType { NoProblem = -1,
    	  ProductionRateExceeded = 0, /**< Excessive production in an instant. */
    	  ConsumptionRateExceeded, /**< Excessive consumption in an instant. */
    	  ProductionSumExceeded, /**< Total production for the resource exceded. */
    	  ConsumptionSumExceeded, /**< Total consumption fo rthe resource exceded. */
    	  LevelTooHigh, /**< Level is outside the upper limit, taking into account remaining possible production or consumption. */
    	  LevelTooLow /**< Level is outside the lower limit, taking into account remaining possible production or consumption. */
      };

      /**
       * @brief Constructor
       * @see Object
       */
      Resource(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open);

      /**
       * @brief Constructor
       * @see Object
       */
      Resource(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open);

      virtual ~Resource();

      /**
       * @brief Accessor for the lower limit on capacity.
       */
      double getLowerLimit() const {return m_lowerLimit;}

      /**
       * @brief Accessor for the upper limit on capacity.
       */
      double getUpperLimit() const {return m_upperLimit;}

      /**
       * @brief Accessor for the maximum consumption possible at an instant.
       */
      double getMaxInstConsumption() const {return m_maxInstConsumption;}

      /**
       * @brief Accessor for the maximum production possible at an instant.
       */
      double getMaxInstProduction() const {return m_maxInstProduction;}

      /**
       * @brief Accessor for the maximum consumption possible over the lifetime of the resource.
       */
      double getMaxConsumption() const {return m_maxConsumption;}

      /**
       * @brief Accessor for the maximum production possible over the lifetime of the resource.
       */
      double getMaxProduction() const {return m_maxProduction;}

      const ProfileId getProfile() const {return m_profile;}

      virtual void add(const TokenId& token);

      virtual void remove(const TokenId& token);

      virtual void getOrderingChoices(const TokenId& token,
				      std::vector<std::pair<TokenId, TokenId> >& results,
				      unsigned int limit = PLUS_INFINITY);

      virtual void getOrderingChoices(const InstantId& inst,
                                      std::vector<std::pair<TransactionId, TransactionId> >& results,
                                      unsigned int limit = PLUS_INFINITY);

      virtual void getTokensToOrder(std::vector<TokenId>& results);

      virtual void getFlawedInstants(std::vector<InstantId>& results);

      bool hasTokensToOrder() const;
      //ResourceId getId() {return m_id;}
      //subclasses will need to override getOrderingChoices, getTokensToOrder

      int getFlawCount(const TokenId& token) const;

      static const char* getProblemString(ProblemType t) { return problemLabels[t]; }

      // PS Methods:
      virtual PSResourceProfile* getLimits();
      virtual PSResourceProfile* getLevels();

      virtual PSList<PSEntityKey> getOrderingChoices(TimePoint t);





    protected:
      friend class FVDetector;
      /**
       * @brief Initialize the Resource.  This exists because when the Resource is instantiated from a model, the levels are available from member variables,
       * not at construction time.
       * @param initCapacityLb The lower bound of the Resource's initial level. (For example, a battery may be only half charged initially.)
       * @param initCapacityUb The upper bound of the Resource's initial level. (For example, a battery may be only half charged initially.)
       * @param lowerLimit The lower capacity limit. (For example, a battery can't have less than no energy stored, or it might be considered unsafe to allow
       *                   the capacity to get below 25%.)
       * @param upperLimit The upper capacity limit. (For example, a battery has a maximum charge, or it might be unsafe to fill a beaker more than three-quarters full.)
       * @param maxInstProduction The maximum amount of production possible at an instant. (For example, a power bus might only allow 2.5A to be drawn at one instant.)
       * @param maxInstConsumption The maximum amount of consumption possible at an instant. (For example, a power bus might only allow 2.5A to be drawn at one instant.)
       * @param maxProduction The maximum amount of production possible on this resource.
       * @param maxConsumption The maximum amount of consumption possible on this resource.
       */
      void init(const double initCapacityLb, const double initCapacityUb, const double lowerLimit,
                const double upperLimit, const double maxInstProduction, const double maxInstConsumption,
                const double maxProduction, const double maxConsumption, const LabelStr& detectorName, const LabelStr& profileName);

      /**
       * @brief Receive notification that an Instant has been removed from the profile.
       * @param inst The deleted Instant.
       */
      virtual void notifyDeleted(const InstantId inst);

      /**
       * @brief Receive notification of a violation at an instant.  The subclass should take appropriate
       *        action to push this information to the planner (i.e. empty a variable's domain).
       * @param inst The violated instant.
       */
      virtual void notifyViolated(const InstantId inst, ProblemType problem);

      virtual void notifyNoLongerViolated(const InstantId inst);

      /**
       * @brief Receive notification of a flaw at an instant.  The subclass should take appropriate action
       *        to push this information to the planner (i.e. call notifyOrderingRequired)
       * @param inst The flawed instant.
       */
      virtual void notifyFlawed(const InstantId inst);

      /**
       * @brief Receive notification that an instant that had previously been flawed is no longer.  The subclass should take appropriate action
       *        to push this information to the planner (i.e. call notifyOrderingNoLongerRequired)
       * @param inst The formerly flawed instant.
       */
      virtual void notifyNoLongerFlawed(const InstantId inst);

      /*
       * called by FVDetector when profile is recomputed
       */
      void resetViolations(InstantId inst);
      void resetViolations();

    private:
      friend class ResourceTokenRelation;


      virtual bool transConstrainedToPrecede(const TransactionId& predecessor, const TransactionId& successor);
      virtual void addToProfile(const TokenId& tok) = 0;
      virtual void removeFromProfile(const TokenId& tok) = 0;

      bool noFlawedTokensForInst(const InstantId& inst) const;
      //ResourceId m_id;
      double m_initCapacityLb, m_initCapacityUb; /*<! The bounds of the initial capacity*/
      double  m_lowerLimit, m_upperLimit; /*<! The bounds on the capacity*/
      double m_maxInstProduction, m_maxInstConsumption; /*<! The maximum production and consumption allowed at an instant */
      double m_maxProduction, m_maxConsumption; /*<! The maximum production and consumption allowed over the lifetime of the resource */
    protected:
      FVDetectorId m_detector; /*<! The flaw and violation detector for this resource. */
      ProfileId m_profile; /*<! The profile calculator for this resource. */
      std::map<TransactionId, TokenId> m_transactionsToTokens;
      std::map<TokenId, std::set<InstantId> > m_flawedTokens;
      std::map<int, InstantId> m_flawedInstants;

      TokenId getTokenForTransaction(TransactionId t);
      ResourceTokenRelationId getRTRConstraint(TokenId tok);
    };
}
#endif
