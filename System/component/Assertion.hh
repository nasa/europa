#ifndef _H_Assertion
#define _H_Assertion

#include "Test.hh"
#include "Error.hh"
#include "AbstractDomain.hh"
#include "NumericDomain.hh"
#include "EventAggregator.hh"
#include "AggregateListener.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include <string>
#include <queue>

namespace Prototype {

  class AssertionErr {
  public:
    DECLARE_ERROR(AssertionFailedError);
  };

  typedef bool(*DomComp)(const AbstractDomain &d1, const AbstractDomain& d2);

  class Assertion : public Test, public AggregateListener {
  public:
    enum FailMode {
      FAIL_FAST = 0,
      FAIL_WARN,
      FAIL_WAIT
    };

    Assertion(const PlanDatabaseId& dbId, const std::string& file = "UnknownFile", const int lineNo = 0,
              const std::string& lineText = "UnknownText");
    virtual ~Assertion(){};
    virtual bool check(){return false;};  //should the assertion be run now?
    virtual Test::Status execute(){return m_status;};
    static void dumpErrors(std::ostream& os = std::cerr);
    static void failFast(){s_mode = FAIL_FAST;}
    static void failWarn(){s_mode = FAIL_WARN;}
    static void failWait(){s_mode = FAIL_WAIT;}
    Test::Status result() {return m_status;}
    void dumpResults();
  protected:
    std::string failureString();
    void fail();
    static bool eq(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool ne(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool gt(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool lt(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool ge(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool le(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool in(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool out(const AbstractDomain&d1, const AbstractDomain& d2);
    static bool intersects(const AbstractDomain& d1, const AbstractDomain& d2);
    static double getGreatest(const AbstractDomain& d);
    static double getLeast(const AbstractDomain& d);
    static bool boolPredicate(const TokenId& tok, DomComp cmp, const AbstractDomain& d);
    static bool boolStart(const TokenId& tok, DomComp cmp, const AbstractDomain& d);
    static bool boolEnd(const TokenId& tok, DomComp cmp, const AbstractDomain& d);
    static bool boolStatus(const TokenId& tok, DomComp cmp, const AbstractDomain& d);
    static bool boolVar(const TokenId& tok, DomComp nameComp, const AbstractDomain& nameDom, DomComp cmp, const AbstractDomain& d);
    static bool boolName(const ObjectId& obj, DomComp cmp, const AbstractDomain& d);
    static bool boolVar(const ObjectId& obj, DomComp nameComp, const AbstractDomain& nameDom, DomComp cmp, const AbstractDomain& d);
    static void trimTokensByPredicate(TokenSet& tokens, DomComp cmp, const AbstractDomain& d);
    static void trimTokensByStart(TokenSet& tokens, DomComp cmp, const AbstractDomain& d);
    static void trimTokensByEnd(TokenSet& tokens, DomComp cmp, const AbstractDomain& d);
    static void trimTokensByStatus(TokenSet& tokens, DomComp cmp, const AbstractDomain& d);
    static void trimTokensByVariable(TokenSet& tokens, DomComp nameComp, const AbstractDomain& nameDom,
                                     DomComp cmp, const AbstractDomain& d);
    static void trimObjectsByName(ObjectSet& objs, DomComp cmp, const AbstractDomain& d);
    static void trimObjectsByVariable(ObjectSet& objs, DomComp nameCmp, const AbstractDomain& nameDom,
                                      DomComp cmp, const AbstractDomain& d);
    static NumericDomain count(const AbstractDomain& d);
    static NumericDomain tokenSetToDomain(const TokenSet& tokens);
    static NumericDomain objectSetToDomain(const ObjectSet& objs);
    PlanDatabaseId m_db;
    std::string m_file;
    int m_lineNo;
    std::string m_lineText;
    Test::Status m_status;
    int m_transactionCounter;
  private:

    static std::queue<std::string> s_errors;
    static FailMode s_mode;

  };
}
#undef BOOLEAN_FUNC
#endif
