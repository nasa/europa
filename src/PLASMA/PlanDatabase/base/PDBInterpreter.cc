/**
 * @file PDBInterpreter.cc
 * @brief Core classes to interpret XML statements
 * @author Javier Barreiro
 * @date December, 2006
 */

#include "PDBInterpreter.hh"

#include "Debug.hh"
#include "Error.hh"
#include "Schema.hh"
#include "Utils.hh"
#include "Token.hh"


namespace EUROPA {

  /*
   * DataRef
   */
  DataRef DataRef::null;

  DataRef::DataRef()
    : m_value(ConstrainedVariableId::noId())
  {
  }

  DataRef::DataRef(const ConstrainedVariableId v)
    : m_value(v)
  {
  }

  DataRef::~DataRef()
  {
  }

  const ConstrainedVariableId DataRef::getValue() { return m_value; }

  /*
   * EvalContext
   */
  EvalContext::EvalContext(EvalContext* parent)
      : m_parent(parent), m_variables(), m_tokens()
  {
  }

  EvalContext::~EvalContext()
  {
  }

  void EvalContext::addVar(const std::string& name,const ConstrainedVariableId v)
  {
    m_variables[name] = v;
    debugMsg("Interpreter:EvalContext","Added var:" << name << " to EvalContext");
  }

  ConstrainedVariableId EvalContext::getVar(const std::string& name)
  {
    std::map<std::string,ConstrainedVariableId>::iterator it =
      m_variables.find(name);

    if( it != m_variables.end() )
      return it->second;
    else if (m_parent != NULL)
      return m_parent->getVar(name);
    else
      return ConstrainedVariableId::noId();
  }

  void EvalContext::addToken(const std::string& name,const TokenId t)
  {
    m_tokens[name] = t;
  }

  TokenId EvalContext::getToken(const std::string& name)
  {
    std::map<std::string,TokenId>::iterator it =
      m_tokens.find(name);

    if( it != m_tokens.end() )
      return it->second;
    else if (m_parent != NULL)
      return m_parent->getToken(name);
    else
      return TokenId::noId();
  }

void* EvalContext::getElement(const std::string&) const {return NULL;}

  std::string EvalContext::toString() const
  {
    std::ostringstream os;

    if (m_parent == NULL)
      os << "EvalContext {" << std::endl;
    else
      os << m_parent->toString();

    std::map<std::string,ConstrainedVariableId>::const_iterator varIt = m_variables.begin();
    os << "    vars {";
    for (;varIt != m_variables.end();++varIt)
      os << varIt->first << " " << varIt->second->toString() << ",";
    os << "    }" << std::endl;

    std::map<std::string,TokenId>::const_iterator tokenIt = m_tokens.begin();
    os << "    tokens {";
    for (;tokenIt != m_tokens.end();++tokenIt)
      os << tokenIt->first << " " << tokenIt->second->getPredicateName() << ",";
    os << "    }"  << std::endl;

    if (m_parent == NULL)
      os << "}" << std::endl;

    return os.str();
  }

  const DataTypeId Expr::getDataType() const
  {
	  return VoidDT::instance();
  }

  std::string Expr::toString() const
  {
	  return "Expr";
  }

ExprList::ExprList() : m_children() {}

  ExprList::~ExprList()
  {
    for(std::vector<Expr*>::iterator it = m_children.begin(); it != m_children.end();
        ++it)
      delete *it;
  }

  const std::vector<Expr*>& ExprList::getChildren() const { return m_children; }

  DataRef ExprList::eval(EvalContext& context) const
  {
      DataRef result;
      for(std::vector<Expr*>::const_iterator it = m_children.begin(); 
          it != m_children.end(); ++it)
        result = (*it)->eval(context);

      return result;
  }

  void ExprList::addChild(Expr* child)
  {
      m_children.push_back(child);
  }

  std::string ExprList::toString() const
  {
      std::ostringstream os;

      for(std::vector<Expr*>::const_iterator it = m_children.begin(); 
          it != m_children.end(); ++it)
        os << (*it)->toString() << std::endl;

      return os.str();
  }

  ExprNoop::ExprNoop(const std::string& str)
      : m_str(str)
  {
  }

  ExprNoop::~ExprNoop()
  {
  }

DataRef ExprNoop::eval(EvalContext&) const {
  std::cout << "Noop:" << m_str << std::endl;
  return DataRef::null;
}
}

