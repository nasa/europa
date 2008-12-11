#ifndef _H_Transaction
#define _H_Transaction

/**
 * @file Transaction.hh
 * @author Sailesh Ramakrishnan
 * @brief Defines the public interface for resource transactions.
 * @date   Jan, 2004
*/

#include "ResourceDefs.hh"
#include "EventToken.hh"
#include <iostream>

namespace EUROPA {

  /**
   * @class Transaction
   * @brief Transactions on a resource are caused by actions in a plan to change the state of a resource.
   *
   * The Transactions we care about are: 
   * @li use 
   * @li consume
   * @li produce. 
   * @par However, we only provide a single transaction
   * that changes the quantity of the resource. A positive change is a production. A negative change is a consumption.
   * A transaction acts over a time instant. The instant may be given by an interval. The quantity to changle the level
   * may also be given by an interval. The interval may span 0, indicating that the transaction may be either a producer
   * or a consumer.
   *
   * It is assumed that Transactions are created and destroyed externally, and may be spercialized.
   * @todo When we want to support a unary resource, or the use operation, it will be important to
   * support assessing if 2 transactions arise from the same parent, or esle supporting the notion
   * of a composite transaction. This will be necessary to retain the notion of primitive transactions occurring at an instant
   * and still allowing constraints that 2 transactions (consume and produce) are adjacent with no intervening
   * transactions as would be the case with a use operation. The nested transaction might be a good approach.
   * @see Instant, Resource
   */
  class Transaction: public EventToken
  {
  public:

    Transaction(const PlanDatabaseId& planDatabase,
		const LabelStr& predicateName,
		const IntervalIntDomain& timeBaseDomain = IntervalIntDomain(),
		double min = MINUS_INFINITY,
		double max = PLUS_INFINITY,
		bool closed = true);

    Transaction(const PlanDatabaseId& planDatabase,
		const LabelStr& predicateName,
		bool rejectable,
		bool isFact,
		const IntervalIntDomain& timeBaseDomain,
		const LabelStr& objectName,
		bool closed);

    Transaction(const TokenId& parent,
		const LabelStr& relation,
		const LabelStr& predicateName,
		const IntervalIntDomain& timeBaseDomain,
		const LabelStr& objectName,
		bool closed);

    /**
     * @brief Set the earliest time with a new value.
     * This operation may increase or decrease the earliest occurence of the Transaction.
     * Any change to the time will cause recomputation.
     * @arg earliest - earliest must be <= m_latest or it will cause an error
     */
    void setEarliest(eint earliest);

    /**
     * @brief Accessor
     */
    eint getEarliest() const;


    /**
     * @brief Set the latest time with a new value.
     * This operation may increase or decrease the latest occurence of the Transaction.
     * Any change to the time may cause recomputation.
     * @arg latest - latest must be >= m_latest or it will cause an error
     */
    void setLatest(eint latest);

    /**
     * @brief Accessor
     */
    eint getLatest() const;

    /**
     * @brief Set the minimu quanityt of change allowed for this transaction.
     * Any change may cause recomputation.
     * @arg min - min must be <= getMax() or it will cause an error
     */
    void setMin(double min);

    /**
     * @brief Accessor
     */
    double getMin() const ;

    /**
     * @brief Set the minimu quanityt of change allowed for this transaction.
     * Any change will cause recomputation.
     * @arg max - max must be >= getMin() or it will cause an error
     */
    void setMax(double max);

    /**
     * @brief Accessor
     */
    double getMax() const ;

    /**
     * @brief Accessor
     * @return a reference to the resource to which the transaction is assigned. May return Id::noId() if unassigned.
     */
    ResourceId getResource() const;

    /**
     * @brief Accessor
     * @return true if getMax() > 0, false otherwise.
     */
    bool canProduce() const {return m_usage->lastDomain().getUpperBound() > 0;}

    /**
     * @brief Accessor
     * @return true if getMin() > 0, false otherwise.
     */
    bool canConsume() const {return m_usage->lastDomain().getLowerBound() < 0;}

    /**
     * @brief Output details. Primarily for debugging.
     * @arg The target output stream
     */
    void print(ostream& os);

    virtual void close();

  protected:
    ResVarId m_usage;

    void commonInit(bool closed);

  private:
    // Resource is only class privy to send notifications.
    friend class Resource;

    /**
     * @brief Helper method to verify all the data is correct
     * Checks for:
     * @li All Transaction ID's are valid
     * @li Valid Key
     * @li Valid Temporal Bounds (earliest <= latest)
     * @li Valid Quantity Bounds (min <= max)
     * @todo Consider making this function call a protected virtual function to add additional validity checks
     * in derived classes.
     */
    bool isValid() const;
  };

  /**
   * Comparator used to let the set of transactions work correclty. Required by std::set
   */
  struct TxCompare
  {
    bool operator()(const TransactionId& a, const TransactionId& b) const
    {
      return(a->getKey() < b->getKey());
    }
  };

  typedef std::set<TransactionId, TxCompare> TransactionSet;


} // namespace EUROPA
#endif
