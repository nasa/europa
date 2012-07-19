#ifndef _H_Instant
#define _H_Instant

#include "UnifyMemento.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "ResourceDefs.hh"
#include "Resource.hh"
#include "ResourceProblem.hh"
#include "ResourceViolation.hh"
#include "ResourceFlaw.hh"
#include "Transaction.hh"

#include <string>

namespace EUROPA {

  /**
   * @class Instant
   * @brief Represents the state of the Resource at an instant in time within the horizon of the resource
   *
   * A core assumption of this work is that change on a resource takes place at an instant.
   * Instants are only used in the context of a Resource. Only a resource may create, delete or modifiy
   * an Instant. In general, Instants only arise where distinct timepoints occur. We can therefor say that
   * the time for an instant equals at least one of the following:
   * @li the start of the resource horizon
   * @li the end of the resource horizon
   * @li the earliest time for at least on transaction inserted on the resource.
   * @li the latest time for at least one transaction on the resource.
   *
   * An Instant refers to a set of Transactions. We require that for all t in the set referred to
   * by an Insatnt, the time of the instant is in [t.earliest, t.latest]
   *
   * An Instant supports linkage to other instants. Links have the following semantics:
   * @li It may have a predecessor. If it does, the predecessor must occur before it.
   * @li It may have a successor. If it does, the successor must occur after it.
   * @li If an Instant has no predecessor, then it must occur at the start of the horizon of a resource.
   * @li If an Instant has no successor, then it must occur at the end of the horizon of a resource.
   * @todo Extend validation check to enforce all invariants listed above.
   */
  class Instant : public Entity
  {
  public:

    void print(std::ostream& os) const;
    std::string toString() const;
    int getTime() const {return m_time;}
    double getCompletedMax() const {return m_completedMax;}
    double getCompletedMin() const {return m_completedMin;}

	// PartialPlanWriter wants these two methods
    double getLevelMax() const {return m_upperMax;}
    double getLevelMin() const {return m_lowerMin;}

    // Accessors
    double getLowerMin() const { return m_lowerMin; }
    double getLowerMax() const { return m_lowerMax; }
    double getUpperMin() const { return m_upperMin; }
    double getUpperMax() const { return m_upperMax; }

    double getProductionMin() const {return m_productionMin;}
    double getConsumptionMin() const {return m_consumptionMin;}
    double getProductionSum() const {return m_productionSum;}
    double getConsumptionSum()const {return m_consumptionSum;}
    int getTransactionCount() const {return m_transactions.size();}
    const TransactionSet& getTransactions() const {return m_transactions;}
    const std::list<ResourceViolationId>& getResourceViolations() {return m_violations;}
    const std::list<ResourceFlawId>& getResourceFlaws() {return m_flaws;}

  private:
    friend class Resource;
    Instant(int time);
    ~Instant();
    const InstantId& getId() {return m_id;}
    void insert(const TransactionId& tx);
    bool remove(const TransactionId& tx);

    void updateBounds(double lowerMin, double lowerMax, double upperMin, double upperMax);

    // this is the original one
    void updateBounds(double completedMax, double completedMin, double levelMax, double levelMin,
                      double productionMin, double consumptionMin, double productionSum, double consumptionSum);

    void addResourceViolation(ResourceProblem::Type type);
    void addResourceFlaw(ResourceProblem::Type type);
    void reset();
    bool isValid() const;

    InstantId m_id;

    int m_time; /**< The time point of the Instant. */

    /* Data points calculated as part of the transaction profile */
    double m_lowerMin;
    double m_lowerMax;
    double m_upperMin;
    double m_upperMax;

    double m_completedMax; /**< The upper bound on a summation of transactions whose end time has occurred by this instant. */
    double m_completedMin; /**< The lower  bound on a summation of transactions whose end time has occurred by this instant. */
    double m_productionMin; /**< The minimum production occurring for this Instant. */
    double m_consumptionMin; /**< The minimum consumption occurring for this Instant. */
    double m_productionSum; /**< The sum of all m_productionMin values to date. */
    double m_consumptionSum; /**< The sume of all m_consumptionMin values to date. */

	/* TBS:  Not used anywhere */
//    double m_levelMax; /**< The maximumn level of the resource at this time, for the set of currently inserted transactions. */
//    double m_levelMin; /**< The minimumn level of the resource at this time, for the set of currently inserted transactions. */

    TransactionSet m_transactions; /**< The set of all Transactions where m_time is in [earliest, latest]. */
    std::list<ResourceViolationId> m_violations; /**< The set of all Violations detected given the boundary conditions
                                            on the resource and the Transaction Profile. */
    std::list<ResourceFlawId> m_flaws;
  };
}

#endif
