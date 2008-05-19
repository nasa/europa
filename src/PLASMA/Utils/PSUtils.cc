
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

  const std::string& PSEntity::getEntityName() const 
  {      
	  return getName().toString();
  }
  
  const LabelStr& PSEntity::getName() const {
	  static const LabelStr NO_NAME("NO_NAME_PSEntity");
	  return NO_NAME;
  }
  
  const std::string& PSEntity::getEntityType() const {
	  static const std::string UNKNOWN_STR("UNKNOWN");
	  return UNKNOWN_STR; 
  }
  
  
  std::string PSEntity::toString() const
  {
  	std::ostringstream os;  	
  	os << "PSEntity(" << getKey() << "," << getName() << ")";
  	return os.str();
  }
}


