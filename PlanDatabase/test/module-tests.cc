#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "ObjectSet.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "./ConstraintEngine/TestSupport.hh"
#include "./ConstraintEngine/IntervalIntDomain.hh"

#include <iostream>


class ObjectTest {
public:
  static bool test(){
    runTest(testBasicAllocation, "BasicAllocation");
    runTest(testObjectSet, "ObjetSet");
    return true;
  }
private:
  static bool testBasicAllocation(){
    PlanDatabase db(ENGINE);
    Object o1(db.getId(), LabelStr("AllObjects"), LabelStr("o1"));
    Object o2(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    ObjectId id0((new Object(o1.getId(), LabelStr("AllObjects"), LabelStr("id0")))->getId());
    Object o3(o2.getId(), LabelStr("AllObjects"), LabelStr("o3"));
    assert(db.getObjects().size() == 4);
    assert(o1.getComponents().size() == 1);
    assert(o3.getParent() == o2.getId());
    delete (Object*) id0;
    assert(db.getObjects().size() == 3);
    assert(o1.getComponents().empty());

    ObjectId id1((new Object(db.getId(), LabelStr("AllObjects"), LabelStr("id1")))->getId());
    ObjectId id2((new Object(id1, LabelStr("AllObjects"), LabelStr("id2")))->getId());
    ObjectId id3((new Object(id1, LabelStr("AllObjects"), LabelStr("id3")))->getId());
    assert(db.getObjects().size() == 6);
    assert(id3->getName().toString() == "id1:id3");
    delete (Object*) id1;
    assert(db.getObjects().size() == 3);
    return true;
  }

  static bool testObjectSet(){
    PlanDatabase db(ENGINE);
    std::list<ObjectId> values;
    Object o1(db.getId(), LabelStr("AllObjects"), LabelStr("o1"));
    Object o2(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    assert(db.getObjects().size() == 2);
    values.push_back(o1.getId());
    values.push_back(o2.getId());
    ObjectSet os1(values, true);
    assert(os1.isMember(o1.getId()));
    os1.remove(o1.getId());
    assert(!os1.isMember(o1.getId()));
    assert(os1.isSingleton());
    return true;
  }
};

class TokenTest {
public:
  static bool test(){
    runTest(testBasicTokenAllocation, "BasicTokenAllocation");
    return true;
  }

private:
  static bool testBasicTokenAllocation(){
    Schema schema;
    PlanDatabase db(ENGINE, schema.getId());
    Object staticObject(db.getId(), LabelStr("AllObjects"), LabelStr("o1"));
    std::vector<ConstrainedVariableId> noParameters;
    Token staticToken(db.getId(), LabelStr("Predicate"), noParameters, LabelStr("o1"));
    return true;
  }
};

void main(){
  initConstraintLibrary();
  runTestSuite(ObjectTest::test, "Token Tests");
  runTestSuite(TokenTest::test, "Token Tests");
  cout << "Finished" << endl;
}
