
#include "PSUtils.hh"
#include <sstream>

namespace EUROPA 
{
  PSEntity::PSEntity() 
  {      
  }

  PSEntity::PSEntity(const EntityId& entity)
      : m_entity(entity)
  {      
  }
  
  PSEntityKey PSEntity::getEntityKey() const 
  {
      return (getEntity().isId() ? PSEntityKey(getEntity()->getKey()) : PSEntityKey(-1));
  }
  
  const std::string NULL_STR("NULL");
  const std::string UNKNOWN_STR("UNKNOWN");
  
  const std::string& PSEntity::getEntityName() const 
  {      
      return (getEntity().isId() ? getEntity()->getName().toString() : NULL_STR);
  }
  
  const std::string& PSEntity::getEntityType() const {return UNKNOWN_STR;}
  
  std::string PSEntity::toString() const
  {
  	std::ostringstream os;  	
  	os << "Entity(" << getEntityKey() << "," << getEntityName() << ")";
  	return os.str();
  }
}


