#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "RulesEngine.hh"
#include "Rule.hh"
#include "RuleContext.hh"
#include "ObjectFilter.hh"
#include "DbLogger.hh"
#include "TestRule.hh"

#include "../ConstraintEngine/TestSupport.hh"
#include "../ConstraintEngine/Utils.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "../ConstraintEngine/Domain.hh"
#include "../ConstraintEngine/DefaultPropagator.hh"
#include "../ConstraintEngine/EqualityConstraintPropagator.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <cassert>


class DefaultSchemaAccessor{
public:
  static const SchemaId& instance(){
    if (s_instance.isNoId()){
      s_instance = (new Schema())->getId();
    }

    return s_instance;
  }

  static void reset(){
    if(!s_instance.isNoId()){
      delete (Schema*) s_instance;
      s_instance = SchemaId::noId();
    }
  }

private:
  static SchemaId s_instance;
};

SchemaId DefaultSchemaAccessor::s_instance;

#define SCHEMA DefaultSchemaAccessor::instance()

#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce;\
    Schema schema;\
    PlanDatabase db(ce.getId(), schema.getId());\
    new DefaultPropagator(LabelStr("Default"), ce.getId());\
    if(loggingEnabled()){\
     new CeLogger(std::cout, ce.getId());\
     new DbLogger(std::cout, db.getId());\
    }\
    RulesEngine re(db.getId()); \
    new EqualityConstraintPropagator(LabelStr("EquivalenceClass"), ce.getId());\
    Object object(db.getId(), LabelStr("AllObjects"), LabelStr("o1"));\
    if(autoClose) db.close();

class ObjectTest {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testObjectSet);
    runTest(testObjectVariables);
    runTest(testObjectFilteringConstraint);
    runTest(testFilterAndConstrain);
    return true;
  }
private:
  static bool testBasicAllocation(){
    PlanDatabase db(ENGINE, SCHEMA);
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
    PlanDatabase db(ENGINE, SCHEMA);
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

  static bool testObjectVariables(){
    PlanDatabase db(ENGINE, SCHEMA);
    Object o1(db.getId(), LabelStr("AllObjects"), LabelStr("o1"), true);
    assert(!o1.isComplete());
    o1.addVariable(IntervalIntDomain());
    o1.addVariable(BoolDomain());
    o1.close();
    assert(o1.isComplete());

    Object o2(db.getId(), LabelStr("AllObjects"), LabelStr("o2"), true);
    assert(!o2.isComplete());
    o2.addVariable(IntervalIntDomain(15, 200));
    o2.close();

    // Add a unary constraint
    ConstraintLibrary::createConstraint(LabelStr("SubsetOf"), 
					db.getConstraintEngine(), 
					o1.getVariables()[0],
					IntervalIntDomain(10, 20));

    // Now add a constraint equating the variables and test propagation
    std::vector<ConstrainedVariableId> constrainedVars;
    constrainedVars.push_back(o1.getVariables()[0]);
    constrainedVars.push_back(o2.getVariables()[0]);
    ConstraintId constraint = ConstraintLibrary::createConstraint(LabelStr("Equal"),
								  db.getConstraintEngine(),
								  constrainedVars);

    assert(db.getConstraintEngine()->propagate());
    assert(o1.getVariables()[0]->lastDomain() == o1.getVariables()[0]->lastDomain());

    // Delete one of the constraints to force automatic clean-up path and explciit clean-up
    delete (Constraint*) constraint;

    return true;
  }

  static bool testObjectFilteringConstraint(){
    PlanDatabase db(ENGINE, SCHEMA);

    static const int NUM_OBJECTS = 10;

    std::list<ObjectId> objects;

    // Allocate objects with 2 field variables
    for (int i=0;i<NUM_OBJECTS;i++){
      std::stringstream ss;
      ss << "Object" << i;
      std::string objectName(ss.str());
      ObjectId object = (new Object(db.getId(), LabelStr("AllObjects"), LabelStr(objectName.c_str()), true))->getId();
      object->addVariable(IntervalIntDomain(i, i));
      object->addVariable(BoolDomain());
      object->close();
      objects.push_back(object);
    }

    // Set up the object variable
    Variable<ObjectSet> objectVar(db.getConstraintEngine(), objects);

    // Set up the filter variable
    EnumeratedDomain filterBaseDomain = ObjectFilter::constructUnion(objects, 0);
    Variable<EnumeratedDomain> filterVar(db.getConstraintEngine(), filterBaseDomain);

    // Construct the filter constraint
    ObjectFilter filter(LabelStr("Default"), db.getConstraintEngine(), objectVar.getId(), 0, filterVar.getId());

    assert(db.getConstraintEngine()->propagate());

    // Iterate and restrict the filter by 1 each time. It should restrict the object variable by 1 also
    EnumeratedDomain workingDomain(filterBaseDomain);
    assert(filterBaseDomain.getSize() == NUM_OBJECTS);
    assert(workingDomain.getSize() == NUM_OBJECTS);
    assert(objectVar.getDerivedDomain().getSize() == NUM_OBJECTS);

    for (int i=1; i< NUM_OBJECTS; i++){
      workingDomain.remove(i);
      filterVar.specify(workingDomain);
      int resultingSize = objectVar.getDerivedDomain().getSize();
      assert(resultingSize == (filterBaseDomain.getSize() - i));
    }

    filterVar.reset();
    assert(objectVar.getDerivedDomain().getSize() == filterBaseDomain.getSize());

    objectVar.specify(objects.front());
    assert(filterVar.getDerivedDomain().isSingleton());
    return true;
  }

  /**
   * This case will emulate the following in nddl
   * class Fact {
   *  int p1;
   *  int p2;
   * }

   */
  static bool testFilterAndConstrain(){
    PlanDatabase db(ENGINE, SCHEMA);

    static const int NUM_OBJECTS = 10;

    std::list<ObjectId> objects;

    // Allocate objects with 2 field variables
    for (int i=0;i<NUM_OBJECTS;i++){
      std::stringstream ss;
      ss << "Object" << i;
      std::string objectName(ss.str());
      ObjectId object = (new Object(db.getId(), LabelStr("AllObjects"), LabelStr(objectName.c_str()), true))->getId();
      object->addVariable(IntervalIntDomain(i, i)); // p1
      object->addVariable(IntervalIntDomain(NUM_OBJECTS - i, NUM_OBJECTS - i)); // p2
      object->close();
      objects.push_back(object);
    }

    // Prepare variables
    Variable<ObjectSet> objectVar(db.getConstraintEngine(), objects);

    EnumeratedDomain filterBaseDomain0 = ObjectFilter::constructUnion(objects, 0);
    Variable<EnumeratedDomain> filterVar0(db.getConstraintEngine(), filterBaseDomain0);
    ObjectFilter filter0(LabelStr("Default"), db.getConstraintEngine(), objectVar.getId(), 0, filterVar0.getId());

    EnumeratedDomain filterBaseDomain1 = ObjectFilter::constructUnion(objects, 1);
    Variable<EnumeratedDomain> filterVar1(db.getConstraintEngine(), filterBaseDomain1);
    ObjectFilter filter1(LabelStr("Default"), db.getConstraintEngine(), objectVar.getId(), 1, filterVar1.getId());

    // OK - now create 2 variables to constrain against, using the equals constraint
    Variable<EnumeratedDomain> v0(db.getConstraintEngine(), filterBaseDomain0);
    EqualConstraint c0(LabelStr("eq"), LabelStr("Default"), db.getConstraintEngine(), makeScope(v0.getId(), filterVar0.getId()));

    Variable<EnumeratedDomain> v1(db.getConstraintEngine(), filterBaseDomain1);
    EqualConstraint c1(LabelStr("eq"), LabelStr("Default"), db.getConstraintEngine(), makeScope(v1.getId(), filterVar1.getId()));

    assert(objectVar.getDerivedDomain().getSize() == NUM_OBJECTS);
    assert(v0.getDerivedDomain() == filterBaseDomain0);
    assert(v1.getDerivedDomain() == filterBaseDomain1);

    // Specify one variable to a singleton
    v0.specify(3);
    assert(v1.getDerivedDomain().isSingleton());
    assert(v1.getDerivedDomain().getSingletonValue() == (NUM_OBJECTS - 3 ));

    return true;
  }
};

class TokenTest {
public:
  static bool test(){
    runTest(testBasicTokenAllocation);
    runTest(testStateModel);
    runTest(testMasterSlaveRelationship);
    runTest(testBasicMerging);
    runTest(testMergingPerformance);
    runTest(testTokenCompatibility);
    return true;
  }

private:
  static bool testBasicTokenAllocation(){
    DEFAULT_SETUP(ce, db, schema, true);

    // Event Token
    EventToken eventToken(db.getId(), LabelStr("Predicate"), true, IntervalIntDomain(0, 1000), Token::noObject(), false);
    assert(eventToken.getStart()->getDerivedDomain() == eventToken.getEnd()->getDerivedDomain());
    assert(eventToken.getDuration()->getDerivedDomain() == IntervalIntDomain(0, 0));
    eventToken.getStart()->specify(IntervalIntDomain(5, 10));
    assert(eventToken.getEnd()->getDerivedDomain() == IntervalIntDomain(5, 10));
    eventToken.addParameter(IntervalDomain(-1.08, 20.18), LabelStr("TestParam"));
    eventToken.close();

    // IntervalToken
    IntervalToken intervalToken(db.getId(), 
				LabelStr("Predicate"), 
				true, 
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

    // Create and delete a Token
    TokenId token = (new IntervalToken(db.getId(), 
				       LabelStr("Predicate"), 
				       true, 
				       IntervalIntDomain(0, 1000),
				       IntervalIntDomain(0, 1000),
				       IntervalIntDomain(2, 10),
				       Token::noObject(), true))->getId();

    delete (Token*) token; // It is inComplete
    return true;
  }

  static bool testStateModel(){
    DEFAULT_SETUP(ce, db, schema, true);

    IntervalToken t0(db.getId(), 
		     LabelStr("Predicate"), 
		     true, 
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(2, 10),
		     Token::noObject(), false);

    assert(t0.isIncomplete());
    t0.close();
    assert(t0.isInactive());
    t0.reject();
    assert(t0.isRejected());
    t0.cancel();
    assert(t0.isInactive());
    t0.activate();
    assert(t0.isActive());
    t0.cancel();
    assert(t0.isInactive());

    IntervalToken t1(db.getId(), 
		     LabelStr("Predicate"), 
		     true, 
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(2, 10),
		     Token::noObject(), true);

    // Constraint the start variable of both tokens
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(t0.getStart(), t1.getStart()));

    assert(t1.isInactive());
    t0.activate();
    t1.merge(t0.getId());
    assert(t1.isMerged());
    t1.cancel();
    assert(t1.isInactive());
    t1.merge(t0.getId());
    return true;
  }

  static bool testMasterSlaveRelationship(){
    DEFAULT_SETUP(ce, db, schema, true);

    IntervalToken t0(db.getId(), 
		     LabelStr("Predicate"), 
		     false, 
		     IntervalIntDomain(0, 1),
		     IntervalIntDomain(0, 1),
		     IntervalIntDomain(1, 1));
    t0.activate();

    TokenId t1 = (new IntervalToken(db.getId(), 
				    LabelStr("Predicate"), 
				    false,
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();
    t1->activate();

    TokenId t2 = (new IntervalToken(t0.getId(), 
				    LabelStr("Predicate"), 
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t3 = (new IntervalToken(t0.getId(), 
				    LabelStr("Predicate"), 
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t4 = (new IntervalToken(t0.getId(), 
				    LabelStr("Predicate"), 
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t5 = (new IntervalToken(t1, 
				    LabelStr("Predicate"), 
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(0, 1),
				    IntervalIntDomain(1, 1)))->getId();

    TokenId t6 = (new EventToken(t0.getId(), 
				    LabelStr("Predicate"), 
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
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);

    IntervalToken t1(db.getId(),
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    t1.getDuration()->specify(IntervalIntDomain(5, 7));

    // Activate & deactivate - ensure proper handling of rejectability variable
    assert(!t0.getState()->getDerivedDomain().isSingleton());
    t0.activate();
    assert(t0.getState()->getDerivedDomain().isSingleton());
    assert(t0.getState()->getDerivedDomain().getSingletonValue() == Token::ACTIVE);
    t0.cancel();
    assert(!t0.getState()->getDerivedDomain().isSingleton());

    // Now activate and merge
    t0.activate();
    t1.merge(t0.getId());

    // Make sure the necessary restrictions have been imposed due to merging i.e. restruction due to specified domain
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    assert(t1.isMerged());

    // Do a split and make sure the old values are reinstated.
    t1.cancel();
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);
    assert(t1.isInactive());

    // Now post equality constraint between t1 and extra token t2 and remerge
    IntervalToken t2(db.getId(), 
		     LabelStr("P2"), 
		     true,
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

    assert(!t0.getMergedTokens().empty());

    // Verify that the equality constraint has migrated and original has been deactivated.
    //TBW: when stacking instead of merging tokens, the next check is not true
    // assert(!equalityConstraint->isActive());
    assert(t0.getEnd()->getDerivedDomain().getLowerBound() == 8);
    assert(t0.getEnd()->getDerivedDomain() == t2.getEnd()->getDerivedDomain());

    // Undo the merge and check for initial conditions being established
    t1.cancel();
    assert(equalityConstraint->isActive());

    // Redo the merge
    t1.merge(t0.getId());

    // Confirm deletion of the constraint is handled correctly
    delete (Constraint*) equalityConstraint;
    assert(t0.getEnd()->getDerivedDomain() != t2.getEnd()->getDerivedDomain());

    // Confirm previous restriction due to specified domain, then reset and note the change
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 7);
    t1.getDuration()->reset();
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 20);


    // Test unary
    t1.cancel();
    ConstraintId subsetOfConstraint = ConstraintLibrary::createConstraint(LabelStr("SubsetOf"),
									  db.getConstraintEngine(),
									  t1.getDuration(),
									  IntervalIntDomain(5, 6));
    t1.merge(t0.getId());
    assert(t0.getDuration()->getDerivedDomain().getUpperBound() == 6);
    delete (Constraint*) subsetOfConstraint;

    // Deletion will now occur and test proper cleanup.
    return true;
  }

  // add backtracking and longer chain, also add a before constraint
  static bool testMergingPerformance(){
    DEFAULT_SETUP(ce, db, schema, false);
    ObjectId timeline = (new Timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2")))->getId();
    db.close();

    typedef Europa::Id<IntervalToken> IntervalTokenId;
    
    static const int NUMTOKS=3;
    static const int UNIFIED=1;
    static const int NUMPARAMS=1;

    //Create tokens with the same domains.  We will impose a constraint on
    //each token variable.  Tokens will have 5 parameter variables.
    std::vector< std::vector<IntervalTokenId> > tokens;

    for (int i=0; i < NUMTOKS; i++) {
      std::vector<IntervalTokenId> tmp;
      for (int j=0; j < UNIFIED; j++) {
	IntervalTokenId t = (new IntervalToken(db.getId(), 
					       LabelStr("P1"), 
					       true,
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
    assert(sdom1.getLowerBound() == 0);
    assert(sdom1.getUpperBound() == 210);

    IntervalIntDomain edom1(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom1.getLowerBound() == 1);
    assert(edom1.getUpperBound() == 220);

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar1(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom1(pvar1->getDerivedDomain());
    assert(pdom1.getLowerBound() == 500);
    assert(pdom1.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++) {
      tokens[i][0]->activate();
      timeline->constrain(tokens[i][0]);
    }

    IntervalIntDomain sdom2(tokens[0][0]->getStart()->getDerivedDomain());
    assert(sdom2.getLowerBound() == 0);
    assert(sdom2.getUpperBound() == 208);

    IntervalIntDomain edom2(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom2.getLowerBound() == 1);
    assert(edom2.getUpperBound() == 209);

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar2(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom2(pvar2->getDerivedDomain());
    assert(pdom2.getLowerBound() == 500);
    assert(pdom2.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) { 
	tokens[i][j]->merge(tokens[i][0]);
	ce.propagate();
      }

    IntervalIntDomain sdom3(tokens[0][0]->getStart()->getDerivedDomain());
    assert(sdom3.getLowerBound() == 0);
    assert(sdom3.getUpperBound() == 208);

    IntervalIntDomain edom3(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom3.getLowerBound() == 1);
    assert(edom3.getUpperBound() == 209);

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar3(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom3(pvar3->getDerivedDomain());
    assert(pdom3.getLowerBound() == 500+UNIFIED-1);
    assert(pdom3.getUpperBound() == 1000);

    for (int i=0; i < NUMTOKS; i++)
      for (int j=1; j < UNIFIED; j++) {
	tokens[i][j]->cancel();
	ce.propagate();
      }

    IntervalIntDomain sdom4(tokens[0][0]->getStart()->getDerivedDomain());
    assert(sdom4.getLowerBound() == sdom2.getLowerBound());
    assert(sdom4.getUpperBound() == sdom2.getUpperBound());

    IntervalIntDomain edom4(tokens[0][0]->getEnd()->getDerivedDomain());
    assert(edom4.getLowerBound() == edom2.getLowerBound());
    assert(edom4.getUpperBound() == edom2.getUpperBound());

    Europa::Id<TokenVariable<IntervalIntDomain> > pvar4(tokens[0][0]->getParameters()[0]);
    IntervalIntDomain pdom4(pvar4->getDerivedDomain());
    assert(pdom4.getLowerBound() == pdom2.getLowerBound());
    assert(pdom4.getUpperBound() == pdom2.getUpperBound());

    return true;
  }    

  static bool testTokenCompatibility(){
    DEFAULT_SETUP(ce, db, schema, true);

    // Create 2 mergeable tokens - predicates, types and base domaiuns match
    IntervalToken t0(db.getId(), 
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000),
		     Token::noObject(), false);
    t0.addParameter(IntervalDomain(1, 20));
    t0.close();

    // Same predicate and has an intersection
    IntervalToken t1(db.getId(),
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000),
		     Token::noObject(), false);
    t1.addParameter(IntervalDomain(10, 40)); // There is an intersection - but it is not a subset. Still should match
    t1.close();

    t0.activate();
    std::vector<TokenId> compatibleTokens;
    assert(ce.propagate());
    db.getCompatibleTokens(t1.getId(), compatibleTokens);
    assert(compatibleTokens.size() == 1);
    assert(compatibleTokens[0] == t0.getId());

    compatibleTokens.clear();
    t0.cancel();
    assert(ce.propagate());
    db.getCompatibleTokens(t1.getId(), compatibleTokens);
    assert(compatibleTokens.empty()); // No match since no tokens are active

    IntervalToken t2(db.getId(),
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000),
		     Token::noObject(), false);
    t2.addParameter(IntervalDomain(0, 0)); // Force no intersection
    t2.close();

    t0.activate();
    assert(ce.propagate());
    compatibleTokens.clear();
    db.getCompatibleTokens(t2.getId(), compatibleTokens);
    assert(compatibleTokens.empty()); // No match since parameter variable has no intersection


    IntervalToken t3(db.getId(),
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000),
		     Token::noObject(), false);
    t3.addParameter(IntervalDomain()); // Force no intersection
    t3.close();

    // Post equality constraint between t3 and t0. Should permit a match since it is a binary constraint
    EqualConstraint c0(LabelStr("eq"), LabelStr("Default"), db.getConstraintEngine(), makeScope(t0.getStart(), t3.getStart()));
    db.getConstraintEngine()->propagate();
    compatibleTokens.clear();
    db.getCompatibleTokens(t3.getId(), compatibleTokens);
    assert(compatibleTokens.size() == 1); // Expect a single match

    // Now post a constraint with higher arity. But including t0. Should exclude it.
    Variable<IntervalIntDomain> v0(db.getConstraintEngine(), IntervalIntDomain());
    AddEqualConstraint c1(LabelStr("addEq"), LabelStr("Default"), db.getConstraintEngine(), makeScope(t0.getStart(), t3.getStart(), v0.getId()));
    db.getConstraintEngine()->propagate();
    compatibleTokens.clear();
    db.getCompatibleTokens(t3.getId(), compatibleTokens);
    assert(compatibleTokens.empty()); // No match since t0 excluded due to additional constraint c1, which cannot be migrated.

    return true;
  }
};

class TimelineTest {
public:
  static bool test(){
    runTest(testBasicInsertion);
    runTest(testObjectTokenRelation);
    runTest(testTokenOrderQuery);
    runTest(testEventTokenInsertion);
    return true;
  }

private:
  static bool testBasicInsertion(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    IntervalToken tokenA(db.getId(), 
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db.getId(), 
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    assert(!timeline.hasTokensToOrder());
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    std::vector<TokenId> tokens;
    timeline.getTokensToOrder(tokens);
    assert(tokens.size() == 3);
    assert(timeline.getTokenSequence().size() == 0);
    assert(timeline.hasTokensToOrder());

    timeline.constrain(tokenA.getId());
    timeline.constrain(tokenB.getId());
    timeline.constrain(tokenC.getId(), tokenA.getId());

    assert(tokenA.getEnd()->getDerivedDomain().getUpperBound() <= tokenB.getStart()->getDerivedDomain().getUpperBound());
    assert(timeline.getTokenSequence().size() == 3);
    assert(!timeline.hasTokensToOrder());

    timeline.free(tokenA.getId());
    assert(timeline.getTokenSequence().size() == 2);
    tokens.clear();
    timeline.getTokensToOrder(tokens);
    assert(tokens.size() == 1);
    assert(timeline.hasTokensToOrder());

    // Now force it to be part of this timeline, even though it is not otherwise constrained
    tokenA.getObject()->specify(timeline.getId());
    tokens.clear();
    timeline.getTokensToOrder(tokens);
    assert(tokens.size() == 1); // Won't affect this quantity
    assert(tokens.front() == tokenA.getId());

    timeline.constrain(tokenA.getId());
    assert(!timeline.hasTokensToOrder());
    assert(timeline.getTokenSequence().size() == 3);
    timeline.free(tokenC.getId());

    assert(timeline.getTokenSequence().size() == 2);
    assert(timeline.hasTokensToOrder());
    return true;
  }

  static bool testObjectTokenRelation(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    IntervalToken tokenA(db.getId(), 
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db.getId(), 
		     LabelStr("P1"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    // Object variables are not singletons - so query for tokens to order should return nothing
    std::vector<TokenId> tokensToOrder;
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.empty());

    // Specify the object variable of one - but still should return no tokens since they are all inactive
    tokenA.getObject()->specify(timeline.getId());
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.empty());

    // Now activate all of them
    tokenA.activate();
    tokenB.activate();
    tokenC.activate();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 3);

    // Set remainders so they are singeltons and get all back
    tokenB.getObject()->specify(timeline.getId());
    tokenC.getObject()->specify(timeline.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 3);

    // Now incrementally constrain and show reduction in tokens to order
    timeline.constrain(tokenA.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 2);

    timeline.constrain(tokenB.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 1);

    timeline.constrain(tokenC.getId());
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.empty());


    // Test destruction call path
    Token* tokenD = new IntervalToken(db.getId(), 
				      LabelStr("P1"), 
				      true,
				      IntervalIntDomain(0, 10),
				      IntervalIntDomain(0, 20),
				      IntervalIntDomain(1, 1000));
    tokenD->activate();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.size() == 1);
    delete tokenD;
    tokensToOrder.clear();
    timeline.getTokensToOrder(tokensToOrder);
    assert(tokensToOrder.empty());

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
					 true,
					 IntervalIntDomain(start, start),
					 IntervalIntDomain(start+DURATION, start+DURATION),
					 IntervalIntDomain(DURATION, DURATION)))->getId();
      assert(!token->getObject()->getBaseDomain().isSingleton());
      token->getObject()->specify(timeline->getId());
      token->activate();
    }

    assert(timeline->getTokens().size() == 0);
    ce.propagate();
    assert(timeline->getTokens().size() == COUNT);

    int i = 0;
    std::vector<TokenId> tokensToOrder;
    timeline->getTokensToOrder(tokensToOrder);

    while(!tokensToOrder.empty()){
      assert(timeline->getTokenSequence().size() == i);
      assert(tokensToOrder.size() == (COUNT - i));
      std::vector<TokenId> choices;
      TokenId toConstrain = tokensToOrder.front();
      timeline->getOrderingChoices(toConstrain, choices);
      assert(!choices.empty());
      TokenId successor = choices.front();
      timeline->constrain(toConstrain, successor);
      assert(ce.propagate());
      tokensToOrder.clear();
      timeline->getTokensToOrder(tokensToOrder);
      i++;
      assert(ce.propagate());
    }

    const std::list<TokenId>& tokenSequence = timeline->getTokenSequence();
    assert(tokenSequence.front()->getStart()->getDerivedDomain().getSingletonValue() == 0);
    assert(tokenSequence.back()->getEnd()->getDerivedDomain().getSingletonValue() == COUNT*DURATION);

    // Now ensure the query can correctly indicate no options available
    TokenId token = (new IntervalToken(db.getId(), 
				       LabelStr("P1"),
				       true,
				       IntervalIntDomain(0, 0),
				       IntervalIntDomain(),
				       IntervalIntDomain(DURATION, DURATION)))->getId();
    token->getObject()->specify(timeline->getId());
    token->activate();
    std::vector<TokenId> choices;
    timeline->getOrderingChoices(token, choices);
    assert(choices.empty());

    return true;
  }

  static bool testEventTokenInsertion(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    IntervalToken it1(db.getId(), 
		      LabelStr("P1"), 
		      true,
		      IntervalIntDomain(0, 10),
		      IntervalIntDomain(0, 1000),
		      IntervalIntDomain(1, 1000));

    it1.getObject()->specify(timeline.getId());
    it1.activate();
    timeline.constrain(it1.getId(), TokenId::noId());

    // Insert at the end after a token
    EventToken et1(db.getId(), 
		   LabelStr("P2"), 
		   true, 
		   IntervalIntDomain(0, 100), 
		   Token::noObject());

    et1.getObject()->specify(timeline.getId());
    et1.activate();
    timeline.constrain(et1.getId(), TokenId::noId());
    assert(it1.getEnd()->getDerivedDomain().getUpperBound() == 100);

    // Insert between a token and an event
    EventToken et2(db.getId(), 
		   LabelStr("P2"), 
		   true, 
		   IntervalIntDomain(0, 100), 
		   Token::noObject());

    et2.getObject()->specify(timeline.getId());
    et2.activate();
    timeline.constrain(et2.getId(), et1.getId());
    assert(it1.getEnd()->getDerivedDomain().getUpperBound() == 100);

    // Insert before a token
    EventToken et3(db.getId(), 
		   LabelStr("P2"), 
		   true, 
		   IntervalIntDomain(10, 100), 
		   Token::noObject());

    et3.getObject()->specify(timeline.getId());
    et3.activate();
    timeline.constrain(et3.getId(), it1.getId());
    assert(it1.getStart()->getDerivedDomain().getLowerBound() == 10);

    // Insert between events
    EventToken et4(db.getId(), 
		   LabelStr("P2"), 
		   true, 
		   IntervalIntDomain(0, 100), 
		   Token::noObject());

    et4.getObject()->specify(timeline.getId());
    et4.activate();
    timeline.constrain(et4.getId(), et1.getId());
    assert(ce.propagate());

    return true;
  }
};

class RulesEngineTest {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testActivation);
    runTest(testRuleFiringAndCleanup);
    return true;
  }
private:
  static bool testBasicAllocation(){
    DEFAULT_SETUP(ce, db, schema, false);
    TestRule r(LabelStr("Type::Predicate"));
    return true;
  }

  static bool testActivation(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();

    TestRule r(LabelStr("AllObjects::Predicate"));

    IntervalToken tokenA(db.getId(),
		     LabelStr("AllObjects::Predicate"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    assert(Rule::getRules().size() == 1);
    assert(re.getRuleInstances().empty());

    int num_constraints = ce.getConstraints().size();
    // Activate and confirm the rule instance is created
    tokenA.activate();
    assert(re.getRuleInstances().size() == 1);
    // New constraint added to listen to rule variables
    assert(ce.getConstraints().size() == num_constraints + 1);

    // Deactivate to ensure the rule instance is removed
    tokenA.cancel();
    assert(re.getRuleInstances().empty());
    assert(ce.getConstraints().size() == num_constraints);

    // Activate again to test deletion through automatic cleanup.
    tokenA.activate();
    assert(re.getRuleInstances().size() == 1);
    return true;
  }

  static bool testRuleFiringAndCleanup(){
    DEFAULT_SETUP(ce, db, schema, false);
    Timeline timeline(db.getId(), LabelStr("AllObjects"), LabelStr("o2"));
    db.close();
    TestRule r(LabelStr("AllObjects::Predicate"));

    IntervalToken tokenA(db.getId(), 
		     LabelStr("AllObjects::Predicate"), 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    assert(ce.propagate());
    assert(db.getTokens().size() == 1);

    tokenA.activate();
    assert(ce.propagate());
    assert(db.getTokens().size() == 1);

    tokenA.getObject()->specify(timeline.getId());
    assert(ce.propagate());
    // 2 tokens added since fire will trigger twice due to composition
    assert(db.getTokens().size() == 3);
    assert(tokenA.getSlaves().size() == 2);
    TokenId slave = *(tokenA.getSlaves().begin());
    assert(slave->getDuration()->getDerivedDomain().isSingleton()); // Due to constraint on local variable, propagated through interim constraint

    // Test reset which should backtrack the rule
    tokenA.getObject()->reset();
    assert(ce.propagate());
    assert(db.getTokens().size() == 1);

    // Set again, and deactivate
    tokenA.getObject()->specify(timeline.getId());
    assert(ce.propagate());
    assert(db.getTokens().size() == 3);
    tokenA.cancel();
    assert(db.getTokens().size() == 1);

    // Now repeast to ensure correct automatic cleanup
    tokenA.activate();
    assert(ce.propagate());
    assert(db.getTokens().size() == 3); // Rule should fire since specified domain already set!
    return true;
  }
};

int main(){
  initConstraintLibrary();
  
  REGISTER_NARY(EqualConstraint, "CoTemporal", "Default");
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "Before", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_NARY(EqualConstraint, "EqualConstraint", "EquivalenceClass");
  // Allocate default schema initially so tests don't fail because of ID's
  SCHEMA;
  runTestSuite(ObjectTest::test);
  runTestSuite(TokenTest::test);
  runTestSuite(TimelineTest::test);
  runTestSuite(RulesEngineTest::test);
  std::cout << "Finished" << std::endl;
}
