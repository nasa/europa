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
    std::map<double, std::list<TokenId>::iterator > m_tokenIndex;
    std::multimap<double, ConstraintId> m_constraints;
  };
}

#endif
