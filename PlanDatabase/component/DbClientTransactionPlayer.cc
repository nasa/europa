#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionTokenMapper.hh"
#include "DbClient.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
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

  DbClientTransactionPlayer::DbClientTransactionPlayer(const PlanDatabaseId & db, const DbClientTransactionTokenMapperId & tokenMapper)
  {
    m_db = db;
    m_client = db->getClient();
    m_tokenMapper = tokenMapper;
    m_objectCount = 0;
  }

  DbClientTransactionPlayer::~DbClientTransactionPlayer()
  {
  }

  void DbClientTransactionPlayer::play(std::istream& is)
  {
    TiXmlElement element("");
    while (!is.eof()) {
      if (is.peek() != '<') {
        is.get(); // discard characters up to '<'
        continue;
      }
      m_db->getConstraintEngine()->propagate();
      is >> element;
      const char * tagname = element.Value();
      if (strcmp(tagname, "new") == 0) {
        playNamedObjectCreated(element);
      } else if (strcmp(tagname, "close") == 0) {
        playClosed(element);
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
      } else {
        check_error(ALWAYS_FAILS);
      }
      element.Clear();
    }
  }

  void DbClientTransactionPlayer::playNamedObjectCreated(const TiXmlElement & element)
  {
    const char * name = element.Attribute("name");
    check_error(name != NULL);
    const char * type = element.Attribute("type");
    check_error(type != NULL);
    m_client->createObject(LabelStr(type), LabelStr(name));
  }

  void DbClientTransactionPlayer::playObjectCreated(const TiXmlElement & element)
  {
    check_error(ALWAYS_FAILS); // Not ready test test this yet!
    std::stringstream name;
    name << "$OBJECT[" << m_objectCount++ << "]" << std::ends;
    const char * type = element.Attribute("name");
    check_error(type != NULL);
    m_client->createObject(LabelStr(type), LabelStr(name.str()));
  }

  void DbClientTransactionPlayer::playClosed(const TiXmlElement & element)
  {
    m_client->close();
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
    ObjectId object = m_db->getObject(LabelStr(name));
    check_error(object.isValid());

    TiXmlElement * token_el = object_el->NextSiblingElement();
    check_error(token_el != NULL);
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());

    TiXmlElement * successor_el = token_el->NextSiblingElement();
    TokenId successor = TokenId::noId();
    if (successor_el != NULL) {
      check_error(strcmp(successor_el->Value(), "token") == 0);
      const char * successor_path = successor_el->Attribute("path");
      check_error(successor_path != NULL);
      successor = m_tokenMapper->getTokenByPath(pathAsVector(successor_path));
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
    ObjectId object = m_db->getObject(LabelStr(name));
    check_error(object.isValid());

    TiXmlElement * token_el = object_el->NextSiblingElement();
    check_error(token_el != NULL);
    check_error(strcmp(token_el->Value(), "token") == 0);
    const char * path = token_el->Attribute("path");
    check_error(path != NULL);
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
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
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
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
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());

    TiXmlElement * active_el = token_el->NextSiblingElement();
    check_error(active_el != NULL);
    check_error(strcmp(active_el->Value(), "token") == 0);
    const char * active_path = active_el->Attribute("path");
    check_error(active_path != NULL);
    TokenId active_token = m_tokenMapper->getTokenByPath(pathAsVector(active_path));
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
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
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
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());
    m_client->cancel(token);    
  }

  void DbClientTransactionPlayer::playVariableSpecified(const TiXmlElement & element)
  {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    check_error(strcmp(var_el->Value(), "variable") == 0);

    const char * path = var_el->Attribute("token");
    check_error(path != NULL);
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());

    int index;
    const char * i = var_el->Attribute("index", &index);
    check_error(i != NULL);
    check_error(0 <= index);
    check_error((unsigned)index < token->getVariables().size());
    ConstrainedVariableId variable = token->getVariables()[index];

    TiXmlElement * value_el = var_el->NextSiblingElement();
    check_error(value_el != NULL);
    double value;
    if (strcmp(value_el->Value(), "bool") == 0) {
      const char * value_st = value_el->Attribute("value");
      check_error(value_st != NULL);
      if (strcmp(value_st, "true") == 0) {
        value = 1;
      } else if (strcmp(value_st, "false") == 0) {
        value = 0;
      } else {
        check_error(ALWAYS_FAILS);
      }
    } else if (strcmp(value_el->Value(), "int") == 0) {
      int i;
      const char * value_st = value_el->Attribute("value", &i);
      check_error(value_st != NULL);
      value = i;
    } else if (strcmp(value_el->Value(), "float") == 0) {
      const char * value_st = value_el->Attribute("value", &value);
      check_error(value_st != NULL);
    } else if (strcmp(value_el->Value(), "string") == 0) {
      const char * value_st = value_el->Attribute("value");
      check_error(value_st != NULL);
      value = LabelStr(value_st);
    } else if (strcmp(value_el->Value(), "object") == 0) {
      const char * value_st = value_el->Attribute("value");
      check_error(value_st != NULL);
      ObjectId object = m_db->getObject(LabelStr(value_st));
      check_error(object.isValid());
      value = (double)object;
    } else {
      check_error(ALWAYS_FAILS);
    }
    m_client->specify(variable, value);    
  }

  void DbClientTransactionPlayer::playVariableReset(const TiXmlElement & element)
  {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    check_error(strcmp(var_el->Value(), "variable") == 0);

    const char * path = var_el->Attribute("token");
    check_error(path != NULL);
    TokenId token = m_tokenMapper->getTokenByPath(pathAsVector(path));
    check_error(token.isValid());

    int index;
    const char * i = var_el->Attribute("index", &index);
    check_error(i != NULL);
    check_error(0 <= index);
    check_error((unsigned)index < token->getVariables().size());
    ConstrainedVariableId variable = token->getVariables()[index];
    m_client->reset(variable);    
  }

}
