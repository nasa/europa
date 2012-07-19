#ifndef _H_Condition
#define _H_Condition

#include "CBPlannerDefs.hh"
#include "Entity.hh"
#include <set>

namespace EUROPA {

  class Condition : public Entity {
  public:
    Condition(const DecisionManagerId& dm, bool isDynamic = true);
    virtual ~Condition();

    /**
     * @brief Tests if the condition is dynamically changing and may need to be re-evaluated.
     */
    bool isDynamic() const;

    const ConditionId& getId() const;

    /**
     * @brief Tests if the given entity is permitted.
     * @param entity The entity which may be a Variable, Token, or Object
     * @return false if the entity should be excluded as a flaw.
     */
    virtual bool test(const EntityId& entity) = 0;
    
    virtual void print(std::ostream &os = std::cout);

  protected:
    ConditionId m_id;
    bool m_isDynamic;
    const DecisionManagerId m_decMgr;
  };

  std::ostream& operator <<(std::ostream& os, const ConditionId& cond);
} 

#endif
