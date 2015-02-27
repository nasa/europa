#include "Debug.hh"
#include "Utils.hh"
#include "Domains.hh"
#include "tinyxml.h"
#include "Object.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "DbClientTransactionLog.hh"

namespace EUROPA {
  DbClientTransactionLog::DbClientTransactionLog(const DbClientId client, bool chronologicalBacktracking)
    : DbClientListener(client)
    , m_bufferedTransactions()
    , m_chronologicalBacktracking(chronologicalBacktracking)
    , m_tokensCreated(0)
    , m_client(client)
  {}

  DbClientTransactionLog::~DbClientTransactionLog(){
    cleanup(m_bufferedTransactions);
  }

  const std::list<TiXmlElement*>& DbClientTransactionLog::getBufferedTransactions() const {return m_bufferedTransactions;}

  bool DbClientTransactionLog::isBool(const std::string& typeName)  {
    return (strcmp(typeName.c_str(),"bool") == 0 ||
            strcmp(typeName.c_str(), "BOOL" ) == 0 ||
            strcmp(typeName.c_str(),BoolDT::NAME().c_str()) == 0);
  }

  bool DbClientTransactionLog::isInt(const std::string& typeName)  {
    return (strcmp(typeName.c_str(),"int") == 0 ||
            strcmp(typeName.c_str(),BoolDT::NAME().c_str()) == 0);
  }

  void DbClientTransactionLog::insertBreakpoint() {
    pushTransaction(allocateXmlElement("breakpoint"));
  }

  void DbClientTransactionLog::removeBreakpoint() {
    check_error(!m_bufferedTransactions.empty());
    checkError(m_bufferedTransactions.back()->Value() == std::string("breakpoint"),
               "Last transaction is a " << m_bufferedTransactions.back()->Value() <<
               ", not a breakpoint.");
    popTransaction();
  }

void DbClientTransactionLog::notifyVariableCreated(const ConstrainedVariableId variable) {
  if(!variable->isInternal()) {
    TiXmlElement * element = allocateXmlElement("var");
    const Domain& baseDomain = variable->baseDomain();
    std::string type = baseDomain.getTypeName();
    if (m_client->getSchema()->isObjectType(type)) {
      ObjectId object = Entity::getTypedEntity<Object>(baseDomain.getLowerBound());
      check_error(object.isValid());
      type = object->getType();
    }
    element->SetAttribute( "type", type );
    element->SetAttribute( "name", variable->getName() );
    debugMsg("notifyVariableCreated"," variable name = " << variable->getName().c_str() << " typeName = " << type << " type = " << baseDomain.getTypeName().c_str());
      
    element->SetAttribute("index", static_cast<int>(m_client->getIndexByVariable(variable)));
      
    if (!baseDomain.isEmpty()) {
      TiXmlElement * value = abstractDomainAsXml(&baseDomain);
      element->LinkEndChild(value);
    }
    pushTransaction(element);
  }
}

void DbClientTransactionLog::notifyVariableDeleted(const ConstrainedVariableId variable) {
  if(!variable->isInternal()) {
    TiXmlElement* element = allocateXmlElement("deletevar");
    element->SetAttribute("index", static_cast<int>(m_client->getIndexByVariable(variable)));
    element->SetAttribute("name", variable->getName() );
    element->SetAttribute("type", variable->baseDomain().getTypeName() );
    pushTransaction(element);
  }
}

  void DbClientTransactionLog::notifyObjectCreated(const ObjectId object){
    const std::vector<const Domain*> noArguments;
    notifyObjectCreated(object, noArguments);
  }

void DbClientTransactionLog::notifyObjectCreated(const ObjectId object,
                                                 const std::vector<const Domain*>& arguments){
  TiXmlElement * element = allocateXmlElement("new");
  element->SetAttribute("name", object->getName());
  element->SetAttribute("type", object->getType());
  std::vector<const Domain*>::const_iterator iter;
  for (iter = arguments.begin() ; iter != arguments.end() ; iter++) {
    element->LinkEndChild(abstractDomainAsXml(*iter));
  }
  pushTransaction(element);
}

  void DbClientTransactionLog::notifyObjectDeleted(const ObjectId object) {
    TiXmlElement* element = allocateXmlElement("deleteobject");
    element->SetAttribute("name", object->getName());
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyClosed(){
    TiXmlElement * element = allocateXmlElement("invoke");
    element->SetAttribute("name", "close");
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyClosed(const std::string& objectType){
    TiXmlElement * element = allocateXmlElement("invoke");
    element->SetAttribute("name", "close");
    element->SetAttribute("identifier", objectType);
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyTokenCreated(const TokenId token){
    TiXmlElement * element = (token->isFact() ? allocateXmlElement("fact") :
                              allocateXmlElement("goal"));
    TiXmlElement * instance = allocateXmlElement("predicateinstance");
    instance->SetAttribute("name", m_tokensCreated++);
    instance->SetAttribute("type", token->getPredicateName());
    instance->SetAttribute("path", m_client->getPathAsString(token));
    element->LinkEndChild(instance);
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyTokenDeleted(const TokenId token,
                                                  const std::string& name) {
    TiXmlElement* element = allocateXmlElement("deletetoken");
    element->SetAttribute("type", token->getPredicateName());
    element->SetAttribute("path", m_client->getPathAsString(token));
    if(!name.empty())
      element->SetAttribute("name", name);
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyConstrained(const ObjectId object, const TokenId predecessor, const TokenId successor){
    TiXmlElement * element = allocateXmlElement("constrain");
    TiXmlElement * object_el = allocateXmlElement("object");
    object_el->SetAttribute("name", object->getName());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(predecessor));
    element->LinkEndChild(tokenAsXml(successor));
    pushTransaction(element);
  }


  void DbClientTransactionLog::notifyFreed(const ObjectId object, const TokenId predecessor, const TokenId successor){
    if(m_chronologicalBacktracking) {
      check_error(strcmp(m_bufferedTransactions.back()->Value(), "constrain") == 0,
                  "Chronological backtracking assumption violated");
      popTransaction();
      return;
    }
    TiXmlElement * element = allocateXmlElement("free");
    TiXmlElement * object_el = allocateXmlElement("object");
    object_el->SetAttribute("name", object->getName());
    element->LinkEndChild(object_el);
    element->LinkEndChild(tokenAsXml(predecessor));
    element->LinkEndChild(tokenAsXml(successor));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyActivated(const TokenId token){
    TiXmlElement * element = allocateXmlElement("activate");
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyMerged(const TokenId token, const TokenId activeToken){
    TiXmlElement * element = allocateXmlElement("merge");
    element->LinkEndChild(tokenAsXml(token));
    element->LinkEndChild(tokenAsXml(activeToken));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyMerged(const TokenId token){
    TiXmlElement * element = allocateXmlElement("merge");
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyRejected(const TokenId token){
    TiXmlElement * element = allocateXmlElement("reject");
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

  void DbClientTransactionLog::notifyCancelled(const TokenId token){
    if (m_chronologicalBacktracking) {
      check_error((strcmp(m_bufferedTransactions.back()->Value(), "activate") == 0) ||
                  (strcmp(m_bufferedTransactions.back()->Value(), "reject") == 0) ||
                  (strcmp(m_bufferedTransactions.back()->Value(), "merge") == 0),
                  "Chronological backtracking assumption violated");
      popTransaction();
      return;
    }
    TiXmlElement * element = allocateXmlElement("cancel");
    element->LinkEndChild(tokenAsXml(token));
    pushTransaction(element);
  }

void DbClientTransactionLog::notifyConstraintCreated(const ConstraintId constraint){
  TiXmlElement * element = allocateXmlElement("invoke");
  element->SetAttribute("name", constraint->getName());
  element->SetAttribute("index", static_cast<int>(m_client->getIndexByConstraint(constraint)));
  const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
  std::vector<ConstrainedVariableId>::const_iterator iter;
  for (iter = variables.begin() ; iter != variables.end() ; iter++) {
    const ConstrainedVariableId variable = *iter;
    element->LinkEndChild(variableAsXml(variable));
    }
  pushTransaction(element);
}

void DbClientTransactionLog::notifyConstraintDeleted(const ConstraintId constraint) {
  TiXmlElement* element = allocateXmlElement("deleteconstraint");
  element->SetAttribute("name", constraint->getName());
  element->SetAttribute("index", static_cast<int>(m_client->getIndexByConstraint(constraint)));
  const std::vector<ConstrainedVariableId>& variables = constraint->getScope();
  std::vector<ConstrainedVariableId>::const_iterator iter;
  for (iter = variables.begin() ; iter != variables.end() ; iter++) {
    const ConstrainedVariableId variable = *iter;
    element->LinkEndChild(variableAsXml(variable));
  }
  pushTransaction(element);
}

	void DbClientTransactionLog::notifyVariableSpecified(const ConstrainedVariableId variable){
		if(!variable->isInternal()) {
			checkError(variable->lastDomain().isSingleton(), variable->toString() << " is not a singleton.");
			TiXmlElement * element = allocateXmlElement("specify");
			element->LinkEndChild(variableAsXml(variable));
			element->LinkEndChild(domainValueAsXml(&variable->lastDomain(), variable->lastDomain().getSingletonValue()));
			pushTransaction(element);
		}
	}

	void DbClientTransactionLog::notifyVariableRestricted(const ConstrainedVariableId variable){
		if(!variable->isInternal()) {
			TiXmlElement * element = allocateXmlElement("restrict");
			element->LinkEndChild(variableAsXml(variable));
			element->LinkEndChild(abstractDomainAsXml(&variable->baseDomain()));
			pushTransaction(element);
		}
  }

  void DbClientTransactionLog::notifyVariableReset(const ConstrainedVariableId variable){
		if(!variable->isInternal()) {
			if (m_chronologicalBacktracking) {
				check_error(strcmp(m_bufferedTransactions.back()->Value(), "specify") == 0,
										"Chronological backtracking assumption violated");
				popTransaction();
				return;
			}
			TiXmlElement * element = allocateXmlElement("reset");
			element->LinkEndChild(variableAsXml(variable));
			pushTransaction(element);
		}
  }

  void DbClientTransactionLog::flush(std::ostream& os){
    std::list<TiXmlElement*>::const_iterator iter;
    for (iter = m_bufferedTransactions.begin() ; iter != m_bufferedTransactions.end() ; iter++) {
      os << **iter << std::endl;
    }
    cleanup(m_bufferedTransactions);
  }

  std::string
  DbClientTransactionLog::domainValueAsString(const Domain * domain, edouble value)
  {
    if (isBool(domain->getTypeName())) {
      return (value == 1 ? "true" : "false");
    }
    else
      if (domain->isNumeric()) {
        // CMG: Do not use snprintf. Not supported on DEC
        std::stringstream ss;
        if (isInt(domain->getTypeName())) {
          ss << cast_int(value);
        } else {
          ss << value;
        }
        return ss.str();
      } else if (LabelStr::isString(domain->getUpperBound())) {
        return LabelStr(value).toString();
      } else {
        ObjectId object = Entity::getTypedEntity<Object>(value);
        check_error(object.isValid());
        return object->getName();
      }
  }

  TiXmlElement *
  DbClientTransactionLog::domainValueAsXml(const Domain * domain, edouble value)
  {
    std::string typeName = domain->getTypeName();
    if (m_client->getSchema()->isObjectType(typeName)) {
      TiXmlElement * element = allocateXmlElement("object");
      element->SetAttribute("value", domainValueAsString(domain, value));
      return element;
    }

    debugMsg("domainValueAsXml"," domain type = " << domain->getTypeName().c_str()  << " domain name = " << typeName.c_str());

    if (isBool(domain->getTypeName())) {
      TiXmlElement * element = allocateXmlElement("value");
      element->SetAttribute("type", "bool");
      element->SetAttribute("name", domainValueAsString(domain, value));
      return(element);
    }
    else {
      if (domain->isNumeric()) {
        TiXmlElement * element = allocateXmlElement("value");
        element->SetAttribute("type", typeName);
        element->SetAttribute("name", domainValueAsString(domain, value));
        return(element);
      }
      else {
        TiXmlElement * element = allocateXmlElement("symbol");
        element->SetAttribute("type", typeName);
        element->SetAttribute("value", domainValueAsString(domain, value));
        return(element);
      }
    }

  }

  TiXmlElement *
  DbClientTransactionLog::abstractDomainAsXml(const Domain * domain)
  {
    check_error(!domain->isEmpty());
    if (domain->isSingleton()) {
      return domainValueAsXml(domain, domain->getSingletonValue());
    } else if (domain->isEnumerated()) {
      TiXmlElement * element = allocateXmlElement("set");
      element->SetAttribute("type", domain->getTypeName());
      std::list<edouble> values;
      domain->getValues(values);
      std::list<edouble>::const_iterator iter;
      for (iter = values.begin() ; iter != values.end() ; iter++) {
        element->LinkEndChild(domainValueAsXml(domain, *iter));
      }
      return element;
    } else if (domain->isInterval()) {
      TiXmlElement * element = allocateXmlElement("interval");
      std::string typeName = domain->getTypeName();
      element->SetAttribute("type",typeName);
      element->SetAttribute("min", domainValueAsString(domain, domain->getLowerBound()));
      element->SetAttribute("max", domainValueAsString(domain, domain->getUpperBound()));
      return element;
    }
    check_error(ALWAYS_FAILS);
    return NULL;
  }

  TiXmlElement *
  DbClientTransactionLog::tokenAsXml(const TokenId token) const
  {
    TiXmlElement * token_el = allocateXmlElement("token");
    token_el->SetAttribute("path", m_client->getPathAsString(token));
    return token_el;
  }

TiXmlElement *
DbClientTransactionLog::variableAsXml(const ConstrainedVariableId variable) const {
  TiXmlElement * var_el = allocateXmlElement("variable");
  const EntityId parent = variable->parent();
  if (parent != EntityId::noId()) {
    if (TokenId::convertable(parent)) {
      TokenId token = parent;
      check_error(token.isValid());
      var_el->SetAttribute("token", m_client->getPathAsString(token));
    }
    else if (ObjectId::convertable(parent)) {
      ObjectId object = parent;
      check_error(object.isValid());
      var_el->SetAttribute("object", object->getName());
    }
    else {
      var_el->SetAttribute("index", static_cast<int>(m_client->getIndexByVariable(variable)));
      return var_el;
    }
  }
  else {
    var_el->SetAttribute("index", static_cast<int>(m_client->getIndexByVariable(variable)));
    return var_el;
  }
  if (variable->getIndex() != ConstrainedVariable::NO_INDEX) {
    var_el->SetAttribute("index", static_cast<int>(variable->getIndex()));
  }
  else {
    var_el->SetAttribute("index", static_cast<int>(m_client->getIndexByVariable(variable)));
    return var_el;
  }
  return var_el;
}

  TiXmlElement * DbClientTransactionLog::allocateXmlElement(const std::string& name) const {
    TiXmlElement * element = new TiXmlElement(name);
    return element;
  }

  void DbClientTransactionLog::pushTransaction(TiXmlElement * tx){
    m_bufferedTransactions.push_back(tx);
  }

  void DbClientTransactionLog::popTransaction(){
    TiXmlElement* tx = m_bufferedTransactions.back();
    m_bufferedTransactions.pop_back();
    delete tx;
  }

}
