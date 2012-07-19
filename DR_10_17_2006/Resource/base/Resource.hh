#ifndef _H_Resource
#define _H_Resource

/**
 * @file Resource.hh
 * @author Conor McGann
 * @brief Defines the public interface for the resource object model.
 * @date   May, 2003
 * @ingroup Resource
*/

#include "ResourceDefs.hh"
#include "Instant.hh"
#include "Object.hh"
#include "Transaction.hh"
#include "Entity.hh"
#include "ResourceProblem.hh"
#include <string>
#include <map>
#include <iostream>
#include <set>
#include <list>

namespace EUROPA {

  /**
   * @class Resource
   * @brief The base class for different resource implementations
   * 
   * The Resource is provided as a base class from which a variety of implementations for similar resources, or
   * a variety of resources, can be derived. The goal is to capture the collaboration model between Transactions
   * and Instants, to compute the transaction profiles, and to enforce the formal model defined in the
   * ICAPS paper i.e.:
   * @li specified properties
   * @li derived properties
   * @li transactions
   * @li queries
   * 
   * @todo Current implementation and interface limit property specification to constants independent of time. This needs
   * to be thought through to be more extendible.
   * @todo Current API will not permit derived classes to be usefully employed. Change things around so that current implementation
   * is just a particular derived class and make th Resource class an abstract base class.
   */
  class Resource :public Object
  {
  public:
    Resource(const PlanDatabaseId& planDatabase,
             const LabelStr& type,
             const LabelStr& name,
             double initialCapacity = 0,
             double limitMin = MINUS_INFINITY,
             double limitMax = PLUS_INFINITY,
             double productionRateMax = PLUS_INFINITY,
             double productionMax = PLUS_INFINITY,
             double consumptionRateMax = MINUS_INFINITY,
             double consumptionMax = MINUS_INFINITY);

    Resource(const PlanDatabaseId& planDatabase,
             const LabelStr& type,
             const LabelStr& name,
             bool open);
    
    Resource(const ObjectId parent,
             const LabelStr& type,
             const LabelStr& name,
             bool open);

    /**
     * @brief Destructor
     */
    virtual ~Resource();

    /**
     * @brief Retrieve all transactions in a given time interval
     *
     * @arg resultSet The target list to append resulting transactions.
     * @arg lowerBound Exclude Transactions whose latest time < lowerBound.
     * @arg upperBound Exclude Transactions whose earliest time > upperBound
     * @arg propagate Require constraint propagation before returning transactions
     * @par Errors:
     * @li resultSet.isEmpty()
     * @li lowerBound > upperBound
     */
    void getTransactions(std::set<TransactionId>& resultSet, 
                         int lowerBound = MINUS_INFINITY, int upperBound = PLUS_INFINITY, bool propagate = true);

    /**
     * @brief Retrieve all Instants in a given time interval
     *
     * @arg resultSet The target list to append resulting Instants.
     * @arg lowerBound Exclude Instants whose time < lowerBound.
     * @arg upperBound Exclude Instant whose time > upperBound
     * @par Errors:
     * @li !resultSet.isEmpty()
     * @li lowerBound > upperBound
     */
    void getInstants(std::list<InstantId>& resultSet, int lowerBound = MINUS_INFINITY, int upperBound = PLUS_INFINITY);


    const std::map<int, InstantId>& getInstants() const;

    /**
     * @brief Output the Transaction Profile with indications where Violations occur.
     * @arg os - output stream to write to
     */
    void print(std::ostream& os);

    /**
     * @brief Output the Transaction Profile to a string with indications where Violations occur
     */
    std::string toString();

    /**
     * @brief Check if the Resource is violated.
     * 
     * This call may cuase computation to determine the Transaction Profile and deetct Violations.
     * @return true if and only if there is no way to make the resource comply with its speciifed properties without
     * removing a transaction or relaxing a temporal or quanitty variable. Otherwise return false.
     * @see Violation
     */
    bool isViolated();

    /**
     * @brief Check if the resource is flawed.
     * @return true if any derived properties MAY be outside the specified limits.
     * @todo deprecate in favour of isFlawed.
     */
    bool isFlawed();

    /** @see isFlawed **/

    bool hasFlaws();

    /**
     * @brief Check if the resource is dirty
     */
    bool isDirty() const;

    /**
     * @brief Retrieves the set of Violations within the given time interval
     *
     * If !isViolated() there will be no Violations returned.
     * @arg resultSet The target list to append resulting Violations.
     * @arg lowerBound Exclude Violations occuring at an Instant whose time <= lowerBound.
     * @arg upperBound Exclude Violations occuring at an Instant whose time >= upperBound
     * @par Errors:
     * @li !resultSet.isEmpty()
     * @li lowerBound > upperBound
     * @see isViolated()
     */
    void getResourceViolations( std::list<ResourceViolationId>& results, 
				int lowerBound = MINUS_INFINITY, 
				int upperBound = PLUS_INFINITY);

    void getResourceFlaws( std::list<ResourceFlawId>& results, 
			   int lowerBound = MINUS_INFINITY, 
			   int upperBound = PLUS_INFINITY);

    /**< Accessors for specified resource properties */
    double getLimitMax(int = MINUS_INFINITY) const {return m_limitMax;}
    double getLimitMin(int = MINUS_INFINITY) const {return m_limitMin;}
    double getInitialCapacity(void) const {return m_initialCapacity;}
    double getProductionRateMax(int  = MINUS_INFINITY) const {return m_productionRateMax;}
    double getProductionMax(int = MINUS_INFINITY) const {return m_productionMax;}
    double getConsumptionRateMax(int = MINUS_INFINITY) const {return m_consumptionRateMax;}
    double getConsumptionMax(int = MINUS_INFINITY) const {return m_consumptionMax;}

    /**< The following methods are here in case the planner needs to update
       resource parameters */
    void setProductionMax( const double& value );
    void setProductionRateMax( const double& value );
    void setConsumptionMax( const double& value );
    void setConsumptionRateMax( const double& value );

    /**
     * @brief Retrieve the level as an interval at the given timepoint.
     * @param timepoint The time at which we want to query the level
     * @param result The resulting interval to populate. Values for the result are interpolated based on
     * the internal representation within the resource.
     */
    void getLevelAt(int timepoint, IntervalDomain& result );

    void updateInitialState(const IntervalDomain& value );

  protected:
    friend class ResourceConstraint;

    void handleDiscard();

    void markDirty();

    /**
     * @brief Helper method for construction
     */
    void init(double initialCapacity = 0,
              double limitMin = MINUS_INFINITY,
              double limitMax = PLUS_INFINITY,
              double productionRateMax = PLUS_INFINITY,
              double productionMax = PLUS_INFINITY,
              double consumptionRateMax = MINUS_INFINITY,
              double consumptionMax = MINUS_INFINITY);


    /**
     * @brief Compute actual profile
     * @see updateTransactionProfile
     */
    virtual void computeTransactionProfile();

    /**
     * @brief Compute running totals for an instant
     * @see computeTransactionProfile
     */
    virtual void computeRunningTotals(const InstantId& instant, 
				      const TransactionId& tx, 
				      double& runningLowerMin, 
				      double& runningLowerMax,                                      
				      double& runningUpperMin, 
				      double& runningUpperMax);

    /**
     * @brief Compute actual profile
     * @see updateTransactionProfile
     */
    static void updateInstantBounds(InstantId& inst, 
				    const double lowerMin, 
                                    const double lowerMax, 
				    const double upperMin, 
                                    const double upperMax);
  private:
 
    /**
     * @brief This is the core algorithm for computing the Transaction Profile
     * @todo Update to work incrementally
     */
    void updateTransactionProfile();

    /**
     * @brief insert Transaction on the Resource.
     *
     * If the Transaction has been successfully inserted, the resource of the transaction will be set, and the
     * Transaction will be notified using a call back on tx. If the Transaction insertion failed, the Transaction
     * resource will not be changed from its initial value. Insertion of a Transaction impacts the Transaction 
     * Profile
     *
     * @arg tx - the transaction to be inserted
     * @return false iff any of the following are true:
     * @li tx.isNoId()
     * @li !tx->getResource().isNoId()
     * @li there is no possibility that the transaction requirements can ever be met.
     */
    bool insert(const TransactionId& tx);

    /**
     * @brief Handles addition of a Token since the token should be active and this resource has been
     * added to the object variable. Records that the resource has changed. Will delegate to the parent.
     */
    void add(const TokenId& token);

    /**
     * @brief Handles removal of token which may occur due to a deletion or through a restriction of the
     * domain of the object variable. Records that the resource has changed. Will delegate to the parent.
     */
    void remove(const TokenId& token);

    /**< Global violations encapsulate violation types that are global to the resource */
    void addGlobalResourceViolation(ResourceProblem::Type type);
    const std::set<ResourceViolationId>& getGlobalResourceViolations() const { return m_globalViolations; }

    /**
     * @brief Prevent copy constructor
     */
    Resource(const Resource&);

    /**
     * @brief Helper method to clean up internal data structures when removing a token
     * @see free, remove
     */
    void cleanup(const TransactionId& tx);

    /**
     * @brief Helper method to see if there are any transactions that start or end on the given instant
     * @return true if the instant contains a transaction whose earliest or latest time coincides with 
     * the time of the instant. Otherwise false.
     */
    bool hasStartOrEndTransaction(const InstantId& instant) const;

    /**
     * @brief Algorithm to detect Flaws and Violations given the derived and specified proprties at an Instant
     */
    bool applyConstraints(const InstantId& instant);

    /**
     * @brief Check integrity of data structures
     */
    bool isValid() const;

    void resetProfile();

    void rebuildProfile();

    /** 
     * @brief Recompute m_totalUsedConsumption and m_totalUsedProduction 
     */
    void recomputeTotals();

    /**
     * Helper method for Instant allocation
     */
    std::map<int, InstantId>::iterator createInstant(int time, const std::map<int, InstantId>::iterator& pos);

    /*
      Begin specified property values - see ICAPS paper for details. Note that the approach here of using constants
      is the simplest form and more sophisticated implementations are desireable.
    */

    double m_initialCapacity; /*!< The seed from which the Transaction Profile is calculated */
    double m_limitMin; /*!< The lower limit on the level in the Transaction Profile for the resource to be consistent */
    double m_limitMax; /*!< The upper limit on the level in the Transaction Profile for the resource to be consistent */
    double m_productionRateMax; /*!< The maximum rate of production allowed at any instant */
    double m_productionMax; /*!< The maximum cumulative production possible for the resource in the given time horizon */
    double m_consumptionRateMax; /*!< The maximum rate of consumption that can occur at any instant. */
    double m_consumptionMax; /*!< The maximum cumulative consumption that can occur on the resource 
			       in the given time horizon.*/

    double m_totalUsedConsumption; /*!< The optimistic sum (min consumption=max value) over all transactions */
    double m_totalUsedProduction;  /*!< The optimistic sum (min production=min value) over all transactions */

    /*< General implementation helper data */
    bool m_dirty;

    std::map<int, InstantId>  m_instants; /*!< Time ordered sequence of Instants */
    /*!< Set of ResourceListeners to be notified about existence (and later degree) of flaws */
    std::set<ResourceViolationId> m_globalViolations; /*!< All violations on the resource */
  };
}
#endif
