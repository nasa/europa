#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include "DbClient.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "Token.hh"
#include "EnumeratedDomain.hh"
#include "IntervalDomain.hh"
#include "../TinyXml/tinyxml.h"
#include "../ConstraintEngine/ConstraintEngine.hh"

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

  DbClientTransactionPlayer::DbClientTransactionPlayer(const DbClientId & client) : m_client(client), m_objectCount(0){}

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
      if (strcmp(tagname, "new") == 0) {
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
      } else {
        check_error(ALWAYS_FAILS);
      }

      m_client->propagate();
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
      return "object";
    }
  }

  void DbClientTransactionPlayer::playObjectCreated(const TiXmlElement & element)
  {
    const char * name = element.Attribute("name");
    std::stringstream gen_name;
    if (name == NULL) {
      gen_name << "$OBJECT[" << m_objectCount++ << "]" << std::ends;
    }
    const char * type = element.Attribute("type");
    check_error(type != NULL);

    std::vector<ConstructorArgument> arguments;
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      AbstractDomain * domain;
      std::string type;
      if (strcmp(child_el->Value(), "object") != 0) {
        domain = xmlAsAbstractDomain(*child_el);
        type = domainTypeString(domain);
      } else {
        const char * name = child_el->Attribute("value");
        check_error(name != NULL);
        ObjectId object = m_client->getObject(LabelStr(name));
        check_error(object.isValid());
        type = object->getType().toString();
        domain = new ObjectDomain(object);
      }
      arguments.push_back(ConstructorArgument(LabelStr(type.c_str()), domain));
    }

    m_client->createObject(LabelStr(type), LabelStr(name != NULL ? name : gen_name.str()), arguments);
  }

  void DbClientTransactionPlayer::playTokenCreated(const TiXmlElement & element)
  {
    TiXmlElement * child = element.FirstChildElement();
    check_error(child != NULL);
    const char * type = child->Attribute("type");
    check_error(type != NULL);
    m_client->createToken(LabelStr(type));
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
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_client->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());

    TiXmlElement * successor_el = token_el->NextSiblingElement();
    TokenId successor = TokenId::noId();
    if (successor_el != NULL) {
      check_error(strcmp(successor_el->Value(), "token") == 0);
      const char * successor_path = successor_el->Attribute("path");
      check_error(successor_path != NULL);
      successor = m_client->getTokenByPath(pathAsVector(successor_path));
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
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_client->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());

    m_client->free(object, token);
  }

  void DbClientTransactionPlayer::playActivated(const TiXmlElement & element)
  {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_client->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());
    m_client->activate(token);    
  }

  void DbClientTransactionPlayer::playMerged(const TiXmlElement & element)
  {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_client->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());

    TiXmlElement * active_el = token_el->NextSiblingElement();
    check_error(active_el != NULL);
    check_error(strcmp(active_el->Value(), "token") == 0);
    const char * active_path = active_el->Attribute("path");
    check_error(active_path != NULL);
    TokenId active_token = m_client->getTokenByPath(pathAsVector(active_path));
    check_error(active_token.isValid());

    m_client->merge(token, active_token);
  }

  void DbClientTransactionPlayer::playRejected(const TiXmlElement & element)
  {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_client->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());
    m_client->reject(token);    
  }

  void DbClientTransactionPlayer::playCancelled(const TiXmlElement & element)
  {
    TiXmlElement * token_el = element.FirstChildElement();
    check_error(token_el != NULL);
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_client->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());
    m_client->cancel(token);    
  }

  void DbClientTransactionPlayer::playVariableSpecified(const TiXmlElement & element)
  {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    ConstrainedVariableId variable = xmlAsVariable(*var_el);

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
    std::vector<ConstrainedVariableId> variables;
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
      if (strcmp(child_el->Value(), "variable") == 0) {
        variables.push_back(xmlAsVariable(*child_el));
      } else {
        // unary constraint
        check_error(variables.size() == 1);
        check_error(child_el->NextSiblingElement() == NULL);
        AbstractDomain * domain = xmlAsAbstractDomain(*child_el);
        m_client->createConstraint(LabelStr(name), variables[0], *domain);
        delete domain;
        return;
      }
    }
    if (strcmp(name, "close") == 0) {
      // close database special case
      check_error(variables.size() == 0);
      m_client->close();
      return;
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

  //! XML input functions

  AbstractDomain * 
  DbClientTransactionPlayer::xmlAsAbstractDomain(const TiXmlElement & element)
  {
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
      return new EnumeratedDomain(m_client->getObject(LabelStr(value_st)));
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
    if (strcmp(type_st, "float") == 0) {
      double min = parseFloat(min_st);
      double max = parseFloat(max_st);
      return new IntervalDomain(min, max);
    }
    if (strcmp(type_st, "int") == 0) {
      int min = parseInt(min_st);
      int max = parseInt(max_st);
      return new IntervalIntDomain(min, max);
    }
    check_error(ALWAYS_FAILS);
    return NULL;
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

  double DbClientTransactionPlayer::xmlAsValue(const TiXmlElement & value)
  {
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
    check_error(ALWAYS_FAILS);
    return 0;
  }

  ConstrainedVariableId DbClientTransactionPlayer::xmlAsVariable(const TiXmlElement & variable)
  {
    check_error(strcmp(variable.Value(), "variable") == 0);

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

}
