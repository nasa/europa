#ifndef _H_Transaction
#define _H_Transaction

/**
@file Transaction.hh
@author Sailesh Ramakrishnan
@brief Defines the public interface for resource transactions.
@date   Jan, 2004
*/

#include "ResourceDefs.hh"
#include "EventToken.hh"
#include <string>
#include <map>
#include <iostream>
#include <set>
#include <list>

namespace Prototype {

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
		double min = -LARGEST_VALUE,
		double max = LARGEST_VALUE,
		bool closed = true);

    Transaction(const PlanDatabaseId& planDatabase,
		const LabelStr& predicateName,
		bool rejectable,
		const IntervalIntDomain& timeBaseDomain,
		const LabelStr& objectName,
		bool closed);

    Transaction(const TokenId& parent,
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
    void setEarliest(int earliest);

    /**
     * @brief Accessor
     */
    int getEarliest() const;


    /**
     * @brief Set the latest time with a new value.
     * This operation may increase or decrease the latest occurence of the Transaction.
     * Any change to the time may cause recomputation.
     * @arg latest - latest must be >= m_latest or it will cause an error
     */
    void setLatest(int latest);

    /**
     * @brief Accessor
     */
    int getLatest() const;

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
    const ResourceId& getResource() const {return m_resource;}

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

    /**
     * @brief Notify the transaction whenever the time or quanitity values change
     */
    void notifyChanged();

    /**
     * @brief Checks if the transaction values have changed and clears the change status.
     * @return true if the Transaction data has been modified since the last time this function was called, otherwise false.
     */
    bool checkAndClearChange();

    virtual void close();

  protected:
    static const int USAGE = 4; /**< Position of quantity variable in list of m_variables when constructed as a NddlTransaction. */
    ResVarId m_usage;

  private:
    // Resource is only class privy to send notifications.
    friend class Resource;

    /**
     * @todo: Move the code in these notifications to other appropriate notifications inherited from the token or object.
     */

    /**
     * @brief Resource notifies the transaction when it has been inserted on the resource.
     * @arg resource - the resource to which the transaction has been inserted. Must be a valid Id.
     * @todo Consider making the implementation call a protected, stubbed out, virtual function to support core behaviour
     * but allow extension in a derived class.
     */
    void notifyInserted(ResourceId& resource);

    /**
     * @brief Resource notifies the transaction when it has been removed from the resource.
     * @arg resource - the resource from which the transaction has been removed. Must be a valid Id.
     * @todo Consider making the implementation call a protected, stubbed out, virtual function to support core behaviour
     * but allow extension in a derived class.
     */
    void notifyRemoved(ResourceId& resource);

    /**
     * @brief Resource notifies the tx if it is deleted, yet still holds the transaction.
     * @arg resource - the resource which has been deleted. Must be a valid Id.
     * @todo Consider making the implementation call a protected, stubbed out, virtual function to support core behaviour
     * but allow extension in a derived class.
     */
    void notifyDeleted(ResourceId& resource);


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
    // double m_min;
    // double m_max;
    bool m_changed;

    ResourceId m_resource;
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


} // namespace prototype
#endif
