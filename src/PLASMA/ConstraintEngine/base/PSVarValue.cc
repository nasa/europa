#include "PSVarValue.hh"
#include "Debug.hh"
#include "Entity.hh"

#include <iomanip>

namespace EUROPA {

  std::string PSVarValue::toString() const {
    std::ostringstream os;

    switch (getType()) {
    case INTEGER:
      os << asInt();
      break;
    case DOUBLE:
      os << std::setprecision(MAX_PRECISION);
      os << asDouble();
      break;
    case BOOLEAN:
      os << asBoolean();
      break;
    case STRING:
      os << asString();
      break;
    case OBJECT:
      {
        PSEntity* obj = asObject();
        os << "OBJECT:" << obj->toString();
      }
      break;

    default:
      check_error(ALWAYS_FAILS, "Unknown type");    
    }

    return os.str();
  }      

  PSVarValue::PSVarValue(const double val, const PSVarType type) : m_val(val), m_type(type) {}
  PSVarValue::PSVarValue(const edouble val, const PSVarType type) : m_val(cast_double(val)), m_type(type) {}

  PSVarType PSVarValue::getType() const {return m_type;}

  int PSVarValue::asInt() const {check_runtime_error(m_type == INTEGER); return (int) m_val;}
  
  double PSVarValue::asDouble() const {return m_val;}

  bool PSVarValue::asBoolean() const {check_runtime_error(m_type == BOOLEAN); return (bool) m_val;}

  const std::string& PSVarValue::asString() const {
    check_runtime_error(m_type == STRING);
    return LabelStr(m_val).toString();
  }

  PSEntity* PSVarValue::asObject() const {
    check_runtime_error(m_type == OBJECT);
    // only works if PSEntity remains pure virtual
    //Id<PSEntity> id(Entity::getEntity(edouble(m_val)));
    //EntityId entity = Entity::getEntity(edouble(m_val));
    EntityId entity = Entity::getEntity((edouble) m_val);
    return (PSEntity *) entity;
  }


}
