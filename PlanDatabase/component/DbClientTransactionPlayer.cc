#include "Debug.hh"
#include "tinyxml.h"
#include "EnumeratedDomain.hh"
#include "BoolDomain.hh"
#include "StringDomain.hh"
#include "SymbolDomain.hh"
#include "ConstraintEngine.hh"
#include "TypeFactory.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "DbClient.hh"
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include <algorithm>

namespace EUROPA {

  static const std::vector<int> 
  pathAsVector(const std::string & path) {
    size_t path_back, path_front, path_end;
    std::vector<int> result;
    path_back = 0;
    path_end = path.size();
    while (true) {
      path_front = path.find('.', path_back);
      if ((path_front == std::string::npos) || (path_front > path_end)) {
        break;
      }
      std::string numstr = path.substr(path_back, path_front-path_back);
      result.push_back(atoi(numstr.c_str()));
      path_back = path_front + 1;
    }
    std::string numstr = path.substr(path_back, path_end-path_back);
    result.push_back(atoi(numstr.c_str()));
    return result;
  }

  DbClientTransactionPlayer::DbClientTransactionPlayer(const DbClientId & client)
    : m_client(client), m_objectCount(0), m_varCount(0) {
  }

  DbClientTransactionPlayer::~DbClientTransactionPlayer() {
  }

  void DbClientTransactionPlayer::play(std::istream& is) {
    int txCounter = 0;
    check_error(is, "Invalid input stream for playing transactions.");

    while (!is.eof()) {
      if (is.peek() != '<') {
        is.get(); // discard characters up to '<'
        continue;
      }

      TiXmlElement tx("");
      is >> tx;
      processTransaction(tx);
      txCounter++;
    }
    check_error(txCounter > 0, "Failed to find any transactions in stream.");
  }

  void DbClientTransactionPlayer::play(const DbClientTransactionLogId& txLog) {
    const std::list<TiXmlElement*>& transactions = txLog->getBufferedTransactions();
    for (std::list<TiXmlElement*>::const_iterator it = transactions.begin();
         it != transactions.end();
         ++it) {
      const TiXmlElement& tx = **it;
      processTransaction(tx);
    }
  }

  void DbClientTransactionPlayer::processTransaction(const TiXmlElement & element) {
    static unsigned int sl_txCount(0);
    const char * tagname = element.Value();

    sl_txCount++;
    if (strcmp(tagname, "class") == 0)
      playDefineClass(element);
    else if (strcmp(tagname, "enum") == 0)
      playDefineEnumeration(element);
    else if (strcmp(tagname, "compat") == 0)
      playDefineCompat(element);
    else if (strcmp(tagname, "var") == 0)
      playVariableCreated(element);
    else if (strcmp(tagname, "new") == 0)
      playObjectCreated(element);
    else if (strcmp(tagname, "goal") == 0)
      playTokenCreated(element);
    else if (strcmp(tagname, "constrain") == 0)
      playConstrained(element);
    else if (strcmp(tagname, "free") == 0)
      playFreed(element);
    else if (strcmp(tagname, "activate") == 0)
      playActivated(element);
    else if (strcmp(tagname, "merge") == 0)
      playMerged(element);
    else if (strcmp(tagname, "reject") == 0)
      playRejected(element);
    else if (strcmp(tagname, "cancel") == 0)
      playCancelled(element);
    else if (strcmp(tagname, "specify") == 0)
      playVariableSpecified(element);
    else if (strcmp(tagname, "reset") == 0)
      playVariableReset(element);
    else if (strcmp(tagname, "invoke") == 0)
      playInvokeConstraint(element);
    else {
      check_error(strcmp(tagname, "nddl") == 0, "Unknown tag name" + std::string(tagname));
      for (TiXmlElement * child_el = element.FirstChildElement() ;
           child_el != NULL ; child_el = child_el->NextSiblingElement()) {
        processTransaction(*child_el);
	if (!m_client->propagate())
	  return;
      }
    }
    m_client->propagate();
  }

  void DbClientTransactionPlayer::playVariableCreated(const TiXmlElement & element) {
    const char * type = element.Attribute("type");
    check_error(type != NULL);
    const char * name = element.Attribute("name");
    check_error(name != NULL);

    TiXmlElement * value = element.FirstChildElement();

    ConstrainedVariableId variable = xmlAsCreateVariable(type, name, value);
    check_error(variable.isValid());

    debugMsg("DbClientTransactionPlayer:playVariableCreated", "after xmlAsCreateVariable var = " << name << " type = " << type << " domain typeName =  " << variable->baseDomain().getTypeName().c_str());

    std::string std_name = name;
    m_variables[std_name] = variable;
  }

  void DbClientTransactionPlayer::playObjectCreated(const TiXmlElement & element) {
    const char * name = element.Attribute("name");
    xmlAsValue(element, name);
  }

  void DbClientTransactionPlayer::playTokenCreated(const TiXmlElement & element) {
    const char * relation = element.Attribute("relation");
    if (relation != NULL) {
      // inter-token temporal relation
      const char * origin = element.Attribute("origin");
      const char * target = element.Attribute("target");
      TokenId origin_token = parseToken(origin);
      TokenId target_token = parseToken(target);
      debugMsg("DbClientTransactionPlayer:playTokenCreated",
	       "got token " << origin_token->getKey() << " and " << 
	       target_token->getKey() << " for relation " << relation);
      checkError(origin_token.isValid(), "Invalid token for label '" << origin << "'");
      checkError(target_token.isValid(), "Invalid token for label '" << target << "'");
      if (strcmp(relation, "before") == 0) {
        construct_constraint(precedes, origin, End, target, Start);
        return;
      }
      else if (strcmp(relation, "after") == 0) {
        construct_constraint(precedes, target, End, origin, Start);
        return;
      }
      else if (strcmp(relation, "meets") == 0) {
        construct_constraint(concurrent, origin, End, target, Start);
        return;
      }
      else if (strcmp(relation, "met_by") == 0) {
        construct_constraint(concurrent, origin, Start, target, End);
        return;
      }
      else if ((strcmp(relation, "equal") == 0) || 
               (strcmp(relation, "equals") == 0)) {
        construct_constraint(concurrent, origin, Start, target, Start);
        construct_constraint(concurrent, origin, End, target, End);
        return;
      }
      else if (strcmp(relation, "contains") == 0) {
        construct_constraint(precedes, origin, Start, target, Start);
        construct_constraint(precedes, target, End, origin, End);
        return;
      }
      else if (strcmp(relation, "contained_by") == 0) {
        construct_constraint(precedes, target, Start, origin, Start);
        construct_constraint(precedes, origin, End, target, End);
        return;
      }
      else if (strcmp(relation, "paralleled_by") == 0) {
        construct_constraint(precedes, target, Start, origin, Start);
        construct_constraint(precedes, target, End, origin, End);
        return;
      }
      else if (strcmp(relation, "parallels") == 0) {
        construct_constraint(precedes, origin, Start, target, Start);
        construct_constraint(precedes, origin, End, target, End);
        return;
      }
      else if (strcmp(relation, "starts") == 0) {
        construct_constraint(concurrent, origin, Start, target, Start);
        return;
      }
      else if (strcmp(relation, "ends") == 0) {
        construct_constraint(concurrent, origin, End, target, End);
        return;
      }
      else if (strcmp(relation, "ends_after") == 0) {
        construct_constraint(precedes, target, Start, origin, End);
        return;
      }
      else if (strcmp(relation, "ends_before") == 0) {
        construct_constraint(precedes, origin, End, target, Start);
        return;
      }
      else if (strcmp(relation, "ends_after_start") == 0) {
        construct_constraint(precedes, target, Start, origin, End);
        return;
      }
      else if (strcmp(relation, "starts_before_end") == 0) {
        construct_constraint(precedes, origin, Start, target, End);
        return;
      }
      else if (strcmp(relation, "starts_during") == 0) {
        construct_constraint(precedes, target, Start, origin, Start);
        construct_constraint(precedes, origin, Start, target, End);
        return;
      }
      else if (strcmp(relation, "contains_start") == 0) {
        construct_constraint(precedes, origin, Start, target, Start);
        construct_constraint(precedes, target, Start, origin, End);
        return;
      }
      else if (strcmp(relation, "ends_during") == 0) {
        construct_constraint(precedes, target, Start, origin, End);
        construct_constraint(precedes, origin, End, target, End);
        return;
      }
      else if (strcmp(relation, "contains_end") == 0) {
        construct_constraint(precedes, origin, Start, target, End);
        construct_constraint(precedes, target, End, origin, End);
        return;
      }
      else if (strcmp(relation, "starts_after") == 0) {
        construct_constraint(precedes, target, Start, origin, Start);
        return;
      }
      else if (strcmp(relation, "starts_before") == 0) {
        construct_constraint(precedes, origin, Start, target, Start);
        return;
      }
      check_error(strcmp(relation, "any") == 0,
                  std::string("unknown temporal relation name ") + std::string(relation));
      // The "any" relation is not an actual constraint, so don't create one.
      return;
    }
    // simple token creation
    TiXmlElement * child = element.FirstChildElement();
    check_error(child != NULL);
    const char * type = child->Attribute("type");
    check_error(type != NULL);

    // The type may be qualified with an object name, in which case we should get the
    // object and specify it. We will also have to generate the appropriate type designation
    // by extracting the class from the object
    TokenId token;

    const char * mandatory = element.Attribute("mandatory");
    bool b_mandatory = false;
    if(mandatory != NULL && (strcmp(mandatory, "true") == 0))
      b_mandatory = true;

    if (Schema::instance()->isPredicate(type))
       token = m_client->createToken(type, !b_mandatory);
    else {
      LabelStr typeStr(type);
      LabelStr prefix = typeStr.getElement(0, Schema::getDelimiter());
      LabelStr suffix = typeStr.getElement(1, Schema::getDelimiter());
      ObjectId object = m_client->getObject(prefix.c_str());
      check_error(object.isValid(), "Failed to find an object named " + prefix.toString());
      std::string newType(object->getType().toString() + Schema::getDelimiter() + suffix.toString());
      token = m_client->createToken(newType.c_str(), !b_mandatory);
      m_client->specify(token->getObject(), object);
    }

    // If it is mandatory, then activate immediately so there is less for a
    // client to figure out, or specify in the transactions
    if (b_mandatory){
      m_client->activate(token);
      token->getState()->restrictBaseDomain(token->getState()->lastDomain());
    }

    const char * name = child->Attribute("name");

    if (name != NULL) {
      std::string std_name = name;
      m_tokens[std_name] = token;
    }
  }

  void DbClientTransactionPlayer::playConstrained(const TiXmlElement & element) {
    TiXmlElement * object_el = element.FirstChildElement();
    check_error(object_el != NULL);
    check_error(strcmp(object_el->Value(), "object") == 0);
    const char * name = object_el->Attribute("name");
    check_error(name != NULL);
    ObjectId object = m_client->getObject(name);
    check_error(object.isValid());

    TiXmlElement * token_el = object_el->NextSiblingElement();
    check_error(token_el != NULL);
    TokenId predecessor = xmlAsToken(*token_el);
    check_error(predecessor.isValid());

    TiXmlElement * successor_el = token_el->NextSiblingElement();
    TokenId successor = predecessor;
    if(successor_el != NULL){
      successor = xmlAsToken(*successor_el);
      checkError(successor.isValid(), "Invalid id for successor: " << *successor_el);
    }

    m_client->constrain(object, predecessor, successor);
  }

  void DbClientTransactionPlayer::playFreed(const TiXmlElement & element) {
    TiXmlElement * object_el = element.FirstChildElement();
    check_error(object_el != NULL);
    check_error(strcmp(object_el->Value(), "object") == 0);
    const char * name = object_el->Attribute("name");
    check_error(name != NULL);
    ObjectId object = m_client->getObject(name);
    check_error(object.isValid());

    TiXmlElement * token_el = object_el->NextSiblingElement();
    check_error(token_el != NULL);
    TokenId predecessor = xmlAsToken(*token_el);
    check_error(predecessor.isValid());
    TiXmlElement * successor_el = token_el->NextSiblingElement();
    TokenId successor = predecessor;
    if (successor_el != NULL) {
      successor = xmlAsToken(*successor_el);
      check_error(successor.isValid());
    }
    m_client->free(object, predecessor, successor);
  }

  void DbClientTransactionPlayer::playActivated(const TiXmlElement & element) {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());
    m_client->activate(token);
  }

  void DbClientTransactionPlayer::playMerged(const TiXmlElement & element) {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());

    // It may or may not have another sibling element
    TiXmlElement * active_el = token_el->NextSiblingElement();
    checkError(active_el != NULL, "Active element required for merge.");
    TokenId active_token = xmlAsToken(*active_el);
    check_error(active_token.isValid());
    m_client->merge(token, active_token);
  }

  void DbClientTransactionPlayer::playRejected(const TiXmlElement & element) {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());
    m_client->reject(token);    
  }

  void DbClientTransactionPlayer::playCancelled(const TiXmlElement & element)
  {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());
    m_client->cancel(token);    
  }

  void DbClientTransactionPlayer::playVariableSpecified(const TiXmlElement & element) {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    ConstrainedVariableId variable = xmlAsVariable(*var_el);
    check_error(variable.isValid());

    TiXmlElement * value_el = var_el->NextSiblingElement();
    check_error(value_el != NULL);
    const AbstractDomain * value = xmlAsAbstractDomain(*value_el);
    if (value->isSingleton()) {
      double v = value->getSingletonValue();
      m_client->specify(variable, v);
    } else
      m_client->specify(variable, *value);
  }

  void DbClientTransactionPlayer::playVariableReset(const TiXmlElement & element) {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    m_client->reset(xmlAsVariable(*var_el));
  }

  void DbClientTransactionPlayer::playInvokeConstraint(const TiXmlElement & element) {
    const char * name = element.Attribute("name");
    check_error(name != NULL);
    const char * identifier = element.Attribute("identifier");
    if ((identifier != NULL) || (element.FirstChildElement() == NULL)) {
      // it's a transaction, not a constraint
      playInvokeTransaction(element);
      return;
    }
    // normal constraints
    std::vector<ConstrainedVariableId> variables;
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement())
      variables.push_back(xmlAsVariable(*child_el));
    m_client->createConstraint(name, variables);
  }

  void DbClientTransactionPlayer::playInvokeTransaction(const TiXmlElement & element) {
    const char * name = element.Attribute("name");
    check_error(name != NULL);
    const char * identifier = element.Attribute("identifier");
    if (strcmp(name, "close") == 0) {
      if (identifier == NULL) {
        // close database special case
        m_client->close();
        return;
      }
      m_client->close(identifier);
      return;
    }

    if (strcmp(name, "constrain") == 0) {
      // constrain token(s) special case
      const char * object_name = identifier;
      ObjectId object = m_client->getObject(object_name);
      check_error(object.isValid(),
                  "constrain transaction refers to an undefined object: '"
                   + std::string(object_name) + "'");
      TiXmlElement * predecessor_el = element.FirstChildElement();
      check_error(predecessor_el != NULL, "missing mandatory predecessor identifier for constrain transaction");
      TokenId predecessor = xmlAsToken(*predecessor_el);
      check_error(predecessor.isValid(),
                  "invalid predecessor identifier for constrain transaction");
      
      TiXmlElement * successor_el = predecessor_el->NextSiblingElement();
      TokenId successor = predecessor;
      if (successor_el != NULL) {
        successor = xmlAsToken(*successor_el);
        check_error(successor.isValid(),
                    "invalid successor token identifier for constrain transaction");
      }

      m_client->constrain(object, predecessor, successor);
      return;
    }

    if (strcmp(name, "free") == 0) {
      // free token(s) special case
      const char * object_name = identifier;
      ObjectId object = m_client->getObject(object_name);
      check_error(object.isValid(),
                  "free transaction refers to an undefined object: '"
                   + std::string(object_name) + "'");
      TiXmlElement * predecessor_el = element.FirstChildElement();
      check_error(predecessor_el != NULL, "missing mandatory predecessor identifier for free transaction");
      TokenId predecessor = xmlAsToken(*predecessor_el);
      check_error(predecessor.isValid(),
                  "invalid predecessor identifier for free transaction");
      
      TiXmlElement * successor_el = predecessor_el->NextSiblingElement();
      TokenId successor = predecessor;
      if (successor_el != NULL) {
        successor = xmlAsToken(*successor_el);
        check_error(successor.isValid(),
                    "invalid successor token identifier for free transaction");
      }

      m_client->free(object, predecessor, successor);
      return;
    }

    if (strcmp(name, "activate") == 0) {
      // activate token special case
      std::string token_name = identifier;
      TokenId token = m_tokens[token_name];
      check_error(token.isValid(), "Invalid token name '" + token_name + "' for activation.");
      if(!token->isActive()) // Temporary. Pull out when we scrub test input files
	m_client->activate(token);
      return;
    }

    if (strcmp(name, "merge") == 0) {
      // merge token special case
      std::string token_name = identifier;
      TokenId token = m_tokens[token_name];
      check_error(token.isValid());
      TiXmlElement * active_el = element.FirstChildElement();
      check_error(active_el != NULL);
      TokenId activeToken = xmlAsToken(*active_el);
      check_error(activeToken.isValid());
      m_client->merge(token, activeToken);
      return;
    }

    if (strcmp(name, "reject") == 0) {
      // reject token special case
      std::string token_name = identifier;
      TokenId token = m_tokens[token_name];
      check_error(token.isValid());
      m_client->reject(token);
      return;
    }

    if (strcmp(name, "cancel") == 0) {
      // cancel token special case
      std::string token_name = identifier;
      TokenId token = m_tokens[token_name];
      check_error(token.isValid());
      m_client->cancel(token);
      return;
    }

    if (strcmp(name, "specify") == 0) {
      // specify variable special case
      debugMsg("DbClientTransactionPlayer:playInvokeTransaction", "specifying for " << identifier);
      ConstrainedVariableId variable = parseVariable(identifier);
      debugMsg("DbClientTransactionPlayer:playInvokeTransaction", "found variable " << variable->getKey());
      TiXmlElement * value_el = element.FirstChildElement();
      check_error(value_el != NULL);
      const AbstractDomain * value = xmlAsAbstractDomain(*value_el);
      debugMsg("DbClientTransactionPlayer:playInvokeTransaction", "specifying to " << (*value));
      if (value->isSingleton()) {
        double v = value->getSingletonValue();
        m_client->specify(variable, v);
      } else {
        m_client->specify(variable, *value);
      }
      return;
    }

    check_error(ALWAYS_FAILS, "unexpected transaction invoked: '" + std::string(name) + "'");
  }

  //! string input functions

  ConstrainedVariableId
  DbClientTransactionPlayer::parseVariable(const char * varString)
  {
    check_error(varString != NULL);
    std::string variable = varString;    
    size_t ident_back, ident_front, variable_end;
    ident_back = 0;
    variable_end = variable.size();
    ident_front = variable.find('.', ident_back);
    if ((ident_front == std::string::npos) || (ident_front > variable_end)) {
      // simple identifier (might be a symbol)
      if (m_variables.find(variable) != m_variables.end()) {
        return m_variables[variable];
      } else {
        // presumably a symbol
        return ConstrainedVariableId::noId();
      }
    }
    // compound identifier
    std::string ident = variable.substr(ident_back, ident_front - ident_back);
    TokenId token = m_tokens[ident];
    if (token != token.noId()) {
      std::string name = variable.substr(ident_front + 1);
      const std::vector<ConstrainedVariableId> & variables = token->getVariables();
      std::vector<ConstrainedVariableId>::const_iterator iter = variables.begin();
      while (iter != variables.end()) {
        ConstrainedVariableId variable = *iter++;
        if (LabelStr(name.c_str()) == variable->getName()) {
          return variable;
        }        
      }
      check_error(ALWAYS_FAILS, "Failed to process transaction for " + ident + ":" + name);
    }
    ConstrainedVariableId var = m_variables[ident];
    check_error(var.isValid(), "Invalid id for " + ident);
    ObjectId object = var->specifiedDomain().getSingletonValue();
    check_error(object.isValid());
    var = object->getVariable(LabelStr(varString));
    check_error(var.isValid());
    return var;
  }

  TokenId 
  DbClientTransactionPlayer::parseToken(const char * tokString)
  {
     check_error(tokString != NULL);
     std::string token = tokString;
     return m_tokens[token];
  }

  //! XML input functions

  const AbstractDomain * 
  DbClientTransactionPlayer::xmlAsAbstractDomain(const TiXmlElement & element, const char * name) {
    static unsigned int sl_counter(0);
    sl_counter++;
    const char * tag = element.Value();
    check_error(strcmp(tag, "ident") != 0,
                "ident in transaction xml is deprecated");
    if (strcmp(tag, "new") == 0) {
      const char * type = element.Attribute("type");
      check_error(type != NULL);
      return(new ObjectDomain(xmlAsValue(element, name), type));
    }
    if (strcmp(tag, "id") == 0) {
      const char * name = element.Attribute("name");
      ConstrainedVariableId var = parseVariable(name);
      check_error(var.isValid());
      return(var->baseDomain().copy());
    }
    if (strcmp(tag, "set") == 0)
      return(xmlAsEnumeratedDomain(element));
    if (strcmp(tag, "interval") == 0)
      return(xmlAsIntervalDomain(element));
    if (strcmp(tag, "value") == 0) {
      // New XML style for simple types.
      const char * type = element.Attribute("type");
      check_error(type != NULL, "missing type for domain in transaction XML");
      const char * name = element.Attribute("name");
      check_error(name != NULL, "missing name for domain in transaction XML");

      AbstractDomain * domain = TypeFactory::baseDomain(type).copy();
      check_error(domain != 0, "unknown type, lack of memory, or other problem with domain in transaction XML");
      double value = TypeFactory::createValue(type, name);
      if(domain->isOpen() && !domain->isMember(value))
	domain->insert(value);
      domain->set(value);
      return(domain);
    }
    const char * value_st = element.Attribute("value");
    check_error(value_st != NULL);
    if (strcmp(tag, "symbol") == 0) {
      const char * type = element.Attribute("type");
      check_error(type != NULL);
      AbstractDomain * domain = TypeFactory::baseDomain(type).copy();
      domain->set(TypeFactory::createValue(tag, value_st));
      return(domain);
    }

    check_error(strcmp(tag, "object") == 0);
    ObjectId object = m_client->getObject(value_st);
    check_error(object.isValid());
    return(new ObjectDomain(object, object->getType().c_str()));
  }

  IntervalDomain *
  DbClientTransactionPlayer::xmlAsIntervalDomain(const TiXmlElement & element) {
    const char * type_st = element.Attribute("type");
    check_error(type_st != NULL);
    const char * min_st = element.Attribute("min");
    check_error(min_st != NULL);
    const char * max_st = element.Attribute("max");
    check_error(max_st != NULL);
    IntervalDomain * domain = dynamic_cast<IntervalDomain*>(TypeFactory::baseDomain(type_st).copy());
    check_error(domain != NULL, "type '" + std::string(type_st) + "' should indicate an interval domain type");
    double min = TypeFactory::createValue(type_st, min_st);
    double max = TypeFactory::createValue(type_st, max_st);
    domain->intersect(min, max);
    return domain;
  }
  
  EnumeratedDomain *
  DbClientTransactionPlayer::xmlAsEnumeratedDomain(const TiXmlElement & element) {
    enum { ANY, BOOL, INT, FLOAT, STRING, SYMBOL, OBJECT } type = ANY;
    std::string typeName;
    // determine most specific type
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      std::string thisType;
      if (strcmp(child_el->Value(), "value") == 0)
        // New style XML for simple types: the type is within and tag is always 'value'
        thisType = child_el->Attribute("type");
      else
        // Non-simple type or old style XML for a simple type: type is the tag and/or within (e.g., specific type is within if symbol even old style)
        thisType = child_el->Value();

      debugMsg("DbClientTransactionPlayer:xmlAsEnumeratedDomain", " thisType= " << thisType);

      check_error(thisType != "", "no type for domain in XML");
      if (strcmp(thisType.c_str(), "bool") == 0 ||
          strcmp(thisType.c_str(), "BOOL") == 0 ||
          strcmp(thisType.c_str(), BoolDomain::getDefaultTypeName().toString().c_str()) == 0) {
        if (type == ANY) {
          type = BOOL;
          typeName = "bool";
        }
        if (type == BOOL)
          continue;
      }
      if (strcmp(thisType.c_str(), "int") == 0 ||
          strcmp(thisType.c_str(), "INT") == 0 ||
          strcmp(thisType.c_str(), IntervalIntDomain::getDefaultTypeName().toString().c_str()) == 0) {
        if (type == ANY) {
          type = INT;
          typeName = "int";
        }
        if ((type == FLOAT) || (type == INT))
          continue;
      }
      if (strcmp(thisType.c_str(), "float") == 0 ||
          strcmp(thisType.c_str(), "FLOAT") == 0 ||
          strcmp(thisType.c_str(), IntervalDomain::getDefaultTypeName().toString().c_str()) == 0) {
        if ((type == ANY) || (type == INT)) {
          type = FLOAT;
          typeName = "float";
        }
        if (type == FLOAT)
          continue;
      }
      if (strcmp(thisType.c_str(), "string") == 0 ||
          strcmp(thisType.c_str(), "STRING") == 0 ||
          strcmp(thisType.c_str(), StringDomain::getDefaultTypeName().toString().c_str()) == 0) {
        if (type == ANY) {
          type = STRING;
          typeName = "string";
        }
        if (type == STRING)
          continue;
      }
      if (strcmp(thisType.c_str(), "symbol") == 0) {
        if (type == ANY) {
          type = SYMBOL;
          typeName = child_el->Attribute("type");
        }
        if (type == SYMBOL) {
          check_error(strcmp(typeName.c_str(), child_el->Attribute("type")) == 0,
                      "symbols from different types in the same enumerated set");
          debugMsg("DbClientTransactionPlayer:xmlAsEnumeratedDomain:symbol", " thisType= " << thisType << " typeName = " <<typeName);
          continue;
        }
        debugMsg("DbClientTransactionPlayer:xmlAsEnumeratedDomain:symbol", " thisType= " << thisType << " typeName = " <<typeName);
      }
//      if (strcmp(thisType.c_str(), "object") == 0) {
//        if (type == ANY) {
//          type = OBJECT;
//        }
//        if (type == OBJECT)
//          //!!This needs a similar type check to SYMBOL, just above (but more complex due to inheritance?)
//          continue;
//      }
      check_error(ALWAYS_FAILS, "unknown or inappropriately mixed type(s) for value(s) in an enumerated set");
    }
    check_error(type != ANY);
    // gather the values
    std::list<double> values;
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      const char * value_st = child_el->Attribute("value");
      if (value_st == NULL) // Check for the new style XML for simple types
        value_st = child_el->Attribute("name");
      check_error(value_st != NULL);
      switch (type) {
       case BOOL: case INT: case FLOAT: case STRING: case SYMBOL:
         values.push_back(TypeFactory::createValue(typeName.c_str(), value_st));
         break;
       case OBJECT:
         values.push_back(m_client->getObject(value_st));
         break;
       default:
         check_error(ALWAYS_FAILS);
      }
    }
    // return the domain
    switch (type) {
    case BOOL: case INT: case FLOAT:
      return(new EnumeratedDomain(values, true, typeName.c_str()));
    case STRING: 
      return(new StringDomain(values, typeName.c_str()));
    case SYMBOL: 
      return(new SymbolDomain(values, typeName.c_str()));
    case OBJECT:
      return(new EnumeratedDomain(values, false, typeName.c_str()));
    default:
      check_error(ALWAYS_FAILS);
      return(0);
    }
  }

  double DbClientTransactionPlayer::xmlAsValue(const TiXmlElement & value, const char * name) {
    const char * tag = value.Value();
    if (strcmp(tag, "new") == 0) {
      std::string gen_name;
      if (name == NULL) {
        std::stringstream gen_stream;
        gen_stream << "$OBJECT[" << m_objectCount++ << "]" << std::ends;
        gen_name = gen_stream.str();
        name = gen_name.c_str();
      }
      const char * type = value.Attribute("type");
      check_error(type != NULL);
      std::vector<ConstructorArgument> arguments;
      for (TiXmlElement * child_el = value.FirstChildElement() ;
           child_el != NULL ; child_el = child_el->NextSiblingElement()) {
        const AbstractDomain * domain = xmlAsAbstractDomain(*child_el);
        arguments.push_back(ConstructorArgument(domain->getTypeName(), domain));
      }
      ObjectId object = m_client->createObject(type, name, arguments);
      check_error(object.isValid());

      // Now deallocate domains created for arguments
      for (std::vector<ConstructorArgument>::const_iterator it = arguments.begin(); it != arguments.end(); ++it)
        delete it->second;
      return (double)object;
    }
    if (strcmp(tag, "value") == 0) {
      // New style XML for simple types.
      const char * type_st = value.Attribute("type");
      check_error(type_st != NULL, "missing type for value in transaction XML");
      const char * name_st = value.Attribute("name");
      check_error(name_st != NULL, "missing name for value in transaction XML");
      return(TypeFactory::createValue(type_st, name_st));
    }
    const char * value_st = value.Attribute("value");
    check_error(value_st != NULL, "missing value in transaction xml");
    if (strcmp(tag, "symbol") == 0) {
      const char * type_st = value.Attribute("type");
      check_error(type_st != NULL, "missing type for symbol '" + std::string(value_st) + "' in transaction xml");
      return(TypeFactory::createValue(type_st, value_st));
    }
    if (strcmp(tag, "object") == 0) {
      ObjectId object = m_client->getObject(value_st);
      check_error(object.isValid());
      return((double)object);
    }
    ObjectId object = m_client->getObject(value_st);
    if (object != ObjectId::noId())
      return((double)object);
    check_error(ALWAYS_FAILS);
    return(0);
  }

  ConstrainedVariableId DbClientTransactionPlayer::xmlAsVariable(const TiXmlElement & variable)
  {
    check_error(strcmp(variable.Value(), "ident") != 0,
                "ident in transaction xml is deprecated");
    if (strcmp(variable.Value(), "id") == 0) {
      const char * name = variable.Attribute("name");
      return parseVariable(name);
    }
    if (strcmp(variable.Value(), "variable") == 0) {
      int index;
      const char * i = variable.Attribute("index", &index);
      check_error(i != NULL);
      check_error(0 <= index);
  
      const char * token_path = variable.Attribute("token");
      if (token_path != NULL) {
        TokenId token = m_client->getTokenByPath(pathAsVector(token_path));
        check_error(token.isValid());
        check_error((unsigned)index < token->getVariables().size());
        return token->getVariables()[index];
      }
  
      const char * object_name = variable.Attribute("object");
      if (object_name != NULL) {
        ObjectId object = m_client->getObject(object_name);
        check_error(object.isValid());
        check_error((unsigned)index < object->getVariables().size());
        return object->getVariables()[index];
      }
  
      // rule variables
      return m_client->getVariableByIndex(index);
    }

    ConstrainedVariableId var = xmlAsCreateVariable(NULL, NULL, &variable);
    check_error(var.isValid());

    return var;
  }

  TokenId DbClientTransactionPlayer::xmlAsToken(const TiXmlElement & token)
  {
    check_error(strcmp(token.Value(), "ident") != 0,
                "ident in transaction xml is deprecated");
    if (strcmp(token.Value(), "id") == 0) {
      const char * name = token.Attribute("name");
      TokenId tok = parseToken(name);
      check_error(tok.isValid());
      return tok;
    }
    if (strcmp(token.Value(), "token") == 0) {
      const char * path = token.Attribute("path");
      if (path != NULL) {
        return m_client->getTokenByPath(pathAsVector(path));
      }
  
      const char * name = token.Attribute("name");
      if (name != NULL) {
        TokenId tok = parseToken(name);
        check_error(tok.isValid());
        return tok;
      }
    }  
    check_error(ALWAYS_FAILS);
    return TokenId::noId();
  }


  ConstrainedVariableId
  DbClientTransactionPlayer::xmlAsCreateVariable(const char * type, const char * name, const TiXmlElement * value)
  {
    std::string gen_name;
    if (name == NULL) {
      std::stringstream gen_stream;
      gen_stream << "$VAR[" << m_varCount++ << "]" << std::ends;
      gen_name = gen_stream.str();
      name = gen_name.c_str();
    }
 
    const AbstractDomain * baseDomain = NULL;
    if (value != NULL) {
      baseDomain = xmlAsAbstractDomain(*value, name);
      debugMsg("DbClientTransactionPlayer:xmlAsCreateVariable", " type = " << type << " name = " << name << " domain type = " << baseDomain->getTypeName().c_str());
      if (type == NULL) {
        type = value->Value();
        if ((strcmp(type, "value") == 0) || (strcmp(type, "new") == 0) || (strcmp(type, "symbol") == 0) || (strcmp(type, "interval") == 0)) {
          type = value->Attribute("type");
        }
      }
    }

    check_error(type != NULL);
    if (baseDomain != NULL) {
      ConstrainedVariableId variable = m_client->createVariable(type, *baseDomain, name);
      delete baseDomain;
      return variable;
    }

    return m_client->createVariable(type, name);
  }
}
