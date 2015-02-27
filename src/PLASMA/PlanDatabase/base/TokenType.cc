#include "TokenType.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "Utils.hh"

#include <boost/cast.hpp>

namespace EUROPA {

TokenType::TokenType(const ObjectTypeId ot, const std::string& signature)
    : m_id(this)
    , m_objType(ot)
    , m_signature(signature)
    , m_predicateName()
    , m_attributes(0)
    , m_args()
    , m_subgoalsByAttr()
  {
    m_predicateName = signature.substr(signature.find('.') + 1);
  }

  TokenType::~TokenType()
  {
    m_id.remove();
  }

  const TokenTypeId TokenType::getId() const {return m_id;}

  // TODO: should probably cache this?, maybe in the constructor?
  const TokenTypeId TokenType::getParentType() const { return m_objType->getParentType(getId()); }

  const ObjectTypeId TokenType::getObjectType() const { return m_objType; }

  const std::string& TokenType::getPredicateName() const { return m_predicateName; }

  const std::string& TokenType::getSignature() const {return m_signature;}

const std::map<std::string,DataTypeId>& TokenType::getArgs() const { return m_args; }

  // TODO: this should live in one place only
  static RestrictedDT StateDT("TokenStates",SymbolDT::instance(),StateDomain());

const DataTypeId TokenType::getArgType(const std::string& argName) const {
  std::map<std::string,DataTypeId>::const_iterator it = m_args.find(argName);

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


  void TokenType::addArg(const DataTypeId type, const std::string& name)
  {
    checkRuntimeError(m_args.find(name) == m_args.end(),
		      m_objType->getName() << "." << m_predicateName
		      << " already has a parameter called " << name);
    m_args[name] = type;
  }

  int TokenType::getAttributes() const
  {
    return m_attributes;
  }

  void TokenType::setAttributes(int attrs)
  {
    m_attributes=attrs;
  }

  void TokenType::addAttributes(int attrMask)
  {
    m_attributes |= attrMask;
  }

  bool TokenType::hasAttributes( int attrMask ) const
  {
    return (m_attributes & attrMask) == attrMask;
  }

std::string TokenType::toString() const {
  std::stringstream oss;
  //oss << getId() << " - ";
  oss << "\t" <<getName() <<" ( ";
  for (std::map<std::string,DataTypeId>::const_iterator it = m_args.begin(); it != m_args.end(); ++it) {
    oss<< it->first;
    oss<< " ";
  }
  oss<<")";

  return oss.str();
}

std::string TokenType::toLongString() const {
  std::stringstream oss;
  //oss << getId() << " - ";
  oss << "\t" <<getName() <<" ( ";
  for (std::map<std::string,DataTypeId>::const_iterator it = m_args.begin(); it != m_args.end(); ++it) {
    oss<< it->first;
    oss<< " ";
  }
  oss<<")";

  /**
   * TODO: condition on variable v and no effect changes v, then condition
   * needs to be taken as an side-effect
   */
  if(hasAttributes(PSTokenType::ACTION)){
    oss << ":" << std::endl;

    oss << "\t\t" << "Conditions:" << std::endl;
    PSList<PSTokenType*> conditions = getSubgoalsByAttr( PSTokenType::CONDITION);

    for ( int i = 0; i < conditions.size(); i++ ){
      TokenType* tt = boost::polymorphic_cast<TokenType*>(conditions.get(i));
      oss << "\t\t\t";
      oss<< tt->toLongString();
    }

    oss << "\t\t" << "Effects:" << std::endl;
    PSList<PSTokenType*> effects = getSubgoalsByAttr( PSTokenType::EFFECT);

    for ( int i = 0; i < effects.size(); i++ ){
      TokenType* tt = boost::polymorphic_cast<TokenType*>(effects.get(i));
      oss << "\t\t\t";
      oss<< tt->toLongString();
    }
  }
  else
    oss << std::endl;

  return oss.str();
}

  void TokenType::addSubgoalByAttr( TokenTypeId type, int attr ){
    for( int attrMask = 1; attrMask <= attr; attrMask = attrMask << 1 ){
      if( ( attr & attrMask ) == attrMask ){
	m_subgoalsByAttr[ attrMask ].push_back(id_cast<PSTokenType>(type));
      }
    }
  }

  PSList<PSTokenType*> TokenType::getSubgoalsByAttr( int attrMask) const{
    std::map< int , PSList<PSTokenType*> >::const_iterator cit =  m_subgoalsByAttr.find( attrMask );

    PSList<PSTokenType*> retEmpty;
    return ( cit == m_subgoalsByAttr.end() ) ?  retEmpty : cit->second ;
  }

PSList<std::string> TokenType::getParameterNames() const {
  PSList<std::string> retval;
  for (std::map<std::string,DataTypeId>::const_iterator it = m_args.begin(); it != m_args.end(); ++it) {
    retval.push_back(it->first);
  }
  return retval;
}

PSDataType* TokenType::getParameterType(int index) const {
  check_error(static_cast<unsigned int>(index) < m_args.size(), "Index out of bounds");
  std::map<std::string,DataTypeId>::const_iterator it = m_args.begin();
  while (index-- > 0) ++it;
  return it->second;
}

  PSDataType* TokenType::getParameterType(const std::string& name) const {
    return getArgType(name.c_str());
  }

}
