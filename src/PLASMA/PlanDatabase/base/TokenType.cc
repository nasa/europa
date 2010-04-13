#include "TokenType.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "Utils.hh"

namespace EUROPA {

    TokenType::TokenType(const ObjectTypeId& ot, const LabelStr& signature)
        : m_id(this)
        , m_objType(ot)
        , m_signature(signature)
    {
        m_predicateName = signature.getElement(1,".");
    }

    TokenType::~TokenType()
    {
        m_id.remove();
    }

    const TokenTypeId& TokenType::getId() const {return m_id;}

    // TODO: should probably cache this?, maybe in the constructor?
    const TokenTypeId& TokenType::getParentType() const { return m_objType->getParentType(getId()); }

    const LabelStr& TokenType::getPredicateName() const { return m_predicateName; }

    const LabelStr& TokenType::getSignature() const {return m_signature;}

    const std::map<LabelStr,DataTypeId>& TokenType::getArgs() const { return m_args; }

    // TODO: this should live in one place only
    static RestrictedDT StateDT("TokenStates",SymbolDT::instance(),StateDomain());

    const DataTypeId& TokenType::getArgType(const char* argName) const
    {
        std::map<LabelStr,DataTypeId>::const_iterator it = m_args.find(argName);

        if (it != m_args.end())
            return it->second;

        std::string name(argName);

        if (name == "state")
            return StateDT.getId();

        if (name == "object")
            return m_objType->getVarType();

        if (name=="start" || name=="end" || name=="duration")
            return IntDT::instance();

        TokenTypeId parent = getParentType();
        if (parent.isId())
            return parent->getArgType(argName);

        return DataTypeId::noId();
    }


    void TokenType::addArg(const DataTypeId& type, const LabelStr& name)
    {
        m_args[name] = type;
    }

    PSList<std::string> TokenType::getParameterNames() const {
   	  PSList<std::string> retval;
   	  for (std::map<LabelStr,DataTypeId>::const_iterator it = m_args.begin(); it != m_args.end(); ++it) {
   		  retval.push_back(it->first.toString());
   	  }
   	  return retval;
    }

    PSDataType* TokenType::getParameterType(int index) const {
      check_error((unsigned int) index < m_args.size(), "Index out of bounds");
   	  std::map<LabelStr,DataTypeId>::const_iterator it = m_args.begin();
   	  while (index-- > 0) ++it;
   	  return it->second;
    }

    PSDataType* TokenType::getParameterType(const std::string& name) const {
      return getArgType(name.c_str());
    }

}
