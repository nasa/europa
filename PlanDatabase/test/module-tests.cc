#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "ObjectSet.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "./ConstraintEngine/TestSupport.hh"
#include "./ConstraintEngine/IntervalIntDomain.hh"
#include "./ConstraintEngine/IntervalRealDomain.hh"
#include "./ConstraintEngine/LabelSet.hh"
#include "./ConstraintEngine/DefaultPropagator.hh"

#include <iostream>


#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce;\
    Schema schema;\
    PlanDatabase db(ce.getId(), schema.getId());\
    new DefaultPropagator(LabelStr("Default"), ce.getId());\
    Object object(db.getId(), LabelStr("AllObjects"), LabelStr("o1"));\
    if(autoClose) db.close();

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
    runTest(testMasterSlaveRelationship, "MasterSlaveRelationship");
    runTest(testBasicMerging, "BasicMerging");
    return true;
  }

private:
  static bool testBasicTokenAllocation(){
    DEFAULT_SETUP(ce, db, schema, true);

    // Event Token
    EventToken eventToken(db.getId(), LabelStr("Predicate"), BooleanDomain(), IntervalIntDomain(0, 1000), Token::noObject(), false);
    assert(eventToken.getStart()->getDerivedDomain() == eventToken.getEnd()->getDerivedDomain());
    assert(eventToken.getDuration()->getDerivedDomain() == IntervalIntDomain(0, 0));
    eventToken.getStart()->specify(IntervalIntDomain(5, 10));
    assert(eventToken.getEnd()->getDerivedDomain() == IntervalIntDomain(5, 10));
    eventToken.addParameter(IntervalRealDomain(-1.08, 20.18));
    eventToken.close();

    // IntervalToken
    IntervalToken intervalToken(db.getId(), 
				LabelStr("Predicate"), 
				BooleanDomain(), 
				IntervalIntDomain(0, 1000),
				IntervalIntDomain(0, 1000),
				IntervalIntDomain(2, 10),
				Token::noObject(), false);

    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("L1"));
    values.push_back(Prototype::LabelStr("L4"));
    values.push_back(Prototype::LabelStr("L2"));
    values.push_back(Prototype::LabelStr("L5"));
    values.push_back(Prototype::LabelStr("L3"));
    intervalToken.addParameter(LabelSet(values, true));
    intervalToken.close();
    assert(intervalToken.getEnd()->getDerivedDomain().getLowerBound() == 2);
    intervalToken.getStart()->specify(IntervalIntDomain(5, 10));
    assert(intervalToken.getEnd()->getDerivedDomain() == IntervalIntDomain(7, 20));
    intervalToken.getEnd()->specify(IntervalIntDomain(9, 10));
    assert(intervalToken.getStart()->getDerivedDomain() == IntervalIntDomain(5, 8));
    assert(intervalToken.getDuration()->getDerivedDomain() == IntervalIntDomain(2, 5));

    return true;
  }

  static bool testMasterSlaveRelationship(){
    DEFAULT_SETUP(ce, db, schema, true);

    IntervalToken t0(db.getId(), 
		     LabelStr("Predicate"), 
		     BooleanDomain(false), 
		     IntervalIntDomain(0, 1),
		     IntervalIntDomain(0, 1),
		     IntervalIntDomain(1, 1));
    t0.activate();

    TokenId t1 = (new IntervalToken(db.getId(), 
				    LabelStr("Predicate"), 
				    BooleanDomain(false),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();
    t1->activate();

    TokenId t2 = (new IntervalToken(t0.getId(), 
				    LabelStr("Predicate"), 
				    BooleanDomain(false),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t3 = (new IntervalToken(t0.getId(), 
				    LabelStr("Predicate"), 
				    BooleanDomain(false), 
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t4 = (new IntervalToken(t0.getId(), 
				    LabelStr("Predicate"), 
				    BooleanDomain(false), 
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t5 = (new IntervalToken(t1, 
				    LabelStr("Predicate"), 
				    BooleanDomain(false), 
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t6 = (new EventToken(t0.getId(), 
				    LabelStr("Predicate"), 
				    BooleanDomain(false),
				    IntervalIntDomain(0, 1)))->getId();

    // Delete slave only
    delete (Token*) t2;

    // Delete master & slaves
    delete (Token*) t1;

    // Remainder should be cleaned up automatically
    return true;
  }

  static bool testBasicMerging(){
    DEFAULT_SETUP(ce, db, schema, true);

    // Create 2 mergeable tokens - predicates, types and base domaiuns match
    IntervalToken t0(db.getId(), 
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    check_error(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);

    IntervalToken t1(db.getId(),
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    t1.getDuration()->specify(IntervalIntDomain(5, 7));

    // Activate one and merge the other with it.
    t0.activate();
    t1.merge(t0.getId());

    // Make sure the necessary restrictions have been imposed due to merging i.e. restruction due to specified domain
    check_error(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    check_error(t1.isMerged());

    // Do a split and make sure the old values are reinstated.
    t1.split();
    check_error(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
    check_error(t1.isInactive());

    // Now post equality constraint between t1 and extra token t2 and remerge
    IntervalToken t2(db.getId(), 
		     LabelStr("P2"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    t2.getEnd()->specify(IntervalIntDomain(8, 10));

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(t1.getEnd());
    temp.push_back(t2.getEnd());
    ConstraintId equalityConstraint = ConstraintLibrary::createConstraint(LabelStr("CoTemporal"),
									  db.getConstraintEngine(),
									  temp);
    t1.merge(t0.getId());

    // Verify that the equality constraint has migrated and original has been deactivated.
    check_error(!equalityConstraint->isActive());
    check_error(t0.getEnd()->getDerivedDomain().getLowerBound() == 8);
    check_error(t0.getEnd()->getDerivedDomain() == t2.getEnd()->getDerivedDomain());

    // Undo the merge and check for initial conditions being established
    t1.split();
    check_error(equalityConstraint->isActive());

    // Redo the merge
    t1.merge(t0.getId());

    // Confirm deletion of the constraint is handled correctly
    delete (Constraint*) equalityConstraint;
    check_error(t0.getEnd()->getDerivedDomain() != t2.getEnd()->getDerivedDomain());

    // Confirm previous restriction due to specified domain, then reset and note the change
    check_error(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    t1.getDuration()->reset();
    check_error(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);


    // Test unary
    t1.split();
    ConstraintId subsetOfConstraint = ConstraintLibrary::createConstraint(LabelStr("SubsetOf"),
									  db.getConstraintEngine(),
									  t1.getDuration(),
									  IntervalIntDomain(5, 6));
    t1.merge(t0.getId());
    check_error(t0.getDuration()->getDerivedDomain().getUpperBound() == 6);
    delete (Constraint*) subsetOfConstraint;

    // Deletion will now occur and test proper cleanup.
    return true;
  }
};

class TimelineTest {
public:
  static bool test(){
    runTest(testBasicInsertion, "BasicInsertion");
    runTest(testObjectTokenRelation, "ObjectTokenRelation");
    runTest(testTokenOrderQuery, "TokenOrderQuery");
    return true;
  }

private:
  static bool testBasicInsertion(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    IntervalToken tokenA(db.getId(), 
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db.getId(), 
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    timeline.constrain(tokenA.getId());
    timeline.constrain(tokenB.getId());
    timeline.constrain(tokenC.getId(), tokenA.getId());

    check_error(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());
    return true;
  }

  static bool testObjectTokenRelation(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    IntervalToken tokenA(db.getId(), 
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db.getId(), 
		     LabelStr("P1"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    // Object variables are not singletons - so query for tokens to order should return nothing
    std::vector<TokenId> tokensToOrder;
    timeline.getTokensToOrder(tokensToOrder);
    check_error(tokensToOrder.empty());

    // Specify the object variable of one - but still should return no tokens since they are all inactive
    tokenA.getObject()->specify(timeline.getId());
    timeline.getTokensToOrder(tokensToOrder);
    check_error(tokensToOrder.empty());

    // Now activate all of them - should only get back the one that was specified to a singleton
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    timeline.getTokensToOrder(tokensToOrder);
    check_error(tokensToOrder.size() == 1 && tokensToOrder.front() == tokenA.getId());

    // Set remainders so they are singeltons and get all back
    tokenB.getObject()->specify(timeline.getId());
    tokenC.getObject()->specify(timeline.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    check_error(tokensToOrder.size() == 3);

    // Now incrementally constrain and show reduction in tokens to order
    timeline.constrain(tokenA.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    check_error(tokensToOrder.size() == 2);

    timeline.constrain(tokenB.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    check_error(tokensToOrder.size() == 1);

    timeline.constrain(tokenC.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    check_error(tokensToOrder.empty());

    return true;
  }

  static bool testTokenOrderQuery(){
    DEFAULT_SETUP(ce, db, schema, false);
    Europa::Id<Timeline> timeline = (new Timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2")))->getId();
    db.close();

    const int COUNT = 5;
    const int DURATION = 10;
    
    for (int i=0;i<COUNT;i++){
      int start = i*DURATION;
      TokenId token = (new IntervalToken(db.getId(), 
					 LabelStr("P1"),
					 BooleanDomain(),
					 IntervalIntDomain(start, start),
					 IntervalIntDomain(start+DURATION, start+DURATION),
					 IntervalIntDomain(DURATION, DURATION)))->getId();
      check_error(!token->getObject()->getBaseDomain().isSingleton());
      token->getObject()->specify(timeline->getId());
      token->activate();
    }

    check_error(timeline->getTokens().size() == 0);
    ce.propagate();
    check_error(timeline->getTokens().size() == COUNT);

    int i = 0;
    std::vector<TokenId> tokensToOrder;
    timeline->getTokensToOrder(tokensToOrder);

    while(!tokensToOrder.empty()){
      check_error(timeline->getTokenSequence().size() == i);
      check_error(tokensToOrder.size() == (COUNT - i));
      std::vector<TokenId> choices;
      TokenId toConstrain = tokensToOrder.front();
      timeline->getOrderingChoices(toConstrain, choices);
      check_error(!choices.empty());
      TokenId successor = choices.front();
      timeline->constrain(toConstrain, successor);
      check_error(ce.propagate());
      tokensToOrder.clear();
      timeline->getTokensToOrder(tokensToOrder);
      i++;
      check_error(ce.propagate());
    }

    const std::list<TokenId>& tokenSequence = timeline->getTokenSequence();
    check_error(tokenSequence.front()->getStart()->getDerivedDomain().getSingletonValue() == 0);
    check_error(tokenSequence.back()->getEnd()->getDerivedDomain().getSingletonValue() == COUNT*DURATION);

    // Now ensure the query can correctly indicate no options available
    TokenId token = (new IntervalToken(db.getId(), 
				       LabelStr("P1"),
				       BooleanDomain(),
				       IntervalIntDomain(0, 0),
				       IntervalIntDomain(),
				       IntervalIntDomain(DURATION, DURATION)))->getId();
    token->getObject()->specify(timeline->getId());
    token->activate();
    std::vector<TokenId> choices;
    timeline->getOrderingChoices(token, choices);
    check_error(choices.empty());

    return true;
  }
};

void main(){
  initConstraintLibrary();
  
  REGISTER_NARY(EqualConstraint, "CoTemporal", "Default");
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "Before", "Default");
  REGISTER_UNARY(ObjectTokenRelation, "ObjectRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");

  runTestSuite(ObjectTest::test, "Object Tests");
  runTestSuite(TokenTest::test, "Token Tests");
  runTestSuite(TimelineTest::test, "Timeline Tests");
  cout << "Finished" << endl;
}
