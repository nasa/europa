
#include "PSUtils.hh"
#include <sstream>

namespace EUROPA 
{
  PSEntity::PSEntity()
  	:m_key(allocateKey())
  {      	  
  }

  int PSEntity::allocateKey(){
    static int sl_key(0);
    sl_key++;
    return sl_key;
  }
  
  const std::string& PSEntity::getEntityType() const {
	  static const std::string UNKNOWN_STR("UNKNOWN");
	  return UNKNOWN_STR; 
  }
  
  std::string PSEntity::toString() const
  {
	  std::stringstream sstr;
	  sstr << getEntityName() << "(" << getKey() << ")";
	  return sstr.str();
  }
}


