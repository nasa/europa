#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "ObjectSet.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "RulesEngine.hh"
#include "Rule.hh"
#include "../ConstraintEngine/TestSupport.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "../ConstraintEngine/IntervalRealDomain.hh"
#include "../ConstraintEngine/LabelSet.hh"
#include "../ConstraintEngine/DefaultPropagator.hh"
#include "../ConstraintEngine/EqualityConstraintPropagator.hh"

#include <iostream>

#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce;\
    Schema schema;\
    PlanDatabase db(ce.getId(), schema.getId());\
    new DefaultPropagator(LabelStr("Default"), ce.getId());\
    RulesEngine re(db.getId()); \
    new EqualityConstraintPropagator(LabelStr("EquivalenceClass"), ce.getId());\
    Object object(db.getId(), LabelStr("AllObjects"), LabelStr("o1"));\
    if(autoClose) db.close();

class TestRule: public Rule {
public:

  TestRule(const RulesEngineId& rulesEngine, const LabelStr& name): Rule(rulesEngine, name){}

  /**
   * Initialize the context with some variables form the token and add a local variable for the rule too. This
   * will test cleanup.
   */
  void initializeContext(const TokenId& token, std::vector<ConstrainedVariableId>& scope) const{
    check_error(scope.empty());
    scope.push_back(token->getObject());
    scope.push_back(token->getRejectability());
    ConstrainedVariableId localVariable = 
      (new Variable<IntervalIntDomain>(getRulesEngine()->getPlanDatabase()->getConstraintEngine(), IntervalIntDomain(1, 1)))->getId();
    scope.push_back(localVariable);
  }

  bool handleSet(const RuleContextId& context, int index, const ConstrainedVariableId& var) const{
    std::vector<TokenId> newTokens;
    std::vector<ConstraintId> newConstraints;

    // Allocate a new slave Token
    TokenId slave = (new IntervalToken(context->getToken(), 
				       LabelStr("Predicate"), 
				       BooleanDomain(false)))->getId();
    newTokens.push_back(slave);

    // Allocate a new constraint equating the start variable of the new token with the end variable of
    // the existing token
    {
      std::vector<ConstrainedVariableId> constrainedVars;
      constrainedVars.push_back(context->getToken()->getEnd());
      constrainedVars.push_back(slave->getStart());
      ConstraintId meets = ConstraintLibrary::createConstraint(LabelStr("Equal"),
							       getRulesEngine()->getPlanDatabase()->getConstraintEngine(),
							       constrainedVars);
      newConstraints.push_back(meets);
    }

    // Allocate a constraint restricting the duration of the slave using
    {
      std::vector<ConstrainedVariableId> constrainedVars;
      constrainedVars.push_back(slave->getDuration());
      constrainedVars.push_back(context->getVariables().back());
      ConstraintId restrictDuration = ConstraintLibrary::createConstraint(LabelStr("Equal"),
									  getRulesEngine()->getPlanDatabase()->getConstraintEngine(),
									  constrainedVars);
      newConstraints.push_back(restrictDuration);
    }
    context->execute(newTokens, newConstraints);
    return true;
  }

  bool handleReset(const RuleContextId& context, int index, const ConstrainedVariableId& var) const{
    context->undo();
    return true;
  }
};

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
    runTest(testMergingPerformance, "MergingPerformance");
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

    // Activate & deactivate - ensure proper handling of rejectability variable
    check_error(!t0.getRejectability()->getDerivedDomain().isSingleton());
    t0.activate();
    check_error(t0.getRejectability()->getDerivedDomain().isSingleton());
    check_error(t0.getRejectability()->getDerivedDomain().getSingletonValue() == false);
    t0.deactivate();
    check_error(!t0.getRejectability()->getDerivedDomain().isSingleton());

    // Now activate and merge
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
    //TBW: when stacking instead of merging tokens, the next check is not true
    // check_error(!equalityConstraint->isActive());
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

  // add backtracking and longer chain, also add a before constraint
  static bool testMergingPerformance(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    typedef Europa::Id<IntervalToken> IntervalTokenId;
    
    static const int UNIFIED=5;
    static const int NUMTOKS=100;
    static const int NUMPARAMS=2;

    //Create tokens with the same domains.  We will impose a constraint on
    //each token variable.  Tokens will have 5 parameter variables.
    std::vector< std::vector<IntervalTokenId> > tokens;

    for (int i=0; i < NUMTOKS; i++) {
      std::vector<IntervalTokenId> tmp;
      for (int j=0; j < UNIFIED; j++) {
	IntervalTokenId t = (new IntervalToken(db.getId(), 
					       LabelStr("P1"), 
					       BooleanDomain(),
					       IntervalIntDomain(0, 210),
					       IntervalIntDomain(0, 220),
					       IntervalIntDomain(1, 110),
					       Token::noObject(), false))->getId();
	for (int k=0; k < NUMPARAMS; k++)
	  t->addParameter(IntervalIntDomain(500+j,1000));
	t->close();
	tmp.push_back(t);
      }
      tokens.push_back(tmp);
    }

    IntervalIntDomain sdom1(tokens[0][0]->getStart()->getDerivedDomain());
    check_error(sdom1.getLowerBound() == 0);
    check_error(sdom1.getUpperBound() == 210);

    IntervalIntDomain edom1(tokens[0][0]->getEnd()->getDerivedDomain());
    check_error(edom1.getLowerBound() == 1);
    check_error(edom1.getUpperBound() == 220);

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar1(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom1(pvar1->getDerivedDomain());
    check_error(pdom1.getLowerBound() == 500);
    check_error(pdom1.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++) {
      tokens[i][0]->activate();
      timeline.constrain(tokens[i][0]);
    }

    IntervalIntDomain sdom2(tokens[0][0]->getStart()->getDerivedDomain());
    check_error(sdom2.getLowerBound() == 0);
    check_error(sdom2.getUpperBound() == 209);

    IntervalIntDomain edom2(tokens[0][0]->getEnd()->getDerivedDomain());
    check_error(edom2.getLowerBound() == 1);
    check_error(edom2.getUpperBound() == 210);

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar2(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom2(pvar2->getDerivedDomain());
    check_error(pdom2.getLowerBound() == 500);
    check_error(pdom2.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) { 
	tokens[i][j]->merge(tokens[i][0]);
	ce.propagate();
      }

    IntervalIntDomain sdom3(tokens[0][0]->getStart()->getDerivedDomain());
    check_error(sdom3.getLowerBound() == 0);
    check_error(sdom3.getUpperBound() == 209);

    IntervalIntDomain edom3(tokens[0][0]->getEnd()->getDerivedDomain());
    check_error(edom3.getLowerBound() == 1);
    check_error(edom3.getUpperBound() == 210);

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar3(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom3(pvar3->getDerivedDomain());
    check_error(pdom3.getLowerBound() == 500+UNIFIED-1);
    check_error(pdom3.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) {
	tokens[i][j]->split();
	ce.propagate();
      }

    IntervalIntDomain sdom4(tokens[0][0]->getStart()->getDerivedDomain());
    check_error(sdom4.getLowerBound() == sdom1.getLowerBound());
    check_error(sdom4.getUpperBound() == sdom1.getUpperBound());

    IntervalIntDomain edom4(tokens[0][0]->getEnd()->getDerivedDomain());
    check_error(edom4.getLowerBound() == edom1.getLowerBound());
    check_error(edom4.getUpperBound() == edom1.getUpperBound());

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar4(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom4(pvar4->getDerivedDomain());
    check_error(pdom4.getLowerBound() == pdom1.getLowerBound());
    check_error(pdom4.getUpperBound() == pdom1.getUpperBound());

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

class RulesEngineTest {
public:
  static bool test(){
    runTest(testBasicAllocation, "BasicAllocation");
    runTest(testActivation, "Activation");
    runTest(testRuleFiringAndCleanup, "RuleFiringAndCleanup");
    return true;
  }
private:
  static bool testBasicAllocation(){
    DEFAULT_SETUP(ce, db, schema, false);
    new TestRule(re.getId(), LabelStr("AnyType::AnyPredicate"));
    return true;
  }

  static bool testActivation(){
    DEFAULT_SETUP(ce, db, schema, false);
    db.close();
    new TestRule(re.getId(), LabelStr("Type::Predicate"));

    IntervalToken tokenA(db.getId(), 
		     LabelStr("Type::Predicate"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    check_error(re.getRules().size() == 1);
    check_error(re.getRuleInstances().empty());

    int num_constraints = ce.getConstraints().size();
    // Activate and confirm the rule instance is created
    tokenA.activate();
    check_error(re.getRuleInstances().size() == 1);
    // New constraints added to restrict rejectability and to listen to rule variables
    check_error(ce.getConstraints().size() == num_constraints + 2);

    // Deactivate to ensure the rule instance is removed
    tokenA.deactivate();
    check_error(re.getRuleInstances().empty());
    check_error(ce.getConstraints().size() == num_constraints);

    // Activate again to test deletion through automatic cleanup.
    tokenA.activate();
    check_error(re.getRuleInstances().size() == 1);
    return true;
  }

  static bool testRuleFiringAndCleanup(){
    DEFAULT_SETUP(ce, db, schema, false);
    db.close();
    new TestRule(re.getId(), LabelStr("Type::Predicate"));

    IntervalToken tokenA(db.getId(), 
		     LabelStr("Type::Predicate"), 
		     BooleanDomain(),
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    check_error(ce.propagate());
    check_error(db.getTokens().size() == 1);

    tokenA.activate();
    check_error(ce.propagate());
    check_error(db.getTokens().size() == 1);

    tokenA.getRejectability()->specify(false);
    check_error(ce.propagate());
    check_error(db.getTokens().size() == 2);
    check_error(tokenA.getSlaves().size() == 1);
    TokenId slave = *(tokenA.getSlaves().begin());
    check_error(slave->getDuration()->getDerivedDomain().isSingleton()); // Due to constraint on local variable

    // Test reset which should backtrack the rule
    tokenA.getRejectability()->reset();
    check_error(ce.propagate());
    check_error(db.getTokens().size() == 1);

    // Set again, and deactivate
    tokenA.getRejectability()->specify(false);
    check_error(ce.propagate());
    check_error(db.getTokens().size() == 2);
    tokenA.deactivate();
    check_error(db.getTokens().size() == 1);

    // Now repeast to ensure correct automatic cleanup
    tokenA.activate();
    check_error(ce.propagate());
    check_error(db.getTokens().size() == 2); // Rule should fire since specified domain already set!
    return true;
  }
};

int main(){
  initConstraintLibrary();
  
  REGISTER_NARY(EqualConstraint, "CoTemporal", "Default");
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "Before", "Default");
  REGISTER_UNARY(ObjectTokenRelation, "ObjectRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_NARY(EqualConstraint, "EqualConstraint", "EquivalenceClass");
  
  runTestSuite(ObjectTest::test, "Object Tests");
  runTestSuite(TokenTest::test, "Token Tests");
  runTestSuite(TimelineTest::test, "Timeline Tests");
  runTestSuite(RulesEngineTest::test, "RulesEngine Tests");
  std::cout << "Finished" << std::endl;
}
