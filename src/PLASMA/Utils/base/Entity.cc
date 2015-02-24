#include "Entity.hh"
#include "Debug.hh"
#include "Mutex.hh"

#include <sstream>

#include <boost/ref.hpp>

namespace EUROPA {
class EntityInternals {
 public:
  EntityInternals(): m_entitiesByKey(), m_discardedEntities(), m_purgeStatus(false), 
                     m_gcActive(false), m_gcRequired(false), m_key(0) {}

  eint allocateKey(const Entity* const e){
    eint retval = m_key++;
    m_entitiesByKey.insert(std::make_pair(retval, 
                                          reinterpret_cast<unsigned long int>(e)));
    return retval;
  }
  
  void erase(const eint key) {
    m_entitiesByKey.erase(key);
  }
  EntityId getEntity(const eint key) const {
    EntityId entity;
    std::map<eint, unsigned long int>::const_iterator it = m_entitiesByKey.find(key);
    if(it != m_entitiesByKey.end())
      entity = static_cast<EntityId>(it->second);
    return entity;
  }
  void getEntities(std::set<EntityId>& resultSet) const {
    for(std::map<eint, unsigned long int>::const_iterator it = m_entitiesByKey.begin();
	it != m_entitiesByKey.end();
	++it){
      resultSet.insert(static_cast<EntityId>(it->second));
    }

  }
  void purgeStarted() {
    check_error(!m_purgeStatus);
    m_purgeStatus = true;
  }
  void purgeEnded() {
    check_error(m_purgeStatus);
    m_purgeStatus = false;
  }
  bool isPurging() const {
    return m_purgeStatus;
  }
  bool isPooled(Entity* e) const {
    return m_discardedEntities.find(e) != m_discardedEntities.end();
  }
  bool gcActive() const {return m_gcActive;}
  unsigned int garbageCollect() {
    // Flag activation of garbage collector
    m_gcActive = true;
    
    unsigned int count(0);
    while(!m_discardedEntities.empty()){
      std::set<Entity*>::iterator it = m_discardedEntities.begin();
      Entity* entity = *it;
      m_discardedEntities.erase(entity);
      checkError(isPurging() || entity->canBeDeleted(),
		 "Key:" << entity->getKey() << " RefCount:" << entity->refCount());
      debugMsg("Entity:garbageCollect",
               "Garbage collecting entity " << entity->getEntityName() << "(" << 
               entity->getKey() << ")");
      delete entity;
      count++;
    }

    // Flag completion of garbage collector
    m_gcActive = false;

    return count;

  }
  bool gcRequired() const {return m_gcRequired;}
  void discard(Entity* e) {
    m_discardedEntities.erase(e);
  }
  void pool(Entity* e) {
    m_discardedEntities.insert(e);
  }
 private:
  EntityInternals(const EntityInternals& o);

  std::map<eint, unsigned long int> m_entitiesByKey;
  std::set<Entity*> m_discardedEntities;
  bool m_purgeStatus, m_gcActive, m_gcRequired;
  int m_key;
};


namespace {
static EntityInternals entityInternals;
#ifdef __APPLE__
static pthread_mutex_t entityMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
static pthread_mutex_t entityMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

typedef std::pair<MutexGrabber, boost::reference_wrapper<EntityInternals> >
internals_accessor;
internals_accessor internals() {
  MutexGrabber grabber(entityMutex);
  return std::make_pair<MutexGrabber,
                        boost::reference_wrapper<EntityInternals> >(grabber, 
                                                                       boost::ref(entityInternals));
}
}


Entity::Entity(): m_externalEntity(), m_key(0), m_refCount(1), m_discarded(false), 
                  m_dependents() {
  internals_accessor i(internals());
  m_key = i.second.get().allocateKey(this);
  check_error(!i.second.get().isPurging());
  debugMsg("Entity:Entity", "Allocating " << m_key);
}

Entity::~Entity(){
  internals_accessor i = internals();
  checkError(i.second.get().gcActive() || !i.second.get().gcRequired(), 
             m_key << " deleted outside of gabage collection when prohibited from " <<
             "doing so.");
  i.second.get().discard(this);
  discard(false);
}

void Entity::handleDiscard(){
  internals_accessor i = internals();
  if(!i.second.get().isPurging()){
    //explicitly releasing the mutex here because these notifications may cause
    //client code to get executed
    i.first.release();
    // Notify dependents
    for(std::set<Entity*>::const_iterator it = m_dependents.begin(); it != m_dependents.end(); ++it){
      Entity* entity = *it;
      entity->notifyDiscarded(this);
    }
    
    m_dependents.clear();
    
    check_error(m_externalEntity.isNoId() || m_externalEntity.isValid());
    // If this entity has been integrated with an external entity, then delete the external
    // entity.
    if(!m_externalEntity.isNoId())
      m_externalEntity->discard();
    
    debugMsg("Entity:discard", "Deallocating " << m_key);
    
    condDebugMsg(!canBeDeleted(), "Entity:warning",
                 "(" << getKey() << ") being deleted with " << m_refCount << " outstanding references.");
  }
  i.second.get().erase(m_key);
}


const std::string& Entity::getEntityType() const {
  static const std::string ENTITY_STR("Entity");
  return ENTITY_STR;
}

  std::string Entity::toString() const
  {
	  std::stringstream sstr;
	  sstr << getEntityName() << "(" << getKey() << ")";
	  return sstr.str();
  }

  // By default, same thing as toString()
  std::string Entity::toLongString() const
  {
	  return toString();
  }

const std::string& Entity::getEntityName() const {
  return getName();
}

const std::string& Entity::getName() const {
  static const std::string NO_NAME("NO_NAME_Entity");
  return NO_NAME;
}

  bool Entity::canBeCompared(const EntityId) const{ return true;}

  EntityId Entity::getEntity(const eint key){
    return internals().second.get().getEntity(key);
  }

  void Entity::getEntities(std::set<EntityId>& resultSet){
    return internals().second.get().getEntities(resultSet);
  }

  void Entity::setExternalEntity(const EntityId externalEntity){
    check_error(m_externalEntity.isNoId());
    check_error(externalEntity.isValid());
    m_externalEntity = externalEntity;
  }

  void Entity::setExternalPSEntity(const PSEntity* externalEntity) {
    m_externalEntity = Entity::getEntity(externalEntity->getEntityKey());
  }

  void Entity::clearExternalEntity(){
    m_externalEntity = EntityId::noId();
  }

  void Entity::clearExternalPSEntity() {
    clearExternalEntity();
  }

const EntityId Entity::getExternalEntity() const{
  check_error(m_externalEntity.isNoId() || m_externalEntity.isValid());
  return m_externalEntity;
}

const PSEntity* Entity::getExternalPSEntity() const {
  return dynamic_cast<const PSEntity*>(static_cast<Entity*>(getExternalEntity()));
}

  void Entity::purgeStarted(){
    internals().second.get().purgeStarted();
  }

void Entity::purgeEnded(){
  internals().second.get().purgeEnded();
}

bool Entity::isPurging(){
  return internals().second.get().isPurging();
}

  unsigned int Entity::refCount() const { return m_refCount; }

  void Entity::incRefCount() {m_refCount++;}

  bool Entity::decRefCount() {
    if(m_refCount > 0)
      m_refCount--;

    debugMsg("Entity:decRefCount",
	     "Decremented ref count of " << toString() << "(" << getKey() << ") to " <<
	     m_refCount);

    if(m_refCount == 0 && !m_discarded){
      discard();
      return true;
    }

    return false;
  }

  bool Entity::canBeDeleted() const{ return m_refCount < 2;}

  void Entity::discard(bool pool){
    if(m_discarded)
      return;

    m_discarded = true;
    m_refCount = 0;

    handleDiscard();

    if(pool)
      internals().second.get().pool(this);
  }

  bool Entity::isDiscarded() const {
    return m_discarded;
  }

  void Entity::addDependent(Entity* entity){
    m_dependents.insert(entity);
  }

  void Entity::removeDependent(Entity* entity){
    m_dependents.erase(entity);
  }

  void Entity::notifyDiscarded(const Entity*) {}

  bool Entity::isPooled(Entity* entity) {
    return internals().second.get().isPooled(entity);
  }

  unsigned int Entity::garbageCollect(){
    return internals().second.get().garbageCollect();
  }
}
