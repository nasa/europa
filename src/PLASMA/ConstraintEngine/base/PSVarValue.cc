#include "PSVarValue.hh"
#include "Debug.hh"

namespace EUROPA {

	std::string PSVarValue::toString() const {
		std::ostringstream os;

		switch (getType()) {
		case INTEGER:
			os << asInt();
			break;
		case DOUBLE:
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
			os << asObject()->toString();
		}
		break;

		default:
			check_error(ALWAYS_FAILS, "Unknown type");    
		}

		return os.str();
	}      

  PSVarValue::PSVarValue(const double val, const PSVarType type) : m_val(val), m_type(type) {}
  PSVarType PSVarValue::getType() const {return m_type;}

  int PSVarValue::asInt() const {check_runtime_error(m_type == INTEGER); return (int) m_val;}
  
  double PSVarValue::asDouble() const {return m_val;}

  bool PSVarValue::asBoolean() const {check_runtime_error(m_type == BOOLEAN); return (bool) m_val;}

  const std::string& PSVarValue::asString() const {
    check_runtime_error(m_type == STRING);
    return LabelStr(m_val).toString();
  }

  PSEntity* PSVarValue::asObject() const 
  {
    check_runtime_error(m_type == OBJECT);
    Id<PSEntity> id(m_val);
    return (PSEntity *) id;
  }


}
