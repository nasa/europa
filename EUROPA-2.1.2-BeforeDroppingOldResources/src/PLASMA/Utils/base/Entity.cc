#include "Entity.hh"
#include "Debug.hh"
#include <sstream>

namespace EUROPA {

  Entity::Entity(): m_key(allocateKey()), m_refCount(1), m_discarded(false){
    entitiesByKey().insert(std::pair<int, unsigned long int>(m_key, (unsigned long int) this));
    check_error(!isPurging());
    debugMsg("Entity:Entity", "Allocating " << m_key);
  }

  Entity::~Entity(){
    checkError(gcActive() || ! gcRequired(), m_key << " deleted outside of gabage collection when prohibited from doing so.");
    discardedEntities().erase(this);
    discard(false);
  }

  void Entity::handleDiscard(){
    if(!Entity::isPurging()){

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
    entitiesByKey().erase(m_key);
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

  const std::string& Entity::getEntityName() const
  {
	  return getName().toString();
  }
  
  const LabelStr& Entity::getName() const {
	  static const LabelStr NO_NAME("NO_NAME_Entity");
	  return NO_NAME;
  }
  
  bool Entity::canBeCompared(const EntityId&) const{ return true;}

  EntityId Entity::getEntity(int key){
    EntityId entity;
    std::map<int, unsigned long int>::const_iterator it = entitiesByKey().find(key);
    if(it != entitiesByKey().end())
      entity = (EntityId) it->second;
    return entity;
  }

  void Entity::getEntities(std::set<EntityId>& resultSet){
    for(std::map<int, unsigned long int>::const_iterator it = entitiesByKey().begin();
	it != entitiesByKey().end();
	++it){
      resultSet.insert((EntityId) it->second);
    }
  }

  void Entity::setExternalEntity(const EntityId& externalEntity){
    check_error(m_externalEntity.isNoId());
    check_error(externalEntity.isValid());
    m_externalEntity = externalEntity;
  }

  void Entity::setExternalPSEntity(const PSEntity* externalEntity) {
    m_externalEntity = Entity::getEntity(externalEntity->getKey());
  }

  void Entity::clearExternalEntity(){
    m_externalEntity = EntityId::noId();
  }
  
  void Entity::clearExternalPSEntity() {
    clearExternalEntity();
  }

  const EntityId& Entity::getExternalEntity() const{
    check_error(m_externalEntity.isNoId() || m_externalEntity.isValid());
    return m_externalEntity;
  }
  
  const PSEntity* Entity::getExternalPSEntity() const {
    return (const PSEntity*) getExternalEntity();
  }

  std::map<int, unsigned long int>& Entity::entitiesByKey(){
    static std::map<int, unsigned long int> sl_entitiesByKey;
    return sl_entitiesByKey;
  }

  void Entity::purgeStarted(){
    check_error(!isPurging());
    getPurgeStatus() = true;
  }

  void Entity::purgeEnded(){
    check_error(isPurging());
    getPurgeStatus() = false;
  }

  bool Entity::isPurging(){
    return getPurgeStatus();
  }

  bool& Entity::getPurgeStatus(){
    static bool sl_isPurging(false);
    return sl_isPurging;
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
      discardedEntities().insert(this);
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

  void Entity::notifyDiscarded(const Entity* entity) {}

  bool Entity::isPooled(Entity* entity) {
    std::set<Entity*>& entities = discardedEntities();
    return entities.find(entity) != entities.end();
  }

  unsigned int Entity::garbageCollect(){
    // Flag activation of garbage collector
    gcActive() = true;

    std::set<Entity*>& entities = discardedEntities();
    unsigned int count(0);
    while(!entities.empty()){
      std::set<Entity*>::iterator it = entities.begin();
      Entity* entity = *it;
      entities.erase(entity);
      checkError(isPurging() || entity->canBeDeleted(), 
		 "Key:" << entity->getKey() << " RefCount:" << entity->refCount());
      debugMsg("Entity:garbageCollect", "Garbage collecting entity " << entity->getKey() << "(" << entity << ")");
      delete (Entity*) entity;
      count++;
    }

    // Flag completion of garbage collector
    gcActive() = false;

    return count;
  }

  bool& Entity::gcActive(){
    static bool sl_val(false);
    return sl_val;
  }

  bool& Entity::gcRequired(){
    static bool sl_val(false);
    return sl_val;
  }

  std::set<Entity*>& Entity::discardedEntities(){
    static std::set<Entity*> sl_instance;
    return sl_instance;
  }

  PSEntityKey Entity::allocateKey(){
    static int sl_key(0);
    sl_key++;
    return sl_key;
  }



}
