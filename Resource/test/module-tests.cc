#include "ResourceDefs.hh"
#include "Resource.hh"
#include "Transaction.hh"
#include "ResourceConstraint.hh"
#include "ResourceTransactionConstraint.hh"
#include "ResourcePropagator.hh"

#include "TestSupport.hh"
#include "IntervalIntDomain.hh"
#include "IntervalDomain.hh"
#include "DefaultPropagator.hh"
#include "Constraint.hh"
#include "CeLogger.hh"
#include "Utils.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "DbLogger.hh"

#include <iostream>
#include <string>
#include <list>

// Useful constants when doing constraint vio9lation tests
const double initialCapacity = 5;
const int horizonStart = 0;
const int horizonEnd = 10;
const double limitMax = 10;
const double limitMin = 0;
const double productionRateMax = 5;
const double productionMax = 40;
const double consumptionRateMax = -8;
const double consumptionMax = -50;

#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce; \
    Schema schema; \
    PlanDatabase db(ce.getId(), schema.getId()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new ResourcePropagator(LabelStr("Resource"), ce.getId(), db.getId()); \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce.getId()); \
      dbLId = (new DbLogger(std::cout, db.getId()))->getId(); \
    } \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN() \
    delete (DbLogger*) dbLId;

class DefaultSetupTest {
public:
  static bool test() {
    runTest(testDefaultSetup);
    return true;
  }
private:
  static bool testDefaultSetup() {
    DEFAULT_SETUP(ce,db,schema,false);
    
    assert(db.isClosed() == false);
    db.close();
    assert(db.isClosed() == true);

    DEFAULT_TEARDOWN();
    return true;
  }
};

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
    runTest(testRateConstraintViolation);
    runTest(testLowerTotalProductionExceededViolation);
    runTest(testLowerTotalConsumptionExceededViolation);
    runTest(testLowerProductionRateExceededViolation);
    runTest(testUpperLimitExceededViolation);
    runTest(testSummationConstraintViolation);
    return true;
  }
    
private:
  
  static bool testResourceConstructionAndDestruction()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource (db.getId(), LabelStr("AllObjects"), LabelStr("r1")))->getId();
    std::list<InstantId> instants;
    r->getInstants(instants);
    assert(instants.size() == 2);
    InstantId id = instants.front();
    assert(id->getTime() == -LATEST_TIME);
    id = instants.back();
    assert(id->getTime() == LATEST_TIME);

    // Construction with argument setting
    ResourceId rid = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r2"), 189.34, 0, 1000))->getId();
    instants.clear();
    rid->getInstants(instants);
    assert(instants.size() == 2);
    id = instants.front();
    assert(id->getTime() == 0);
    id = instants.back();
    assert(id->getTime() == 1000);
    assert(rid->getHorizonStart() == 0);
    assert(rid->getHorizonEnd() == 1000);

    db.close();
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testBasicTransactionInsertion()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), 10, 0, 1000))->getId();

    //just another resource so that the resource doesnt get bound to singleton and get autoinserted by the propagator
    ResourceId r2 = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r2"), 10, 0, 2000))->getId();

    assert(!r.isNoId() && !r2.isNoId() && r != r2);

    db.close();

    // Test insertion of transaction constructed with defaults
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change")))->getId();
    bool prop = ce.propagate();
    assert(prop);
    r->constrain(t1);
    ce.propagate();

    std::list<TransactionId> transactions;
    r->getTransactions(transactions);
    assert(transactions.size() == 1);
    r->free(t1);

    transactions.clear();
    r->getTransactions(transactions);
    assert(transactions.empty());

    // Test insertion of t that is outside the horizon of the resource      
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1001, 2000)))->getId();
    bool res = t2->getObject()->getDerivedDomain().intersects(Domain<ResourceId>(r));
    assert( !res);

    // Test double insertion 
    r->constrain(t1);
    ce.propagate();
    r->free(t1);
    r->constrain(t1);
    ce.propagate();
    r->free(t1);

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTransactionChangeHandling()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), 0, 0, 1000))->getId();
    db.close();

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, LATEST_TIME), 45, 45))->getId();
    r->constrain(t1);
    ce.propagate();
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, LATEST_TIME), 35, 35))->getId();
    r->constrain(t2);
    ce.propagate();
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, LATEST_TIME), 20, 20))->getId();
    r->constrain(t3);
    ce.propagate();
    assert(checkLevelArea(r) == (1*45 + 1*80 + 998*100));

    t2->setEarliest(2);
    assert(checkLevelArea(r) == (1*45 + 1*45 + 998*100));
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testCorrectTransactionAllocation()
  {
    // Test that the right insertion behaviour (in terms of instants) is occuring
    DEFAULT_SETUP(ce,db,schema,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), 0, 1, 7))->getId();
    db.close();

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 6)))->getId();
    r->constrain(t1);
    ce.propagate();
    assert(checkSum(r) == (1*0 + 2*1 + 3*1 + 4*0)); 

    TransactionId t2  = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(-4, 10)))->getId();
    r->constrain(t2);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*2 + 3*2 + 4*1)); 

    TransactionId t3  = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 3)))->getId();
    r->constrain(t3);
    ce.propagate();
    assert(checkSum(r) == (1*2 + 2*2 + 3*2 + 4*2 + 5*1)); 

    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 2)))->getId();
    r->constrain(t4);
    ce.propagate();
    assert(checkSum(r) == (1*3 + 2*3 + 3*2 + 4*2 + 5*2 + 6*1)); 

    TransactionId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 7)))->getId();
    r->constrain(t5);
    ce.propagate();
    assert(checkSum(r) == (1*3 + 2*3 + 3*3 + 4*3 + 5*3 + 6*2)); 

    TransactionId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 7)))->getId();
    r->constrain(t6);
    ce.propagate();
    assert(checkSum(r) == (1*3 + 2*3 + 3*3 + 4*4 + 5*4 + 6*3)); 

    // Insert for a singleton value
    TransactionId t7 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5,5)))->getId();
    r->constrain(t7);
    ce.propagate();
    assert(checkSum(r) == (1*3 + 2*3 + 3*3 + 4*4 + 5*5 + 6*4 + 7*3));

    // Confirm transaction counts
    std::list<TransactionId> transactions;
    r->getTransactions(transactions);
    assert(transactions.size() == 7);

    // Now do the removal and ensure correctness along the way
    r->free(t7);
    assert(checkSum(r) == (1*3 + 2*3 + 3*3 + 4*4 + 5*4 + 6*3)); 
    r->free(t6);
    assert(checkSum(r) == (1*3 + 2*3 + 3*3 + 4*3 + 5*3 + 6*2)); 
    r->free(t5);
    assert(checkSum(r) == (1*3 + 2*3 + 3*2 + 4*2 + 5*2 + 6*1)); 
    r->free(t4);
    assert(checkSum(r) == (1*2 + 2*2 + 3*2 + 4*2 + 5*1)); 
    r->free(t3);
    assert(checkSum(r) == (1*1 + 2*2 + 3*2 + 4*1)); 
    r->free(t2);
    assert(checkSum(r) == (1*0 + 2*1 + 3*1 + 4*0)); 
    r->free(t1);
    assert(checkSum(r) == (1*0 + 2*0)); 

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLevelCalculation()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), 1, 1))->getId();
    r->constrain(t1);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*1 +3*0)); 
    assert(checkLevelArea(r) == 1);

    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 3), -4, -4))->getId();
    r->constrain(t2);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*2 +3*1 +4*0)); 
    assert(checkLevelArea(r) == (1 + 4*2));

    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 4), 8, 8))->getId();
    r->constrain(t3);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*2 + 3*2 + 4*2 + 5*1 + 5*0)); 
    assert(checkLevelArea(r) == (1*1 + 4*1 + 12*1 + 8*1));

    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 6), 2, 2))->getId();
    r->constrain(t4);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*2 +3*2 + 4*3 + 5*2 + 6*1 + 7*0));
    assert(checkLevelArea(r) == (1*1 + 4*1 + 12*1 + 10*1 + 2*2));
 
    TransactionId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 10), -6, -6))->getId();  
    r->constrain(t5);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*2 +3*3 + 4*4 + 5*3 + 6*2 + 7*1));
    assert(checkLevelArea(r) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 6*4));

    TransactionId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 8), 3, 3))->getId();
    r->constrain(t6);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*2 +3*3 + 4*4 + 5*3 + 6*3 + 7*2 + 8*1));
    assert(checkLevelArea(r) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*2 + 6*2));

    TransactionId t7 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(7, 8), -4, -4))->getId();
    r->constrain(t7);
    ce.propagate();
    assert(checkSum(r) == (1*1 + 2*2 +3*3 + 4*4 + 5*3 + 6*3 + 7*3 + 8*3 + 9*1));
    assert(checkLevelArea(r) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*1 + 13*1 + 6*2));

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionUpdates()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 10), 10, 10))->getId();
    r->constrain(t1);
    ce.propagate();
    assert(checkLevelArea(r) == 10*10);

    t1->setEarliest(1);
    assert(checkLevelArea(r) == 10*9);

    t1->setLatest(6);
    assert(checkLevelArea(r) == 10*5);

    // Now try some relaxations
    t1->setLatest(8);
    assert(checkLevelArea(r) == 10*7);

    t1->setMin(-4);
    t1->setMax(1);
    ce.propagate();
    assert(checkLevelArea(r) == 5*7 + 5*2);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionRemoval()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    // Insertion and removal at extremes
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 0), 10, 10))->getId();
    r->constrain(t1);
    ce.propagate();
    r->free(t1);
    ce.propagate();
    r->constrain(t1);
    ce.propagate();

    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(10, 10), 10, 10))->getId();
    r->constrain(t2);
    ce.propagate();
    r->free(t2);
    ce.propagate();
    r->constrain(t2);
    ce.propagate();

    // Insertion and removal to create and delete an instant
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5, 5), 10, 10))->getId();
    r->constrain(t3);
    ce.propagate();
    r->free(t3);
    ce.propagate();
    r->constrain(t3);
    ce.propagate();

    // Insertion of overlapping spanning transactions, all internal
    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 8), 10, 10))->getId();
    r->constrain(t4);
    ce.propagate();
    TransactionId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 9), 10, 10))->getId();
    r->constrain(t5);
    ce.propagate();
    TransactionId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 9), 10, 10))->getId();
    r->constrain(t6);
    ce.propagate();

    // Remove transactions in spurious order
    r->free(t4);
    r->free(t1);
    r->free(t3);
    r->free(t6);
    r->free(t2);
    r->free(t5);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testIntervalCapacityValues()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), 0, 0, 10))->getId();
    db.close();

    // Test producer
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 10), 5, 10))->getId();
    r->constrain(t1);
    ce.propagate();
    assert(checkLevelArea(r) == 10*10);


    // This tests a transaction that could be a producer or a consumer. We don't know yet!
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("dontknowyet"), IntervalIntDomain(4, 8), -4, 3))->getId();
    r->constrain(t2);
    ce.propagate();
    assert(checkLevelArea(r) == 10*4 + 17*4 + 17*2);

    // Test consumer
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(1, 5), -4, -1))->getId();
    r->constrain(t3);
    ce.propagate();
    assert(checkLevelArea(r) == 10*1 + 14*3 + 21*1 + 20*3 + 20*2);
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testConstraintCheckOnInsertion()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), initialCapacity, horizonStart, horizonEnd, 
				 limitMax, limitMin, productionRateMax, productionMax, consumptionRateMax, consumptionMax))->getId();
    db.close();

    // Make sure that it will reject a transaction that violates the spec up front
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), productionRateMax + 1, productionRateMax + 1))->getId();
    assert(r->getTokens().count(t1) == 0);
    //    r->constrain(t1);
    //assert(ce.provenInconsistent());
    //r->free(t1);
    //assert(!ce.provenInconsistent());    

    t1->setMin(productionRateMax);
    r->constrain(t1);
    assert(!ce.provenInconsistent());    
    r->free(t1);

    // Make sure that it will reject a transaction that violates the spec up front
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), consumptionRateMax - 1, consumptionRateMax - 1))->getId();
    assert(r->getTokens().count(t2) == 0);
    /*
      r->constrain(t2);
      assert(ce.provenInconsistent());
      r->free(t2);
      assert(!ce.provenInconsistent());    
    */

    t2->setMax(consumptionRateMax);
    r->constrain(t2);
    assert(!ce.provenInconsistent());    
    r->free(t2);

    DEFAULT_TEARDOWN();
    return(true);
  }

  // Test that a violation can be detected if a concurrent transaction violates a rate constraint
  static bool testRateConstraintViolation()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    std::list<InstantId> allInstants;
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), initialCapacity, horizonStart, horizonEnd, 
				 limitMax, limitMin, productionRateMax, productionMax, consumptionRateMax, consumptionMax))->getId();
    db.close();

    std::list<ViolationId> violations;

    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), productionRateMax, productionRateMax + 1))->getId();
    r->constrain(t1);
    ce.propagate();
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), 1, 1))->getId();
    r->constrain(t3);
    assert(!ce.propagate());

    r->getViolations(violations);
    assert(violations.size() == 1);
    assert(violations.front()->getType() == Violation::ProductionRateExceeded);
    r->free(t1);
    r->free(t3);
    assert(ce.propagate());

    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), consumptionRateMax -1, consumptionRateMax))->getId();
    r->constrain(t2);
    ce.propagate();
    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(0, 1), -1, -1))->getId();
    r->constrain(t4);
    assert(!ce.propagate());

    violations.clear();
    r->getViolations(violations);
    assert(violations.size() == 1);
    assert(violations.front()->getType() == Violation::ConsumptionRateExceeded);
    r->free(t2);
    r->free(t4);
    assert(ce.propagate());
      
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalProductionExceededViolation()
  {
    // Define input constrains for the resource spec

    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), initialCapacity, horizonStart, horizonEnd, 
				 limitMax, limitMin, productionMax, productionMax, MINUS_INFINITY, MINUS_INFINITY))->getId();
    db.close();

    std::list<ViolationId> violations;

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 2), -8, -8))->getId();
    r->constrain(t1);
    assert(ce.propagate());
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 3), -8, -8))->getId();
    r->constrain(t2);
    assert(ce.propagate());    
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 4), -8, -8))->getId();
    r->constrain(t3);
    assert(ce.propagate());
    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5, 5), -8, -8))->getId();
    r->constrain(t4);
    assert(ce.propagate());
    TransactionId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 6), -8, -8))->getId();
    r->constrain(t5);
    assert(ce.propagate());
    // This will push it over the edge
    TransactionId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(10, 10), -8, -8))->getId(); 
    r->constrain(t6);
    assert(!ce.propagate());

    assert(checkLevelArea(r) == 0);
    r->getViolations(violations);
    assert(violations.front()->getType() == Violation::LevelTooLow);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalConsumptionExceededViolation()
  {
    // Define input constrains for the resource spec

    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), initialCapacity, horizonStart, horizonEnd, 
				 limitMax, limitMin, PLUS_INFINITY, PLUS_INFINITY, consumptionMax, consumptionMax))->getId();
    db.close();

    std::list<ViolationId> violations;

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 2), -8, -8))->getId();
    r->constrain(t1);
    assert(ce.propagate());
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 3), -8, -8))->getId();
    r->constrain(t2);
    assert(ce.propagate());    
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 4), -8, -8))->getId();
    r->constrain(t3);
    assert(ce.propagate());
    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5, 5), -8, -8))->getId();
    r->constrain(t4);
    assert(ce.propagate());
    TransactionId t5 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(6, 6), -8, -8))->getId();
    r->constrain(t5);
    assert(ce.propagate());
    TransactionId t6 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(8, 8), -8, -8))->getId(); 
    r->constrain(t6);
    assert(ce.propagate());
    // This will push it over the edge
    TransactionId t7 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(10, 10), -8, -8))->getId(); 
    r->constrain(t7);
    assert(!ce.propagate());


    assert(checkLevelArea(r) == 0);
    r->getViolations(violations);
    assert(violations.front()->getType() == Violation::ConsumptionSumExceeded);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerProductionRateExceededViolation()
  {
    // Define input constrains for the resource spec

    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), initialCapacity, horizonStart, horizonEnd, 
				 limitMax, limitMin, productionRateMax, productionMax, consumptionRateMax, consumptionMax))->getId();
    db.close();

    std::list<ViolationId> violations;

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    TransactionId t1 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(2, 2), -8, -8))->getId();
    r->constrain(t1);
    assert(ce.propagate());
    TransactionId t2 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(3, 3), -8, -8))->getId();
    r->constrain(t2);
    assert(ce.propagate());    
    TransactionId t3 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(4, 4), -8, -8))->getId();
    r->constrain(t3);
    assert(ce.propagate());
    TransactionId t4 = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(5, 5), -8, -8))->getId();
    r->constrain(t4);
    assert(!ce.propagate());

    assert(checkLevelArea(r) == 0);
    r->getViolations(violations);
    assert(violations.front()->getType() == Violation::LevelTooLow);

    DEFAULT_TEARDOWN();
    return(true);
  }


  static bool testUpperLimitExceededViolation()
  {
    // Define input constrains for the resource spec
    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), initialCapacity + 1, horizonStart, horizonEnd, 
				 limitMax, limitMin, productionRateMax, productionMax + 100, consumptionRateMax, consumptionMax))->getId();
    db.close();


    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // consumption
    std::list<TransactionId> transactions;
    for (int i = 0; i < 11; i++){
      TransactionId t = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(i, i), productionRateMax, productionRateMax))->getId();
      r->constrain(t);
      ce.propagate();
      transactions.push_back(t);
    }

    assert(checkLevelArea(r) == 0);

    std::list<ViolationId> violations;
    r->getViolations(violations);
    assert(violations.size() == 1);
    assert(violations.front()->getType() == Violation::LevelTooHigh);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testSummationConstraintViolation()
  {
    DEFAULT_SETUP(ce,db,schema,false);
    
    ResourceId r = (new Resource(db.getId(), LabelStr("AllObjects"), LabelStr("r1"), initialCapacity, horizonStart, horizonEnd, 
				 limitMax, limitMin, productionRateMax, productionMax, consumptionRateMax, consumptionMax))->getId();
    db.close();

    // Set up constraints so that all rate and level constraints are OK - balanced consumption
    // and production
    std::list<TransactionId> transactions;
    for (int i = 0; i < 11; i++){
      TransactionId t = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(i, i), productionRateMax, productionRateMax))->getId();
      r->constrain(t);
      transactions.push_back(t);
    }
    for (int i = 0; i < 11; i++){
      TransactionId t = (new Transaction(db.getId(), LabelStr("Resource.change"), IntervalIntDomain(i, i), -productionRateMax, -productionRateMax))->getId();
      r->constrain(t);
      transactions.push_back(t);
    }

    ce.propagate();
    assert(checkLevelArea(r) == 0);

    // Ensure the violations remain unchanged
    std::list<ViolationId> violations;     
    r->getViolations(violations);
    int times[4] = {8,9,10,10}; int i = 0;
    for(std::list<ViolationId>::iterator it = violations.begin(); it != violations.end(); ++it){
      assert((*it)->getInstant()->getTime() == times[i]);
      i++;
    }	
    assert(violations.size() == 4);
    assert(violations.front()->getType() == Violation::ProductionSumExceeded);
    assert(violations.back()->getType() == Violation::ConsumptionSumExceeded);

    DEFAULT_TEARDOWN();
    return(true);
  }


  /* Utility methods for testing */

  /**
   * Sums the instances of transactions in each instant.
   */
  static int checkSum(ResourceId r) {
    std::list<InstantId> allInstants;
    r->getInstants(allInstants);
    int sum = 0;
    int i = 1;
    std::list<InstantId>::iterator it = allInstants.begin();
    // std::cout << "        Transactions  ";
    while (it != allInstants.end()) {
      InstantId current = *it;
      // std::cout <<  current->getTime() << ":[" << current->getTransactionCount() << "] "; 
      sum += i++ * current->getTransactionCount();
      it++;
    }
    return(sum);
  }

  /**
   * Sums the instances of transactions in each instant.
  */
  static double checkLevelArea(ResourceId r) {
    assert(r != ResourceId::noId());
    r->updateTransactionProfile();
    double area = 0;
    InstantId current = r->getProfileHead();
    while(current != r->getProfileTail()){
      area += ((current->getLevelMax() - current->getLevelMin()) * (current->getNext()->getTime() - current->getTime()));
      current = current->getNext();
    }

    // std::cout << "        Level      ";
    // r->print(std::cout);
    return area;
  }

  static void printViolations(std::list<ViolationId>& violations){
    for(std::list<ViolationId>::iterator it = violations.begin(); it != violations.end(); ++it){
      (*it)->print(std::cout);
      std::cout << std::endl;
    }
  }

};


int main() {

  initConstraintLibrary();

  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_NARY(ResourceConstraint, "ResourceRelation", "Resource");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_NARY(ResourceTransactionConstraint, "HorizonRelation", "Default");
  runTestSuite(DefaultSetupTest::test);
  runTestSuite(ResourceTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
}
