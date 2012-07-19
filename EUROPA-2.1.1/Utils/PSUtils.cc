
#include "PSUtils.hh"
#include <sstream>

namespace EUROPA 
{
  PSEntity::PSEntity(const EntityId& entity) : m_entity(entity) {}

  PSEntityKey PSEntity::getKey() const {return PSEntityKey(m_entity->getKey());}
  
  const std::string& PSEntity::getName() const {return m_entity->getName().toString();}

  const std::string UNKNOWN("UNKNOWN");
  
  const std::string& PSEntity::getEntityType() const {return UNKNOWN;}
  
  std::string PSEntity::toString()
  {
  	std::ostringstream os;
  	
  	os << "Entity(" << getKey() << "," << getName() << ")";
  	return os.str();
  }
}


