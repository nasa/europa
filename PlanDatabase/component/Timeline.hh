#ifndef _H_Timeline
#define _H_Timeline

#include "Object.hh"
#include <list>
#include <map>
#include <vector>

namespace EUROPA {

  class Timeline: public Object {
  public:
    enum Policy {EARLIEST = 0,
		 LATEST};

    Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open = false);
    Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open = false);
    virtual ~Timeline();

    void getOrderingChoices(const TokenId& token, std::vector<TokenId>& results);
    void getTokensToOrder(std::vector<TokenId>& results);
    bool hasTokensToOrder() const;
    const std::list<TokenId>& getTokenSequence() const;
    void constrain(const TokenId& token, const TokenId& successor = TokenId::noId());
    void free(const TokenId& token);
  private:
    void remove(const TokenId& token); // Over-ride base class implementation
    void cleanup(const TokenId& token);
    bool isValid(bool cleaningUp = false) const;

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

    static const bool CLEANING_UP = true;


  };
}

#endif
