#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include "DbClient.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "Token.hh"
#include "EnumeratedDomain.hh"
#include "Domain.hh"
#include "tinyxml.h"
#include "ConstraintEngine.hh"
#include "TypeFactory.hh"
#include <algorithm>

namespace Prototype {

  static const std::vector<int> 
  pathAsVector(const std::string & path)
  {
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

  static std::string
  domainTypeString(const AbstractDomain * domain)
  {
    return domain->getTypeName().toString();
  }

  DbClientTransactionPlayer::DbClientTransactionPlayer(const DbClientId & client) : m_client(client), m_objectCount(0), m_varCount(0){}

  DbClientTransactionPlayer::~DbClientTransactionPlayer(){}

  void DbClientTransactionPlayer::play(std::istream& is)
  {
    int txCounter = 0;
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
  }

  void DbClientTransactionPlayer::play(const DbClientTransactionLogId& txLog)
  {
    const std::list<TiXmlElement*>& transactions = txLog->getBufferedTransactions();
    for (std::list<TiXmlElement*>::const_iterator it = transactions.begin();
         it != transactions.end(); ++it) {
      const TiXmlElement& tx = **it;
      processTransaction(tx);
    }
  }

  void DbClientTransactionPlayer::processTransaction(const TiXmlElement & element) {
      const char * tagname = element.Value();
      if (strcmp(tagname, "class") == 0) {
        playDefineClass(element);
      } else if (strcmp(tagname, "enum") == 0) {
        playDefineEnumeration(element);
      } else if (strcmp(tagname, "compat") == 0) {
//        playDefineCompat(element);
      } else if (strcmp(tagname, "var") == 0) {
        playVariableCreated(element);
      } else if (strcmp(tagname, "new") == 0) {
        playObjectCreated(element);
      } else if (strcmp(tagname, "goal") == 0) {
        playTokenCreated(element);
      } else if (strcmp(tagname, "constrain") == 0) {
        playConstrained(element);
      } else if (strcmp(tagname, "free") == 0) {
        playFreed(element);
      } else if (strcmp(tagname, "activate") == 0) {
        playActivated(element);
      } else if (strcmp(tagname, "merge") == 0) {
        playMerged(element);
      } else if (strcmp(tagname, "reject") == 0) {
        playRejected(element);
      } else if (strcmp(tagname, "cancel") == 0) {
        playCancelled(element);
      } else if (strcmp(tagname, "specify") == 0) {
        playVariableSpecified(element);
      } else if (strcmp(tagname, "reset") == 0) {
        playVariableReset(element);
      } else if (strcmp(tagname, "invoke") == 0) {
        playInvokeConstraint(element);
      } else if (strcmp(tagname, "nddl") == 0) {
        for (TiXmlElement * child_el = element.FirstChildElement() ;
             child_el != NULL ; child_el = child_el->NextSiblingElement()) {
          processTransaction(*child_el);
        }
      } else {
        check_error(ALWAYS_FAILS);
      }

      m_client->propagate();
  }

  void DbClientTransactionPlayer::playDefineClass(const TiXmlElement & element)
  {
    const char * name = element.Attribute("name");
    check_error(name != NULL);
    
    std::string std_name = name;
    m_classes.push_back(std_name);
  }

  void DbClientTransactionPlayer::playDefineEnumeration(const TiXmlElement & element)
  {
    const char * name = element.Attribute("name");
    check_error(name != NULL);

    std::string std_name = name;
    m_enumerations.push_back(std_name);
  }

  void DbClientTransactionPlayer::playVariableCreated(const TiXmlElement & element)
  {
    const char * type = element.Attribute("type");
    check_error(type != NULL);
    const char * name = element.Attribute("name");
    check_error(name != NULL);

    TiXmlElement * value = element.FirstChildElement();
    ConstrainedVariableId variable = xmlAsCreateVariable(type, name, value);
    check_error(variable.isValid());

    std::string std_name = name;
    m_variables[std_name] = variable;
  }

  void DbClientTransactionPlayer::playObjectCreated(const TiXmlElement & element)
  {
    const char * name = element.Attribute("name");
    xmlAsValue(element, name);
  }

  void DbClientTransactionPlayer::playTokenCreated(const TiXmlElement & element)
  {
    const char * relation = element.Attribute("relation");
    if (relation != NULL) {
      // inter-token temporal relation
      const char * origin = element.Attribute("origin");
      const char * target = element.Attribute("target");
      TokenId origin_token = parseToken(origin);
      TokenId target_token = parseToken(target);
      check_error(origin_token.isValid());
      check_error(target_token.isValid());
      if (strcmp(relation, "before") == 0) {
        std::vector<ConstrainedVariableId> variables;
        variables.push_back(origin_token->getEnd());
        variables.push_back(target_token->getStart());
        m_client->createConstraint(LabelStr("leq"), variables);
        return;
      }
      // TODO: the others
      check_error(ALWAYS_FAILS);
      return;
    }
    // simple token creation
    TiXmlElement * child = element.FirstChildElement();
    check_error(child != NULL);
    const char * type = child->Attribute("type");
    check_error(type != NULL);

    TokenId token = m_client->createToken(LabelStr(type));
    const char * mandatory = element.Attribute("mandatory");

    // If mandatory, remove the option to reject it from the base domain
    if ((mandatory != NULL) && (strcmp(mandatory, "true") == 0)) {
      StateDomain allowableStates;
      allowableStates.insert(Token::ACTIVE);
      allowableStates.insert(Token::MERGED);
      allowableStates.close();
      token->getState()->restrictBaseDomain(allowableStates);
    }

    const char * name = child->Attribute("name");

    if (name != NULL) {
      std::string std_name = name;
      m_tokens[std_name] = token;
    }
  }

  void DbClientTransactionPlayer::playConstrained(const TiXmlElement & element)
  {
    TiXmlElement * object_el = element.FirstChildElement();
    check_error(object_el != NULL);
    check_error(strcmp(object_el->Value(), "object") == 0);
    const char * name = object_el->Attribute("name");
    check_error(name != NULL);
    ObjectId object = m_client->getObject(LabelStr(name));
    check_error(object.isValid());

    TiXmlElement * token_el = object_el->NextSiblingElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());

    TiXmlElement * successor_el = token_el->NextSiblingElement();
    TokenId successor = TokenId::noId();
    if (successor_el != NULL) {
      successor = xmlAsToken(*successor_el);
      check_error(successor.isValid());
    }

    m_client->constrain(object, token, successor);
  }

  void DbClientTransactionPlayer::playFreed(const TiXmlElement & element)
  {
    TiXmlElement * object_el = element.FirstChildElement();
    check_error(object_el != NULL);
    check_error(strcmp(object_el->Value(), "object") == 0);
    const char * name = object_el->Attribute("name");
    check_error(name != NULL);
    ObjectId object = m_client->getObject(LabelStr(name));
    check_error(object.isValid());

    TiXmlElement * token_el = object_el->NextSiblingElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());

    m_client->free(object, token);
  }

  void DbClientTransactionPlayer::playActivated(const TiXmlElement & element)
  {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());
    m_client->activate(token);    
  }

  void DbClientTransactionPlayer::playMerged(const TiXmlElement & element)
  {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    TokenId token = xmlAsToken(*token_el);
    check_error(token.isValid());

    TiXmlElement * active_el = token_el->NextSiblingElement();
    TokenId active_token = xmlAsToken(*active_el);
    check_error(active_token.isValid());

    m_client->merge(token, active_token);
  }

  void DbClientTransactionPlayer::playRejected(const TiXmlElement & element)
  {
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

  void DbClientTransactionPlayer::playVariableSpecified(const TiXmlElement & element)
  {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    ConstrainedVariableId variable = xmlAsVariable(*var_el);
    check_error(variable.isValid());

    TiXmlElement * value_el = var_el->NextSiblingElement();
    check_error(value_el != NULL);
    double value = xmlAsValue(*value_el);
    m_client->specify(variable, value);    
  }

  void DbClientTransactionPlayer::playVariableReset(const TiXmlElement & element)
  {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    m_client->reset(xmlAsVariable(*var_el));
  }

  void DbClientTransactionPlayer::playInvokeConstraint(const TiXmlElement & element)
  {
    const char * name = element.Attribute("name");
    check_error(name != NULL);
    if (strcmp(name, "close") == 0) {
      const char * identifier = element.Attribute("identifier");
      if (identifier == NULL) {
        // close database special case
        m_client->close();
        return;
      }
      else
	m_client->close(LabelStr(identifier));

      return;
    }

    if (strcmp(name, "constrain") == 0) {
      // constrain token(s) special case
      const char * identifier = element.Attribute("identifier");
      std::string object_name = identifier;
      ObjectId object = m_client->getObject(LabelStr(object_name.c_str()));
      check_error(object.isValid(),
                  "constrain transaction refers to an undefined object: '"
                   + object_name + "'");
      TiXmlElement * token_el = element.FirstChildElement();
      check_error(token_el != NULL, "missing mandatory token identifier for constrain transaction");
      TokenId token = xmlAsToken(*token_el);
      check_error(token.isValid(),
                  "invalid token identifier for constrain transaction");
      
      TiXmlElement * successor_el = token_el->NextSiblingElement();
      TokenId successor = TokenId::noId();
      if (successor_el != NULL) {
        successor = xmlAsToken(*successor_el);
        check_error(successor.isValid(),
                    "invalid successor token identifier for constrain transaction");
      }

      m_client->constrain(object, token, successor);
      return;
    }

    if (strcmp(name, "free") == 0) {
      // free token special case
      const char * identifier = element.Attribute("identifier");
      std::string object_name = identifier;
      ObjectId object = m_client->getObject(LabelStr(object_name.c_str()));
      check_error(object.isValid(),
                  "free transaction refers to an undefined object: '"
                   + object_name + "'");
      TiXmlElement * token_el = element.FirstChildElement();
      check_error(token_el != NULL, "missing mandatory token identifier for free transaction");
      TokenId token = xmlAsToken(*token_el);
      check_error(token.isValid(),
                  "invalid token identifier for free transaction");

      m_client->free(object, token);
      return;
    }

    if (strcmp(name, "activate") == 0) {
      // activate token special case
      const char * identifier = element.Attribute("identifier");
      std::string token_name = identifier;
      TokenId token = m_tokens[token_name];
      check_error(token.isValid(), "Invalid token name '" + token_name + "' for activation.");
      m_client->activate(token);
      return;
    }
    if (strcmp(name, "merge") == 0) {
      // merge token special case
      const char * identifier = element.Attribute("identifier");
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
      const char * identifier = element.Attribute("identifier");
      std::string token_name = identifier;
      TokenId token = m_tokens[token_name];
      check_error(token.isValid());
      m_client->reject(token);
      return;
    }
    if (strcmp(name, "cancel") == 0) {
      // cancel token special case
      const char * identifier = element.Attribute("identifier");
      std::string token_name = identifier;
      TokenId token = m_tokens[token_name];
      check_error(token.isValid());
      m_client->cancel(token);
      return;
    }
    if (strcmp(name, "specify") == 0) {
      // specify variable special case
      const char * identifier = element.Attribute("identifier");
      ConstrainedVariableId variable = parseVariable(identifier);
      TiXmlElement * value_el = element.FirstChildElement();
      check_error(value_el != NULL);
      const AbstractDomain * value = xmlAsAbstractDomain(*value_el);
      if (value->isSingleton()) {
        double v = value->getSingletonValue();
        m_client->specify(variable, v);
      } else {
        m_client->specify(variable, *value);
      }
      return;
    }
    // normal constraints
    std::vector<ConstrainedVariableId> variables;
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      variables.push_back(xmlAsVariable(*child_el));
    }
    m_client->createConstraint(LabelStr(name), variables);
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
    check_error(var.isValid());
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
  DbClientTransactionPlayer::xmlAsAbstractDomain(const TiXmlElement & element, const char * name)
  {
    const char * tag = element.Value();
    if (strcmp(tag, "new") == 0) {
      const char * type = element.Attribute("type");
      check_error(type != NULL);
      return new ObjectDomain(xmlAsValue(element, name), LabelStr(type));
    }
    if (strcmp(tag, "id") == 0) {
      const char * name = element.Attribute("name");
      ConstrainedVariableId var = parseVariable(name);
      check_error(var.isValid());
      return var->baseDomain().copy();
    }
    if (strcmp(tag, "set") == 0) {
      return xmlAsEnumeratedDomain(element);
    }
    if (strcmp(tag, "interval") == 0) {
      return xmlAsIntervalDomain(element);
    }
    const char * value_st = element.Attribute("value");
    check_error(value_st != NULL);
    if ((strcmp(tag, "bool") == 0) || (strcmp(tag, "BOOL") == 0) ||
        (strcmp(tag, "int") == 0) || (strcmp(tag, "INT_INTERVAL") == 0) ||
        (strcmp(tag, "float") == 0) || (strcmp(tag, "REAL_INTERVAL") == 0) ||
        (strcmp(tag, "string") == 0) || (strcmp(tag, "STRING_ENUMERATION") == 0)) {
      AbstractDomain * domain = TypeFactory::baseDomain(LabelStr(tag)).copy();
      domain->set(TypeFactory::createValue(LabelStr(tag), value_st));
      return domain;
    }
    if (strcmp(tag, "symbol") == 0) {
      const char * type = element.Attribute("type");
      check_error(type != NULL);
      AbstractDomain * domain = TypeFactory::baseDomain(LabelStr(type)).copy();
      domain->set(TypeFactory::createValue(LabelStr(tag), value_st));
      return domain;
    }
    if (strcmp(tag, "ident") == 0) {
      std::cerr << "ident in transaction xml is deprecated" << std::endl;
      const char * value = element.Attribute("value");
      check_error(value != NULL);
      std::string std_value = value;
      ConstrainedVariableId var = m_variables[std_value];
      if (var != ConstrainedVariableId::noId()) {
        return var->baseDomain().copy();
      }
      // must be an enumerated domain
      return new LabelSet(LabelStr(value));
    }
    check_error(strcmp(tag, "object") == 0);
    ObjectId object = m_client->getObject(LabelStr(value_st));
    check_error(object.isValid());
    return new ObjectDomain(object, object->getType());
  }

  IntervalDomain *
  DbClientTransactionPlayer::xmlAsIntervalDomain(const TiXmlElement & element)
  {
    const char * type_st = element.Attribute("type");
    check_error(type_st != NULL);
    const char * min_st = element.Attribute("min");
    check_error(min_st != NULL);
    const char * max_st = element.Attribute("max");
    check_error(max_st != NULL);
    IntervalDomain * domain = dynamic_cast<IntervalDomain*>(TypeFactory::baseDomain(LabelStr(type_st)).copy());
    check_error(domain != NULL, "type '" + std::string(type_st) + "' should indicate an interval domain type");
    double min = TypeFactory::createValue(LabelStr(type_st), min_st);
    double max = TypeFactory::createValue(LabelStr(type_st), max_st);
    domain->intersect(min, max);
    return domain;
  }
  
  EnumeratedDomain *
  DbClientTransactionPlayer::xmlAsEnumeratedDomain(const TiXmlElement & element)
  {
    enum { ANY, BOOL, INT, FLOAT, STRING, SYMBOL, OBJECT } type = ANY;
    const char * typeName = NULL;
    // determine most specific type
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      if (strcmp(child_el->Value(), "bool") == 0) {
        if (type == ANY) {
          type = BOOL;
          typeName = "bool";
        }
        if (type == BOOL) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "int") == 0) {
        if (type == ANY) {
          type = INT;
          typeName = "int";
        }
        if ((type == FLOAT) || (type == INT)) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "float") == 0) {
        if ((type == ANY) || (type == INT)) {
          type = FLOAT;
          typeName = "float";
        }
        if (type == FLOAT) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "string") == 0) {
        if (type == ANY) {
          type = STRING;
          typeName = "string";
        }
        if (type == STRING) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "symbol") == 0) {
        if (type == ANY) {
          type = SYMBOL;
          typeName = child_el->Attribute("type");
        }
        if (type == SYMBOL) {
          continue;
        }
      }
//      if (strcmp(child_el->Value(), "object") == 0) {
//        if (type == ANY) {
//          type = OBJECT;
//        }
//        if (type == OBJECT) {
//          continue;
//        }
//      }
      check_error(ALWAYS_FAILS);
    }
    check_error(type != ANY);
    // gather the values
    std::list<double> values;
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      const char * value_st = child_el->Attribute("value");
      check_error(value_st != NULL);
      switch (type) {
      case BOOL: case INT: case FLOAT:
      case STRING: case SYMBOL: {
        values.push_back(TypeFactory::createValue(LabelStr(typeName), value_st));
        break;
      }
      case OBJECT: {
        values.push_back(m_client->getObject(LabelStr(value_st)));
        break;
      }
      default:
        check_error(ALWAYS_FAILS);
      }
    }
    // return the domain
    switch (type) { 
    case BOOL: case INT: case FLOAT:
      return new EnumeratedDomain(values, true, DomainListenerId::noId(), true, LabelStr(typeName));
    case STRING: case SYMBOL: case OBJECT:
      return new EnumeratedDomain(values, true, DomainListenerId::noId(), false, LabelStr(typeName));
    default:
      check_error(ALWAYS_FAILS);
      return NULL;
    }
  }

  double DbClientTransactionPlayer::xmlAsValue(const TiXmlElement & value, const char * name)
  {
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
        std::string type = domainTypeString(domain);
        arguments.push_back(ConstructorArgument(LabelStr(type.c_str()), domain));
      }
      ObjectId object = m_client->createObject(LabelStr(type), LabelStr(name), arguments);
      check_error(object.isValid());

      // Now deallocate domains created for arguments
      for(std::vector<ConstructorArgument>::const_iterator it = arguments.begin(); it != arguments.end(); ++it){
        delete it->second;
      }

      return (double)object;
    }
    const char * value_st = value.Attribute("value");
    check_error(value_st != NULL, "missing value in transaction xml");
    if ((strcmp(tag, "bool") == 0) || (strcmp(tag, "BOOL") == 0) ||
        (strcmp(tag, "int") == 0) || (strcmp(tag, "INT_INTERVAL") == 0) ||
        (strcmp(tag, "float") == 0) || (strcmp(tag, "REAL_INTERVAL") == 0) ||
        (strcmp(tag, "string") == 0) || (strcmp(tag, "STRING_ENUMERATION") == 0)) {
      return TypeFactory::createValue(LabelStr(tag), value_st);
    }
    if (strcmp(tag, "symbol") == 0) {
      const char * type_st = value.Attribute("type");
      check_error(type_st != NULL, "missing type for symbol '" + std::string(value_st) + "' in transaction xml");
      return TypeFactory::createValue(LabelStr(type_st), value_st);
    }
    if (strcmp(tag, "object") == 0) {
      ObjectId object = m_client->getObject(LabelStr(value_st));
      check_error(object.isValid());
      return (double)object;
    }
    ObjectId object = m_client->getObject(LabelStr(value_st));
    if (object != ObjectId::noId()) {
      return (double)object;
    }
    check_error(ALWAYS_FAILS);
    return 0;
  }

  ConstrainedVariableId DbClientTransactionPlayer::xmlAsVariable(const TiXmlElement & variable)
  {
    if (strcmp(variable.Value(), "ident") == 0) {
      const char * value = variable.Attribute("value");
      return parseVariable(value);
    }
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
        ObjectId object = m_client->getObject(LabelStr(object_name));
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
    if (strcmp(token.Value(), "ident") == 0) {
      const char * value = token.Attribute("value");
      TokenId tok = parseToken(value);
      check_error(tok.isValid());
      return tok;
    }
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
      if (type == NULL) {
        type = value->Value();
        if (strcmp(type, "new") == 0) {
          type = value->Attribute("type");
        }
      }
    }

    check_error(type != NULL);
    if (baseDomain != NULL) {
      ConstrainedVariableId variable = m_client->createVariable(LabelStr(type), *baseDomain, LabelStr(name));
      delete baseDomain;
      return variable;
    }

    return m_client->createVariable(LabelStr(type), LabelStr(name));
  }
}
