#include "ResourceDefs.hh"
#include "Resource.hh"
#include "Transaction.hh"
#include "ResourceConstraint.hh"
#include "ResourcePropagator.hh"

#include "TestSupport.hh"
#include "IntervalIntDomain.hh"
#include "IntervalDomain.hh"
#include "DefaultPropagator.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Debug.hh"

#include "LockManager.hh"

#include <iostream>
#include <string>
#include <list>

// Useful constants when doing constraint vio9lation tests
const double initialCapacity = 5;
const double limitMin = 0;
const double limitMax = 10;
const double productionRateMax = 5;
const double productionMax = 40;
const double consumptionRateMax = -8;
const double consumptionMax = -50;

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    SchemaId schema = Schema::instance();\
    schema->reset();\
    schema->addObjectType(LabelStr("Resource")); \
    schema->addPredicate(LabelStr("Resource.change"));\
    schema->addMember(LabelStr("Resource.change"), IntervalDomain().getTypeName(), LabelStr("quantity")); \
    PlanDatabase db(ce.getId(), schema); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
    new ResourcePropagator(LabelStr("Resource"), ce.getId(), db.getId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN()

class DefaultSetupTest {
public:
  static bool test() {
    runTest(testDefaultSetup);
    return true;
  }
private:
  static bool testDefaultSetup() {
    DEFAULT_SETUP(ce,db,false);
    
    assertTrue(db.isClosed() == false);
    db.close();
    assertTrue(db.isClosed() == true);

    DEFAULT_TEARDOWN();
    return true;
  }
};

void printViolationSet(const std::set<ResourceViolationId>& violations) {
  std::set<ResourceViolationId>::iterator it = violations.begin();
  for ( ; it!=violations.end(); ++it ) {
    std::cout << " ";
    (*it)->print(std::cout);
  }
  std::cout << std::endl;
}
void printViolationList(const std::list<ResourceViolationId>& violations) {
  std::cout << std::endl;
  std::list<ResourceViolationId>::const_iterator it = violations.begin();
  for ( ; it!=violations.end(); ++it ) {
    std::cout << " ";
    (*it)->print(std::cout);
  }
  std::cout << std::endl << "Total " << violations.size() << std::endl;
}


class ResourceTest
{
public:

  static bool test(){
    runTest(testResourceConstructionAndDestruction);
    runTest(testBasicTransactionInsertion);
    runTest(testTransactionChangeHandling);
    runTest(testCorrectTransactionAllocation);
    runTest(testLevelCalculation);
    runTest(testTransactionUpdates);
    runTest(testTransactionRemoval);
    runTest(testIntervalCapacityValues);
    runTest(testConstraintCheckOnInsertion);
    runTest(testLowerTotalProductionExceededResourceViolation);
    runTest(testLowerTotalConsumptionExceededResourceViolation);
    runTest(testUpperLimitExceededResourceViolation);
    runTest(testSummationConstraintResourceViolation);
    runTest(testPointProfileQueries);
    return true;
  }
    
private:
  
  static bool testResourceConstructionAndDestruction()
  {
    DEFAULT_SETUP(ce,db,false);
    
    ResourceId r = (new Resource (db.getId(), LabelStr("Resource"), LabelStr("r1")))->getId();
    std::list<InstantId> instants;
    r->getInstants(instants);
    assertTrue(instants.size() == 0);

    // Construction with argument setting
    new Resource(db.getId(), LabelStr("Resource"), LabelStr("r2"), 189.34, 0, 1000);

    db.close();
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testBasicTransactionInsertion()
  {
    DEFAULT_SETUP(ce,db,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 10, 0, 1000))->getId();

    //just another resource so that the resource doesnt get bound to singleton and get autoinserted by the propagator
    ResourceId r2 = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r2"), 10, 0, 2000))->getId();

    assertTrue(!r.isNoId() && !r2.isNoId() && r != r2);

    db.close();

    // Test insertion of transaction constructed with defaults
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change")))->getId();
    bool prop = ce.propagate();
    assertTrue(prop);
    r->constrain(t1, t1);
    ce.propagate();

    std::set<TransactionId> transactions;
    r->getTransactions(transactions);
    assertTrue(transactions.size() == 1);
    r->free(t1, t1);
    transactions.clear();
    r->getTransactions(transactions);
    assertTrue(transactions.empty());

    // Test double insertion 
    r->constrain(t1, t1);
    ce.propagate();
    r->free(t1, t1);
    r->constrain(t1, t1);
    ce.propagate();
    r->free(t1, t1);

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTransactionChangeHandling()
  {
    DEFAULT_SETUP(ce,db,false);
    const int HORIZON_END = 1000;

    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 0, 0, 1000))->getId();
    db.close();
    assertTrue(checkLevelArea(r) == 0);

    TokenId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, HORIZON_END), 45, 45))->getId();
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*1));
    assertTrue(checkLevelArea(r) == 1000 * 45);

    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, HORIZON_END), 35, 35))->getId();
    assertTrue(ce.propagate());
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*2));
    assertTrue(checkLevelArea(r) == (1*45 + 80*999));

    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, HORIZON_END), 20, 20))->getId();
    assertTrue(ce.propagate());
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*3 + 4*3));
    assertTrue(checkLevelArea(r) == (1*45 + 1*80 + 998*100));

    t2->setEarliest(2);
    assertTrue(ce.propagate());
    assertTrue(checkSum(r) == (1*1 + 2*3 + 3*3));
    assertTrue(checkLevelArea(r) == (2*45 + 998*100));

    t2->setEarliest(1);
    assertTrue(ce.propagate());
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*3 + 4*3));
    assertTrue(checkLevelArea(r) == (1*45 + 1*80 + 998*100));

    delete (Token*) t1;
    delete (Token*) t2;
    delete (Token*) t3;
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testCorrectTransactionAllocation()
  {
    // Test that the right insertion behaviour (in terms of instants) is occuring
    DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 0, 0, 1000))->getId();
    db.close();
    assertTrue(ce.propagate() && checkSum(r) == 0); 

    TokenId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 6)))->getId();
    //r->constrain(t1);
    assertTrue(ce.propagate() && checkSum(r) == (1*1 + 2*1));

    TokenId t2  = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(-4, 10)))->getId();
    //r->constrain(t2);
    assertTrue(ce.propagate() && checkSum(r) == (1*1 + 2*2 + 3*2 + 4*1));

    TokenId t3  = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 3)))->getId();
    //r->constrain(t3);
    assertTrue(ce.propagate() && checkSum(r) == (1*1 + 2*2 +3*2 + 4*2 + 5*2 + 6*1)); 

    TokenId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 2)))->getId();
    //r->constrain(t4);
    assertTrue(ce.propagate() && checkSum(r) == (1*1 + 2*3 + 3*3 + 4*2 + 5*2 + 6*2 + 7*1)); 

    TokenId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 7)))->getId();
    //r->constrain(t5);
    assertTrue(ce.propagate() && checkSum(r) == (1*1 + 2*3 + 3*3 + 4*3 + 5*3 + 6*3 + 7*2 + 8*1)); 

    TokenId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 7)))->getId();
    //r->constrain(t6);
    assertTrue(ce.propagate() && checkSum(r) == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*4 + 7*3 + 8*1)); 

    // Insert for a singleton value
    TokenId t7 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5,5)))->getId();
    //r->constrain(t7);
    assertTrue(ce.propagate() && checkSum(r) == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*5 + 7*4 + 8*3 + 9*1)); 

    // Now free them and check the retractions are working correctly
    delete (Transaction*) (t7);
    assertTrue(ce.propagate());
    assertTrue(ce.propagate() && checkSum(r)  == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*4 + 7*3 + 8*1));
    delete (Transaction*) (t6);
    assertTrue(ce.propagate() && checkSum(r)  == (1*1 + 2*3 + 3*3 + 4*3 + 5*3 + 6*3 + 7*2 + 8*1));
    delete (Transaction*) (t5);
    assertTrue(ce.propagate() && checkSum(r)  == (1*1 + 2*3 + 3*3 + 4*2 + 5*2 + 6*2 + 7*1));
    delete (Transaction*) (t4);
    assertTrue(ce.propagate() && checkSum(r)  == (1*1 + 2*2 +3*2 + 4*2 + 5*2 + 6*1));
    delete (Transaction*) (t3);
    assertTrue(ce.propagate() && checkSum(r)  == (1*1 + 2*2 + 3*2 + 4*1));
    delete (Transaction*) (t2);
    assertTrue(ce.propagate() && checkSum(r)  == (1*1 + 2*1));
    delete (Transaction*) (t1);
    assertTrue(ce.propagate() && checkSum(r) == 0);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLevelCalculation()
  {
    DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    TokenId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), 1, 1))->getId();
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*1)); 
    assertTrue(checkLevelArea(r) == 1);

    TokenId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 3), -4, -4))->getId();
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*1)); 
    assertTrue(checkLevelArea(r) == (1 + 4*2));

    TokenId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 4), 8, 8))->getId();
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*2 + 4*2 + 5*1)); 
    assertTrue(checkLevelArea(r) == (1*1 + 4*1 + 12*1 + 8*1));

    TokenId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 6), 2, 2))->getId();
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*2 + 4*3 + 5*2 + 6*1));
    assertTrue(checkLevelArea(r) == (1*1 + 4*1 + 12*1 + 10*1 + 2*2));
 
    TokenId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 10), -6, -6))->getId(); 
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*2 + 7*1));
    assertTrue(checkLevelArea(r) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 6*4));

    TokenId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 8), 3, 3))->getId();
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*3 + 7*2 + 8*1));
    assertTrue(checkLevelArea(r) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*2 + 6*2));

    TokenId t7 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(7, 8), -4, -4))->getId();
    ce.propagate();
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*3 + +7* 3 + 8*3 + 9*1));
    assertTrue(checkLevelArea(r) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*1 + 13*1 + 6*2));

    delete (Token*) t1;
    delete (Token*) t2;
    delete (Token*) t3;
    delete (Token*) t4;
    delete (Token*) t5;
    delete (Token*) t6;
    delete (Token*) t7;

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionUpdates()
  {
    DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 10), 10, 10))->getId();
    ce.propagate();
    assertTrue(checkLevelArea(r) == 10*10);

    t1->setEarliest(1);
    assertTrue(checkLevelArea(r) == 10*9);

    t1->setLatest(6);
    assertTrue(checkLevelArea(r) == 10*5);

    // Now try some relaxations
    t1->setLatest(8);
    assertTrue(checkLevelArea(r) == 10*7);

    t1->setMin(-4);
    t1->setMax(1);
    ce.propagate();
    assertTrue(checkLevelArea(r) == 5*7);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionRemoval()
  {
    DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r0"), 0, 0, 10))->getId();
    // Add another resource so we can assign and unassign
    new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 0, 0, 10);
    db.close();

    // Insertion and removal at extremes
    TokenId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 0), 10, 10))->getId();
    t1->getObject()->specify(r);
    ce.propagate();
    t1->getObject()->reset();
    ce.propagate();
    t1->getObject()->specify(r);
    ce.propagate();

    TokenId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(10, 10), 10, 10))->getId();
    t2->getObject()->specify(r);
    ce.propagate();
    t2->getObject()->reset();
    ce.propagate();
    t2->getObject()->specify(r);
    ce.propagate();

    // Insertion and removal to create and delete an instant
    TokenId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5, 5), 10, 10))->getId();
    t3->getObject()->specify(r);
    ce.propagate();
    t2->getObject()->reset();
    ce.propagate();
    t3->getObject()->specify(r);
    ce.propagate();

    // Insertion of overlapping spanning transactions, all internal
    TokenId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 8), 10, 10))->getId();
    t4->getObject()->specify(r);
    ce.propagate();
    TokenId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 9), 10, 10))->getId();
    t5->getObject()->specify(r);
    ce.propagate();
    TokenId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 9), 10, 10))->getId();
    t6->getObject()->specify(r);
    ce.propagate();

    // Remove transactions in spurious order
    t4->getObject()->reset();
    t1->getObject()->reset();
    t3->getObject()->reset();
    t6->getObject()->reset();
    t2->getObject()->reset();
    t5->getObject()->reset();

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testIntervalCapacityValues()
  {
    DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    // Test producer
    TokenId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 10), 5, 10))->getId();
    ce.propagate();
    assertTrue(checkLevelArea(r) == 10*10);

    // This tests a transaction that could be a producer or a consumer. We don't know yet!
    TokenId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 8), -4, 3))->getId();
    ce.propagate();
    assertTrue(checkLevelArea(r) == 10*4 + 17*4 + 17*2);

    // Test consumer
    TokenId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 5), -4, -1))->getId();
    ce.propagate();
    assertTrue(checkLevelArea(r) == 10*1 + 14*3 + 21*1 + 20*3 + 20*2);

    delete (Token*) t1;
    delete (Token*) t2;
    delete (Token*) t3;
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testConstraintCheckOnInsertion()
  {
    DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity,  
		 limitMin, limitMax, productionRateMax, productionMax, consumptionRateMax, consumptionMax);

    db.close();

    // Make sure that it will reject a transaction that violates the spec up front
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), productionRateMax + 1, productionRateMax + 1))->getId();
    assertTrue(!ce.propagate());    

    t1->setMin(productionRateMax);
    assertTrue(!ce.provenInconsistent());
    delete (Transaction*) t1;

    // Make sure that it will reject a transaction that violates the spec up front
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), consumptionRateMax - 1, consumptionRateMax - 1))->getId();
    assertTrue(!ce.propagate());
    t2->setMax(consumptionRateMax);
    assertTrue(!ce.provenInconsistent());
    DEFAULT_TEARDOWN();
    return(true);
  }

  // Test that a violation can be detected if a concurrent transaction violates a rate constraint
  static bool testRateConstraintViolation()
  {
    DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity,  
				 limitMin, limitMax, productionRateMax, productionMax, consumptionRateMax, consumptionMax))->getId();
    db.close();

    std::list<ResourceViolationId> violations;

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), productionRateMax, productionRateMax + 1))->getId();
    ce.propagate();
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), 1, 1))->getId();

    // no violation because of temporal flexibility
    assertTrue(ce.propagate());
    t1->setEarliest(1);
    t3->setEarliest(1);
    assertTrue(!ce.propagate());

    r->getResourceViolations(violations);
    assertTrue(violations.size() == 1);
    assertTrue(violations.front()->getType() == ResourceViolation::ProductionRateExceeded);
    delete (Transaction*) t1;
    delete (Transaction*) t3;
    assertTrue(ce.propagate());

    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), consumptionRateMax -1, consumptionRateMax))->getId();
    ce.propagate();
    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), -1, -1))->getId();
    // no violation because of temporal flexibility
    assertTrue(ce.propagate());
    t2->setEarliest(1);
    t4->setEarliest(1);
    assertTrue(!ce.propagate());

    violations.clear();
    r->getResourceViolations(violations);
    assertTrue(violations.size() == 1);
    assertTrue(violations.front()->getType() == ResourceViolation::ConsumptionRateExceeded);
    delete (Transaction*) t2;
    delete (Transaction*) t4;
    assertTrue(ce.propagate());
      
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalProductionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    DEFAULT_SETUP(ce,db,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity,  
				 limitMin, limitMax, productionMax, productionMax, MINUS_INFINITY, MINUS_INFINITY))->getId();
    db.close();

    std::list<ResourceViolationId> violations;

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    TokenId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 2), -8, -8))->getId();
    assertTrue(ce.propagate());
    TokenId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 3), -8, -8))->getId();
    assertTrue(ce.propagate());    
    TokenId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 4), -8, -8))->getId();
    assertTrue(ce.propagate());
    TokenId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5, 5), -8, -8))->getId();
    assertTrue(ce.propagate());
    TokenId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 6), -8, -8))->getId();
    assertTrue(ce.propagate());
    // This will push it over the edge
    TokenId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(10, 10), -8, -8))->getId();
    assertTrue(!ce.propagate());
    assertTrue(checkLevelArea(r) == 0);
    r->getResourceViolations(violations);
    assertTrue(violations.front()->getType() == ResourceViolation::LevelTooLow);

    delete (Token*) t1;
    delete (Token*) t2;
    delete (Token*) t3;
    delete (Token*) t4;
    delete (Token*) t5;
    delete (Token*) t6;

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalConsumptionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    DEFAULT_SETUP(ce,db,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity,  
				 limitMin, limitMax, PLUS_INFINITY, PLUS_INFINITY, consumptionMax, consumptionMax))->getId();
    db.close();

    std::list<ResourceViolationId> violations;

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    TokenId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 2), -8, -8))->getId();
    assertTrue(ce.propagate());
    TokenId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 3), -8, -8))->getId();
    assertTrue(ce.propagate());    
    TokenId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 4), -8, -8))->getId();
    assertTrue(ce.propagate());
    TokenId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5, 5), -8, -8))->getId();
    assertTrue(ce.propagate());
    TokenId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 6), -8, -8))->getId();
    assertTrue(ce.propagate());
    TokenId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(8, 8), -8, -8))->getId();
    assertTrue(ce.propagate());
    // This will push it over the edge
    TokenId t7 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(10, 10), -8, -8))->getId();
    assertTrue(!ce.propagate());

    assertTrue(checkLevelArea(r) == 0);
    r->getResourceViolations(violations);
    assertTrue(!violations.empty());
    assertTrue(violations.front()->getType() == ResourceViolation::ConsumptionSumExceeded);

    delete (Token*) t1;
    delete (Token*) t2;
    delete (Token*) t3;
    delete (Token*) t4;
    delete (Token*) t5;
    delete (Token*) t6;
    delete (Token*) t7;

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testUpperLimitExceededResourceViolation()
  {
    // Define input constrains for the resource spec
    DEFAULT_SETUP(ce,db,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity + 1,  
				 limitMin, limitMax, productionRateMax, productionMax + 100, consumptionRateMax, consumptionMax))->getId();
    db.close();


    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // consumption
    std::list<TokenId> transactions;
    for (int i = 0; i < 11; i++){
      TokenId t = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(i, i), productionRateMax, productionRateMax))->getId();
      //r->constrain(t);
      ce.propagate();
      transactions.push_back(t);
    }

    assertTrue(checkLevelArea(r) == 0);

    std::list<ResourceViolationId> violations;
    r->getResourceViolations(violations);
    assertTrue(violations.size() == 1);
    assertTrue(violations.front()->getType() == ResourceViolation::LevelTooHigh);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testSummationConstraintResourceViolation()
  {
    DEFAULT_SETUP(ce,db,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity,  
				 limitMin, limitMax, productionRateMax, productionMax, consumptionRateMax, consumptionMax))->getId();
    db.close();

    // Set up constraints so that all rate and level constraints are OK - balanced consumption
    // and production
    std::list<TokenId> transactions;
    for (int i = 0; i < 11; i++){
      TokenId t = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(i, i), productionRateMax, productionRateMax))->getId();
      //r->constrain(t);
      transactions.push_back(t);
    }
    for (int i = 0; i < 11; i++){
      TokenId t = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(i, i), -productionRateMax, -productionRateMax))->getId();
      //r->constrain(t);
      transactions.push_back(t);
    }

    ce.propagate();
    assertTrue(checkLevelArea(r) == 0);

    // Ensure the violations remain unchanged
    std::list<ResourceViolationId> violations;     
    r->getResourceViolations(violations);
    assertTrue(violations.size() > 0);
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testPointProfileQueries()
  {
    // Define input constrains for the resource spec
    DEFAULT_SETUP(ce,db,false);
    ResourceId r = (new Resource( db.getId(), LabelStr("Resource"), LabelStr("r1"), 
				  initialCapacity, 
				  limitMin, limitMax, productionRateMax, 5, consumptionRateMax, consumptionMax))->getId();
    db.close();

    IntervalDomain result;
    // Verify correct behaviour for the case with no transactions
    r->getLevelAt(10, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == initialCapacity);

    // Test that a flaw is signalled when there is a possibility to violate limits
    TransactionId producer = (new Transaction(db.getId(), LabelStr("Resource.change"), 
					      IntervalIntDomain(5, 5), 5, 5))->getId();

    // Have a single transaction, test before, at and after.
    r->getLevelAt(0, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == initialCapacity);
    r->getLevelAt(5, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5));
    r->getLevelAt(1000, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5));

    TransactionId c1 = (new Transaction(db.getId(), LabelStr("Resource.change"), 
					IntervalIntDomain(0, 7), -5, -5))->getId();

    TransactionId c2 = (new Transaction(db.getId(), LabelStr("Resource.change"), 
					IntervalIntDomain(2, 10), -5, -5))->getId();

    // Confirm that we can query in the middle
    r->getLevelAt(6, result);
    assertTrue(result == IntervalDomain(initialCapacity+5-10, initialCapacity+5));

    // Confirm that we can query at the end
    r->getLevelAt(1000, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5 - 10));

    // There should be no violations, only flaws
    assertTrue(ce.propagate());

    delete (Token*) producer;
    delete (Token*) c1;
    delete (Token*) c2;

    DEFAULT_TEARDOWN();
    return(true);
  }


  /* Utility methods for testing */

  /**
   * Sums the instances of transactions in each instant.
   */
  static int checkSum(ResourceId r) {
    assertTrue(r != ResourceId::noId());

    if(r->getPlanDatabase()->getConstraintEngine()->pending())
      r->getPlanDatabase()->getConstraintEngine()->propagate();
    int sum = 0;
    int count = 1;
    debugMsg("ResourceTest:checkSum","        Transactions");
    const std::map<int, InstantId>& instants = r->getInstants();
    for(std::map<int, InstantId>::const_iterator it = instants.begin(); it != instants.end(); ++it){
      InstantId current = it->second;
      debugMsg("ResourceTest:checkSum", "           " << current->getTime() << ":[" << current->getTransactionCount() << "] ");
      sum += count * current->getTransactionCount();
      count++;
    }

    return(sum);
  }

  /**
   * Sums the instances of transactions in each instant.
   */
  static double checkLevelArea(ResourceId r) {
    assertTrue(r != ResourceId::noId());

    if(r->getPlanDatabase()->getConstraintEngine()->pending())
      r->getPlanDatabase()->getConstraintEngine()->propagate();

    const std::map<int, InstantId>& instants = r->getInstants();
    double area = 0;

    if(instants.empty())
      return 0;

    std::map<int, InstantId>::const_iterator it = instants.begin();
    InstantId current = it->second;
    ++it;

    while(it != instants.end()){
      InstantId next = it->second;
      area += ((current->getLevelMax() - current->getLevelMin()) * (next->getTime() - current->getTime()));
      current = next;
      ++it;
    }

    return area;
  }

  static void printResourceViolations(std::list<ResourceViolationId>& violations){
    for(std::list<ResourceViolationId>::iterator it = violations.begin(); it != violations.end(); ++it){
      (*it)->print(std::cout);
      std::cout << std::endl;
    }
  }

};


int main() {
  LockManager::instance().connect();
  LockManager::instance().lock();
  
  Schema::instance();
  initConstraintLibrary();
  REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  runTestSuite(DefaultSetupTest::test);
  runTestSuite(ResourceTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
