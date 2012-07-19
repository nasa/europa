#ifndef _H_DecisionPoint
#define _H_DecisionPoint

#include "CBPlannerDefs.hh"
#include "Entity.hh"

namespace EUROPA {

  class DecisionPoint : public Entity {
  public:
    DecisionPoint(const DbClientId& client, const EntityId& entity);

    const DecisionPointId& getId() const;
    int getEntityKey() const;
    bool isOpen() const;

    virtual bool assign()=0;
    virtual bool retract()=0;
    virtual bool hasRemainingChoices()=0;

    virtual void print(std::ostream& os) const;

    virtual ~DecisionPoint(); // Must be public to use with Id
  protected:
    DecisionPointId m_id;
    bool m_open; /**< keeps track of status; !m_open == closed decision */ 
    DbClientId m_dbClient;
    const EntityId m_entity; /**< contains the timeline, token or variable */
    const int m_entityKey; /**< caches the entity key for easy and safe lookup */
  };

  std::ostream& operator <<(std::ostream& os, const DecisionPointId&);

}
#endif 
