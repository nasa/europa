#ifndef _H_Timeline
#define _H_Timeline

#include "Object.hh"
#include <list>
#include <map>
#include <vector>

namespace Prototype {

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
    void constrainToSingleton(const TokenId& token, const AbstractDomain& domain, const ConstrainedVariableId& var);

    /**
     * 1. All Tokens in the sequence == all Tokens in the index.
     * 2. Each Token appears in the sequence at most once.
     * 3. All Toiens in the sequence are part of Object::getTokens()
     * 4. All Tokens in the sequence must be active
     */
    std::list<TokenId> m_tokenSequence; /*!< A list indicating the temporal order of Tokens constrained for this timeline */
    std::map<int, std::list<TokenId>::iterator > m_tokenIndex; /*!< Index to find position in sequence by Token */

    /**
     * @class ConstraintEntry
     * @brief Stores token, constraint and variable data for when a constraint is imposed by the Timeline. Used to allow
     * cleanup.
     */
    class ConstraintEntry{
    public:
      /**
       * @brief Construct data structure.
       * @param constraint The constraint added, can be unary or binary
       * @param first The predecessor token.
       * @param second The successor token - may be a noId indicating a unary constraint.
       */
      ConstraintEntry(const ConstraintId& constraint, const TokenId& first, const TokenId& second = TokenId::noId());
      bool isValid() const;
      ConstraintId m_constraint;
      TokenId m_first;
      TokenId m_second;
    };

    std::list<ConstraintEntry*> m_constraints;

    static const bool CLEANING_UP = true;
  };
}

#endif
