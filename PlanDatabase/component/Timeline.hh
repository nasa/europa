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

    Timeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);
    Timeline(const ObjectId& parent, const LabelStr& type, const LabelStr& localName);
    virtual ~Timeline();

    void getOrderingChoices(const TokenId& token, std::vector<TokenId>& results);
    void getTokensToOrder(std::vector<TokenId>& results);
    const std::list<TokenId>& getTokenSequence() const;
    void constrain(const TokenId& token, const TokenId& successor = TokenId::noId());
    void free(const TokenId& token);
  private:
    bool isValid() const;
    void constrainToSingleton(const TokenId& token, const AbstractDomain& domain, const ConstrainedVariableId& var);

    std::list<TokenId> m_tokenSequence;
    std::map<int, std::list<TokenId>::iterator > m_tokenIndex;

    class ConstraintEntry{
    public:
      ConstraintEntry(const ConstraintId& constrain, const TokenId& first, const TokenId& second = TokenId::noId());
      ~ConstraintEntry();
      bool isValid() const;
      ConstraintId m_constraint;
      TokenId m_first;
      TokenId m_second;
    };

    std::list<ConstraintEntry*> m_constraints;
  };
}

#endif
