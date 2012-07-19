#ifndef _H_SAVH_Transaction
#define _H_SAVH_Transaction

/**
 * @file SAVH_Transaction.hh
 * @author Michael Iatauro
 * @brief Defines a class for representing a single, temporally flexible transaction on a Resource
 * @date March, 2006
 * @ingroup Resource
 */

#include "SAVH_ResourceDefs.hh"
#include "ConstraintEngineDefs.hh"

namespace EUROPA {
  namespace SAVH {

    /**
     * @class Transaction
     * @brief A class representing a single "transaction" on a resource--consumption or production--that may have flexible time or quantity.
     * In our model, transactions are instantaneous events.  Temporal flexibility merely represents a range of instants at which the
     * transaction may occur.
     */
    class Transaction {
    public:
      /**
       * @brief Constructor
       * @param time The time at which the transaction occurrs.
       * @param quantity The quantity of the resource that is produced or consumed.
       * @param isConsumer Because all transactions have positive quantities, it is necessary to indicate that a transaction is
       *  consuming the resource in some way other than a negative quantity.  This flag is what indicates it.
       */
      Transaction(ConstrainedVariableId time, ConstrainedVariableId quantity, bool isConsumer, EntityId owner=EntityId::noId());
      ~Transaction();

      /**
       * @brief Accessor for the time variable of the transaction.
       */
      ConstrainedVariableId time() const {return m_time;}

      /**
       * @brief Accessor for the quantity variable of the transaction.
       */
      ConstrainedVariableId quantity() const {return m_quantity;}

      /**
       * @brief Determines if this transaction represents consumption or production.
       */
      bool isConsumer() const {return m_isConsumer;}
      TransactionId getId() const {return m_id;}
      std::string toString() const;

      EntityId& getOwner() {return m_owner;}

    protected:
      TransactionId m_id;
      ConstrainedVariableId m_time, m_quantity; /*<! The variables for the time and amount of the transaction */
      bool m_isConsumer; /*<! The flag indicating whether this transaction consumes or produces some amount of resource */
      EntityId m_owner;
    };
  }
}
#endif
