#ifndef _H_Entity
#define _H_Entity

#include "LabelStr.hh"
#include "Id.hh"

#include <map>
#include <set>
#include <string>
#include <vector>

#include "PSUtils.hh"

namespace EUROPA{

#define DECLARE_ENTITY_TYPE(type) \
  static const LabelStr& entityTypeName() { \
    static LabelStr sl_type(#type); \
    return sl_type; \
  } \
  virtual const LabelStr& entityType() { \
    return entityTypeName(); \
  } \

/**
 * @class Entity
 * @brief Basic entity in system
 * @author Conor McGann
 * @ingroup Utility
 */
  class Entity;
  typedef Id<Entity> EntityId;

  // virtual inheritance because we have a diamond (Constraint inherits both Entity and PSConstraint, ie two PSEntities) 
  class Entity: public virtual PSEntity {
  public:
    DECLARE_ENTITY_TYPE(Entity);

    template<class COLLECTION>
    static void discardAll(COLLECTION& objects){
      typedef typename COLLECTION::iterator object_iterator;
      for(object_iterator it = objects.begin(); it != objects.end(); ++it){
	check_error((*it).isValid());
	Entity* elem = (Entity*) (*it);
	elem->discard();
      }
      objects.clear();
    }

    virtual ~Entity();

    inline eint getKey() const {return m_key;}
    inline PSEntityKey getEntityKey() const {return cast_int(m_key);}

    
    virtual const std::string& getEntityName() const;
    virtual const LabelStr& getName() const;

    virtual const std::string& getEntityType() const;

    virtual std::string toString() const;
    virtual std::string toLongString() const;

    
    virtual bool canBeCompared(const EntityId&) const;

    /**
     * @brief Set an external entity reference, indicating an external entity is shadowing this entity.
     */
    void setExternalEntity(const EntityId& externalEntity);

    /**
     * @brief Special case to reset the external entity to a noId.
     */
    void clearExternalEntity();

    /**
     * @brief Retrieve an external entity reference, if present.
     * @return Will return EntityId::noId() if not assigned.
     */
    const EntityId& getExternalEntity() const;

    /**
     * @brief Get the number of outstanding references
     */
    unsigned int refCount() const;

    /**
     * @brief Increment the reference count. Use if you wish to prevent deletion of the entity
     */
    void incRefCount();

    /**
     * @brief Decrement the reference count. Return true if this self-destructs as a result
     */
    bool decRefCount();

    /**
     * @brief Test of the entity can be deleted. RefCount == 0
     */
    bool canBeDeleted() const;

    /**
     * @brief Discard the entity.
     * @param pool If true it will be pooled for garbage colection
     */
    void discard(bool pool = true);

    /**
     * @brief Test of the class has already been discarded.
     */
    bool isDiscarded() const;

    /**
     * @brief Add a dependent entity. It will be notified when this is discarded
     * @see notifyDiscarded
     */
    void addDependent(Entity* entity);

    /**
     * @brief Remove a dependent entity.
     */
    void removeDependent(Entity* entity);

    /**
     * @brief Retrieve an Entity by key.
     * @return The Id of the requested Entity if present, otherwise a noId;
     */
    static EntityId getEntity(eint key);

    /**
     * @brief Get all entities
     */
    static void getEntities(std::set<EntityId>& resultSet);


    /**
     * @brief Indicates a system is being terminated
     */
    static void purgeStarted();

    /**
     * @brief Indicates a system is finished terminating
     */
    static void purgeEnded();

    /**
     * @brief Tests if purge in progress
     */
    static bool isPurging();

    /**
     * @brief Test of the entity by the given key is pooled for deallocation
     */
    static bool isPooled(Entity* entity);

    /**
     * @brief Garbage collect discarded entities
     * @return The number of entities deleted
     */
    static unsigned int garbageCollect();

    /**
     * @brief Configure system to require garbage collection
     */
    static bool& gcRequired();

  protected:
    Entity();

    /**
     * @brief Over-ride to custmize deallocation
     */
    virtual void handleDiscard();

    EntityId m_externalEntity; /*!< Helfpul to make synchronization with other data structures easy */

  private:

	  
    static eint allocateKey();

	  
    /**
     * @brief Internal variable indicating if garbage collection is active
     */
    static bool& gcActive();

    /**
     * @brief Subclasses should over-ride this to handle special data structure management.
     */
    virtual void notifyDiscarded(const Entity* entity);

    const eint m_key;
    
    unsigned int m_refCount;
    bool m_discarded;
    std::set<Entity*> m_dependents;
    static std::map<eint, unsigned long int>& entitiesByKey();
    static std::set<Entity*>& discardedEntities();
    static bool& getPurgeStatus();
  };

  /**
   * @brief Key comparator class for ordering in stl containers
   */
  template <class T>
  class EntityComparator{
  public:
    bool operator() (const T& t1, const T& t2) const {
      checkError(t1.isValid(), t1);
      checkError(t2.isValid(), t2);
      return t1->getKey() < t2->getKey();
    }

    bool operator==(const EntityComparator& c){return true;}
  };

}
#endif
