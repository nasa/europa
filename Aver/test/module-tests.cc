#include "AbstractDomain.hh"
#include "AssertionExecutor.hh"
#include "Assertion.hh"
#include "AverHelper.hh"
#include "AverInterp.hh"
#include "ConstraintEngineDefs.hh"
#include "ConstraintEngine.hh"
#include "DefaultPropagator.hh"
#include "EnumeratedDomain.hh"
#include "IntervalDomain.hh"
#include "IntervalToken.hh"
#include "LabelStr.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "StandardAssembly.hh"
#include "SymbolDomain.hh"
#include "TemporalConstraints.hh"
#include "TemporalPropagator.hh"
#include "Test.hh"
#include "Token.hh"

#include "tinyxml.h"

#include "TestSupport.hh"

#include <list>

//#include "execution-tests.cc"

#define binopPush(op,d1,d2) { \
  AverHelper::s_domStack.push((d1)); \
  AverHelper::s_domStack.push((d2)); \
  AverHelper::s_opStack.push((op)); \
}

namespace EUROPA {

  struct TestAssembly {
    ConstraintEngineId m_ce;
    PlanDatabaseId m_db;

    TestAssembly() {
      m_ce = (new ConstraintEngine())->getId();
      m_db = (new PlanDatabase(m_ce, Schema::instance()))->getId();

      new DefaultPropagator(LabelStr("Default"), m_ce);
      new TemporalPropagator(LabelStr("Temporal"), m_ce);
      initConstraintEngine();
    }
    ~TestAssembly() {
      Entity::purgeStarted();
      uninitConstraintEngine();
      delete (PlanDatabase*) m_db;
      delete (ConstraintEngine*) m_ce;
      ConstraintLibrary::purgeAll();
      Schema::instance()->reset();
      Entity::purgeEnded();
    }
  };

  class FooTest {
  public:
    static bool test() {
      runTest(otherTest);
      return true;
    }
  private:
    static bool otherTest() {
      return true;
    }
  };  
  
  class AverHelperTest {
  public:
    static bool test() {
      runTest(testDomainContainer);
      runTest(testEvaluateDomain);
      runTest(testBoolean);
      runTest(testBuildPath);
      runTest(testExecute);//build various paths/situations and run them
      runTest(testConversions);
      runTest(testBoolOp);
      runTest(testCount);
      runTest(testEntity);
      runTest(testQueryTokens);
      runTest(testQueryObjects);
      runTest(testGreatestLeast);
      runTest(testTrimByPred);
      runTest(testTrimByName);
      runTest(testTrimByStart);
      runTest(testTrimByEnd);
      runTest(testTrimByStatus);
      runTest(testTrimTokensByVariable);
      runTest(testTrimObjectsByVariable);
      runTest(testTokenProperty);
      runTest(testObjectProperty);
      return true;
    }
  private:
    static bool testDomainContainer() {
      IntervalDomain* int1 = new IntervalDomain(0., 1.);
      IntervalDomain* int2 = new IntervalDomain(0., 1.);

      DomainContainer* d1 = new DomainContainer(int1, false, true);
      DomainContainer* d2 = new DomainContainer(int2, true, false);
      
      assert(d1->earlyDeath());
      assert(!d2->earlyDeath());
      
      assert(*(*d1) == *int1);
      assert(*(*d2) == *int2);
      
      delete d1;
      delete d2;

      delete int1;
      return true;
    }

    static bool testEvaluateDomain() {
      std::string strings[4] = {"foo", "bar", "baz", "quux"};
      std::list<double> strs;
      for(int i = 0; i < 4; i++)
        strs.push_back((double)LabelStr(strings[i]));

      SymbolDomain stringEnumDom(strs);

      TiXmlElement stringEnumXml("EnumeratedDomain");
      for(int i = 0; i < 4; i++) {
        TiXmlElement* val = new TiXmlElement("Value");
        val->SetAttribute("type", "string");
        TiXmlText* str = new TiXmlText(strings[i]);
        val->InsertEndChild(*str);
        stringEnumXml.InsertEndChild(*val);
      }
      
      DomainContainer* dom = AverHelper::evaluateDomain(&stringEnumXml);

      assert(stringEnumDom == *(*dom));
      delete dom;

      std::list<double> ints;
      for(int i = 0; i < 4; i++)
        ints.push_back(i);
      
      EnumeratedDomain numEnumDom(ints, true, EnumeratedDomain::getDefaultTypeName().c_str());
    
      TiXmlElement numEnumXml("EnumeratedDomain");
      for(int i = 0; i < 4; i++) {
        TiXmlElement* val = new TiXmlElement("Value");
        val->SetAttribute("type", "number");
        std::stringstream n;
        n << i;
        TiXmlText* num = new TiXmlText(n.str());
        val->InsertEndChild(*num);
      numEnumXml.InsertEndChild(*val);
      }
      
      dom = AverHelper::evaluateDomain(&numEnumXml);
      
      assert(numEnumDom == *(*dom));
      delete dom;
      
      IntervalDomain intervalDom(1., 2.);
      
      TiXmlElement intervalXml("IntervalDomain");
      TiXmlElement lb("LowerBound");
      TiXmlElement ub("UpperBound");
      
      TiXmlElement val1("Value");
      val1.SetAttribute("type", "number");
      TiXmlText txt1("1");
      val1.InsertEndChild(txt1);
      lb.InsertEndChild(val1);
      intervalXml.InsertEndChild(lb);
      
      TiXmlElement val2("Value");
      val2.SetAttribute("type", "number");
      TiXmlText txt2("2");
      val2.InsertEndChild(txt2);
      ub.InsertEndChild(val2);
      intervalXml.InsertEndChild(ub);
     
      dom = AverHelper::evaluateDomain(&intervalXml);
      assert(intervalDom == *(*dom));
      delete dom;
      
      return true;
    }

    static bool testBoolean() {

      EnumeratedDomain d1(1., true, EnumeratedDomain::getDefaultTypeName().c_str());
      EnumeratedDomain d2(2., true, EnumeratedDomain::getDefaultTypeName().c_str());
      EnumeratedDomain d3(2., true, EnumeratedDomain::getDefaultTypeName().c_str());

      EnumeratedDomain d4(true, EnumeratedDomain::getDefaultTypeName().c_str());
      d4.insert(1.);
      d4.insert(3.);
      d4.insert(4.);

      EnumeratedDomain d5(true, EnumeratedDomain::getDefaultTypeName().c_str());
      d5.insert(1.);
      d5.insert(3.);

      EnumeratedDomain d6(true, EnumeratedDomain::getDefaultTypeName().c_str());
      d6.insert(9.);
      d6.insert(10.);

      EnumeratedDomain d7(true, EnumeratedDomain::getDefaultTypeName().c_str());
      d7.insert(3.);
      d7.insert(4.);
      d7.insert(9.);
      d7.insert(10.);

      assert(!AverHelper::evaluateBoolean("=", d1, d2));
      assert(AverHelper::evaluateBoolean("=", d2, d3));
      assert(AverHelper::evaluateBoolean("!=", d1, d2));
      assert(AverHelper::evaluateBoolean("<", d1, d2));
      assert(!AverHelper::evaluateBoolean(">", d1, d2));
      assert(AverHelper::evaluateBoolean("<=", d1, d2));
      assert(AverHelper::evaluateBoolean("<=", d2, d3));
      assert(AverHelper::evaluateBoolean(">=", d2, d1));
      assert(AverHelper::evaluateBoolean(">=", d2, d3));
      assert(AverHelper::evaluateBoolean("in", d1, d4));
      assert(AverHelper::evaluateBoolean("out", d2, d4));
      assert(AverHelper::evaluateBoolean("in", d5, d4));
      assert(AverHelper::evaluateBoolean("out", d6, d4));
      assert(AverHelper::evaluateBoolean("intersects", d4, d7));
      return true;
    }

    static bool testBuildPath() {
      TiXmlDocument paths("testpaths.xml");
      assert(paths.LoadFile());
      TiXmlElement* root = paths.RootElement();
      TiXmlElement* file = NULL;
      while((file = (TiXmlElement*) root->IterateChildren(file)) != NULL) {
        TiXmlElement* path = NULL;
        while((path = (TiXmlElement*) file->IterateChildren(path)) != NULL) {
          AverExecutionPathId compiledPath = AverHelper::buildExecutionPath(path);
          //path->Print(stderr, 0);
          //std::cerr << "\n";
          //(*compiledPath) >> std::cerr;
          //std::cerr << "=======================" << std::endl;
          delete (AverExecutionPath*) compiledPath;
        }
      }
      return true;
    }

    static bool testExecute(){return true;}
    static bool testConversions() {
      TestAssembly assembly;

      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));
      TokenSet tokens;
      std::list<double> tokenIds;
      for(int i = 0; i < 10; i++) {
        TokenId tok = (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true))->getId();
        tokens.insert(tok);
        tokenIds.push_back((double)tok);
      }


      DomainContainer* tokDom = AverHelper::tokenSetToDomain(tokens);
      std::list<double> resultIds;
      (*tokDom)->getValues(resultIds);

      
      resultIds.sort();
      tokenIds.sort();
      assert(resultIds == tokenIds);


      DomainContainer tempDom(new EnumeratedDomain(tokenIds, false, EnumeratedDomain::getDefaultTypeName().c_str()), true, true);
      TokenSet tokSet = AverHelper::domainToTokenSet(&tempDom);
      
      assert(tokSet == tokens);
      return true;
    }

    static bool testBoolOp() {
      DomainContainer d1(new EnumeratedDomain(1., true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      DomainContainer d2(new EnumeratedDomain(2., true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      DomainContainer d3(new EnumeratedDomain(2., true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);

      DomainContainer d4(new EnumeratedDomain(true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      (*d4).insert(1.);
      (*d4).insert(3.);
      (*d4).insert(4.);

      DomainContainer d5(new EnumeratedDomain(true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      (*d5).insert(1.);
      (*d5).insert(3.);

      DomainContainer d6(new EnumeratedDomain(true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      (*d6).insert(9.);
      (*d6).insert(10.);

      DomainContainer d7(new EnumeratedDomain(true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      (*d7).insert(3.);
      (*d7).insert(4.);
      (*d7).insert(9.);
      (*d7).insert(10.);

      binopPush("=", &d1, &d2);
      assert(!AverHelper::boolOp());
      assert(AverHelper::s_domStack.empty()); 
      assert(AverHelper::s_opStack.empty());

      binopPush("=", &d2, &d3);
      assert(AverHelper::boolOp());

      binopPush("!=", &d1, &d2);
      assert(AverHelper::boolOp());

      binopPush("<", &d1, &d2);
      assert(AverHelper::boolOp());

      binopPush(">", &d1, &d2);
      assert(!AverHelper::boolOp());

      binopPush("<=", &d1, &d2);
      assert(AverHelper::boolOp());
      
      binopPush("<=", &d2, &d3);
      assert(AverHelper::boolOp());

      binopPush(">=", &d2, &d1);
      assert(AverHelper::boolOp());

      binopPush(">=", &d2, &d3);
      assert(AverHelper::boolOp());

      binopPush("in", &d1, &d4);
      assert(AverHelper::boolOp());

      binopPush("out", &d2, &d4);
      assert(AverHelper::boolOp());

      binopPush("in", &d5, &d4);
      assert(AverHelper::boolOp());

      binopPush("out", &d6, &d4);
      assert(AverHelper::boolOp());

      binopPush("intersects", &d4, &d7);
      assert(AverHelper::boolOp());
      return true;
    }
    
    static bool testCount() {
      std::list<double> items;
      for(int i = 0; i < 10; i++)
        items.push_back(i);
      DomainContainer dom(new EnumeratedDomain(items, true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      
      AverHelper::s_domStack.push(&dom);
      AverHelper::count();
      
      DomainContainer *count = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      std::list<double> retnum;
      (*count)->getValues(retnum);
      assert(*(retnum.begin()) == items.size());
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());

      delete count;
      return true;
    }

    static bool testEntity() {
      std::list<double> items;
      for(int i = 1; i < 11; i++)
        items.push_back(i);
      DomainContainer dom(new EnumeratedDomain(items, true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
     
      DomainContainer index(new EnumeratedDomain(3, true, EnumeratedDomain::getDefaultTypeName().c_str()), true, false);
      
      AverHelper::s_domStack.push(&dom);
      AverHelper::s_domStack.push(&index);
      AverHelper::entity();

      DomainContainer* entityDom = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      
      std::list<double> entity;
      (*entityDom)->getValues(entity);

      assert(*(entity.begin()) == 4);
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());

      delete entityDom;
      return true;
    }

    static bool testQueryTokens() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;

      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));
      
      std::list<double> tokenIds;
      for(int i = 0; i < 10; i++)
        tokenIds.push_back((double)(new IntervalToken(assembly.m_db, 
                                                      LabelStr("Foo.bar"), true))->getId());
      
      AverHelper::queryTokens();
      
      DomainContainer* tokDom = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());

      std::list<double> ids;
      (*tokDom)->getValues(ids);

      tokenIds.sort();
      ids.sort();
      assert(tokenIds == ids);
      
      delete tokDom;
      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testQueryObjects() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;

      Schema::instance()->addObjectType(LabelStr("Foo"));

      std::list<double> objIds;
      for(int i = 0; i < 10; i++) {
        std::stringstream n;
        n << "Foo" << i;
        objIds.push_back((double)(new Object(assembly.m_db, LabelStr("Foo"),
                                             LabelStr(n.str())))->getId());
      }

      AverHelper::queryObjects();

      DomainContainer* objDom = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());

      std::list<double> objs;
      (*objDom)->getValues(objs);
      
      objs.sort();
      objIds.sort();
      assert(objs == objIds);

      delete objDom;
      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testGreatestLeast() {
      EnumeratedDomain d1(true, EnumeratedDomain::getDefaultTypeName().c_str());
      IntervalDomain d2(-3., 3.);

      d1.insert(-3.);
      d1.insert(0);
      d1.insert(1.25);
      d1.insert(1.251);
      d1.close();
      
      assert(AverHelper::getGreatest(d1) == 1.251);
      assert(AverHelper::getLeast(d1) == -3.);
      assert(AverHelper::getGreatest(d2) == 3.);
      assert(AverHelper::getLeast(d2) == -3.);
      return true;
    }
    
    static bool testTrimByPred() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));  
      Schema::instance()->addPredicate(LabelStr("Foo.baz"));

      DomainContainer d1(new SymbolDomain((double) LabelStr("Foo.bar")), true, false);
      DomainContainer d2(new SymbolDomain((double) LabelStr("Foo.baz")), true, false);

      std::list<double> t1;
      t1.push_back((double)(new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), 
                                              false))->getId());
      std::list<double> t2;
      t2.push_back((double)(new IntervalToken(assembly.m_db, LabelStr("Foo.baz"), 
                                              false))->getId());

      AverHelper::s_opStack.push("=");
      AverHelper::queryTokens();
      AverHelper::s_domStack.push(&d1);
      AverHelper::trimTokensByPredicate();

      DomainContainer *tokens1 = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      std::list<double> ret;
      (*tokens1)->getValues(ret);
      
      ret.sort();
      t1.sort();
      assert(ret == t1);
      delete tokens1;
      ret.clear();

      AverHelper::s_opStack.push("=");
      AverHelper::queryTokens();
      AverHelper::s_domStack.push(&d2);
      AverHelper::trimTokensByPredicate();
      
      tokens1 = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      (*tokens1)->getValues(ret);
      
      ret.sort();
      t2.sort();
      assert(ret == t2);
      delete tokens1;
      ret.clear();

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testTrimByName() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      
      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addObjectType(LabelStr("Bar"));

      DomainContainer d1(new SymbolDomain((double) LabelStr("foo")), true, false);
      DomainContainer d2(new SymbolDomain((double) LabelStr("bar")), true, false);

      std::list<double> o1;
      o1.push_back((double)(new Object(assembly.m_db, LabelStr("Foo"), 
                                       LabelStr("foo")))->getId());
      std::list<double> o2;
      o2.push_back((double)(new Object(assembly.m_db, LabelStr("Bar"),
                                       LabelStr("bar")))->getId());

      AverHelper::s_opStack.push("=");
      AverHelper::queryObjects();
      AverHelper::s_domStack.push(&d1);
      AverHelper::trimObjectsByName();
      
      DomainContainer* objs = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());

      std::list<double> ret;
      (*objs)->getValues(ret);

      ret.sort();
      o1.sort();
      assert(ret == o1);
      delete objs;
      ret.clear();

      AverHelper::s_opStack.push("=");
      AverHelper::queryObjects();
      AverHelper::s_domStack.push(&d2);
      AverHelper::trimObjectsByName();

      objs = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());

      (*objs)->getValues(ret);
      
      ret.sort();
      o2.sort();
      assert(ret == o2);
      delete objs;
      ret.clear();

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testTrimByStart() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));

      TokenId tok = (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true))->getId();
      tok->getStart()->specify(33.);

      for(int i = 0; i < 10; i++)
        new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true);

      AverHelper::queryTokens();
      
      std::list<double> temp;
      DomainContainer* tempD = AverHelper::s_domStack.top();
      (*tempD)->getValues(temp);
      assert(temp.size() > 1);
      temp.clear();

      AverHelper::s_opStack.push("=");
      AverHelper::s_domStack.push(new DomainContainer(new EnumeratedDomain(33., true, EnumeratedDomain::getDefaultTypeName().c_str()), true, true));

      AverHelper::trimTokensByStart();
      
      tempD = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      (*tempD)->getValues(temp);
      assert(temp.size() == 1);
      assert((double)tok == *(temp.begin())); 

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testTrimByEnd() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));

      TokenId tok = (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true))->getId();
      tok->getEnd()->specify(33.);

      for(int i = 0; i < 10; i++)
        new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true);

      AverHelper::queryTokens();
      
      std::list<double> temp;
      DomainContainer* tempD = AverHelper::s_domStack.top();
      (*tempD)->getValues(temp);
      assert(temp.size() > 1);
      temp.clear();

      AverHelper::s_opStack.push("=");
      AverHelper::s_domStack.
        push(new DomainContainer(new EnumeratedDomain(33., true, EnumeratedDomain::getDefaultTypeName().c_str()), true, true));

      AverHelper::trimTokensByEnd();
      
      tempD = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      (*tempD)->getValues(temp);
      assert(temp.size() == 1);
      assert((double)tok == *(temp.begin())); 

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }


    static bool testTrimByStatus() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));
      
      TokenId tok = (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true))->getId();
      tok->getState()->remove((double)Token::INCOMPLETE);
      tok->getState()->remove((double)Token::INACTIVE);
      tok->getState()->remove((double)Token::ACTIVE);
      tok->getState()->remove((double)Token::MERGED);
      

      for(int i = 0; i < 10; i++)
        (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true));
      
      AverHelper::queryTokens();
      
      std::list<double> temp;
      DomainContainer* tempD = AverHelper::s_domStack.top();
      (*tempD)->getValues(temp);
      assert(temp.size() > 1);
      temp.clear();
      
      AverHelper::s_opStack.push("=");
      AverHelper::s_domStack.push(new DomainContainer(new EnumeratedDomain((double)Token::REJECTED, 
                                                       false, "TokenStates"), true, true));

      AverHelper::trimTokensByStatus();
      
      tempD = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      (*tempD)->getValues(temp);
      assert(temp.size() == 1);
      assert((double)tok == *(temp.begin())); 

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testTrimTokensByVariable() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));
      Schema::instance()->addMember(LabelStr("Foo.bar"), 
                                    EnumeratedDomain::getDefaultTypeName().c_str(),
                                    LabelStr("baz"));

      TokenId tok = (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true,
                                       IntervalIntDomain(), IntervalIntDomain(),
                                       IntervalIntDomain(1, PLUS_INFINITY),
                                       Token::noObject(), false))->getId();
      std::list<double> baseVals;
      baseVals.push_back(33.); baseVals.push_back(0.); baseVals.push_back(100.);
      EnumeratedDomain base(baseVals, true, EnumeratedDomain::getDefaultTypeName().c_str());
      //base.close();
      tok->addParameter(base, LabelStr("baz"));
      tok->getVariable(LabelStr("baz"))->specify(33.);
      tok->close();

      for(int i = 0; i < 10; i++) {
        TokenId temp = (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true, 
                                          IntervalIntDomain(), IntervalIntDomain(),
                                          IntervalIntDomain(1, PLUS_INFINITY),
                                          Token::noObject(), false))->getId();
        temp->addParameter(base, LabelStr("baz"));
        temp->getVariable(LabelStr("baz"))->specify(100.);
        temp->close();
      }

      AverHelper::queryTokens();
      
      std::list<double> temp;
      DomainContainer* tempD = AverHelper::s_domStack.top();
      (*tempD)->getValues(temp);
      assert(temp.size() > 1);
      temp.clear();
      
      AverHelper::s_opStack.push("="); //domain op
      //domain
      AverHelper::s_domStack.
        push(new DomainContainer(new EnumeratedDomain(33., true, EnumeratedDomain::getDefaultTypeName().c_str()), true, true));
      AverHelper::s_opStack.push("="); //name op
      AverHelper::s_domStack.push(new DomainContainer(new SymbolDomain((double)LabelStr("baz")), true, true)); //name dom

      AverHelper::trimTokensByVariable();
      
      tempD = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      (*tempD)->getValues(temp);
      assert(temp.size() == 1);
      assert((double)tok == *(temp.begin())); 

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testTrimObjectsByVariable() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      std::string varType = IntervalDomain::getDefaultTypeName().c_str();

      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addMember(LabelStr("Foo"), LabelStr(varType), LabelStr("baz"));

      ObjectId obj = (new Object(assembly.m_db, LabelStr("Foo"), LabelStr("argle"), 
                                 true))->getId();
      IntervalDomain base(0., 100.);
      obj->addVariable(base, "baz");
      obj->getVariable(LabelStr("argle.baz"))->specify(33.);
      obj->close();

      for(int i = 0; i < 10; i++) {
        std::stringstream name;
        name << "argle" << i;
        ObjectId temp = (new Object(assembly.m_db, LabelStr("Foo"), LabelStr(name.str()), 
                                    true))->getId();
        temp->addVariable(base, "baz");
        temp->getVariable(LabelStr(name.str() + ".baz"))->specify(100.);
        temp->close();
      }

      AverHelper::queryObjects();
      
      std::list<double> temp;
      DomainContainer* tempD = AverHelper::s_domStack.top();
      (*tempD)->getValues(temp);
      assert(temp.size() > 1);
      temp.clear();
      
      AverHelper::s_opStack.push("="); //domain op
      AverHelper::s_domStack.push(new DomainContainer(new IntervalDomain(33., 33.), true, true)); //domain
      AverHelper::s_opStack.push("="); //name op
      AverHelper::s_domStack.push(new DomainContainer(new SymbolDomain((double)LabelStr("baz")), true, true)); //name dom

      AverHelper::trimObjectsByVariable();
      
      tempD = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      (*tempD)->getValues(temp);
      assert(temp.size() == 1);
      assert((double)obj == *(temp.begin())); 

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testTokenProperty() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addPredicate(LabelStr("Foo.bar"));
      Schema::instance()->addMember(LabelStr("Foo.bar"), 
                                    EnumeratedDomain::getDefaultTypeName().c_str(),
                                    LabelStr("baz"));

      TokenId tok = (new IntervalToken(assembly.m_db, LabelStr("Foo.bar"), true,
                                       IntervalIntDomain(), IntervalIntDomain(),
                                       IntervalIntDomain(1, PLUS_INFINITY),
                                       Token::noObject(), false))->getId();
      std::list<double> baseVals;
      baseVals.push_back(33.); baseVals.push_back(0.); baseVals.push_back(100.);
      EnumeratedDomain base(baseVals, true, EnumeratedDomain::getDefaultTypeName().c_str());
      tok->addParameter(base, LabelStr("baz"));
      tok->getVariable(LabelStr("baz"))->specify(33.);
      tok->close();
      
      AverHelper::queryTokens(); //for the moment, 'property' requires that there be only
                                 //one thing to get the property of
      
      AverHelper::s_domStack.push(new DomainContainer(new SymbolDomain((double)LabelStr("baz")), true, true));
      AverHelper::property();
      
      DomainContainer* dom = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());

      assert(*(*dom) == EnumeratedDomain(33., true, 
                                      EnumeratedDomain::getDefaultTypeName().c_str()));

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

    static bool testObjectProperty() {
      TestAssembly assembly;
      AverHelper::s_db = assembly.m_db;
      std::string varType = IntervalDomain::getDefaultTypeName().c_str();

      Schema::instance()->addObjectType(LabelStr("Foo"));
      Schema::instance()->addMember(LabelStr("Foo"), LabelStr(varType), LabelStr("baz"));

      ObjectId obj = (new Object(assembly.m_db, LabelStr("Foo"), LabelStr("argle"), 
                                 true))->getId();
      IntervalDomain base(0., 100.);
      obj->addVariable(base, "baz");
      obj->getVariable(LabelStr("argle.baz"))->specify(33.);
      obj->close();

      AverHelper::queryObjects(); //for the moment, 'property' requires that there be only
                                  //one thing to get the property of
      AverHelper::s_domStack.push(new DomainContainer(new SymbolDomain((double)LabelStr("baz")), true, true));
      AverHelper::property();

      DomainContainer* dom = AverHelper::s_domStack.top();
      AverHelper::s_domStack.pop();
      assert(AverHelper::s_domStack.empty());
      assert(AverHelper::s_opStack.empty());
      
      assert(*(*dom) == IntervalDomain(33., 33.));

      AverHelper::s_db = PlanDatabaseId::noId();
      return true;
    }

  };

}

int main(int argc, const char** argv) {
  Schema::instance();
  runTestSuite(EUROPA::FooTest::test);
  runTestSuite(EUROPA::AverHelperTest::test);
  std::cout << "Finished" << std::endl;
  return 0;
}

