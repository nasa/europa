#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include "DbClient.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "Token.hh"
#include "BoolDomain.hh"
#include "IntervalIntDomain.hh"
#include "IntervalDomain.hh"
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
    if (domain->getType() == AbstractDomain::BOOL) {
      return "bool";
    } else if (domain->isNumeric()) {
      if (domain->getType() == AbstractDomain::INT_INTERVAL) {
        return "int";
      } else {
        return "float";
      }
    } else if (LabelStr::isString(domain->getUpperBound())) {
      return "string";
    } else {
      ObjectId object = domain->getLowerBound();
      check_error(object.isValid());
      return object->getType().toString();
    }
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
      TiXmlElement * child_el = element.FirstChildElement();
      if (child_el == NULL) {
        // close database special case
        m_client->close();
        return;
      }
      else {
	const char * identifier = child_el->Attribute("value");
	check_error(identifier != NULL);
	m_client->close(LabelStr(identifier));
      }

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

  double 
  DbClientTransactionPlayer::parseFloat(const char * floatString)
  {
    check_error(floatString != NULL);
    if (strcmp(floatString, "-inf") == 0) {
      return MINUS_INFINITY;
    }
    if (strcmp(floatString, "+inf") == 0) {
      return PLUS_INFINITY;
    }
    return atof(floatString);
  }
  
  int
  DbClientTransactionPlayer::parseInt(const char * intString)
  {
    check_error(intString != NULL);
    if (strcmp(intString, "-inf") == 0) {
      return MINUS_INFINITY;
    }
    if (strcmp(intString, "+inf") == 0) {
      return PLUS_INFINITY;
    }
    return atoi(intString);
  }
  
  bool
  DbClientTransactionPlayer::parseBool(const char * boolString)
  {
    check_error(boolString != NULL);
    if (strcmp(boolString, "true") == 0) {
      return true;
    }
    if (strcmp(boolString, "false") == 0) {
      return false;
    }
    check_error(ALWAYS_FAILS);
    return false;
  }

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
    if (strcmp(element.Value(), "new") == 0) {
      return new ObjectDomain(xmlAsValue(element, name));
    }
    if (strcmp(element.Value(), "ident") == 0) {
      const char * value = element.Attribute("value");
      check_error(value != NULL);
      std::string std_value = value;
      ConstrainedVariableId var = m_variables[std_value];
      if (var != ConstrainedVariableId::noId()) {
        return &var->baseDomain();
      }
      // must be an enumerated domain
      return new LabelSet(LabelStr(value));
    }
    if (strcmp(element.Value(), "id") == 0) {
      const char * name = element.Attribute("name");
      ConstrainedVariableId var = parseVariable(name);
      check_error(var.isValid());
      return &var->baseDomain();
    }
    if (strcmp(element.Value(), "set") == 0) {
      return xmlAsEnumeratedDomain(element);
    }
    if (strcmp(element.Value(), "interval") == 0) {
      return xmlAsIntervalDomain(element);
    }
    const char * value_st = element.Attribute("value");
    check_error(value_st != NULL);
    if (strcmp(element.Value(), "bool") == 0) {
      return new BoolDomain(parseBool(value_st));
    }
    if (strcmp(element.Value(), "int") == 0) {
      return new IntervalIntDomain(parseInt(value_st));
    }
    if (strcmp(element.Value(), "float") == 0) {
      return new IntervalDomain(parseFloat(value_st));
    }
    if (strcmp(element.Value(), "string") == 0) {
      return new LabelSet(LabelStr(value_st));
    }
    if (strcmp(element.Value(), "object") == 0) {
      return new ObjectDomain(m_client->getObject(LabelStr(value_st)));
    }
    std::string klass = element.Value();
    if (std::find(m_classes.begin(), m_classes.end(), klass) != m_classes.end()) {
      return new ObjectDomain(m_client->getObject(LabelStr(value_st)));
    }
    ObjectId object = m_client->getObject(LabelStr(value_st));
    if (object != ObjectId::noId()) {
      return new ObjectDomain(object);
    }
    check_error(ALWAYS_FAILS);
    return NULL;
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
    IntervalDomain * domain = dynamic_cast<IntervalDomain*>(TypeFactory::createDomain(LabelStr(type_st)));
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
    // determine most specific type
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      if (strcmp(child_el->Value(), "bool") == 0) {
        if (type == ANY) {
          type = BOOL;
        }
        if (type == BOOL) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "int") == 0) {
        if (type == ANY) {
          type = INT;
        }
        if ((type == FLOAT) || (type == INT)) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "float") == 0) {
        if ((type == ANY) || (type == INT)) {
          type = FLOAT;
        }
        if (type == FLOAT) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "string") == 0) {
        if (type == ANY) {
          type = STRING;
        }
        if (type == STRING) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "symbol") == 0) {
        if (type == ANY) {
          type = SYMBOL;
        }
        if (type == SYMBOL) {
          continue;
        }
      }
      if (strcmp(child_el->Value(), "object") == 0) {
        if (type == ANY) {
          type = OBJECT;
        }
        if (type == OBJECT) {
          continue;
        }
      }
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
      case BOOL: {
        values.push_back(parseBool(value_st));
        break;
      }
      case INT: {
        values.push_back(parseInt(value_st));
        break;
      }
      case FLOAT: {
        values.push_back(parseFloat(value_st));
        break;
      }
      case STRING: 
      case SYMBOL: {
        values.push_back(LabelStr(value_st));
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
      return new EnumeratedDomain(values);
    case STRING: case SYMBOL: case OBJECT:
      return new EnumeratedDomain(values, true, DomainListenerId::noId(), false);
    default:
      check_error(ALWAYS_FAILS);
      return NULL;
    }
  }

  double DbClientTransactionPlayer::xmlAsValue(const TiXmlElement & value, const char * name)
  {
    if (strcmp(value.Value(), "new") == 0) {
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
      return (double)object;
    }
    const char * value_st = value.Attribute("value");
    check_error(value_st != NULL);
    if (strcmp(value.Value(), "bool") == 0) {
      return parseBool(value_st);
    }
    if (strcmp(value.Value(), "int") == 0) {
      return parseInt(value_st);
    }
    if (strcmp(value.Value(), "float") == 0) {
      return parseFloat(value_st);
    }
    if (strcmp(value.Value(), "string") == 0) {
      return LabelStr(value_st);
    }
    if (strcmp(value.Value(), "object") == 0) {
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
    if (strcmp(type, "bool") == 0) {
      const BoolDomain * domain = (baseDomain ? dynamic_cast<const BoolDomain*>(baseDomain) : new BoolDomain());
      return m_client->createVariable(*domain, LabelStr(name));
    }
    if (strcmp(type, "int") == 0) {
      const IntervalIntDomain * domain = (baseDomain ? dynamic_cast<const IntervalIntDomain*>(baseDomain) : new IntervalIntDomain());
      return m_client->createVariable(*domain, LabelStr(name));
    }
    if (strcmp(type, "float") == 0) {
      const IntervalDomain * domain = (baseDomain ? dynamic_cast<const IntervalDomain*>(baseDomain) : new IntervalDomain());
      return m_client->createVariable(*domain, LabelStr(name));
    }
    if (strcmp(type, "string") == 0) {
      const LabelSet * domain = (baseDomain ? dynamic_cast<const LabelSet*>(baseDomain) : new LabelSet());
      return m_client->createVariable(*domain, LabelStr(name));
    }
    std::string std_type = type;
    if (std::find(m_enumerations.begin(), m_enumerations.end(), std_type) != m_enumerations.end()) {
      // enumerations are label sets too
      const LabelSet * domain = (baseDomain ? dynamic_cast<const LabelSet*>(baseDomain) : new LabelSet());
      return m_client->createVariable(*domain, LabelStr(name));
    }
    if (std::find(m_classes.begin(), m_classes.end(), std_type) != m_classes.end()) {
      const ObjectDomain * domain = (baseDomain ? dynamic_cast<const ObjectDomain*>(baseDomain) : new ObjectDomain());
      return m_client->createVariable(*domain, LabelStr(name));
    }
    if (baseDomain) {
      if (LabelStr::isString(baseDomain->getUpperBound())) {
        const LabelSet * domain = dynamic_cast<const LabelSet*>(baseDomain);
        return m_client->createVariable(*domain, LabelStr(name));
      }
      ObjectId object = baseDomain->getLowerBound();
      check_error(object.isValid());
      const ObjectDomain * domain = dynamic_cast<const ObjectDomain*>(baseDomain);
      return m_client->createVariable(*domain, LabelStr(name));
    }
    // unknown type
    check_error(ALWAYS_FAILS);
    return ConstrainedVariableId::noId();
  }

}
