#ifndef _H_PlanDatabaseListener
#define _H_PlanDatabaseListener

#include "PlanDatabaseDefs.hh"

namespace EUROPA {

  class PlanDatabaseListener{
  public:
    virtual ~PlanDatabaseListener();

    /**
     * @brief Indicates a new Object has been added to the PlanDatabase.
     */
    virtual void notifyAdded(const ObjectId& object);

    /**
     * @brief Indicates an object has been removed from the PlanDatabase
     */
    virtual void notifyRemoved(const ObjectId& object);

    /**
     * @brief Indicates that a new Token has been added to the PlanDatabase
     */
    virtual void notifyAdded(const TokenId& token);

    /**
     * @brief Indicates that a token has been removed from the PlanDatabase.
     */
    virtual void notifyRemoved(const TokenId& token);

    /**
     * @brief Indicates a Token has been activated.
     * @see Token::activate
     */
    virtual void notifyActivated(const TokenId& token);
    virtual void notifyDeactivated(const TokenId& token);
    virtual void notifyMerged(const TokenId& token);
    virtual void notifySplit(const TokenId& token);
    virtual void notifyRejected(const TokenId& token);
    virtual void notifyReinstated(const TokenId& token);

    /**
     * @brief Signals the event where a token has been temporally constrained to precede a given successor
     * on a given object.
     * @param object The object to which the token is assigned.
     * @param predecessor A token to be succeded.
     * @param successor A token to precede. 
     * @see Object::constrain, Object::free
     */
    virtual void notifyConstrained(const ObjectId& object, const TokenId& predecessor, const TokenId& successor);

    /**
     * @brief Removes any constraints added through calls to Object::constrain(token, *).
     * @param object The object from which the Token has been freed.
     * @param predecessor The token that had been succeded.
     * @param successor The token that had been preceded. 
     */
    virtual void notifyFreed(const ObjectId& object, const TokenId& predecessor, const TokenId& successor);

    /**
     * @brief Signals the event where a Token has become a candidate for impacting an object.
     *
     * Things to consider:
     * @li An impact is manifest by constraining a Token with respect to an Object. Such a commitment will bind the
     * tokens object variable to a singleton.
     * @li Only active tokens may be constrained. Therefore, until a Token has been activated, it cannot be added.
     * @li Propagation, or external restrictions on the object variable of the Token may cause assignment to be reversed.
     * @param object The object that may be impacted.
     * @param token The Token that may have an impact. The token must be active. The token's object variable must contain the object.
     * @see notifyRemoved(const ObjectId& object, const TokenId& token)
     * @see Object::getTokensToOrder()
     */
    virtual void notifyAdded(const ObjectId& object, const TokenId& token);

    /**
     * @brief Signals the event where a Token which was previously a candidate for assignment to an object no longer can
     * be.
     *
     * @param object The object that can no longer be assigned the Token.
     * @param token The token that can no longer be assigned to the object. The Token must have been assignable previously.
     * @see notifyAdded(const ObjectId& object, const TokenId& token)
     * @see Object::getTokensToOrder()
     */
    virtual void notifyRemoved(const ObjectId& object, const TokenId& token);

    /**
     * @brief Signals that a token has been committed.
     * @param token The token that has been committed.
     */
    virtual void notifyCommitted(const TokenId& token);

    /**
     * @brief Signals that a token has been terminated
     * @param token The token that has been terminated
     */
    virtual void notifyTerminated(const TokenId& token);

    const PlanDatabaseListenerId& getId() const;

  protected:
    PlanDatabaseListener(const PlanDatabaseId& planDatabase);
    PlanDatabaseListenerId m_id;
    const PlanDatabaseId m_planDatabase;
  };

}


#endif
