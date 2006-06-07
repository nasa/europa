#include "rs-test-module.hh"
#include "ResourceDefs.hh"
#include "Resource.hh"
#include "Transaction.hh"
#include "ResourceConstraint.hh"
#include "ResourcePropagator.hh"
#include "SAVH_Profile.hh"
#include "SAVH_FVDetector.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_TimetableFVDetector.hh"
#include "SAVH_Resource.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_Reservoir.hh"
#include "SAVH_InstantTokens.hh"
#include "SAVH_Reusable.hh"
#include "SAVH_DurativeTokens.hh"

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

#define RESOURCE_DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    SchemaId schema = Schema::instance();\
    schema->reset();\
    schema->addObjectType(LabelStr("Resource")); \
    schema->addObjectType(LabelStr("Reservoir")); \
    schema->addPredicate(LabelStr("Reservoir.consume")); \
    schema->addMember(LabelStr("Reservoir.consume"), IntervalDomain().getTypeName(), LabelStr("quantity")); \
    schema->addObjectType(LabelStr("Reusable")); \
    schema->addPredicate("Reusable.uses"); \
    schema->addMember(LabelStr("Reusable.uses"), IntervalDomain().getTypeName(), LabelStr("quantity")); \
    schema->addObjectType(LabelStr("SAVHResource")); \
    schema->addPredicate(LabelStr("Resource.change"));\
    schema->addMember(LabelStr("Resource.change"), IntervalDomain().getTypeName(), LabelStr("quantity")); \
    PlanDatabase db(ce.getId(), schema); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
    new ResourcePropagator(LabelStr("Resource"), ce.getId(), db.getId()); \
    new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), ce.getId()); \
    if (autoClose) \
      db.close();

#define RESOURCE_DEFAULT_TEARDOWN()

class DefaultSetupTest {
public:
  static bool test() {
    runTest(testDefaultSetup);
    return true;
  }
private:
  static bool testDefaultSetup() {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    assertTrue(db.isClosed() == false);
    db.close();
    assertTrue(db.isClosed() == true);

    RESOURCE_DEFAULT_TEARDOWN();
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
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    ResourceId r = (new Resource (db.getId(), LabelStr("Resource"), LabelStr("r1")))->getId();
    std::list<InstantId> instants;
    r->getInstants(instants);
    assertTrue(instants.size() == 0);

    // Construction with argument setting
    new Resource(db.getId(), LabelStr("Resource"), LabelStr("r2"), 189.34, 0, 1000);

    db.close();
    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testBasicTransactionInsertion()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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

    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTransactionChangeHandling()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
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

    t2->setEarliest(1);
    assertTrue(ce.propagate());
    assertTrue(checkSum(r) == (1*1 + 2*2 + 3*3 + 4*3));
    assertTrue(checkLevelArea(r) == (1*45 + 1*80 + 998*100));

    t2->setEarliest(2);
    assertTrue(ce.propagate());
    assertTrue(checkSum(r) == (1*1 + 2*3 + 3*3));
    assertTrue(checkLevelArea(r) == (2*45 + 998*100));

    delete (Token*) t1;
    delete (Token*) t2;
    delete (Token*) t3;
    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testCorrectTransactionAllocation()
  {
    // Test that the right insertion behaviour (in terms of instants) is occuring
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLevelCalculation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionUpdates()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 10), 10, 10))->getId();
    ce.propagate();
    assertTrue(checkLevelArea(r) == 10*10);

    t1->setEarliest(1);
    assertTrue(checkLevelArea(r) == 10*9);

    t1->setLatest(8);
    assertTrue(checkLevelArea(r) == 10*7);

    t1->setLatest(6);
    assertTrue(checkLevelArea(r) == 10*5);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionRemoval()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testIntervalCapacityValues()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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
    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testConstraintCheckOnInsertion()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    std::list<InstantId> allInstants;
    new Resource(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity,  
		 limitMin, limitMax, productionRateMax, productionMax, consumptionRateMax, consumptionMax);

    db.close();

    // Make sure that it will reject a transaction that violates the spec up front
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), productionRateMax + 1, productionRateMax + 1))->getId();
    assertTrue(!ce.propagate());
    delete (Transaction*) t1;

    // Make sure that it will reject a transaction that violates the spec up front
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), consumptionRateMax - 1, consumptionRateMax - 1))->getId();
    assertTrue(!ce.propagate());
    delete (Transaction*) t2;

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  // Test that a violation can be detected if a concurrent transaction violates a rate constraint
  static bool testRateConstraintViolation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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
      
    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalProductionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalConsumptionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testUpperLimitExceededResourceViolation()
  {
    // Define input constrains for the resource spec
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testSummationConstraintResourceViolation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
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
    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testPointProfileQueries()
  {
    // Define input constrains for the resource spec
    RESOURCE_DEFAULT_SETUP(ce,db,false);
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

    RESOURCE_DEFAULT_TEARDOWN();
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
    debugMsg("ResourceTest:checkSum", "Returning " << sum);
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

    if(instants.empty()) {
      debugMsg("ResourceTest:checkLevelArea", "No Instants.  Returning 0.");
      return 0;
    }

    std::map<int, InstantId>::const_iterator it = instants.begin();
    InstantId current = it->second;
    ++it;

    while(it != instants.end()){
      InstantId next = it->second;
      debugMsg("ResourceTest:checkLevelArea", "Current(time: " << current->getTime() << " lowerLevel: " << current->getLevelMin() <<
	       " upperLevel: " << current->getLevelMax() << ") Next time: " << next->getTime());
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

class DummyProfile : public SAVH::Profile {
public:
  DummyProfile(PlanDatabaseId db, const SAVH::FVDetectorId fv) 
    : Profile(db, fv), m_receivedNotification(0) {}
  SAVH::InstantId getInstant(const int time) {
    return getGreatestInstant(time)->second;
  }
  int gotNotified(){return m_receivedNotification;}
  void resetNotified(){m_receivedNotification = 0;}
private:
  void handleTemporalConstraintAdded(const SAVH::TransactionId predecessor, int preArgIndex,
				     const SAVH::TransactionId successor, int sucArgIndex) { 
    SAVH::Profile::handleTemporalConstraintAdded(predecessor, preArgIndex, successor, sucArgIndex);
    m_receivedNotification++;
  }
  void handleTemporalConstraintRemoved(const SAVH::TransactionId predecessor, int preArgIndex,
				       const SAVH::TransactionId successor, int sucArgIndex) {
    SAVH::Profile::handleTemporalConstraintRemoved(predecessor, preArgIndex, successor, sucArgIndex);
    m_receivedNotification++;
  }
  void initRecompute(SAVH::InstantId inst){}
  void initRecompute(){}
  void recomputeLevels(SAVH::InstantId prev, SAVH::InstantId inst) {
  }
  int m_receivedNotification;
};

class DummyDetector : public SAVH::FVDetector {
public:
  DummyDetector(const SAVH::ResourceId res) : FVDetector(res) {};
  bool detect(const SAVH::InstantId inst) {return false;}
  void initialize(const SAVH::InstantId inst) {}
  void initialize() {}
};


class DummyResource : public SAVH::Resource {
public:
  DummyResource(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name,
		double initCapacityLb = 0, double initCapacityUb = 0, double lowerLimit = MINUS_INFINITY,
		double upperLimit = PLUS_INFINITY, double maxInstProduction = PLUS_INFINITY, double maxInstConsumption = PLUS_INFINITY,
		double maxProduction = PLUS_INFINITY, double maxConsumption = PLUS_INFINITY)
    : SAVH::Resource(planDatabase, type, name, LabelStr("TimetableFVDetector"), LabelStr("TimetableProfile"), initCapacityLb, initCapacityUb, 
		     lowerLimit, upperLimit, maxInstProduction, maxInstConsumption,
		     maxProduction, maxConsumption) {}

  void addTransaction(const SAVH::TransactionId trans) {
    m_profile->addTransaction(trans);
    //m_profile->recompute();
  }
  void removeTransaction(const SAVH::TransactionId trans) {
    m_profile->removeTransaction(trans);
    // m_profile->recompute();
  }
  void addToProfile(const TokenId& token) {}
  void removeFromProfile(const TokenId& token) {}
private:
  void notifyViolated(const SAVH::InstantId inst) {
    SAVH::TransactionId trans = *(inst->getTransactions().begin());
    const_cast<AbstractDomain&>(trans->time()->lastDomain()).empty();
  }

  //no implementation.  no tests for flaw detection
  void notifyFlawed(const SAVH::InstantId inst) {
  }

  void notifyDeleted(const SAVH::InstantId inst) {}
  void notifyNoLongerFlawed(const SAVH::InstantId inst){}
};
/**
   add tests for getClosedTransactions and getPendingTransactions!
 */
class ProfileTest {
public:
  static bool test() {
    runTest(testAdditionRemovalAndIteration);
    runTest(testRestrictionAndRelaxation);
    //converted from old timetable profile tests
    runTest(testTransactionChangeHandling);
    runTest(testCorrectTransactionAllocation);
    runTest(testLevelCalculation);
    runTest(testTransactionUpdates);
    //testTransactionRemoval only relevent for tokens--use to test reservoir
    runTest(testIntervalCapacityValues);
    //violation tests
    runTest(testRateConstraintViolation);
    runTest(testLowerTotalProductionExceededResourceViolation);
    runTest(testLowerTotalConsumptionExceededResourceViolation);
    runTest(testUpperLimitExceededResourceViolation);
    runTest(testSummationConstraintResourceViolation);
    //other tests
    runTest(testPointProfileQueries);
    runTest(testGnats3244);
    return true;
  }
private:
  static bool checkTimes(const std::set<int>& times, SAVH::ProfileIterator& profIt) {
    assertTrue(!profIt.done());
    assertTrue(!times.empty());

    int retval = 0;
    std::set<int>::const_iterator it = times.begin();
    while(!profIt.done()) {
      debugMsg("ResourceTest:checkTimes", *it << " " << profIt.getTime());
      assertTrue(profIt.getInstant()->getTime() == *it);
      profIt.next();
      ++it;
      ++retval;
    }
    assertTrue((unsigned) retval == times.size());
    return true;
  }

  static double checkLevelArea(SAVH::ProfileId prof) {
    prof->recompute();
    double area = 0;
    SAVH::ProfileIterator it(prof);
    if(it.done()) {
      debugMsg("ResourceTest:checkLevelArea", "No Instants.  Returning 0.");
      return 0;
    }
    SAVH::InstantId current = it.getInstant();
    it.next();
    while(!it.done()) {
      SAVH::InstantId next = it.getInstant();
      debugMsg("ResourceTest:checkLevelArea", "Current(time: " << current->getTime() << " lowerLevel: " << current->getLowerLevel() <<
	       " upperLevel: " << current->getUpperLevel() << ") Next time: " << next->getTime());
      area += ((current->getUpperLevel() - current->getLowerLevel()) * (next->getTime() - current->getTime()));
      current = next;
      it.next();
    }
    debugMsg("ResourceTest:checkLevelArea", "Returning " << area);
    return area;
  }

  static int checkSum(SAVH::ProfileId prof) {
    prof->recompute();
    int sum = 0;
    int count = 1;
    debugMsg("ResourceTest:checkSum", "        Transactions");
    SAVH::ProfileIterator it(prof);
    while(!it.done()) {
      SAVH::InstantId inst = it.getInstant();
      debugMsg("ResourceTest:checkSum", "        " << inst->getTime() << ":[" << inst->getTransactions().size() << "] ");
      sum += count * inst->getTransactions().size();
      count++;
      it.next();
    }
    debugMsg("ResourceTest:checkSum", "Returning " << sum);
    return sum;
  }

  static bool testAdditionRemovalAndIteration() {
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    DummyProfile profile(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 0));
    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(10, 10));
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(0, 3));
    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(1, 4));
    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(4, 6));
    Variable<IntervalDomain> quantity(ce.getId(), IntervalDomain(3, 3));

    std::set<int> times;

    SAVH::Transaction trans1(t1.getId(), quantity.getId(), false);
    SAVH::Transaction trans2(t2.getId(), quantity.getId(), false); //distinct cases
    SAVH::Transaction trans3(t3.getId(), quantity.getId(), false); //overlap on left side.  both trans1 and trans2 should appear at time 0
    SAVH::Transaction trans4(t4.getId(), quantity.getId(), false); //overlap in the middle.  should have trans3 and trans4 at time 1 and only trans4 at time 4
    SAVH::Transaction trans5(t5.getId(), quantity.getId(), false); //overlap on right side.  both trans 4 and trans5 should appear at time 4

    profile.addTransaction(trans1.getId());
    times.insert(0);
    SAVH::ProfileIterator prof1(profile.getId());
    assertTrue(checkTimes(times, prof1));

    profile.addTransaction(trans2.getId());
    times.insert(10);
    SAVH::ProfileIterator prof2(profile.getId());
    assertTrue(checkTimes(times, prof2));

    profile.addTransaction(trans3.getId());
    times.insert(3);
    SAVH::ProfileIterator prof3(profile.getId());
    assertTrue(checkTimes(times, prof3));
    SAVH::ProfileIterator oCheck1(profile.getId(), 0, 0);
    assertTrue(!oCheck1.done());
    assertTrue(oCheck1.getInstant()->getTransactions().size() == 2);
    
    profile.addTransaction(trans4.getId());
    times.insert(1);
    times.insert(4);
    SAVH::ProfileIterator prof4(profile.getId());
    assertTrue(checkTimes(times, prof4));
    SAVH::ProfileIterator oCheck2(profile.getId(), 3, 3);
    assertTrue(!oCheck2.done());
    assertTrue(oCheck2.getInstant()->getTransactions().size() == 2);

    profile.addTransaction(trans5.getId());
    times.insert(6);
    SAVH::ProfileIterator prof5(profile.getId());
    assertTrue(checkTimes(times, prof5));
    SAVH::ProfileIterator oCheck3(profile.getId(), 4, 4);
    assertTrue(!oCheck3.done());
    assertTrue(oCheck3.getInstant()->getTransactions().size() == 2);

    profile.removeTransaction(trans4.getId());
    times.erase(1);
    SAVH::ProfileIterator prof6(profile.getId());
    assertTrue(checkTimes(times, prof6));
    SAVH::ProfileIterator oCheck4(profile.getId(), 3, 3);
    assertTrue(oCheck4.getInstant()->getTransactions().size() == 1);
    SAVH::ProfileIterator oCheck5(profile.getId(), 4, 4);
    assertTrue(oCheck5.getInstant()->getTransactions().size() == 1);

    profile.removeTransaction(trans1.getId());
    profile.removeTransaction(trans2.getId());
    times.erase(10);
    SAVH::ProfileIterator prof7(profile.getId());
    assertTrue(checkTimes(times, prof7));
    SAVH::ProfileIterator oCheck6(profile.getId(), 0, 0);
    assertTrue(oCheck6.getInstant()->getTransactions().size() == 1);

    profile.removeTransaction(trans3.getId());
    profile.removeTransaction(trans5.getId());

    SAVH::ProfileIterator prof8(profile.getId());
    assertTrue(prof8.done());

    return true;
  }

  static bool testRestrictionAndRelaxation() {
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    DummyProfile profile(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 0));
    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(10, 10));
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(0, 3));
    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(1, 4));
    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(4, 6));
    Variable<IntervalDomain> quantity(ce.getId(), IntervalDomain(3, 3));

    std::set<int> times;
    times.insert(0); times.insert(1);
    times.insert(3); times.insert(4);
    times.insert(6); times.insert(10);

    SAVH::Transaction trans1(t1.getId(), quantity.getId(), false);
    SAVH::Transaction trans2(t2.getId(), quantity.getId(), false); 
    SAVH::Transaction trans3(t3.getId(), quantity.getId(), false); 
    SAVH::Transaction trans4(t4.getId(), quantity.getId(), false); 
    SAVH::Transaction trans5(t5.getId(), quantity.getId(), false); 

    profile.addTransaction(trans1.getId());
    profile.addTransaction(trans2.getId());
    profile.addTransaction(trans3.getId());
    profile.addTransaction(trans4.getId());
    profile.addTransaction(trans5.getId());

    SAVH::ProfileIterator baseTest(profile.getId());
    assertTrue(checkTimes(times, baseTest));

    //should remove the instant at 3 and add an instant at 2.  the instant at 2 should have two transactions (t3 and t4).
    times.erase(3);
    times.insert(2);
    const_cast<AbstractDomain&>(t3.lastDomain()).intersect(0, 2);
    SAVH::ProfileIterator prof1(profile.getId());
    assertTrue(checkTimes(times, prof1));
    SAVH::ProfileIterator oCheck1(profile.getId(), 2, 2);
    assertTrue(oCheck1.getInstant()->getTransactions().size() == 2);
    assertTrue(oCheck1.getInstant()->getTransactions().find(trans3.getId()) != oCheck1.getInstant()->getTransactions().end());
    assertTrue(oCheck1.getInstant()->getTransactions().find(trans4.getId()) != oCheck1.getInstant()->getTransactions().end());
    SAVH::ProfileIterator oCheck1_1(profile.getId(), 3, 3);
    assertTrue(oCheck1_1.done());

    times.erase(2);
    times.insert(3);
    t3.reset();
    SAVH::ProfileIterator prof2(profile.getId());
    assertTrue(checkTimes(times, prof2));
    SAVH::ProfileIterator oCheck2(profile.getId(), 3, 3);
    assertTrue(oCheck2.getInstant()->getTransactions().size() == 2);
    SAVH::ProfileIterator oCheck2_1(profile.getId(), 2, 2);
    assertTrue(oCheck2_1.done());
    

    assertTrue(profile.gotNotified() == 0);
    ConstraintId constr = db.getClient()->createConstraint("precedes", makeScope(t1.getId(), t2.getId()));
    assertTrue(profile.gotNotified() == 1);
    profile.resetNotified();
    constr->discard();
    assertTrue(profile.gotNotified() == 1);

    profile.resetNotified();
    Variable<IntervalIntDomain> temp(ce.getId(), IntervalIntDomain(-1, -1));
    ConstraintId constr2 = db.getClient()->createConstraint("precedes", makeScope(temp.getId(), t1.getId()));
    assertTrue(profile.gotNotified() == 0);
    constr2->discard();
    assertTrue(profile.gotNotified() == 0);
    return true;
  }

  static bool testTransactionChangeHandling()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    const int HORIZON_END = 1000;

    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::TimetableProfile r(db.getId(), detector.getId());

//     Variable<IntervalIntDomain> t0(ce.getId(), IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
//     Variable<IntervalDomain> q0(ce.getId(), IntervalDomain(0, 0));
//     SAVH::Transaction trans0(t0.getId(), q0.getId(), false);
//     r.addTransaction(trans0.getId());

    assertTrue(checkLevelArea(r.getId()) == 0);

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, HORIZON_END));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(45, 45));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    assertTrue(ce.propagate());
    assertTrue(checkSum(r.getId()) == (1*1 + 2*1));
    assertTrue(checkLevelArea(r.getId()) == 1000 * 45);

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(1, HORIZON_END));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(35, 35));
    SAVH::Transaction trans2(t2.getId(), q2.getId(), false);
    r.addTransaction(trans2.getId());
    assertTrue(ce.propagate());
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*2));
    assertTrue(checkLevelArea(r.getId()) == (1*45 + 80*999));

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(2, HORIZON_END));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(20, 20));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());
    assertTrue(ce.propagate());
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*3));
    assertTrue(checkLevelArea(r.getId()) == (1*45 + 1*80 + 998*100));

    trans2.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans2.time()->lastDomain().getUpperBound()));
    assertTrue(ce.propagate());
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*3));
    assertTrue(checkLevelArea(r.getId()) == (1*45 + 1*80 + 998*100));

    trans2.time()->restrictBaseDomain(IntervalIntDomain(2, (int)trans2.time()->lastDomain().getUpperBound()));
    assertTrue(ce.propagate());
    assertTrue(checkSum(r.getId()) == (1*1 + 2*3 + 3*3));
    assertTrue(checkLevelArea(r.getId()) == (2*45 + 998*100));

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testCorrectTransactionAllocation()
  {
    // Test that the right insertion behaviour (in terms of instants) is occuring
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::TimetableProfile r(db.getId(), detector.getId());

    assertTrue(ce.propagate() && checkSum(r.getId()) == 0); 

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(4, 6));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    assertTrue(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*1));

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(-4, 10));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    SAVH::Transaction trans2(t2.getId(), q2.getId(), false);
    r.addTransaction(trans2.getId());
    assertTrue(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*2 + 3*2 + 4*1));

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(1, 3));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());
    assertTrue(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*2 +3*2 + 4*2 + 5*2 + 6*1)); 

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(1, 2));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    SAVH::Transaction trans4(t4.getId(), q4.getId(), false);
    r.addTransaction(trans4.getId());    
    assertTrue(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*2 + 5*2 + 6*2 + 7*1)); 

    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(3, 7));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    SAVH::Transaction trans5(t5.getId(), q5.getId(), false);
    r.addTransaction(trans5.getId());
    assertTrue(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*3 + 5*3 + 6*3 + 7*2 + 8*1)); 

    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(4, 7));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    SAVH::Transaction trans6(t6.getId(), q6.getId(), false);
    r.addTransaction(trans6.getId());
    assertTrue(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*4 + 7*3 + 8*1)); 

    // Insert for a singleton value
    Variable<IntervalIntDomain> t7(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q7(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    SAVH::Transaction trans7(t7.getId(), q7.getId(), false);
    r.addTransaction(trans7.getId());
    assertTrue(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*5 + 7*4 + 8*3 + 9*1)); 

    // Now free them and check the retractions are working correctly

    r.removeTransaction(trans7.getId());
    assertTrue(ce.propagate());
    assertTrue(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*4 + 7*3 + 8*1));
    r.removeTransaction(trans6.getId());
    assertTrue(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*3 + 3*3 + 4*3 + 5*3 + 6*3 + 7*2 + 8*1));
    r.removeTransaction(trans5.getId());
    assertTrue(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*3 + 3*3 + 4*2 + 5*2 + 6*2 + 7*1));
    r.removeTransaction(trans4.getId());
    assertTrue(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*2 +3*2 + 4*2 + 5*2 + 6*1));
    r.removeTransaction(trans3.getId());
    assertTrue(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*2 + 3*2 + 4*1));
    r.removeTransaction(trans2.getId());
    assertTrue(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*1));
    r.removeTransaction(trans1.getId());
    assertTrue(ce.propagate() && checkSum(r.getId()) == 0);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLevelCalculation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::TimetableProfile r(db.getId(), detector.getId());    

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(1, 1));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());    
    ce.propagate();
    assertTrue(checkSum(r.getId()) == (1*1 + 2*1)); 
    assertTrue(checkLevelArea(r.getId()) == 1);

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(1, 3));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(4, 4));
    SAVH::Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    ce.propagate();
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*1)); 
    assertTrue(checkLevelArea(r.getId()) == (1 + 4*2));

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(2, 4));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());
    ce.propagate();
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*2 + 4*2 + 5*1)); 
    assertTrue(checkLevelArea(r.getId()) == (1*1 + 4*1 + 12*1 + 8*1));

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(3, 6));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(2, 2));
    SAVH::Transaction trans4(t4.getId(), q4.getId(), false);
    r.addTransaction(trans4.getId());
    ce.propagate();
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*2 + 4*3 + 5*2 + 6*1));
    assertTrue(checkLevelArea(r.getId()) == (1*1 + 4*1 + 12*1 + 10*1 + 2*2));
 
    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(2, 10));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(6, 6));
    SAVH::Transaction trans5(t5.getId(), q5.getId(), true);
    r.addTransaction(trans5.getId());
    ce.propagate();
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*2 + 7*1));
    assertTrue(checkLevelArea(r.getId()) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 6*4));

    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(6, 8));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(3, 3));
    SAVH::Transaction trans6(t6.getId(), q6.getId(), false);
    r.addTransaction(trans6.getId());
    ce.propagate();
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*3 + 7*2 + 8*1));
    assertTrue(checkLevelArea(r.getId()) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*2 + 6*2));

    Variable<IntervalIntDomain> t7(ce.getId(), IntervalIntDomain(7, 8));
    Variable<IntervalDomain> q7(ce.getId(), IntervalDomain(4, 4));
    SAVH::Transaction trans7(t7.getId(), q7.getId(), true);
    r.addTransaction(trans7.getId());
    ce.propagate();
    assertTrue(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*3 + +7* 3 + 8*3 + 9*1));
    assertTrue(checkLevelArea(r.getId()) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*1 + 13*1 + 6*2));

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionUpdates()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::TimetableProfile r(db.getId(), detector.getId());    

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 10));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(10, 10));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    ce.propagate();
    assertTrue(checkLevelArea(r.getId()) == 10*10);

    trans1.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans1.time()->lastDomain().getUpperBound()));
    assertTrue(checkLevelArea(r.getId()) == 10*9);

    trans1.time()->restrictBaseDomain(IntervalIntDomain((int)trans1.time()->lastDomain().getLowerBound(), 8));
    assertTrue(checkLevelArea(r.getId()) == 10*7);

    trans1.time()->restrictBaseDomain(IntervalIntDomain((int)trans1.time()->lastDomain().getLowerBound(), 6));
    assertTrue(checkLevelArea(r.getId()) == 10*5);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testIntervalCapacityValues()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::TimetableProfile r(db.getId(), detector.getId());        


    // Test producer
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 10));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(5, 10));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    ce.propagate();
    assertTrue(checkLevelArea(r.getId()) == 10*10);

    //test elided: we don't allow transactions that could produce or consume anymore
    // This tests a transaction that could be a producer or a consumer. We don't know yet!
//     TokenId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 8), -4, 3))->getId();
//     ce.propagate();
//     assertTrue(checkLevelArea(r.getId()) == 10*4 + 17*4 + 17*2);

    // Test consumer
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(1, 5));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(1, 4));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());
    ce.propagate();
    assertTrue(checkLevelArea(r.getId()) == 10*1 + 14*4 + 13*5);//+ 14*3 + 21*1 + 20*3 + 20*2);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testRateConstraintViolation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    DummyResource r(db.getId(), LabelStr("SAVHResource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax, -(consumptionMax));

    db.close();


    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(productionRateMax, productionRateMax + 1));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    ce.propagate();

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(1, 1));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());

    // no violation because of temporal flexibility
    assertTrue(ce.propagate());

    trans1.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans1.time()->lastDomain().getUpperBound()));
    trans3.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans3.time()->lastDomain().getUpperBound()));
    assertTrue(!ce.propagate());

    //r->getResourceViolations(violations);
    //assertTrue(violations.size() == 1);
    //assertTrue(violations.front()->getType() == ResourceViolation::ProductionRateExceeded);
    
    r.removeTransaction(trans3.getId());
    r.removeTransaction(trans1.getId());
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(-(consumptionRateMax), -(consumptionRateMax - 1)));
    SAVH::Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    ce.propagate();

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(1, 1));
    SAVH::Transaction trans4(t4.getId(), q4.getId(), true);
    r.addTransaction(trans4.getId());
    // no violation because of temporal flexibility
    assertTrue(ce.propagate());
    trans2.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans2.time()->lastDomain().getUpperBound()));
    trans4.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans4.time()->lastDomain().getUpperBound()));
    assertTrue(!ce.propagate());

    //violations.clear();
    //r->getResourceViolations(violations);
    //assertTrue(violations.size() == 1);
    //assertTrue(violations.front()->getType() == ResourceViolation::ConsumptionRateExceeded);
    r.removeTransaction(trans4.getId());
    r.removeTransaction(trans2.getId());
    assertTrue(ce.propagate());
      
    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalProductionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    DummyResource r(db.getId(), LabelStr("SAVHResource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax, -(consumptionMax));
    db.close();

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(2, 2));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), true);
    r.addTransaction(trans1.getId());
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(3, 3));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    assertTrue(ce.propagate());    

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(4, 4));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());    
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans4(t4.getId(), q4.getId(), true);
    r.addTransaction(trans4.getId());
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(6, 6));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans5(t5.getId(), q5.getId(), true);
    r.addTransaction(trans5.getId());
    assertTrue(ce.propagate());

    // This will push it over the edge
    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(10, 10));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans6(t6.getId(), q6.getId(), true);
    r.addTransaction(trans6.getId());
    assertTrue(!ce.propagate());

    assertTrue(checkLevelArea(r.getProfile()) == 0);
//     r.getResourceViolations(violations);
//     assertTrue(violations.front()->getType() == ResourceViolation::LevelTooLow);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalConsumptionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyResource r(db.getId(), LabelStr("SAVHResource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    PLUS_INFINITY, -(consumptionMax), PLUS_INFINITY, -(consumptionMax));
    db.close();

    std::list<ResourceViolationId> violations;

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(2, 2));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), true);
    r.addTransaction(trans1.getId());
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(3, 3));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    assertTrue(ce.propagate());   
 
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(4, 4));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans4(t4.getId(), q4.getId(), true);
    r.addTransaction(trans4.getId());
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(6, 6));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans5(t5.getId(), q5.getId(), true);
    r.addTransaction(trans5.getId());
    assertTrue(ce.propagate());

    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(8, 8));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans6(t6.getId(), q6.getId(), true);
    r.addTransaction(trans6.getId());
    assertTrue(ce.propagate());

    // This will push it over the edge
    Variable<IntervalIntDomain> t7(ce.getId(), IntervalIntDomain(10, 10));
    Variable<IntervalDomain> q7(ce.getId(), IntervalDomain(8, 8));
    SAVH::Transaction trans7(t7.getId(), q7.getId(), true);
    r.addTransaction(trans7.getId());
    assertTrue(!ce.propagate());

    assertTrue(checkLevelArea(r.getProfile()) == 0);
//     r->getResourceViolations(violations);
//     assertTrue(!violations.empty());
//     assertTrue(violations.front()->getType() == ResourceViolation::ConsumptionSumExceeded);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testUpperLimitExceededResourceViolation()
  {
    // Define input constrains for the resource spec
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    DummyResource r(db.getId(), LabelStr("SAVHResource"), LabelStr("r1"), initialCapacity + 1, initialCapacity + 1, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax + 100, -(consumptionMax));
    db.close();


    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // consumption
    std::list<SAVH::TransactionId> transactions;
    std::list<ConstrainedVariableId> vars;
    for (int i = 0; i < 11; i++){
      ConstrainedVariableId time = (new Variable<IntervalIntDomain>(ce.getId(), IntervalIntDomain(i, i)))->getId();
      ConstrainedVariableId quantity = (new Variable<IntervalDomain>(ce.getId(), IntervalDomain(productionRateMax, productionRateMax)))->getId();
      SAVH::TransactionId t = (new SAVH::Transaction(time, quantity, false))->getId();
      r.addTransaction(t);
      transactions.push_back(t);
      vars.push_back(quantity);
      vars.push_back(time);
      //r->constrain(t);
      if(i < 10) {
	assertTrue(ce.propagate());
      }
      else {
	assertTrue(!ce.propagate());
      }
    }

    assertTrue(checkLevelArea(r.getProfile()) == 0);

//     std::list<ResourceViolationId> violations;
//     r->getResourceViolations(violations);
//     assertTrue(violations.size() == 1);
//     assertTrue(violations.front()->getType() == ResourceViolation::LevelTooHigh);

    for(std::list<SAVH::TransactionId>::iterator it = transactions.begin(); it != transactions.end(); ++it)
      delete (SAVH::Transaction*) (*it);
    for(std::list<ConstrainedVariableId>::iterator it = vars.begin(); it != vars.end(); ++it)
      delete (ConstrainedVariable*) (*it);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testSummationConstraintResourceViolation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    
    DummyResource r(db.getId(), LabelStr("SAVHResource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax, -(consumptionMax));
    db.close();

    // Set up constraints so that all rate and level constraints are OK - balanced consumption
    // and production
    std::list<SAVH::TransactionId> transactions;
    std::list<ConstrainedVariableId> variables;
    for (int i = 0; i < 11; i++){
      ConstrainedVariableId time = (new Variable<IntervalIntDomain>(ce.getId(), IntervalIntDomain(i, i)))->getId();
      ConstrainedVariableId quantity = (new Variable<IntervalDomain>(ce.getId(), IntervalDomain(productionRateMax, productionRateMax)))->getId();
      SAVH::TransactionId t = (new SAVH::Transaction(time, quantity, false))->getId();
      r.addTransaction(t);
      //r->constrain(t);
      transactions.push_back(t);
      variables.push_back(time);
      variables.push_back(quantity);
    }
    for (int i = 0; i < 11; i++){
      ConstrainedVariableId time = (new Variable<IntervalIntDomain>(ce.getId(), IntervalIntDomain(i, i)))->getId();
      ConstrainedVariableId quantity = (new Variable<IntervalDomain>(ce.getId(), IntervalDomain(productionRateMax, productionRateMax)))->getId();
      SAVH::TransactionId t = (new SAVH::Transaction(time, quantity, true))->getId();
      r.addTransaction(t);
      //r->constrain(t);
      transactions.push_back(t);
      variables.push_back(time);
      variables.push_back(quantity);
    }

    assertTrue(!ce.propagate());
    assertTrue(checkLevelArea(r.getProfile()) == 0);

    // Ensure the violations remain unchanged
//     std::list<ResourceViolationId> violations;     
//     r->getResourceViolations(violations);
//     assertTrue(violations.size() > 0);

    for(std::list<SAVH::TransactionId>::iterator it = transactions.begin(); it != transactions.end(); ++it)
      delete (SAVH::Transaction*) (*it);
    for(std::list<ConstrainedVariableId>::iterator it = variables.begin(); it != variables.end(); ++it)
      delete (ConstrainedVariable*) (*it);
    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testPointProfileQueries()
  {
    // Define input constrains for the resource spec
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyResource r(db.getId(), LabelStr("SAVHResource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), 5, -(consumptionMax));
    db.close();

    IntervalDomain result;
    // Verify correct behaviour for the case with no transactions
    r.getProfile()->getLevel(10, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == initialCapacity);

    // Test that a flaw is signalled when there is a possibility to violate limits
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(5, 5));
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());

    // Have a single transaction, test before, at and after.
    r.getProfile()->getLevel(0, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == initialCapacity);
    r.getProfile()->getLevel(5, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5));
    r.getProfile()->getLevel(1000, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5));

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(0, 7));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(5, 5));
    SAVH::Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(2, 10));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(5, 5));
    SAVH::Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());

    // Confirm that we can query in the middle
    r.getProfile()->getLevel(6, result);
    assertTrue(result == IntervalDomain(initialCapacity+5-10, initialCapacity+5));

    // Confirm that we can query at the end
    r.getProfile()->getLevel(1000, result);
    assertTrue(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5 - 10));

    // There should be no violations, only flaws
    assertTrue(ce.propagate());

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testGnats3244() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);
    DummyDetector detector(SAVH::ResourceId::noId());
    DummyProfile profile(db.getId(), detector.getId());
    
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 10), true, "t1");
    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(10, 15), true, "t2");
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(5, 15), true, "t3");
    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(5, 15), true, "t1");
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(1, 1), true, "q1");
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(1, 1), true, "q2");
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(1, 1), true, "q3");
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(1, 1), true, "q4");
    
    SAVH::Transaction trans1(t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2(t2.getId(), q2.getId(), true);
    SAVH::Transaction trans3(t3.getId(), q3.getId(), true);
    SAVH::Transaction trans4(t4.getId(), q4.getId(), false);

    std::cout << "Creating " << trans1.getId() << std::endl;
    std::cout << "Creating " << trans2.getId() << std::endl;
    std::cout << "Creating " << trans3.getId() << std::endl;
    std::cout << "Creating " << trans4.getId() << std::endl;

    profile.addTransaction(trans1.getId());
    profile.addTransaction(trans2.getId());
    profile.addTransaction(trans3.getId());
    profile.addTransaction(trans4.getId());


    SAVH::InstantId inst = profile.getInstant(0);
    assertTrue(inst.isValid());
    assertTrue(inst->getTime() == 0);
    for(std::set<SAVH::TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      assertTrue((*it).isValid());
      assertTrue((*it) == trans1.getId());
    }

    //for time 5
    std::set<SAVH::TransactionId> trans;
    trans.insert(trans1.getId());
    trans.insert(trans3.getId());
    trans.insert(trans4.getId());

    inst = profile.getInstant(5);
    assertTrue(inst.isValid());
    assertTrue(inst->getTime() == 5);
    for(std::set<SAVH::TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      assertTrue((*it).isValid());
      assertTrue(trans.find(*it) != trans.end());
      trans.erase(*it);
    }
    assertTrue(trans.empty());

    //for time 10
    trans.insert(trans1.getId());
    trans.insert(trans2.getId());
    trans.insert(trans3.getId());
    trans.insert(trans4.getId());
    
    inst = profile.getInstant(10);
    assertTrue(inst.isValid());
    assertTrue(inst->getTime() == 10);
    for(std::set<SAVH::TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      assertTrue((*it).isValid());
      assertTrue(trans.find(*it) != trans.end());
      trans.erase(*it);
    }
    assertTrue(trans.empty());

    //for time 15
    trans.insert(trans2.getId());
    trans.insert(trans3.getId());
    trans.insert(trans4.getId());

    inst = profile.getInstant(15);
    assertTrue(inst.isValid());
    assertTrue(inst->getTime() == 15);
    for(std::set<SAVH::TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      assertTrue((*it).isValid());
      assertTrue(trans.find(*it) != trans.end());
      trans.erase(*it);
    }
    assertTrue(trans.empty());

    
    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }
};

class SAVHResourceTest {
public:
  static bool test() {
    runTest(testReservoir);
    runTest(testReusable);
    return true;
  }
private:
  static bool testReservoir() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);
    SAVH::Reservoir res1(db.getId(), LabelStr("Reservoir"), LabelStr("Battery1"), LabelStr("TimetableFVDetector"), LabelStr("TimetableProfile"),
			 10, 10, 0, 1000);
    SAVH::Reservoir res2(db.getId(), LabelStr("Reservoir"), LabelStr("Battery2"), LabelStr("TimetableFVDetector"), LabelStr("TimetableProfile"),
			 10, 10, 0, 1000);
    
    SAVH::ConsumerToken consumer(db.getId(), LabelStr("Reservoir.consume"), IntervalIntDomain(10),
			   IntervalDomain(5));

    SAVH::ProfileIterator it1(res1.getProfile());
    assertTrue(it1.done());
    SAVH::ProfileIterator it2(res2.getProfile());
    assertTrue(it2.done());

    const_cast<AbstractDomain&>(consumer.getObject()->lastDomain()).remove(res2.getId());
    SAVH::ProfileIterator it3(res1.getProfile());
    assertTrue(!it3.done());
    SAVH::ProfileIterator it4(res2.getProfile());
    assertTrue(it4.done());
    
    const_cast<AbstractDomain&>(consumer.getObject()->lastDomain()).relax(consumer.getObject()->baseDomain());
    SAVH::ProfileIterator it5(res1.getProfile());
    assertTrue(it5.done());
    SAVH::ProfileIterator it6(res2.getProfile());
    assertTrue(it6.done());

    SAVH::ConsumerToken throwaway(db.getId(), LabelStr("Reservoir.consume"), IntervalIntDomain(10),
				  IntervalDomain(5));

    res1.constrain(throwaway.getId(), throwaway.getId());
    throwaway.discard(false);

    //test flaws and violations once we have a decent profile and detector
    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testReusable() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);
    SAVH::Reusable res(db.getId(), LabelStr("Reusable"), LabelStr("Foo"), LabelStr("TimetableFVDetector"), LabelStr("TimetableProfile"));

    SAVH::ReusableToken use(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(0, 5), IntervalIntDomain(10, 15), IntervalIntDomain(1, PLUS_INFINITY),
			    IntervalDomain(10));

    //should create two transactions and four instants
    SAVH::ProfileIterator it(res.getProfile());
    assertTrue(!it.done());
    std::set<SAVH::TransactionId> trans;
    int instCount = 0;
    while(!it.done()) {
      SAVH::InstantId inst = it.getInstant();
      for(std::set<SAVH::TransactionId>::const_iterator transIt = inst->getTransactions().begin(); transIt != inst->getTransactions().end(); ++transIt)
	trans.insert(*transIt);
      instCount++;
      it.next();
    }
    assertTrue(trans.size() == 2);
    assertTrue(instCount == 4);
    
    use.getStart()->specify(0);
    use.getEnd()->specify(10);
    assertTrue(db.getConstraintEngine()->propagate());

    //should reduce the instant count to 2
    SAVH::ProfileIterator it1(res.getProfile());
    assertTrue(!it1.done());
    instCount = 0;
    while(!it1.done()) {
      instCount++;
      it1.next();
    }
    assertTrue(instCount == 2);

    SAVH::ReusableToken throwaway(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(50), IntervalIntDomain(51), IntervalIntDomain(1), IntervalDomain(1));
    throwaway.discard(false);

    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }
};

void ResourceModuleTests::runTests(std::string path) {
  LockManager::instance().connect();
  LockManager::instance().lock();
  setTestLoadLibraryPath(path);  

  Schema::instance();
  initConstraintLibrary();
  REGISTER_PROFILE(EUROPA::SAVH::TimetableProfile, TimetableProfile);
  REGISTER_FVDETECTOR(EUROPA::SAVH::TimetableFVDetector, TimetableFVDetector);
  runTestSuite(DefaultSetupTest::test);
  runTestSuite(ResourceTest::test);
  runTestSuite(ProfileTest::test);
  runTestSuite(SAVHResourceTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  uninitConstraintLibrary();
}

  

