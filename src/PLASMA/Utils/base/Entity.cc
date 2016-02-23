#include "Entity.hh"
#include "Debug.hh"
#include "Mutex.hh"

#include <sstream>

#include <boost/ref.hpp>

//TODO: figure out how to handle notification of dependent entities

namespace EUROPA {
class EntityInternals {
 public:
  EntityInternals(): m_entitiesByKey(), m_purgeStatus(false), m_key(0) {}

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


Entity::Entity(): m_key(0), m_refCount(1), m_dependents() {
  internals_accessor i(internals());
  m_key = i.second.get().allocateKey(this);
  debugMsg("Entity:Entity", "Allocating " << m_key);
}

Entity::~Entity(){
  check_runtime_error(decRefCount());
  internals_accessor i = internals();
  i.second.get().erase(m_key); //Is there a good way to RAII this?
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

    if(m_refCount == 0) {
      //discard(); TODO: what to do here?
      return true;
    }

    return false;
  }

  bool Entity::canBeDeleted() const{ return m_refCount < 2;}

}
