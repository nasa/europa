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
#include "ObjectFactory.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "DbClient.hh"
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include "Utils.hh"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace EUROPA {

  const std::set<std::string>& DbClientTransactionPlayer::MODEL_TRANSACTIONS() {
    static bool called = false;
    static std::set<std::string> sl_retval;
    if(!called) {
      sl_retval.insert("class_decl");
      sl_retval.insert("class");
      sl_retval.insert("enum");
      sl_retval.insert("typedef");
      sl_retval.insert("compat");
      called = true;
    }
    return sl_retval;
  }

  const std::set<std::string>& DbClientTransactionPlayer::STATE_TRANSACTIONS() {
    static bool called = false;
    static std::set<std::string> sl_retval;
    if(!called) {
      sl_retval.insert("var");
      sl_retval.insert("new");
      sl_retval.insert("goal");
      sl_retval.insert("fact");
      sl_retval.insert("constrain");
      sl_retval.insert("free");
      sl_retval.insert("activate");
      sl_retval.insert("merge");
      sl_retval.insert("reject");
      sl_retval.insert("cancel");
      sl_retval.insert("specify");
      sl_retval.insert("assign");
      sl_retval.insert("restrict");
      sl_retval.insert("reset");
      sl_retval.insert("invoke");
      called = true;
    }
    return sl_retval;    
  }

  const std::set<std::string>& DbClientTransactionPlayer::NO_TRANSACTIONS() {
    static bool called = false;
    static std::set<std::string> sl_retval;
    if(!called) {
      sl_retval.insert("breakpoint");
      called = true;
    }
    return sl_retval;
  }

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

  void DbClientTransactionPlayer::setFilter(const std::set<std::string>& filters) {
    
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

  void DbClientTransactionPlayer::rewind(std::istream& is, bool breakpoint) {
    check_error(is, "Invalid input sream for playing transactions.");
    std::list<TiXmlElement*> transactions;
    while(!is.eof()) {
      if (is.peek() != '<') {
	is.get(); // discard characters up to '<'
	continue;
      }
      
      TiXmlElement* elem = new TiXmlElement("");
      is >> (*elem);
      transactions.push_front(elem);
    }
    for(std::list<TiXmlElement*>::iterator it = transactions.begin(); it != transactions.end();
	++it) {
      const TiXmlElement& tx = **it;
      processTransactionInverse(tx, it, transactions.end());
      if(breakpoint && tx.Value() == std::string("breakpoint"))
	break;
    }
    cleanup(transactions);
  }

  void DbClientTransactionPlayer::rewind(const DbClientTransactionLogId& txLog,
					 bool breakpoint) {
    const std::list<TiXmlElement*>& transactions = txLog->getBufferedTransactions();
    while(!transactions.empty()) {
      const TiXmlElement& tx = *(transactions.back());
      processTransactionInverse(tx, transactions.rbegin(), transactions.rend());
      if(breakpoint && tx.Value() == std::string("breakpoint"))
	break;
      txLog->popTransaction();
    }
  }

  bool DbClientTransactionPlayer::transactionFiltered(const TiXmlElement& trans) const {
    for(std::set<std::string>::const_iterator it = m_filters.begin(); it != m_filters.end();
	++it)
      if(transactionMatch(trans, *it))
	return true;
    return false;
  }

  bool DbClientTransactionPlayer::transactionMatch(const TiXmlElement& trans,
						   const std::string& name) const {
    return name == trans.Value() ||
      (std::string("invoke") == trans.Value() && trans.Attribute("name") != NULL &&
       trans.Attribute("name") == name);
  }

  void DbClientTransactionPlayer::processTransaction(const TiXmlElement & element) {
    static unsigned int sl_txCount(0);
    const char * tagname = element.Value();

    sl_txCount++;
    debugMsg("DbClientTransactionPlayer:processTransaction",
	     "Processing transaction " << element);
    if(!transactionFiltered(element)) {
      if(transactionMatch(element, "breakpoint")) {}
      else if (transactionMatch(element, "class_decl"))
	playDeclareClass(element);
      else if (transactionMatch(element, "class"))
	playDefineClass(element);
      else if (transactionMatch(element, "enum"))
	playDefineEnumeration(element);
      else if (transactionMatch(element, "typedef"))
	playDefineType(element);
      else if (transactionMatch(element, "compat"))
	playDefineCompat(element);
      else if (transactionMatch(element, "var"))
	playVariableCreated(element);
      else if(transactionMatch(element, "deletevar"))
	playVariableDeleted(element);
      else if (transactionMatch(element, "new"))
	playObjectCreated(element);
      else if(transactionMatch(element, "deleteobject"))
	playObjectDeleted(element);
      else if (transactionMatch(element, "goal"))
	playTokenCreated(element);
      else if (transactionMatch(element, "fact"))
	playFactCreated(element);
      else if(transactionMatch(element, "deletetoken"))
	playTokenDeleted(element);
      else if (transactionMatch(element, "constrain"))
	playConstrained(element);
      else if (transactionMatch(element, "free"))
	playFreed(element);
      else if (transactionMatch(element, "activate"))
	playActivated(element);
      else if (transactionMatch(element, "merge"))
	playMerged(element);
      else if (transactionMatch(element, "reject"))
	playRejected(element);
      else if (transactionMatch(element, "cancel"))
	playCancelled(element);
      else if (transactionMatch(element, "specify"))
	playVariableSpecified(element);
      else if (transactionMatch(element, "assign"))
	playVariableAssigned(element);
      else if (transactionMatch(element, "restrict"))
	playVariableRestricted(element);
      else if (transactionMatch(element, "reset"))
	playVariableReset(element);
      else if (transactionMatch(element, "invoke"))
	playInvokeConstraint(element);
      else if(transactionMatch(element, "deleteconstraint"))
	playUninvokeConstraint(element);
      else {
	checkError(strcmp(tagname, "nddl") == 0, "Unknown tag name " << tagname);
	for (TiXmlElement * child_el = element.FirstChildElement() ;
	     child_el != NULL ; child_el = child_el->NextSiblingElement()) {
	  processTransaction(*child_el);
	  if (!m_client->propagate())
	    return;
	}
      }
    }
    m_client->propagate();
  }

  template<typename Iterator>
  void DbClientTransactionPlayer::processTransactionInverse(const TiXmlElement& element,
							    Iterator start, Iterator end) {
    const char* tagname = element.Value();
    debugMsg("DbClientTransactionPlayer:processTransactionInverse",
	     "Processing inverse of transaction '" << tagname << "'");
    if(!transactionFiltered(element)) {
      if(transactionMatch(element, "breakpoint")) {} // does nothing
      else if (transactionMatch(element, "class_decl")) {}
	//playDeclareClass(element);
      else if (transactionMatch(element, "class")) {}
	//playDefineClass(element);
      else if (transactionMatch(element, "enum")) {}
      //playDefineEnumeration(element);
      else if (transactionMatch(element, "typedef")) {}
      //playDefineType(element);
      else if (transactionMatch(element, "compat")) {}
      //playDefineCompat(element);
      else if (transactionMatch(element, "var")) 
	playVariableDeleted(element);
      else if(transactionMatch(element, "deletevar"))
	playVariableUndeleted(element, start, end);
      else if (transactionMatch(element, "new")) 
	playObjectDeleted(element);
      else if(transactionMatch(element, "deleteobject")) //has to scan backwards for "new" or "var"
	playObjectUndeleted(element, start, end);
      else if (transactionMatch(element, "goal"))
	playTokenDeleted(element);
      else if (transactionMatch(element, "fact"))
	playTokenDeleted(element);
      else if(transactionMatch(element, "deletetoken"))
	playTokenUndeleted(element, start, end);
      else if (transactionMatch(element, "constrain"))
	playFreed(element);
      else if (transactionMatch(element, "free"))
	playUnfreed(element, start, end);
      else if (transactionMatch(element, "activate"))
	playCancelled(element);
      else if (transactionMatch(element, "merge"))
	playCancelled(element);
      else if (transactionMatch(element, "reject"))
	playCancelled(element);
      else if (transactionMatch(element, "cancel"))
	playUncancelled(element, start, end);
      else if (transactionMatch(element, "specify"))
	playVariableReset(element);

      //these two can have no inverse--we lose base domain information and the CE only allows
      //restrictions of base domains, so we can't use type factories
      else if (transactionMatch(element, "assign")) {}
      else if (transactionMatch(element, "restrict")) {}

      else if (transactionMatch(element, "reset"))
	playVariableUnreset(element, start, end);
      else if (transactionMatch(element, "invoke"))
	playUninvokeConstraint(element, start, end);
      else if(transactionMatch(element, "deleteconstraint"))
	playReinvokeConstraint(element, start, end);
      else {
	check_error(strcmp(tagname, "nddl") == 0, "Unknown tag name " + std::string(tagname));
	for (TiXmlElement * child_el = element.FirstChildElement() ;
	     child_el != NULL ; child_el = child_el->NextSiblingElement()) {
	  processTransactionInverse(*child_el, start, end);
	  if (!m_client->propagate())
	    return;
	}
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


  void DbClientTransactionPlayer::playVariableDeleted(const TiXmlElement& element) {
    debugMsg("DbClientTransactionPlayer:playVariableDeleted",
	     "Playing (possibly the inverse of) " << element);
    const char* name = element.Attribute("name");
    check_error(name != NULL);

    ConstrainedVariableId var = ConstrainedVariableId::noId();

    const char* index = element.Attribute("index");
    if(index != NULL) {
      std::stringstream str;
      str << index;
      unsigned int key;
      str >> key;
      var = m_client->getVariableByIndex(key);
    }
    else {
      check_error(m_client->isGlobalVariable(name));
      var = m_client->getGlobalVariable(name);
    }
    check_error(var.isValid());

    TiXmlElement* child = element.FirstChildElement("new");
    if(child != NULL) {
      playObjectDeleted(*child);
    }

    m_client->deleteVariable(var);
    
    std::string std_name(name);
    m_variables.erase(name);
  }

  template <typename Iterator>
  void DbClientTransactionPlayer::playVariableUndeleted(const TiXmlElement& element,
							Iterator start, Iterator end) {
    debugMsg("DbClientTransactionPlayer:playVariableUndeleted",
	     "Playing inverse of " << element);
    const char* name = element.Attribute("name");
    check_error(name != NULL);
    const char* index = element.Attribute("index");
    check_error(index != NULL);
    const char* type = element.Attribute("type");

    debugMsg("DbClientTransactionPlayer:playVariableUndeleted",
	     "Searching backwards for a creation transaction.");
    for(Iterator it = start; it != end; ++it) {
      debugMsg("DbClientTransactionPlayer:playVariableUndeleted",
	       "Checking " << **it);
      if(strcmp((*it)->Value(), "var") == 0 &&
	 ((*it)->Attribute("name") != NULL && strcmp((*it)->Attribute("name"), name) == 0) &&
	 ((*it)->Attribute("index") == NULL || strcmp((*it)->Attribute("index"), index) == 0) &&
	 (type == NULL || ((*it)->Attribute("type") != NULL &&
			   strcmp((*it)->Attribute("type"), type) == 0))) {
	playVariableCreated(**it);
	return;
      }
    }
    checkError(ALWAYS_FAIL, "No creation transaction to complement " << element);
  }

  void DbClientTransactionPlayer::playObjectCreated(const TiXmlElement & element) {
    const char * name = element.Attribute("name");
    xmlAsValue(element, name);
  }

  void DbClientTransactionPlayer::playObjectDeleted(const TiXmlElement& element) {
    const char* name = element.Attribute("name");
    check_error(name != NULL);
    ObjectId obj = m_client->getObject(name);
    check_error(obj.isValid());
    m_client->deleteObject(obj);
  }

  template <typename Iterator>
  void DbClientTransactionPlayer::playObjectUndeleted(const TiXmlElement& element,
						      Iterator start, Iterator end) {
    debugMsg("DbClientTransactionPlayer:playObjectUndeleted",
	     "Playing inverse of " << element);
    const char* name = element.Attribute("name");
    check_error(name != NULL);
    
    debugMsg("DbClientTransactionPlayer:playObjectUndeleted",
	     "Searching backwards for a creation transaction.");
    for(Iterator it = start; it != end; ++it) {
      TiXmlElement* elem = *it;
      debugMsg("DbClientTransactionPlayer:playObjectUndeleted",
	       "Checking " << *elem);
      if(strcmp(elem->Value(), "var") == 0 && elem->FirstChildElement("new") != NULL)
	elem = elem->FirstChildElement("new");
      if(strcmp(elem->Value(), "new") == 0 && elem->Attribute("name") != NULL &&
	 strcmp(elem->Attribute("name"), name) == 0) {
	playObjectCreated(*elem);
	return;
      }
    }
    checkError(ALWAYS_FAIL, "No creation transaction to complement " << element);
  }

  const char* getObjectAndType(DbClientId& client, const char* predicate,ObjectId& object)
  {
    if (!Schema::instance()->isPredicate(predicate)) {
      LabelStr typeStr(predicate);
      int cnt = typeStr.countElements(Schema::getDelimiter());
      LabelStr prefix = typeStr.getElement(0, Schema::getDelimiter());
      object = client->getObject(prefix.c_str());
      check_error(object.isValid(), "Failed to find an object named " + prefix.toString());
      LabelStr objType = object->getType();      
      LabelStr suffix = typeStr.getElement(1, Schema::getDelimiter());
      
      for (int i=2; i<cnt;i++) {
          objType = Schema::instance()->getMemberType(objType,suffix);
          suffix = typeStr.getElement(i, Schema::getDelimiter());           	
      }
      
      std::string objName(predicate);
      objName = objName.substr(0,objName.length()-suffix.toString().length()-1);
      object = client->getObject(objName.c_str());
      check_error(object.isValid(), "Failed to find an object named " + objName);
      LabelStr newType(objType.toString() + Schema::getDelimiter() + suffix.toString());
      return  newType.c_str();
    }
    else {
    	return predicate;
    }  	
  }
  
  TokenId DbClientTransactionPlayer::createToken(
                                         const char* name,
                                         const char* type, 
                                         bool rejectable, 
                                         bool isFact)
  {
    // The type may be qualified with an object name, in which case we should get the
    // object and specify it. We will also have to generate the appropriate type designation
    // by extracting the class from the object
    ObjectId object;
    const char* predicateType = getObjectAndType(m_client,type,object);

    TokenId token = m_client->createToken(predicateType,rejectable,isFact); 
    
    if (!object.isNoId()) {
        // We restrict the base domain permanently since the name is specifically mentioned on creation
        token->getObject()->restrictBaseDomain(object->getThis()->baseDomain());
    }

    if (name != NULL) {
      std::string std_name = name;
      m_tokens[std_name] = token;
    }
    else {
    	name = "NO_NAME";
    }
    
    debugMsg("DbClientTransactionPlayer:createToken", "created Token:" << name << " of type " << predicateType << " isFact:" << isFact);      
    
    return token;  	
  }
   
  void DbClientTransactionPlayer::playFactCreated(const TiXmlElement & element) {
    TiXmlElement * child = element.FirstChildElement();
    check_error(child != NULL);
    const char * type = child->Attribute("type");
    check_error(type != NULL);

    createToken(
        child->Attribute("name"),
        type,
        false, // rejectable  
        true   // isFact 
    );
  }

  void DbClientTransactionPlayer::playTokenCreated(const TiXmlElement & element) {
    const char * relation = element.Attribute("relation");
    if (relation != NULL) {
      playTemporalRelationCreated(element);
      return;
    }
    // simple token creation
    TiXmlElement * child = element.FirstChildElement();
    check_error(child != NULL);
    const char * type = child->Attribute("type");
    check_error(type != NULL);

    const char * mandatory = element.Attribute("mandatory");
    bool rejectable = true;
    if(mandatory != NULL && (strcmp(mandatory, "true") == 0))
      rejectable = false;

    TokenId token = createToken(
        child->Attribute("name"),
        type,
        rejectable, // rejectable 
        false       // isFact
    );
  }

  //bizarre... playTokenCreated will, separately, create a token and create a temporal relation
  //these should be separated!
  void DbClientTransactionPlayer::playTokenDeleted(const TiXmlElement& element) {
    debugMsg("DbClientTransactionPlayer:playTokenDeleted",
	     "Rewinding " << element);

    if(element.Attribute("relation") != NULL) {
      playTemporalRelationDeleted(element);
      return;
    }

    TokenId tok = TokenId::noId();

    TiXmlElement* child = element.FirstChildElement("predicateinstance");
    check_error(child != NULL);

    const char* name = child->Attribute("name");
    check_error(name != NULL);
    std::map<std::string, TokenId>::iterator tokIt = m_tokens.find(name);
    if(tokIt != m_tokens.end()) {
      tok = tokIt->second;
      m_tokens.erase(tokIt);
    }
    else {
      const char* pathStr = child->Attribute("path");
      check_error(pathStr != NULL);
      
      
      std::vector<std::string> tokens;
      tokenize(pathStr, tokens, ".");
      
      std::vector<int> path;
      for(std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
	std::stringstream str;
	str << *it;
	int element;
	str >> element;
	path.push_back(element);
      }
      tok = m_client->getTokenByPath(path);
    }

    check_error(tok.isValid());
    m_client->deleteToken(tok);
  }

  template <typename Iterator>
  void DbClientTransactionPlayer::playTokenUndeleted(const TiXmlElement& element,
						     Iterator start, Iterator end) {
    const char* type = element.Attribute("type");
    check_error(type != NULL);
    const char* path = element.Attribute("path");
    check_error(path != NULL);
    const char* name = element.Attribute("name");
    check_error(name != NULL);

    for(Iterator it = start; it != end; ++it) {
      if((strcmp((*it)->Value(), "fact") == 0 || strcmp((*it)->Value(), "goal")) &&
	 (*it)->Attribute("relation") == NULL) {
	TiXmlElement* child = (*it)->FirstChildElement();
	check_error(child != NULL);
	if((child->Attribute("type") != NULL && strcmp(child->Attribute("type"), type) == 0) &&
	   (child->Attribute("path") == NULL || strcmp(child->Attribute("path"), path) == 0) &&
	   (child->Attribute("name") != NULL && strcmp(child->Attribute("name"), name) == 0)) {
	  playTokenCreated(**it);
	  return;
	}
      }
    }
    checkError(ALWAYS_FAIL, "No creation transaction to complement " << element);
  }

  void DbClientTransactionPlayer::playTemporalRelationCreated(const TiXmlElement& element) {
    const char* relation = element.Attribute("relation");
    check_error(relation != NULL);
    const char * origin = element.Attribute("origin");
    check_error(origin != NULL);
    const char * target = element.Attribute("target");
    check_error(target != NULL);
    TokenId origin_token = parseToken(origin);
    TokenId target_token = parseToken(target);
    debugMsg("DbClientTransactionPlayer:playTemporalRelationCreated",
	     "got token " << origin_token->getKey() << " and " << 
	     target_token->getKey() << " for relation '" << relation << "'");
    checkError(origin_token.isValid(), "Invalid token for label '" << origin << "'");
    checkError(target_token.isValid(), "Invalid token for label '" << target << "'");
    if (strcmp(relation, "before") == 0) {
      construct_constraint(precedes, origin, End, target, Start);
    }
    else if (strcmp(relation, "after") == 0) {
      construct_constraint(precedes, target, End, origin, Start);
    }
    else if (strcmp(relation, "meets") == 0) {
      construct_constraint(concurrent, origin, End, target, Start);
    }
    else if (strcmp(relation, "met_by") == 0) {
      construct_constraint(concurrent, origin, Start, target, End);
    }
    else if ((strcmp(relation, "equal") == 0) || 
	     (strcmp(relation, "equals") == 0)) {
      construct_constraint(concurrent, origin, Start, target, Start);
      construct_constraint(concurrent, origin, End, target, End);
    }
    else if (strcmp(relation, "contains") == 0) {
      construct_constraint(precedes, origin, Start, target, Start);
      construct_constraint(precedes, target, End, origin, End);
    }
    else if (strcmp(relation, "contained_by") == 0) {
      construct_constraint(precedes, target, Start, origin, Start);
      construct_constraint(precedes, origin, End, target, End);
    }
    else if (strcmp(relation, "paralleled_by") == 0) {
      construct_constraint(precedes, target, Start, origin, Start);
      construct_constraint(precedes, target, End, origin, End);
    }
    else if (strcmp(relation, "parallels") == 0) {
      construct_constraint(precedes, origin, Start, target, Start);
      construct_constraint(precedes, origin, End, target, End);
    }
    else if (strcmp(relation, "starts") == 0) {
      construct_constraint(concurrent, origin, Start, target, Start);
    }
    else if (strcmp(relation, "ends") == 0) {
      construct_constraint(concurrent, origin, End, target, End);
    }
    else if (strcmp(relation, "ends_after") == 0) {
      construct_constraint(precedes, target, Start, origin, End);
    }
    else if (strcmp(relation, "ends_before") == 0) {
      construct_constraint(precedes, origin, End, target, Start);
    }
    else if (strcmp(relation, "ends_after_start") == 0) {
      construct_constraint(precedes, target, Start, origin, End);
    }
    else if (strcmp(relation, "starts_before_end") == 0) {
      construct_constraint(precedes, origin, Start, target, End);
    }
    else if (strcmp(relation, "starts_during") == 0) {
      construct_constraint(precedes, target, Start, origin, Start);
      construct_constraint(precedes, origin, Start, target, End);
    }
    else if (strcmp(relation, "contains_start") == 0) {
      construct_constraint(precedes, origin, Start, target, Start);
      construct_constraint(precedes, target, Start, origin, End);
    }
    else if (strcmp(relation, "ends_during") == 0) {
      construct_constraint(precedes, target, Start, origin, End);
      construct_constraint(precedes, origin, End, target, End);
    }
    else if (strcmp(relation, "contains_end") == 0) {
      construct_constraint(precedes, origin, Start, target, End);
      construct_constraint(precedes, target, End, origin, End);
    }
    else if (strcmp(relation, "starts_after") == 0) {
      construct_constraint(precedes, target, Start, origin, Start);
    }
    else if (strcmp(relation, "starts_before") == 0) {
      construct_constraint(precedes, origin, Start, target, Start);
    }
    else {
      checkError(strcmp(relation, "any") == 0,
		 "unknown temporal relation name '" << relation << "'");
    }
  }

  DbClientTransactionPlayer::TemporalRelations::iterator
  DbClientTransactionPlayer::getTemporalConstraint(const ConstrainedVariableId& fvar,
						   const ConstrainedVariableId& svar,
						   const std::string& name) {
    std::pair<TemporalRelations::iterator, TemporalRelations::iterator> range =
      m_relations.equal_range(std::make_pair(fvar, svar));
    checkError(range.first != m_relations.end(),
	       "No saved temporal constraints between " << fvar->toString() << " and " <<
	       svar->toString());
    LabelStr relName(name);
    for(TemporalRelations::iterator it = range.first; it != range.second; ++it) {
      if(it->second->getName() == relName)
	return it;
    }
    return m_relations.end();
  }

  void DbClientTransactionPlayer::deleteTemporalConstraint(TemporalRelations::iterator it) {
    check_error(it != m_relations.end());
    ConstraintId constr = it->second;
    check_error(constr.isValid());
    m_relations.erase(it);
    m_client->deleteConstraint(constr);
  }
  
  void DbClientTransactionPlayer::removeTemporalConstraint(const ConstrainedVariableId& fvar,
							   const ConstrainedVariableId& svar,
							   const std::string& name) {
    deleteTemporalConstraint(getTemporalConstraint(fvar, svar, name));
  }

  void DbClientTransactionPlayer::playTemporalRelationDeleted(const TiXmlElement& element) {
    const char* relation = element.Attribute("relation");
    check_error(relation != NULL);
    const char * origin = element.Attribute("origin");
    check_error(origin != NULL);
    const char * target = element.Attribute("target");
    check_error(target != NULL);
    TokenId origin_token = parseToken(origin);
    TokenId target_token = parseToken(target);
    debugMsg("DbClientTransactionPlayer:playTemporalRelationDeleted",
	     "got token " << origin_token->getKey() << " and " << 
	     target_token->getKey() << " for relation " << relation);
    checkError(origin_token.isValid(), "Invalid token for label '" << origin << "'");
    checkError(target_token.isValid(), "Invalid token for label '" << target << "'");
    if (strcmp(relation, "before") == 0)
      removeTemporalConstraint(origin_token->getEnd(), target_token->getStart(), "precedes");
    else if (strcmp(relation, "after") == 0)
      removeTemporalConstraint(target_token->getEnd(), origin_token->getStart(), "precedes");
    else if (strcmp(relation, "meets") == 0)
      removeTemporalConstraint(origin_token->getEnd(), target_token->getStart(), "concurrent");
    else if (strcmp(relation, "met_by") == 0)
      removeTemporalConstraint(origin_token->getStart(), target_token->getEnd(), "concurrent");
    else if ((strcmp(relation, "equal") == 0) || 
	     (strcmp(relation, "equals") == 0)) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getStart(), "concurrent");
      removeTemporalConstraint(origin_token->getEnd(), target_token->getEnd(), "concurrent");
    }
    else if (strcmp(relation, "contains") == 0) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getStart(), "precedes");
      removeTemporalConstraint(target_token->getEnd(), origin_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "contained_by") == 0) {
      removeTemporalConstraint(target_token->getStart(), origin_token->getStart(), "precedes");
      removeTemporalConstraint(origin_token->getEnd(), target_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "paralleled_by") == 0) {
      removeTemporalConstraint(target_token->getStart(), origin_token->getStart(), "precedes");
      removeTemporalConstraint(target_token->getEnd(), origin_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "parallels") == 0) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getStart(), "precedes");
      removeTemporalConstraint(origin_token->getEnd(), target_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "starts") == 0) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getStart(), "concurrent");
    }
    else if (strcmp(relation, "ends") == 0) {
      removeTemporalConstraint(origin_token->getEnd(), target_token->getEnd(), "concurrent");
    }
    else if (strcmp(relation, "ends_after") == 0) {
      removeTemporalConstraint(target_token->getStart(), origin_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "ends_before") == 0) {
      removeTemporalConstraint(origin_token->getEnd(), target_token->getStart(), "precedes");
    }
    else if (strcmp(relation, "ends_after_start") == 0) {
      removeTemporalConstraint(target_token->getStart(), origin_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "starts_before_end") == 0) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "starts_during") == 0) {
      removeTemporalConstraint(target_token->getStart(), origin_token->getStart(), "precedes");
      removeTemporalConstraint(origin_token->getStart(), target_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "contains_start") == 0) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getStart(), "precedes");
      removeTemporalConstraint(target_token->getStart(), origin_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "ends_during") == 0) {
      removeTemporalConstraint(target_token->getStart(), origin_token->getEnd(), "precedes");
      removeTemporalConstraint(origin_token->getEnd(), target_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "contains_end") == 0) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getEnd(), "precedes");
      removeTemporalConstraint(target_token->getEnd(), origin_token->getEnd(), "precedes");
    }
    else if (strcmp(relation, "starts_after") == 0) {
      removeTemporalConstraint(target_token->getStart(), origin_token->getStart(), "precedes");
    }
    else if (strcmp(relation, "starts_before") == 0) {
      removeTemporalConstraint(origin_token->getStart(), target_token->getStart(), "precedes");
    }
    else {
      check_error(strcmp(relation, "any") == 0,
		  std::string("unknown temporal relation name ") + std::string(relation));
    }
  }

  void DbClientTransactionPlayer::playConstrained(const TiXmlElement & element) {
    ObjectId object;
    TokenId predecessor, successor;
    getElementsFromConstrain(element, object, predecessor, successor);
    m_client->constrain(object, predecessor, successor);
  }

  void DbClientTransactionPlayer::playFreed(const TiXmlElement & element) {
    ObjectId object;
    TokenId predecessor, successor;
    getElementsFromConstrain(element, object, predecessor, successor);
    m_client->free(object, predecessor, successor);
  }

  void DbClientTransactionPlayer::getElementsFromConstrain(const TiXmlElement& element,
							   ObjectId& object,
							   TokenId& predecessor,
							   TokenId& successor) {
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      object = m_client->getObject(element.Attribute("identifier"));
    }
    else {
      TiXmlElement* object_el = element.FirstChildElement("object");
      check_error(object_el != NULL && object_el->Attribute("name") != NULL);
      object = m_client->getObject(object_el->Attribute("name"));
    }
    check_error(object.isValid());

    TiXmlElement* token = element.FirstChildElement("token");
    if(token == NULL)
      token = element.FirstChildElement("id");
    check_error(token != NULL);

    if(token->Attribute("path") != NULL)
      predecessor = xmlAsToken(*token);
    else {
      check_error(token->Attribute("name") != NULL);
      predecessor = parseToken(token->Attribute("name"));
    }
    check_error(predecessor.isValid());
    successor = predecessor;

    token = token->NextSiblingElement(token->Value());
    if(token != NULL) {
      if(token->Attribute("path") != NULL)
	successor = xmlAsToken(*token);
      else {
	check_error(token->Attribute("name") != NULL);
	successor = parseToken(token->Attribute("name"));
      }
    }
    check_error(successor.isValid());
  }

  template <typename Iterator>
  void DbClientTransactionPlayer::playUnfreed(const TiXmlElement& element, Iterator start,
					      Iterator end) {
    debugMsg("DbClientTransactionPlayer:playUnfreed",
	     "Rewinding transaction " << element);
    ObjectId object;
    TokenId predecessor, successor;
    getElementsFromConstrain(element, object, predecessor, successor);

    //have to search backwards for the last 'constrain' transaction to un-play the free
    for(Iterator it = start; it != end; ++it) {
      debugMsg("DbClientTransactionPlayer:playUnfreed",
	       "Iterating backwards over " << **it);
      if((*it)->Value() == std::string("constrain") ||
	 ((*it)->Value() == std::string("invoke") && (*it)->Attribute("name") != NULL &&
	  (*it)->Attribute("name") == std::string("constrain"))) {
	ObjectId object2;
	TokenId predecessor2, successor2;
	getElementsFromConstrain(**it, object2, predecessor2, successor2);

	if(object2 == object && predecessor2 == predecessor && successor2 == successor) {
	  processTransaction(**it);
	  return;
	}
      }
    }
    checkError(ALWAYS_FAIL, "No constrain transaction prior to " << element);
  }

  void DbClientTransactionPlayer::playActivated(const TiXmlElement & element) {
    TokenId token = TokenId::noId();
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      std::string name(element.Attribute("identifier"));
      token = m_tokens[name];
    }
    else {
      TiXmlElement * token_el = element.FirstChildElement();
      check_error(token_el != NULL);
      token = xmlAsToken(*token_el);
    }
    check_error(token.isValid());
    if(!token->isActive()) //Temporary.  Pull out when we scrub test input files
      m_client->activate(token);
  }

  void DbClientTransactionPlayer::playMerged(const TiXmlElement & element) {
    TokenId token = TokenId::noId();
    TiXmlElement* active_el = NULL;
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      std::string name(element.Attribute("identifier"));
      token = m_tokens[name];
      active_el = element.FirstChildElement("token");
    }
    else {
      TiXmlElement * token_el = element.FirstChildElement("token");
      check_error(token_el != NULL);
      token = xmlAsToken(*token_el);
      active_el = token_el->NextSiblingElement("token");
    }

    check_error(token.isValid());
    checkError(active_el != NULL, "Active element required for merge.");

    TokenId active_token = xmlAsToken(*active_el);
    check_error(active_token.isValid());
    m_client->merge(token, active_token);
  }

  void DbClientTransactionPlayer::playRejected(const TiXmlElement & element) {
    TokenId token = TokenId::noId();
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      std::string name(element.Attribute("identifier"));
      token = m_tokens[name];
    }
    else {
      TiXmlElement * token_el = element.FirstChildElement();
      check_error(token_el != NULL);
      token = xmlAsToken(*token_el);
    }
    check_error(token.isValid());
    m_client->reject(token);    
  }

  void DbClientTransactionPlayer::playCancelled(const TiXmlElement & element)
  {
    TokenId token = TokenId::noId();
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      std::string name(element.Attribute("identifier"));
      token = m_tokens[name];
    }
    else {
      TiXmlElement * token_el = element.FirstChildElement();
      check_error(token_el != NULL);
      token = xmlAsToken(*token_el);
    }
    check_error(token.isValid());
    m_client->cancel(token);    
  }

  template<typename Iterator>
  void DbClientTransactionPlayer::playUncancelled(const TiXmlElement& element, Iterator start,
						  Iterator end) {
    debugMsg("DbClientTransactionPlayer:playUncancelled",
	     "Processing inverse of " << element);
    TokenId token = TokenId::noId();
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      std::string name(element.Attribute("identifier"));
      token = m_tokens[name];
    }
    else {
      TiXmlElement * token_el = element.FirstChildElement();
      check_error(token_el != NULL);
      token = xmlAsToken(*token_el);
    }
    check_error(token.isValid());

    debugMsg("DbClientTransactionPlayer:playUncancelled",
	     "Looking for a prior restriction transaction.");
    for(Iterator it = start; it != end; ++it) {
      debugMsg("DbClientTransactionPlayer:playUncancelled",
	       "Checking " << **it);
      if(strcmp((*it)->Value(), "activate") == 0 || strcmp((*it)->Value(), "merge") == 0 ||
	 strcmp((*it)->Value(), "reject") == 0 ||
	 (strcmp((*it)->Value(), "invoke") == 0 && (*it)->Attribute("name") != NULL &&
	  (strcmp((*it)->Attribute("name"), "activate") == 0 ||
	   strcmp((*it)->Attribute("name"), "merge") == 0 ||
	   strcmp((*it)->Attribute("name"), "reject") == 0))) {
	TokenId transToken = TokenId::noId();
	if(strcmp((*it)->Value(), "invoke") == 0) {
	  check_error((*it)->Attribute("identifier") != NULL);
	  std::string name((*it)->Attribute("identifier"));
	  transToken = m_tokens[name];
	}
	else {
	  TiXmlElement * token_el = (*it)->FirstChildElement();
	  check_error(token_el != NULL);
	  transToken = xmlAsToken(*token_el);
	}
	check_error(transToken.isValid());
	if(transToken == token) {
	  debugMsg("DbClientTransactionPlayer:playUncancelled",
		   "Transaction is the proper inverse.  Playing.");
	  processTransaction(**it);
	  return;
	}
      }
    }
    checkError(ALWAYS_FAIL, "No activate, merge, or reject transaction prior to " << element);
  }

  void DbClientTransactionPlayer::playVariableAssigned(const TiXmlElement & element) {

    const char * name = element.Attribute("name");
    check_error(name != NULL);
    debugMsg("DbClientTransactionPlayer:playVariableAssigned", "assigning for " << name);
    ConstrainedVariableId variable = parseVariable(name);
    debugMsg("DbClientTransactionPlayer:playVariableAssigned", "found variable " << variable->getKey());
    TiXmlElement * value_el = element.FirstChildElement();
    check_error(value_el != NULL);
    const AbstractDomain * value = xmlAsAbstractDomain(*value_el);
    debugMsg("DbClientTransactionPlayer:playVariableAssigned", "specifying to " << (*value));
    variable->restrictBaseDomain(*value);
  }

  void DbClientTransactionPlayer::playVariableSpecified(const TiXmlElement & element) {
    debugMsg("DbClientTransactionPlayer:playVariableSpecified",
	     "Playing " << element);
    ConstrainedVariableId var = ConstrainedVariableId::noId();
    TiXmlElement* value_el = NULL;
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      var = parseVariable(element.Attribute("identifier"));
      value_el = element.FirstChildElement();
    }
    else {
      TiXmlElement * var_el = element.FirstChildElement("variable");
      if(var_el == NULL)
	var_el = element.FirstChildElement("id");
      check_error(var_el != NULL);
      var = xmlAsVariable(*var_el);
      value_el = var_el->NextSiblingElement();
    }

    check_error(var.isValid());
    check_error(value_el != NULL);

    const AbstractDomain * value = xmlAsAbstractDomain(*value_el);
    if (value->isSingleton()) {
      double v = value->getSingletonValue();
      m_client->specify(var, v);
    } 
    else
      m_client->restrict(var, *value);
  }

  void DbClientTransactionPlayer::playVariableRestricted(const TiXmlElement & element) {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    ConstrainedVariableId variable = xmlAsVariable(*var_el);
    check_error(variable.isValid());

    TiXmlElement * value_el = var_el->NextSiblingElement();
    check_error(value_el != NULL);
    const AbstractDomain * value = xmlAsAbstractDomain(*value_el);
    m_client->restrict(variable, *value);
  }

  void DbClientTransactionPlayer::playVariableReset(const TiXmlElement & element) {
    ConstrainedVariableId var = ConstrainedVariableId::noId();
    if(strcmp(element.Value(), "invoke") == 0) {
      check_error(element.Attribute("identifier") != NULL);
      var = parseVariable(element.Attribute("identifier"));
    }
    else {
      TiXmlElement * var_el = element.FirstChildElement();
      check_error(var_el != NULL);
      var = xmlAsVariable(*var_el);
    }
    check_error(var.isValid());
    m_client->reset(var);
  }

  template<typename Iterator>
  void DbClientTransactionPlayer::playVariableUnreset(const TiXmlElement& element,
						      Iterator start, Iterator end) {
    TiXmlElement * var_el = element.FirstChildElement();
    check_error(var_el != NULL);
    ConstrainedVariableId var = xmlAsVariable(*var_el);
    
    for(Iterator it = start; it != end; ++it) {
      if(strcmp((*it)->Value(), "specify") == 0 ||
	 (strcmp((*it)->Value(), "invoke") == 0 && (*it)->Attribute("name") != NULL &&
	  strcmp((*it)->Attribute("name"), "specify") == 0)) {
	ConstrainedVariableId transVar = ConstrainedVariableId::noId();
	if(strcmp((*it)->Value(), "invoke") == 0) {
	  check_error((*it)->Attribute("identifier") != NULL);
	  transVar = parseVariable((*it)->Attribute("identifier"));
	}
	else {
	  TiXmlElement * var_el = (*it)->FirstChildElement("variable");
	  if(var_el == NULL)
	    var_el = (*it)->FirstChildElement("id");
	  check_error(var_el != NULL);
	  transVar = xmlAsVariable(*var_el);
	}
	check_error(transVar.isValid());
	if(transVar == var) {
	  playVariableSpecified(**it);
	  return;
	}
      }
    }
    checkError(ALWAYS_FAIL, "No specify transaction prior to " << element);
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
    std::ostringstream os;
    std::vector<ConstrainedVariableId> variables;
    for (TiXmlElement * child_el = element.FirstChildElement() ;
         child_el != NULL ; child_el = child_el->NextSiblingElement()) {
       ConstrainedVariableId v = xmlAsVariable(*child_el);   	
       variables.push_back(v);
       os << v->toString() <<",";
    }
    debugMsg("DbClientTransactionPlayer:playInvokeConstraint","Adding constraint " << name << " args: " << os.str());
    m_client->createConstraint(name, variables);
    debugMsg("DbClientTransactionPlayer:playInvokeConstraint","Added constraint " << name << " args: " << os.str());
  }

  void DbClientTransactionPlayer::playUninvokeConstraint(const TiXmlElement& element) {
    const char* index = element.Attribute("index");
    check_error(index != NULL);
    
    std::stringstream str;
    str << index;
    int key;
    str >> key;
    ConstraintId constr = m_client->getConstraintByIndex(key);
    m_client->deleteConstraint(constr);
  }

  template<typename Iterator>
  void DbClientTransactionPlayer::playUninvokeConstraint(const TiXmlElement& element,
							 Iterator start, Iterator end) {
    const char* identifier = element.Attribute("identifier");
    if(identifier != NULL || element.FirstChildElement() == NULL) {
      playUninvokeTransaction(element, start, end);
      return;
    }
    playUninvokeConstraint(element);
  }

  template<typename Iterator>
  void DbClientTransactionPlayer::playReinvokeConstraint(const TiXmlElement& element,
							 Iterator start, Iterator end) {
    const char* name = element.Attribute("name");
    check_error(name != NULL);
    const char* index = element.Attribute("index");
    std::vector<ConstrainedVariableId> vars;
    for(TiXmlElement* varElem = element.FirstChildElement("variable"); varElem != NULL;
	varElem = varElem->NextSiblingElement("variable")) {
      ConstrainedVariableId var = xmlAsVariable(*varElem);
      check_error(var.isValid());
      vars.push_back(var);
    }
    check_error(!vars.empty());

    for(Iterator it = start; it != end; ++it) {
      if(strcmp((*it)->Value(), "invoke") == 0 && (*it)->Attribute("identifier") == NULL &&
	 (*it)->Attribute("name") != NULL && strcmp((*it)->Attribute("name"), name) == 0 &&
	 ((index == NULL || (*it)->Attribute("index") == NULL) ||
	  strcmp((*it)->Attribute("index"), index) == 0)) {
	std::vector<ConstrainedVariableId> otherVars;
	for(TiXmlElement* varElem = (*it)->FirstChildElement("variable"); varElem != NULL;
	    varElem = varElem->NextSiblingElement("variable")) {
	  ConstrainedVariableId var = xmlAsVariable(*varElem);
	  check_error(var.isValid());
	  otherVars.push_back(var);
	}
	check_error(!otherVars.empty());
	if(vars == otherVars) {
	  playInvokeConstraint(**it);
	  return;
	}
      }
    }
    checkError(ALWAYS_FAIL, "No creation transaction to complement " << element);
  }

  void DbClientTransactionPlayer::playInvokeTransaction(const TiXmlElement & element) {
    const char * name = element.Attribute("name");
    check_error(name != NULL);
    const char * identifier = element.Attribute("identifier");
    if (strcmp(name, "close") == 0) {
      if(identifier != NULL)
	m_client->close(identifier);
      else
        m_client->close();
    }
    else if (strcmp(name, "constrain") == 0)
      playConstrained(element);
    else if (strcmp(name, "free") == 0)
      playFreed(element);
    else if (strcmp(name, "activate") == 0)
      playActivated(element);
    else if (strcmp(name, "merge") == 0)
      playMerged(element);
    else if (strcmp(name, "reject") == 0)
      playRejected(element);
    else if (strcmp(name, "cancel") == 0)
      playCancelled(element);
    else if (strcmp(name, "specify") == 0)
      playVariableSpecified(element);
    else {
      check_error(ALWAYS_FAILS, "unexpected transaction invoked: '" + std::string(name) + "'");
    }
  }

  template<typename Iterator>
  void DbClientTransactionPlayer::playUninvokeTransaction(const TiXmlElement& element,
							  Iterator start, Iterator end) {
    const char * name = element.Attribute("name");
    check_error(name != NULL);
    if (strcmp(name, "close") == 0) {
      debugMsg("DbClientTransactionPlayer:playUninvokeTransaction", 
	       "Warning:  can't un-close the database or a type.");
      return;
    }
    else if (strcmp(name, "constrain") == 0)
      playFreed(element);
    else if (strcmp(name, "free") == 0)
      playUnfreed(element, start, end);
    else if (strcmp(name, "activate") == 0)
      playCancelled(element);
    else if (strcmp(name, "merge") == 0)
      playCancelled(element);
    else if (strcmp(name, "reject") == 0)
      playCancelled(element);
    else if (strcmp(name, "cancel") == 0)
      playUncancelled(element, start, end);
    else if (strcmp(name, "specify") == 0)
      playVariableReset(element);
    else {
      check_error(ALWAYS_FAILS, "unexpected transaction invoked: '" + std::string(name) + "'");
    }
    
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
    checkError(var.isValid(), "Invalid id for " << ident);
    ObjectId object = var->lastDomain().getSingletonValue();
    checkError(object.isValid(), "Invalid object for " << ident);
    var = object->getVariable(LabelStr(varString));
    checkError(var.isValid(), varString << " not found on " << object->toString());
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
  DbClientTransactionPlayer::xmlAsAbstractDomain(const TiXmlElement & element,
						 const char * name,
						 const char* typeName) {
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
      return(xmlAsEnumeratedDomain(element, typeName));
    if (strcmp(tag, "interval") == 0)
      return(xmlAsIntervalDomain(element, typeName));
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
  DbClientTransactionPlayer::xmlAsIntervalDomain(const TiXmlElement & element,
						 const char* typeName) {
    const char * type_st = element.Attribute("type");
    check_error(type_st != NULL);
    const char * min_st = element.Attribute("min");
    check_error(min_st != NULL);
    const char * max_st = element.Attribute("max");
    check_error(max_st != NULL);
    IntervalDomain * domain = NULL;
    if(typeName != NULL) {
      if(TypeFactory::baseDomain(type_st).minDelta() == 1)
	domain = new IntervalIntDomain(typeName);
      else if(TypeFactory::baseDomain(type_st).minDelta() < 1 &&
	      TypeFactory::baseDomain(type_st).minDelta() > 0)
	domain = new IntervalDomain(typeName);
      else {
	checkError(ALWAYS_FAIL, "Having trouble trying to duplicate type " << type_st);
      }
    }
    else
      domain = dynamic_cast<IntervalDomain*>(TypeFactory::baseDomain(type_st).copy());
    check_error(domain != NULL,
		"type '" + std::string(type_st) + "' should indicate an interval domain type");
    double min = TypeFactory::createValue(type_st, min_st);
    double max = TypeFactory::createValue(type_st, max_st);
    domain->intersect(min, max);
    debugMsg("DbClientTransactionPlayer:xmlAsIntervalDomain",
	     "For " << element << ", created domain " << (*domain).toString());
    return domain;
  }
  
  EnumeratedDomain *
  DbClientTransactionPlayer::xmlAsEnumeratedDomain(const TiXmlElement & element,
						   const char* otherTypeName) {
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
      std::vector<const AbstractDomain*> arguments;
      for (TiXmlElement * child_el = value.FirstChildElement() ;
           child_el != NULL ; child_el = child_el->NextSiblingElement()) {
        const AbstractDomain * domain = xmlAsAbstractDomain(*child_el);
        arguments.push_back(domain);
      }
      ObjectId object = m_client->createObject(type, name, arguments);
      check_error(object.isValid());

      // Now deallocate domains created for arguments
      for (std::vector<const AbstractDomain*>::const_iterator it = arguments.begin(); it != arguments.end(); ++it) {
      	AbstractDomain* tmp = (AbstractDomain*)(*it);
        delete tmp;
      }
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
      int index = -1;
      variable.Attribute("index", &index);
      check_error(variable.Attribute("index", &index) != NULL);
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
    // Default settings for variable allocation
    bool isTmpVar = false;
    bool canBeSpecified = true;
    std::string gen_name;

    // If no name is specified, it should not be decidable by a solver and does not need to be registered
    if (name == NULL) {
      canBeSpecified = false;
      isTmpVar = true;
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
        if ((strcmp(type, "value") == 0) || (strcmp(type, "new") == 0) || (strcmp(type, "symbol") == 0) || (strcmp(type, "interval") == 0)) {
          type = value->Attribute("type");
        }
      }
      debugMsg("DbClientTransactionPlayer:xmlAsCreateVariable", 
	       " type = " << type << " name = " << name << " domain type = " << baseDomain->getTypeName().c_str());
    }

    check_error(type != NULL);
    if (baseDomain != NULL) {
      ConstrainedVariableId variable = m_client->createVariable(type, *baseDomain, name, isTmpVar, canBeSpecified);
      delete baseDomain;
      return variable;
    }

    return m_client->createVariable(type, name);
  }  
}
