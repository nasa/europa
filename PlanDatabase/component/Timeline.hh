#ifndef _H_Timeline
#define _H_Timeline

/**
 * @file Timeline.hh
 * @author Conor McGann
 * @brief
 * @ingroup PlanDatabase
*/

#include "Object.hh"
#include <list>
#include <map>
#include <vector>

namespace EUROPA {
  class OrderingChoicesCache;

  /**
   * @brief Timeline class
   */

  class Timeline: public Object {
  public:
    enum Policy {EARLIEST = 0,
		 LATEST};

    Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open = false);

    Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open = false);

    virtual ~Timeline();

    void getOrderingChoices(const TokenId& token,
			    std::vector< std::pair<TokenId, TokenId> >& results,
			    unsigned int limit = PLUS_INFINITY);

    void getTokensToOrder(std::vector<TokenId>& results);

    bool hasTokensToOrder() const;

    /**
     * @brief Test if the given token is actually assigned to this object. Over-ride the parent to require
     * explicit ordering.
     * @see constrain
     */
    bool hasToken(const TokenId& token) const;

    const std::list<TokenId>& getTokenSequence() const;

    void constrain(const TokenId& predecessor, const TokenId& successor);

    void free(const TokenId& predecessor, const TokenId& successor);

  protected:
    /** Implement notifications to synch cached data **/
    void notifyMerged(const TokenId& token);
    void notifyRejected(const TokenId& token);
    void notifyDeleted(const TokenId& token);

    void handleDiscard();
  private:
    /**
     * @brief Initialization utility
     */
    void commonInit(bool open);

    /** Over-ride base class implementations to add extra handling **/
    void add(const TokenId& token);
    void remove(const TokenId& token);

    void insertToIndex(const TokenId& token, const std::list<TokenId>::iterator& position);
    void removeFromIndex(const TokenId& token);
    bool orderingRequired(const TokenId& token);

    bool isValid(bool cleaningUp = false) const;

    /**
     * @brief True iff this token is the first in the sequence
     */
    bool atStart(const TokenId& token) const;

    /**
     * @brief True iff this token is the last in the sequence
     */
    bool atEnd(const TokenId& token) const;

    /** Helper methods for the 'free' algorithm */
    void unlink(const TokenId& token);
    TokenId removeSuccessor(const TokenId& token);
    TokenId removePredecessor(const TokenId& token);
    bool adjacent(const TokenId& x, const TokenId& y) const;


    /**
     * A list indicating the temporal order of Tokens constrained for this timeline.
     * @note All Tokens in the sequence == all Tokens in the index.
     * @note Each Token appears in the sequence at most once.
     * @note All Tokens in the sequence are part of Object::getTokens().
     * @note All Tokens in the sequence must be active.
     */
    std::list<TokenId> m_tokenSequence;

    /** Index to find position in sequence by Token */
    std::map<int, std::list<TokenId>::iterator > m_tokenIndex;

    Id<OrderingChoicesCache> m_cache;

    static const bool CLEANING_UP = true;
  };
}

#endif
