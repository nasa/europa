#include "Assertion.hh"
#include "StringDomain.hh"
#include <sstream>
#include <list>
#include <algorithm>
#include <iostream>
#include <cstdlib>

namespace Prototype {

  Assertion::FailMode Assertion::s_mode = Assertion::FAIL_FAST;
  std::queue<std::string> Assertion::s_errors = std::queue<std::string>();

  Assertion::Assertion(const PlanDatabaseId& dbId, const std::string& file, const int lineNo, const std::string& lineText)
    : Test(), AggregateListener(), m_db(dbId), m_file(file), m_lineNo(lineNo), m_lineText(lineText), m_status(Test::INCOMPLETE),
      m_transactionCounter(0) {}

  std::string Assertion::failureString() {
    std::stringstream out;
    out << "Assertion FAILED at " << m_file << ", line " << m_lineNo << ".\n";
    out << "Assertion: " << m_lineText << "\n";
    return out.str();
  }

  void Assertion::dumpErrors(std::ostream& os) {
    while(!s_errors.empty()) {
      os << s_errors.front();
      os << "------------------------" << std::endl;
      s_errors.pop();
    }
  }

  bool Assertion::eq(const AbstractDomain&d1, const AbstractDomain& d2) {
    return d1 == d2;
  }
  bool Assertion::ne(const AbstractDomain&d1, const AbstractDomain& d2) {
    return d1 != d2;
  }
  bool Assertion::gt(const AbstractDomain&d1, const AbstractDomain& d2) {
    return getLeast(d1) > getGreatest(d2);
  }
  bool Assertion::lt(const AbstractDomain&d1, const AbstractDomain& d2) {
    return getGreatest(d1) < getLeast(d2);
  }
  bool Assertion::ge(const AbstractDomain&d1, const AbstractDomain& d2) {
    return eq(d1, d2) || gt(d1, d2);
  }
  bool Assertion::le(const AbstractDomain&d1, const AbstractDomain& d2) {
    return eq(d1, d2) || lt(d1, d2);
  }
  bool Assertion::in(const AbstractDomain&d1, const AbstractDomain& d2) {
    return d1.isSubsetOf(d2);
  }
  bool Assertion::out(const AbstractDomain&d1, const AbstractDomain& d2) {
    return !in(d1, d2);
  }
  bool Assertion::intersects(const AbstractDomain&d1, const AbstractDomain& d2) {
    return d1.intersects(d2);
  }

  double Assertion::getGreatest(const AbstractDomain &d) {
    if(d.isInterval()) {
      return d.getUpperBound();
    }
    std::list<double> vals;
    d.getValues(vals);
    return *(max_element(vals.begin(), vals.end()));
  }

  double Assertion::getLeast(const AbstractDomain &d) {
    if(d.isInterval()) {
      return d.getLowerBound();
    }
    std::list<double> vals;
    d.getValues(vals);
    return *(min_element(vals.begin(), vals.end()));
  }
  bool Assertion::boolPredicate(const TokenId& tok, DomComp cmp, const AbstractDomain& d) {
    StringDomain dom(tok->getPredicateName().getKey());
    return (cmp)(dom, d);
  }
  bool Assertion::boolStart(const TokenId& tok, DomComp cmp, const AbstractDomain& d) {
    return (cmp)(tok->getStart()->lastDomain(), d);
  }
  bool Assertion::boolEnd(const TokenId& tok, DomComp cmp, const AbstractDomain& d) {
    return (cmp)(tok->getEnd()->lastDomain(), d);
  }
  bool Assertion::boolStatus(const TokenId& tok, DomComp cmp, const AbstractDomain& d) {
    return (cmp)(tok->getState()->lastDomain(), d);
  }
  bool Assertion::boolVar(const TokenId& tok, DomComp nameComp, const AbstractDomain& nameDom,
                          DomComp cmp, const AbstractDomain& d) {
    const std::vector<ConstrainedVariableId>& params = tok->getParameters();
    for(std::vector<ConstrainedVariableId>::const_iterator it = params.begin();
        it != params.end(); ++it) {
      const ConstrainedVariableId var = *it;
      StringDomain paramNameDom(var->getName().getKey());
      if((nameComp)(paramNameDom, nameDom))
        return (cmp)(var->lastDomain(), d);
    }
    return false;
  }
 bool Assertion::boolVar(const ObjectId& obj, DomComp nameComp, const AbstractDomain& nameDom, 
                         DomComp cmp, const AbstractDomain& d) {
   const std::vector<ConstrainedVariableId>& vars = obj->getVariables();
   for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin();
       it != vars.end(); ++it) {
     const ConstrainedVariableId var = *it;
     StringDomain varNameDom(var->getName().getKey());
     if((nameComp)(varNameDom, nameDom))
       return (cmp)(var->lastDomain(), d);
   }
   return false;
 }
  bool Assertion::boolName(const ObjectId& obj, DomComp cmp, const AbstractDomain& d) {
    StringDomain dom(obj->getName().getKey());
    return (cmp)(dom, d);
  }
  void Assertion::trimTokensByPredicate(TokenSet& tokens, DomComp cmp, const AbstractDomain& d) {
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      if(!boolPredicate(tok, cmp, d))
        tokens.erase(it);
    }
  }
  void Assertion::trimTokensByStart(TokenSet& tokens, DomComp cmp, const AbstractDomain& d) {
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      if(!boolStart(tok, cmp, d))
        tokens.erase(it);
    }
  }
  void Assertion::trimTokensByEnd(TokenSet& tokens, DomComp cmp, const AbstractDomain& d) {
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      if(!boolEnd(tok, cmp, d))
        tokens.erase(it);
    }
  }
  void Assertion::trimTokensByStatus(TokenSet& tokens, DomComp cmp, const AbstractDomain& d) {
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      if(!boolStatus(tok, cmp, d))
        tokens.erase(it);
    }
  }
  void Assertion::trimTokensByVariable(TokenSet& tokens, DomComp nameComp, const AbstractDomain& nameDom,
                                  DomComp cmp, const AbstractDomain& d) {
    for(TokenSet::iterator it = tokens.begin(); it != tokens.end(); ++it) {
      TokenId tok = *it;
      if(!boolVar(tok, nameComp, nameDom, cmp, d))
        tokens.erase(it);
    }
  }
  void Assertion::trimObjectsByName(ObjectSet& objs, DomComp cmp, const AbstractDomain& d) {
    for(ObjectSet::iterator it = objs.begin(); it != objs.end(); ++it) {
      ObjectId obj = *it;
      if(!boolName(obj, cmp, d))
        objs.erase(it);
    }
  }
  void Assertion::trimObjectsByVariable(ObjectSet& objs, DomComp nameCmp, const AbstractDomain& nameDom,
                                   DomComp cmp, const AbstractDomain& d) {
    for(ObjectSet::iterator it = objs.begin(); it != objs.end(); ++it) {
      ObjectId obj = *it;
      if(!boolVar(obj, nameCmp, nameDom, cmp, d))
        objs.erase(it);
    }
  }

  EnumeratedDomain Assertion::count(const AbstractDomain& d) {
    EnumeratedDomain retval(true);
    std::list<double> values;
    d.getValues(values);
    retval.insert(values.size());
    retval.close();
    return retval;
  }

  EnumeratedDomain Assertion::tokenSetToDomain(const TokenSet& tokens) {
    EnumeratedDomain retval(false);
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
      retval.insert((*it)->getKey());
    return retval;
  }

  EnumeratedDomain Assertion::objectSetToDomain(const ObjectSet& objs) {
    EnumeratedDomain retval(false);
    for(ObjectSet::const_iterator it = objs.begin(); it != objs.end(); ++it)
      retval.insert((*it)->getKey());
    return retval;
  }

  void Assertion::fail() {
    if(m_status != Test::FAILED)
      return;
    std::string error = failureString();
    if(s_mode == FAIL_FAST || s_mode == FAIL_WARN) {
      std::cerr << error << std::endl;
      if(s_mode == FAIL_FAST)
        exit(EXIT_FAILURE);
    }
    else
      s_errors.push(error);
  }

  void Assertion::dumpResults() {
    std::cerr << "'" << m_lineText << "' at " << m_file << ", " << m_lineNo << ": ";
    switch(m_status) {
    case Test::FAILED:
      std::cerr << "FAILED";
      break;
    case Test::PASSED:
      std::cerr << "PASSED";
      break;
    default:
      std::cerr << "INCOMPLETE";
      break;
    }
    std::cerr << std::endl;
  }
}
