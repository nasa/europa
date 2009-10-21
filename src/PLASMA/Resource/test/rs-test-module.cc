#include "rs-test-module.hh"
#include "Profile.hh"
#include "FVDetector.hh"
#include "Instant.hh"
#include "Transaction.hh"
#include "TimetableProfile.hh"
#include "OpenWorldFVDetector.hh"
#include "Resource.hh"
#include "ProfilePropagator.hh"
#include "Reservoir.hh"
#include "InstantTokens.hh"
#include "Reusable.hh"
#include "DurativeTokens.hh"

#include "Utils.hh"
#include "Domains.hh"
#include "Propagators.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "Debug.hh"

#include "ThreatDecisionPoint.hh"
#include "Profile.hh"
#include "FlowProfile.hh"
#include "IncrementalFlowProfile.hh"
#include "Reusable.hh"
#include "FVDetector.hh"
#include "ClosedWorldFVDetector.hh"
#include "DurativeTokens.hh"
#include "ResourceThreatDecisionPoint.hh"
#include "ResourceThreatManager.hh"
#include "ProfilePropagator.hh"
#include "ResourceMatching.hh"

#include "Engine.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleSolvers.hh"
#include "ModuleResource.hh"
#include "ModuleNddl.hh"

#include <iostream>
#include <string>
#include <list>

using namespace EUROPA;

class ResourceTestEngine  : public EngineBase
{
  public:
	ResourceTestEngine();
	virtual ~ResourceTestEngine();

  protected:
	void createModules();
};

ResourceTestEngine::ResourceTestEngine()
{
    createModules();
    doStart();
    Schema* schema = (Schema*)getComponent("Schema");
    schema->addObjectType("Resource");
}

ResourceTestEngine::~ResourceTestEngine()
{
    doShutdown();
}

void ResourceTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
    addModule((new ModuleRulesEngine())->getId());
    addModule((new ModuleTemporalNetwork())->getId());
    addModule((new ModuleSolvers())->getId());
    addModule((new ModuleResource())->getId());
    addModule((new ModuleNddl())->getId());
}

// Useful constants when doing constraint vio9lation tests
const double initialCapacity = 5;
const double limitMin = 0;
const double limitMax = 10;
const double productionRateMax = 5;
const double productionMax = 40;
const double consumptionRateMax = -8;
const double consumptionMax = -50;

#define RESOURCE_DEFAULT_SETUP(ce, db, autoClose) \
    ResourceTestEngine rte; \
    ConstraintEngine& ce = *((ConstraintEngine*)rte.getComponent("ConstraintEngine")); \
    PlanDatabase& db = *((PlanDatabase*)rte.getComponent("PlanDatabase")); \
    if (autoClose) \
      db.close();

#define RESOURCE_DEFAULT_TEARDOWN()

class DefaultSetupTest {
public:
  static bool test() {
    EUROPA_runTest(testDefaultSetup);
    return true;
  }
private:
  static bool testDefaultSetup() {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

	ce.getId(); // Only point of this is to remove a compile-time warning (unused-variable)

    CPPUNIT_ASSERT(db.isClosed() == false);
    db.close();
    CPPUNIT_ASSERT(db.isClosed() == true);

    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }
};

class DummyProfile : public Profile {
public:
  DummyProfile(PlanDatabaseId db, const FVDetectorId fv)
    : Profile(db, fv), m_receivedNotification(0) {}
  InstantId getInstant(const int time) {
    return getGreatestInstant(time)->second;
  }
  void getTransactionsToOrder(const InstantId& inst, std::vector<TransactionId>& results) {
    check_error(inst.isValid());
    check_error(results.empty());
    results.insert(results.end(), inst->getTransactions().begin(), inst->getTransactions().end());
  }
  int gotNotified(){return m_receivedNotification;}
  void resetNotified(){m_receivedNotification = 0;}
private:
  void handleTemporalConstraintAdded(const TransactionId predecessor, int preArgIndex,
				     const TransactionId successor, int sucArgIndex) {
    Profile::handleTemporalConstraintAdded(predecessor, preArgIndex, successor, sucArgIndex);
    m_receivedNotification++;
  }
  void handleTemporalConstraintRemoved(const TransactionId predecessor, int preArgIndex,
				       const TransactionId successor, int sucArgIndex) {
    Profile::handleTemporalConstraintRemoved(predecessor, preArgIndex, successor, sucArgIndex);
    m_receivedNotification++;
  }
  void initRecompute(InstantId inst){}
  void initRecompute(){}
  void recomputeLevels(InstantId prev, InstantId inst) {
  }
  int m_receivedNotification;
};

class DummyDetector : public FVDetector {
public:
  DummyDetector(const ResourceId res) : FVDetector(res) {};
  bool detect(const InstantId inst) {return false;}
  void initialize(const InstantId inst) {}
  void initialize() {}
};


class DummyResource : public Resource {
public:
  DummyResource(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name,
		double initCapacityLb = 0, double initCapacityUb = 0, double lowerLimit = MINUS_INFINITY,
		double upperLimit = PLUS_INFINITY, double maxInstProduction = PLUS_INFINITY, double maxInstConsumption = PLUS_INFINITY,
		double maxProduction = PLUS_INFINITY, double maxConsumption = PLUS_INFINITY)
    : Resource(planDatabase, type, name, LabelStr("OpenWorldFVDetector"), LabelStr("TimetableProfile"), initCapacityLb, initCapacityUb,
		     lowerLimit, upperLimit, maxInstProduction, maxInstConsumption,
		     maxProduction, maxConsumption) {}

  void addTransaction(const TransactionId trans) {
    m_profile->addTransaction(trans);
    //m_profile->recompute();
  }
  void removeTransaction(const TransactionId trans) {
    m_profile->removeTransaction(trans);
    // m_profile->recompute();
  }
  void addToProfile(const TokenId& token) {}
  void removeFromProfile(const TokenId& token) {}
  void createTransactions(const TokenId& token) {}
  void removeTransactions(const TokenId& token) {}
private:
  void notifyViolated(const InstantId inst) {
    TransactionId trans = *(inst->getTransactions().begin());
    const_cast<AbstractDomain&>(trans->time()->lastDomain()).empty();
  }

  //no implementation.  no tests for flaw detection
  void notifyFlawed(const InstantId inst) {
  }

  void notifyDeleted(const InstantId inst) {}
  void notifyNoLongerFlawed(const InstantId inst){}
};
/**
   add tests for getClosedTransactions and getPendingTransactions!
 */
class ProfileTest {
public:
  static bool test() {
    EUROPA_runTest(testAdditionRemovalAndIteration);
    EUROPA_runTest(testRestrictionAndRelaxation);
    //converted from old timetable profile tests
    EUROPA_runTest(testTransactionChangeHandling);
    EUROPA_runTest(testCorrectTransactionAllocation);
    EUROPA_runTest(testLevelCalculation);
    EUROPA_runTest(testTransactionUpdates);
    //testTransactionRemoval only relevent for tokens--use to test reservoir
    EUROPA_runTest(testIntervalCapacityValues);
    //violation tests
    EUROPA_runTest(testRateConstraintViolation);
    EUROPA_runTest(testLowerTotalProductionExceededResourceViolation);
    EUROPA_runTest(testLowerTotalConsumptionExceededResourceViolation);
    EUROPA_runTest(testUpperLimitExceededResourceViolation);
    EUROPA_runTest(testSummationConstraintResourceViolation);
    //other tests
    EUROPA_runTest(testPointProfileQueries);
    EUROPA_runTest(testGnats3244);
    return true;
  }
private:
  static bool checkTimes(const std::set<int>& times, ProfileIterator& profIt) {
    CPPUNIT_ASSERT(!profIt.done());
    CPPUNIT_ASSERT(!times.empty());

    int retval = 0;
    std::set<int>::const_iterator it = times.begin();
    while(!profIt.done()) {
      debugMsg("ResourceTest:checkTimes", *it << " " << profIt.getTime());
      CPPUNIT_ASSERT(profIt.getInstant()->getTime() == *it);
      profIt.next();
      ++it;
      ++retval;
    }
    CPPUNIT_ASSERT((unsigned) retval == times.size());
    return true;
  }

  static double checkLevelArea(ProfileId prof) {
    prof->recompute();
    double area = 0;
    ProfileIterator it(prof);
    if(it.done()) {
      debugMsg("ResourceTest:checkLevelArea", "No Instants.  Returning 0.");
      return 0;
    }
    InstantId current = it.getInstant();
    it.next();
    while(!it.done()) {
      InstantId next = it.getInstant();
      debugMsg("ResourceTest:checkLevelArea", "Current(time: " << current->getTime() << " lowerLevel: " << current->getLowerLevel() <<
	       " upperLevel: " << current->getUpperLevel() << ") Next time: " << next->getTime());
      area += ((current->getUpperLevel() - current->getLowerLevel()) * (next->getTime() - current->getTime()));
      current = next;
      it.next();
    }
    debugMsg("ResourceTest:checkLevelArea", "Returning " << area);
    return area;
  }

  static int checkSum(ProfileId prof) {
    prof->recompute();
    int sum = 0;
    int count = 1;
    debugMsg("ResourceTest:checkSum", "        Transactions");
    ProfileIterator it(prof);
    while(!it.done()) {
      InstantId inst = it.getInstant();
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
    DummyDetector detector(ResourceId::noId());
    DummyProfile profile(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 0));
    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(10, 10));
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(0, 3));
    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(1, 4));
    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(4, 6));
    Variable<IntervalDomain> quantity(ce.getId(), IntervalDomain(3, 3));

    std::set<int> times;

    Transaction trans1(t1.getId(), quantity.getId(), false);
    Transaction trans2(t2.getId(), quantity.getId(), false); //distinct cases
    Transaction trans3(t3.getId(), quantity.getId(), false); //overlap on left side.  both trans1 and trans2 should appear at time 0
    Transaction trans4(t4.getId(), quantity.getId(), false); //overlap in the middle.  should have trans3 and trans4 at time 1 and only trans4 at time 4
    Transaction trans5(t5.getId(), quantity.getId(), false); //overlap on right side.  both trans 4 and trans5 should appear at time 4

    profile.addTransaction(trans1.getId());
    times.insert(0);
    ProfileIterator prof1(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof1));

    profile.addTransaction(trans2.getId());
    times.insert(10);
    ProfileIterator prof2(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof2));

    profile.addTransaction(trans3.getId());
    times.insert(3);
    ProfileIterator prof3(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof3));
    ProfileIterator oCheck1(profile.getId(), 0, 0);
    CPPUNIT_ASSERT(!oCheck1.done());
    CPPUNIT_ASSERT(oCheck1.getInstant()->getTransactions().size() == 2);

    profile.addTransaction(trans4.getId());
    times.insert(1);
    times.insert(4);
    ProfileIterator prof4(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof4));
    ProfileIterator oCheck2(profile.getId(), 3, 3);
    CPPUNIT_ASSERT(!oCheck2.done());
    CPPUNIT_ASSERT(oCheck2.getInstant()->getTransactions().size() == 2);

    profile.addTransaction(trans5.getId());
    times.insert(6);
    ProfileIterator prof5(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof5));
    ProfileIterator oCheck3(profile.getId(), 4, 4);
    CPPUNIT_ASSERT(!oCheck3.done());
    CPPUNIT_ASSERT(oCheck3.getInstant()->getTransactions().size() == 2);

    profile.removeTransaction(trans4.getId());
    times.erase(1);
    ProfileIterator prof6(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof6));
    ProfileIterator oCheck4(profile.getId(), 3, 3);
    CPPUNIT_ASSERT(oCheck4.getInstant()->getTransactions().size() == 1);
    ProfileIterator oCheck5(profile.getId(), 4, 4);
    CPPUNIT_ASSERT(oCheck5.getInstant()->getTransactions().size() == 1);

    profile.removeTransaction(trans1.getId());
    profile.removeTransaction(trans2.getId());
    times.erase(10);
    ProfileIterator prof7(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof7));
    ProfileIterator oCheck6(profile.getId(), 0, 0);
    CPPUNIT_ASSERT(oCheck6.getInstant()->getTransactions().size() == 1);

    profile.removeTransaction(trans3.getId());
    profile.removeTransaction(trans5.getId());

    ProfileIterator prof8(profile.getId());
    CPPUNIT_ASSERT(prof8.done());

    return true;
  }

  static bool testRestrictionAndRelaxation() {
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(ResourceId::noId());
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

    Transaction trans1(t1.getId(), quantity.getId(), false);
    Transaction trans2(t2.getId(), quantity.getId(), false);
    Transaction trans3(t3.getId(), quantity.getId(), false);
    Transaction trans4(t4.getId(), quantity.getId(), false);
    Transaction trans5(t5.getId(), quantity.getId(), false);

    profile.addTransaction(trans1.getId());
    profile.addTransaction(trans2.getId());
    profile.addTransaction(trans3.getId());
    profile.addTransaction(trans4.getId());
    profile.addTransaction(trans5.getId());

    ProfileIterator baseTest(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, baseTest));

    //should remove the instant at 3 and add an instant at 2.  the instant at 2 should have two transactions (t3 and t4).
    times.erase(3);
    times.insert(2);
    const_cast<AbstractDomain&>(t3.lastDomain()).intersect(0, 2);
    ProfileIterator prof1(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof1));
    ProfileIterator oCheck1(profile.getId(), 2, 2);
    CPPUNIT_ASSERT(oCheck1.getInstant()->getTransactions().size() == 2);
    CPPUNIT_ASSERT(oCheck1.getInstant()->getTransactions().find(trans3.getId()) != oCheck1.getInstant()->getTransactions().end());
    CPPUNIT_ASSERT(oCheck1.getInstant()->getTransactions().find(trans4.getId()) != oCheck1.getInstant()->getTransactions().end());
    ProfileIterator oCheck1_1(profile.getId(), 3, 3);
    CPPUNIT_ASSERT(oCheck1_1.done());

    times.erase(2);
    times.insert(3);
    t3.reset();
    ProfileIterator prof2(profile.getId());
    CPPUNIT_ASSERT(checkTimes(times, prof2));
    ProfileIterator oCheck2(profile.getId(), 3, 3);
    CPPUNIT_ASSERT(oCheck2.getInstant()->getTransactions().size() == 2);
    ProfileIterator oCheck2_1(profile.getId(), 2, 2);
    CPPUNIT_ASSERT(oCheck2_1.done());


    CPPUNIT_ASSERT(profile.gotNotified() == 0);
    ConstraintId constr = db.getClient()->createConstraint("precedes", makeScope(t1.getId(), t2.getId()));
    CPPUNIT_ASSERT(profile.gotNotified() == 1);
    profile.resetNotified();
    constr->discard();
    CPPUNIT_ASSERT(profile.gotNotified() == 1);

    profile.resetNotified();
    Variable<IntervalIntDomain> temp(ce.getId(), IntervalIntDomain(-1, -1));
    ConstraintId constr2 = db.getClient()->createConstraint("precedes", makeScope(temp.getId(), t1.getId()));
    CPPUNIT_ASSERT(profile.gotNotified() == 0);
    constr2->discard();
    CPPUNIT_ASSERT(profile.gotNotified() == 0);
    return true;
  }

  static bool testTransactionChangeHandling()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    const int HORIZON_END = 1000;

    DummyDetector detector(ResourceId::noId());
    TimetableProfile r(db.getId(), detector.getId());

//     Variable<IntervalIntDomain> t0(ce.getId(), IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
//     Variable<IntervalDomain> q0(ce.getId(), IntervalDomain(0, 0));
//     Transaction trans0(t0.getId(), q0.getId(), false);
//     r.addTransaction(trans0.getId());

    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 0);

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, HORIZON_END));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(45, 45));
    Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    CPPUNIT_ASSERT(ce.propagate());
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 1000 * 45);

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(1, HORIZON_END));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(35, 35));
    Transaction trans2(t2.getId(), q2.getId(), false);
    r.addTransaction(trans2.getId());
    CPPUNIT_ASSERT(ce.propagate());
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*2));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*45 + 80*999));

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(2, HORIZON_END));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(20, 20));
    Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());
    CPPUNIT_ASSERT(ce.propagate());
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*3));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*45 + 1*80 + 998*100));

    trans2.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans2.time()->lastDomain().getUpperBound()));
    CPPUNIT_ASSERT(ce.propagate());
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*3));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*45 + 1*80 + 998*100));

    trans2.time()->restrictBaseDomain(IntervalIntDomain(2, (int)trans2.time()->lastDomain().getUpperBound()));
    CPPUNIT_ASSERT(ce.propagate());
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*3 + 3*3));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (2*45 + 998*100));

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testCorrectTransactionAllocation()
  {
    // Test that the right insertion behaviour (in terms of instants) is occuring
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyDetector detector(ResourceId::noId());
    TimetableProfile r(db.getId(), detector.getId());

    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == 0);

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(4, 6));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*1));

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(-4, 10));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    Transaction trans2(t2.getId(), q2.getId(), false);
    r.addTransaction(trans2.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*2 + 3*2 + 4*1));

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(1, 3));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*2 +3*2 + 4*2 + 5*2 + 6*1));

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(1, 2));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    Transaction trans4(t4.getId(), q4.getId(), false);
    r.addTransaction(trans4.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*2 + 5*2 + 6*2 + 7*1));

    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(3, 7));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    Transaction trans5(t5.getId(), q5.getId(), false);
    r.addTransaction(trans5.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*3 + 5*3 + 6*3 + 7*2 + 8*1));

    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(4, 7));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    Transaction trans6(t6.getId(), q6.getId(), false);
    r.addTransaction(trans6.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*4 + 7*3 + 8*1));

    // Insert for a singleton value
    Variable<IntervalIntDomain> t7(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q7(ce.getId(), IntervalDomain(0, PLUS_INFINITY));
    Transaction trans7(t7.getId(), q7.getId(), false);
    r.addTransaction(trans7.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*5 + 7*4 + 8*3 + 9*1));

    // Now free them and check the retractions are working correctly

    r.removeTransaction(trans7.getId());
    CPPUNIT_ASSERT(ce.propagate());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*3 + 3*3 + 4*3 + 5*4 + 6*4 + 7*3 + 8*1));
    r.removeTransaction(trans6.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*3 + 3*3 + 4*3 + 5*3 + 6*3 + 7*2 + 8*1));
    r.removeTransaction(trans5.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*3 + 3*3 + 4*2 + 5*2 + 6*2 + 7*1));
    r.removeTransaction(trans4.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*2 +3*2 + 4*2 + 5*2 + 6*1));
    r.removeTransaction(trans3.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*2 + 3*2 + 4*1));
    r.removeTransaction(trans2.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId())  == (1*1 + 2*1));
    r.removeTransaction(trans1.getId());
    CPPUNIT_ASSERT(ce.propagate() && checkSum(r.getId()) == 0);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLevelCalculation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyDetector detector(ResourceId::noId());
    TimetableProfile r(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(1, 1));
    Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 1);

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(1, 3));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(4, 4));
    Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1 + 4*2));

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(2, 4));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(8, 8));
    Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*2 + 4*2 + 5*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*1 + 4*1 + 12*1 + 8*1));

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(3, 6));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(2, 2));
    Transaction trans4(t4.getId(), q4.getId(), false);
    r.addTransaction(trans4.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*2 + 4*3 + 5*2 + 6*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*1 + 4*1 + 12*1 + 10*1 + 2*2));

    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(2, 10));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(6, 6));
    Transaction trans5(t5.getId(), q5.getId(), true);
    r.addTransaction(trans5.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*2 + 7*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 6*4));

    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(6, 8));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(3, 3));
    Transaction trans6(t6.getId(), q6.getId(), false);
    r.addTransaction(trans6.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*3 + 7*2 + 8*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*2 + 6*2));

    Variable<IntervalIntDomain> t7(ce.getId(), IntervalIntDomain(7, 8));
    Variable<IntervalDomain> q7(ce.getId(), IntervalDomain(4, 4));
    Transaction trans7(t7.getId(), q7.getId(), true);
    r.addTransaction(trans7.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkSum(r.getId()) == (1*1 + 2*2 + 3*3 + 4*4 + 5*3 + 6*3 + +7* 3 + 8*3 + 9*1));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == (1*1 + 4*1 + 18*1 + 16*1 + 8*2 + 9*1 + 13*1 + 6*2));

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTransactionUpdates()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyDetector detector(ResourceId::noId());
    TimetableProfile r(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 10));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(10, 10));
    Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 10*10);

    trans1.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans1.time()->lastDomain().getUpperBound()));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 10*9);

    trans1.time()->restrictBaseDomain(IntervalIntDomain((int)trans1.time()->lastDomain().getLowerBound(), 8));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 10*7);

    trans1.time()->restrictBaseDomain(IntervalIntDomain((int)trans1.time()->lastDomain().getLowerBound(), 6));
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 10*5);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testIntervalCapacityValues()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyDetector detector(ResourceId::noId());
    TimetableProfile r(db.getId(), detector.getId());


    // Test producer
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 10));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(5, 10));
    Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 10*10);

    // Test consumer
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(1, 5));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(1, 4));
    Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());
    ce.propagate();
    CPPUNIT_ASSERT(checkLevelArea(r.getId()) == 10*1 + 14*4 + 13*5);//+ 14*3 + 21*1 + 20*3 + 20*2);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testRateConstraintViolation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyResource r(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax, -(consumptionMax));

    db.close();


    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(productionRateMax, productionRateMax + 1));
    Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());
    ce.propagate();

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(1, 1));
    Transaction trans3(t3.getId(), q3.getId(), false);
    r.addTransaction(trans3.getId());

    // no violation because of temporal flexibility
    CPPUNIT_ASSERT(ce.propagate());

    trans1.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans1.time()->lastDomain().getUpperBound()));
    trans3.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans3.time()->lastDomain().getUpperBound()));
    CPPUNIT_ASSERT(!ce.propagate());

    //r->getResourceViolations(violations);
    //CPPUNIT_ASSERT(violations.size() == 1);
    //CPPUNIT_ASSERT(violations.front()->getType() == ResourceViolation::ProductionRateExceeded);

    r.removeTransaction(trans3.getId());
    r.removeTransaction(trans1.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(-(consumptionRateMax), -(consumptionRateMax - 1)));
    Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    ce.propagate();

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(0, 1));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(1, 1));
    Transaction trans4(t4.getId(), q4.getId(), true);
    r.addTransaction(trans4.getId());
    // no violation because of temporal flexibility
    CPPUNIT_ASSERT(ce.propagate());
    trans2.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans2.time()->lastDomain().getUpperBound()));
    trans4.time()->restrictBaseDomain(IntervalIntDomain(1, (int)trans4.time()->lastDomain().getUpperBound()));
    CPPUNIT_ASSERT(!ce.propagate());

    //violations.clear();
    //r->getResourceViolations(violations);
    //CPPUNIT_ASSERT(violations.size() == 1);
    //CPPUNIT_ASSERT(violations.front()->getType() == ResourceViolation::ConsumptionRateExceeded);
    r.removeTransaction(trans4.getId());
    r.removeTransaction(trans2.getId());
    CPPUNIT_ASSERT(ce.propagate());

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalProductionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyResource r(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax, -(consumptionMax));
    db.close();

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(2, 2));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(8, 8));
    Transaction trans1(t1.getId(), q1.getId(), true);
    r.addTransaction(trans1.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(3, 3));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(8, 8));
    Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(4, 4));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(8, 8));
    Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(8, 8));
    Transaction trans4(t4.getId(), q4.getId(), true);
    r.addTransaction(trans4.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(6, 6));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(8, 8));
    Transaction trans5(t5.getId(), q5.getId(), true);
    r.addTransaction(trans5.getId());
    CPPUNIT_ASSERT(ce.propagate());

    // This will push it over the edge
    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(10, 10));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(8, 8));
    Transaction trans6(t6.getId(), q6.getId(), true);
    r.addTransaction(trans6.getId());
    CPPUNIT_ASSERT(!ce.propagate());

    CPPUNIT_ASSERT(checkLevelArea(r.getProfile()) == 0);
//     r.getResourceViolations(violations);
//     CPPUNIT_ASSERT(violations.front()->getType() == ResourceViolation::LevelTooLow);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testLowerTotalConsumptionExceededResourceViolation()
  {
    // Define input constrains for the resource spec

    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyResource r(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    PLUS_INFINITY, -(consumptionMax), PLUS_INFINITY, -(consumptionMax));
    db.close();

    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // production
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(2, 2));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(8, 8));
    Transaction trans1(t1.getId(), q1.getId(), true);
    r.addTransaction(trans1.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(3, 3));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(8, 8));
    Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(4, 4));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(8, 8));
    Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(8, 8));
    Transaction trans4(t4.getId(), q4.getId(), true);
    r.addTransaction(trans4.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t5(ce.getId(), IntervalIntDomain(6, 6));
    Variable<IntervalDomain> q5(ce.getId(), IntervalDomain(8, 8));
    Transaction trans5(t5.getId(), q5.getId(), true);
    r.addTransaction(trans5.getId());
    CPPUNIT_ASSERT(ce.propagate());

    Variable<IntervalIntDomain> t6(ce.getId(), IntervalIntDomain(8, 8));
    Variable<IntervalDomain> q6(ce.getId(), IntervalDomain(8, 8));
    Transaction trans6(t6.getId(), q6.getId(), true);
    r.addTransaction(trans6.getId());
    CPPUNIT_ASSERT(ce.propagate());

    // This will push it over the edge
    Variable<IntervalIntDomain> t7(ce.getId(), IntervalIntDomain(10, 10));
    Variable<IntervalDomain> q7(ce.getId(), IntervalDomain(8, 8));
    Transaction trans7(t7.getId(), q7.getId(), true);
    r.addTransaction(trans7.getId());
    CPPUNIT_ASSERT(!ce.propagate());

    CPPUNIT_ASSERT(checkLevelArea(r.getProfile()) == 0);
//     r->getResourceViolations(violations);
//     CPPUNIT_ASSERT(!violations.empty());
//     CPPUNIT_ASSERT(violations.front()->getType() == ResourceViolation::ConsumptionSumExceeded);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testUpperLimitExceededResourceViolation()
  {
    // Define input constrains for the resource spec
    RESOURCE_DEFAULT_SETUP(ce,db,false);
    DummyResource r(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity + 1, initialCapacity + 1, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax + 100, -(consumptionMax));
    db.close();


    // Test that a violation is detected when the excess in the level cannot be overcome by remaining
    // consumption
    std::list<TransactionId> transactions;
    std::list<ConstrainedVariableId> vars;
    for (int i = 0; i < 11; i++){
      ConstrainedVariableId time = (new Variable<IntervalIntDomain>(ce.getId(), IntervalIntDomain(i, i)))->getId();
      ConstrainedVariableId quantity = (new Variable<IntervalDomain>(ce.getId(), IntervalDomain(productionRateMax, productionRateMax)))->getId();
      TransactionId t = (new Transaction(time, quantity, false))->getId();
      r.addTransaction(t);
      transactions.push_back(t);
      vars.push_back(quantity);
      vars.push_back(time);
      //r->constrain(t);
      if(i < 10) {
	CPPUNIT_ASSERT(ce.propagate());
      }
      else {
	CPPUNIT_ASSERT(!ce.propagate());
      }
    }

    CPPUNIT_ASSERT(checkLevelArea(r.getProfile()) == 0);

//     std::list<ResourceViolationId> violations;
//     r->getResourceViolations(violations);
//     CPPUNIT_ASSERT(violations.size() == 1);
//     CPPUNIT_ASSERT(violations.front()->getType() == ResourceViolation::LevelTooHigh);

    for(std::list<TransactionId>::iterator it = transactions.begin(); it != transactions.end(); ++it)
      delete (Transaction*) (*it);
    for(std::list<ConstrainedVariableId>::iterator it = vars.begin(); it != vars.end(); ++it)
      delete (ConstrainedVariable*) (*it);

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testSummationConstraintResourceViolation()
  {
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyResource r(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), productionMax, -(consumptionMax));
    db.close();

    // Set up constraints so that all rate and level constraints are OK - balanced consumption
    // and production
    std::list<TransactionId> transactions;
    std::list<ConstrainedVariableId> variables;
    for (int i = 0; i < 11; i++){
      ConstrainedVariableId time = (new Variable<IntervalIntDomain>(ce.getId(), IntervalIntDomain(i, i)))->getId();
      ConstrainedVariableId quantity = (new Variable<IntervalDomain>(ce.getId(), IntervalDomain(productionRateMax, productionRateMax)))->getId();
      TransactionId t = (new Transaction(time, quantity, false))->getId();
      r.addTransaction(t);
      //r->constrain(t);
      transactions.push_back(t);
      variables.push_back(time);
      variables.push_back(quantity);
    }
    for (int i = 0; i < 11; i++){
      ConstrainedVariableId time = (new Variable<IntervalIntDomain>(ce.getId(), IntervalIntDomain(i, i)))->getId();
      ConstrainedVariableId quantity = (new Variable<IntervalDomain>(ce.getId(), IntervalDomain(productionRateMax, productionRateMax)))->getId();
      TransactionId t = (new Transaction(time, quantity, true))->getId();
      r.addTransaction(t);
      //r->constrain(t);
      transactions.push_back(t);
      variables.push_back(time);
      variables.push_back(quantity);
    }

    CPPUNIT_ASSERT(!ce.propagate());
    CPPUNIT_ASSERT(checkLevelArea(r.getProfile()) == 0);

    // Ensure the violations remain unchanged
//     std::list<ResourceViolationId> violations;
//     r->getResourceViolations(violations);
//     CPPUNIT_ASSERT(violations.size() > 0);

    for(std::list<TransactionId>::iterator it = transactions.begin(); it != transactions.end(); ++it)
      delete (Transaction*) (*it);
    for(std::list<ConstrainedVariableId>::iterator it = variables.begin(); it != variables.end(); ++it)
      delete (ConstrainedVariable*) (*it);
    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testPointProfileQueries()
  {
    // Define input constrains for the resource spec
    RESOURCE_DEFAULT_SETUP(ce,db,false);

    DummyResource r(db.getId(), LabelStr("Resource"), LabelStr("r1"), initialCapacity, initialCapacity, limitMin, limitMax,
		    productionRateMax, -(consumptionRateMax), 5, -(consumptionMax));
    db.close();

    IntervalDomain result;
    // Verify correct behaviour for the case with no transactions
    r.getProfile()->getLevel(10, result);
    CPPUNIT_ASSERT(result.isSingleton() && result.getSingletonValue() == initialCapacity);

    // Test that a flaw is signalled when there is a possibility to violate limits
    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(5, 5));
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(5, 5));
    Transaction trans1(t1.getId(), q1.getId(), false);
    r.addTransaction(trans1.getId());

    // Have a single transaction, test before, at and after.
    r.getProfile()->getLevel(0, result);
    CPPUNIT_ASSERT(result.isSingleton() && result.getSingletonValue() == initialCapacity);
    r.getProfile()->getLevel(5, result);
    CPPUNIT_ASSERT(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5));
    r.getProfile()->getLevel(1000, result);
    CPPUNIT_ASSERT(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5));

    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(0, 7));
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(5, 5));
    Transaction trans2(t2.getId(), q2.getId(), true);
    r.addTransaction(trans2.getId());

    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(2, 10));
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(5, 5));
    Transaction trans3(t3.getId(), q3.getId(), true);
    r.addTransaction(trans3.getId());

    // Confirm that we can query in the middle
    r.getProfile()->getLevel(6, result);
    CPPUNIT_ASSERT(result == IntervalDomain(initialCapacity+5-10, initialCapacity+5));

    // Confirm that we can query at the end
    r.getProfile()->getLevel(1000, result);
    CPPUNIT_ASSERT(result.isSingleton() && result.getSingletonValue() == (initialCapacity + 5 - 10));

    // There should be no violations, only flaws
    CPPUNIT_ASSERT(ce.propagate());

    RESOURCE_DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testGnats3244() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);
    DummyDetector detector(ResourceId::noId());
    DummyProfile profile(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1(ce.getId(), IntervalIntDomain(0, 10), true, "t1");
    Variable<IntervalIntDomain> t2(ce.getId(), IntervalIntDomain(10, 15), true, "t2");
    Variable<IntervalIntDomain> t3(ce.getId(), IntervalIntDomain(5, 15), true, "t3");
    Variable<IntervalIntDomain> t4(ce.getId(), IntervalIntDomain(5, 15), true, "t1");
    Variable<IntervalDomain> q1(ce.getId(), IntervalDomain(1, 1), true, "q1");
    Variable<IntervalDomain> q2(ce.getId(), IntervalDomain(1, 1), true, "q2");
    Variable<IntervalDomain> q3(ce.getId(), IntervalDomain(1, 1), true, "q3");
    Variable<IntervalDomain> q4(ce.getId(), IntervalDomain(1, 1), true, "q4");

    Transaction trans1(t1.getId(), q1.getId(), false);
    Transaction trans2(t2.getId(), q2.getId(), true);
    Transaction trans3(t3.getId(), q3.getId(), true);
    Transaction trans4(t4.getId(), q4.getId(), false);

    profile.addTransaction(trans1.getId());
    profile.addTransaction(trans2.getId());
    profile.addTransaction(trans3.getId());
    profile.addTransaction(trans4.getId());


    InstantId inst = profile.getInstant(0);
    CPPUNIT_ASSERT(inst.isValid());
    CPPUNIT_ASSERT(inst->getTime() == 0);
    for(std::set<TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      CPPUNIT_ASSERT((*it).isValid());
      CPPUNIT_ASSERT((*it) == trans1.getId());
    }

    //for time 5
    std::set<TransactionId> trans;
    trans.insert(trans1.getId());
    trans.insert(trans3.getId());
    trans.insert(trans4.getId());

    inst = profile.getInstant(5);
    CPPUNIT_ASSERT(inst.isValid());
    CPPUNIT_ASSERT(inst->getTime() == 5);
    for(std::set<TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      CPPUNIT_ASSERT((*it).isValid());
      CPPUNIT_ASSERT(trans.find(*it) != trans.end());
      trans.erase(*it);
    }
    CPPUNIT_ASSERT(trans.empty());

    //for time 10
    trans.insert(trans1.getId());
    trans.insert(trans2.getId());
    trans.insert(trans3.getId());
    trans.insert(trans4.getId());

    inst = profile.getInstant(10);
    CPPUNIT_ASSERT(inst.isValid());
    CPPUNIT_ASSERT(inst->getTime() == 10);
    for(std::set<TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      CPPUNIT_ASSERT((*it).isValid());
      CPPUNIT_ASSERT(trans.find(*it) != trans.end());
      trans.erase(*it);
    }
    CPPUNIT_ASSERT(trans.empty());

    //for time 15
    trans.insert(trans2.getId());
    trans.insert(trans3.getId());
    trans.insert(trans4.getId());

    inst = profile.getInstant(15);
    CPPUNIT_ASSERT(inst.isValid());
    CPPUNIT_ASSERT(inst->getTime() == 15);
    for(std::set<TransactionId>::const_iterator it = inst->getTransactions().begin();
	it != inst->getTransactions().end(); ++it) {
      CPPUNIT_ASSERT((*it).isValid());
      CPPUNIT_ASSERT(trans.find(*it) != trans.end());
      trans.erase(*it);
    }
    CPPUNIT_ASSERT(trans.empty());


    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }
};

class ResourceTest {
public:
  static bool test() {
    EUROPA_runTest(testReservoir);
    EUROPA_runTest(testReusable);
    EUROPA_runTest(testReservoirRemove);
    EUROPA_runTest(testDanglingTransaction);
    return true;
  }
private:

  static bool testReservoir() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);

    Reservoir res1(db.getId(), LabelStr("Reservoir"), LabelStr("Battery1"), LabelStr("OpenWorldFVDetector"), LabelStr("TimetableProfile"),
			 10, 10, 0, 1000);
    Reservoir res2(db.getId(), LabelStr("Reservoir"), LabelStr("Battery2"), LabelStr("OpenWorldFVDetector"), LabelStr("TimetableProfile"),
			 10, 10, 0, 1000);

    ConsumerToken consumer(db.getId(), LabelStr("Reservoir.consume"), IntervalIntDomain(10),
			   IntervalDomain(5));

    ProfileIterator it1(res1.getProfile());
    CPPUNIT_ASSERT(it1.done());
    ProfileIterator it2(res2.getProfile());
    CPPUNIT_ASSERT(it2.done());

    const_cast<AbstractDomain&>(consumer.getObject()->lastDomain()).remove(res2.getId());
    ce.propagate();
    ProfileIterator it3(res1.getProfile());
    CPPUNIT_ASSERT(!it3.done());
    ProfileIterator it4(res2.getProfile());
    CPPUNIT_ASSERT(it4.done());

    const_cast<AbstractDomain&>(consumer.getObject()->lastDomain()).relax(consumer.getObject()->baseDomain());
    ProfileIterator it5(res1.getProfile());
    CPPUNIT_ASSERT(it5.done());
    ProfileIterator it6(res2.getProfile());
    CPPUNIT_ASSERT(it6.done());

    ConsumerToken throwaway(db.getId(), LabelStr("Reservoir.consume"), IntervalIntDomain(10),
				  IntervalDomain(5));

    res1.constrain(throwaway.getId(), throwaway.getId());
    throwaway.discard(false);

    //test flaws and violations once we have a decent profile and detector
    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testReservoirRemove() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);
    //setup two reservoirs
    Reservoir res1(db.getId(), LabelStr("Reservoir"), LabelStr("Battery1"), LabelStr("OpenWorldFVDetector"), LabelStr("TimetableProfile"),
                   10, 10, 0, 1000, 0, 5);
    Reservoir res2(db.getId(), LabelStr("Reservoir"), LabelStr("Battery2"), LabelStr("OpenWorldFVDetector"), LabelStr("TimetableProfile"),
			 10, 10, 0, 1000);

    //create a flaw
    ConsumerToken c1(db.getId(), LabelStr("Reservoir.consume"), IntervalIntDomain(0, 10), IntervalDomain(3, 5));
    ConsumerToken c2(db.getId(), LabelStr("Reservoir.consume"), IntervalIntDomain(0, 10), IntervalDomain(3, 5));

    c1.getObject()->specify(res1.getId());
    c2.getObject()->specify(res1.getId());

    CPPUNIT_ASSERT(ce.propagate());
    CPPUNIT_ASSERT(res1.hasTokensToOrder());

    //in the 
    c1.getObject()->reset();

    CPPUNIT_ASSERT(ce.propagate());

    c1.getObject()->specify(res1.getId());

    CPPUNIT_ASSERT(ce.propagate());

    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testReusable() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);

	ce.getId(); // Only point of this is to remove a compile-time warning (unused-variable)

    Reusable res(db.getId(), LabelStr("Reusable"), LabelStr("Foo"), LabelStr("OpenWorldFVDetector"), LabelStr("TimetableProfile"));

    ReusableToken use(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(0, 5), IntervalIntDomain(10, 15), IntervalIntDomain(1, PLUS_INFINITY),
			    IntervalDomain(10));

    //should create two transactions and four instants
    ProfileIterator it(res.getProfile());
    CPPUNIT_ASSERT(!it.done());
    std::set<TransactionId> trans;
    int instCount = 0;
    while(!it.done()) {
      InstantId inst = it.getInstant();
      for(std::set<TransactionId>::const_iterator transIt = inst->getTransactions().begin(); transIt != inst->getTransactions().end(); ++transIt)
	trans.insert(*transIt);
      instCount++;
      it.next();
    }
    CPPUNIT_ASSERT(trans.size() == 2);
    CPPUNIT_ASSERT(instCount == 4);

    use.start()->specify(0);
    use.end()->specify(10);
    CPPUNIT_ASSERT(db.getConstraintEngine()->propagate());

    //should reduce the instant count to 2
    ProfileIterator it1(res.getProfile());
    CPPUNIT_ASSERT(!it1.done());
    instCount = 0;
    while(!it1.done()) {
      instCount++;
      it1.next();
    }
    CPPUNIT_ASSERT(instCount == 2);

    ReusableToken throwaway(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(50), IntervalIntDomain(51), IntervalIntDomain(1), IntervalDomain(1));
    throwaway.discard(false);

    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testDanglingTransaction() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);
    rte.getConfig()->setProperty("nddl.includePath", ".:../component/NDDL");
    rte.executeScript("nddl", "missing-transaction.nddl", true);

    DbClientId client = db.getClient();

    TokenId master = client->getGlobalToken("master");
    CPPUNIT_ASSERT(master.isValid());

    //activate the master
    client->activate(master);
    CPPUNIT_ASSERT(client->propagate());

    //object variable should be singleton here
    CPPUNIT_ASSERT(!master->getObject()->isSingleton());

    //get the main objects
    ObjectId main1 = client->getObject("m1");
    CPPUNIT_ASSERT(main1.isValid());
    ObjectId main2 = client->getObject("m2");
    CPPUNIT_ASSERT(main2.isValid());
        
    //get the foo objects and a battery
    ObjectId foo1 = client->getObject("m1.m_foo");
    CPPUNIT_ASSERT(foo1.isValid());
    ObjectId foo2 = client->getObject("m1.m_foo");
    CPPUNIT_ASSERT(foo2.isValid());
    ObjectId battery = client->getObject("m1.m_battery");
    CPPUNIT_ASSERT(battery.isValid());

    //get the slave that can go on Foo instances
    TokenId fooSlave = master->getSlave(1);
    CPPUNIT_ASSERT(fooSlave.isValid());
    CPPUNIT_ASSERT(fooSlave->getName().toString() == "Foo.foo");
    
    //get the slave that can go on Battery instances
    TokenId battSlave = master->getSlave(0);
    
    //activate the Foo slave
    client->activate(fooSlave);
    //constrain it to the first foo instance
    client->constrain(foo1, fooSlave, fooSlave);
    CPPUNIT_ASSERT(client->propagate());

    //via ProxyVariableRelation, other object variables should propagate to singleton
    CPPUNIT_ASSERT(master->getObject()->isSingleton());
    CPPUNIT_ASSERT(battSlave->getObject()->isSingleton());

    ResourceId res(battery);
    CPPUNIT_ASSERT(res.isValid());
    
    const std::map<int, InstantId>& insts(res->getProfile()->getInstants());
    CPPUNIT_ASSERT(!insts.empty());

    TransactionId trans = *(insts.begin()->second->getTransactions().begin());
    CPPUNIT_ASSERT(trans.isValid());

    client->free(foo1, fooSlave, fooSlave);
    CPPUNIT_ASSERT(client->propagate());
    CPPUNIT_ASSERT(trans.isValid());

    return true;
  }
};

class ConstraintNameListener : public ConstrainedVariableListener {
public:
	ConstraintNameListener(const ConstrainedVariableId& var) : ConstrainedVariableListener(var), m_constraintName("NO_CONSTRAINT") {}
	void notifyConstraintAdded(const ConstraintId& constr, int argIndex) {
		m_constraintName = constr->getName();
	}
	~ConstraintNameListener()
	{

	}

	const LabelStr& getName() {return m_constraintName;}
private:
	LabelStr m_constraintName;
};


// Test that used to be in Solvers/test/solvers-test-module.cc
class ResourceSolverTest {
public:
	static bool test() {
		EUROPA_runTest(testResourceDecisionPoint);
		EUROPA_runTest(testResourceThreatDecisionPoint);
		EUROPA_runTest(testResourceThreatManager);
		return true;
	}
private:

	// TBS:  This tested a previous version of ResourceThreatDecisionPoint (basically a glorified ThreatDecisionPoint, which we now test here), so
	//       there's not much value in this test anymore.
	static bool testResourceDecisionPoint() {

		RESOURCE_DEFAULT_SETUP(ceObj, dbObj, false);

		PlanDatabaseId db = dbObj.getId();
		ConstraintEngineId ce = ceObj.getId();
		DbClientId client = db->getClient();

		Reusable reusable(db, "Reusable", "myReusable", "ClosedWorldFVDetector", "IncrementalFlowProfile", 2, 2, 0);

		ReusableToken tok1(db, "Reusable.uses", IntervalIntDomain(1, 3), IntervalIntDomain(10, 12), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		ReusableToken tok2(db, "Reusable.uses", IntervalIntDomain(11, 13), IntervalIntDomain(15, 17), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		ReusableToken tok3(db, "Reusable.uses", IntervalIntDomain(11, 16), IntervalIntDomain(18, 19), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		CPPUNIT_ASSERT(ce->propagate());
		CPPUNIT_ASSERT(reusable.hasTokensToOrder());
		TiXmlElement dummy("");
		SOLVERS::ThreatDecisionPoint dp1(client, tok1.getId(), dummy);

		dp1.initialize();

		SOLVERS::ThreatDecisionPoint dp2(client, tok2.getId(), dummy);
		dp2.initialize();

		SOLVERS::ThreatDecisionPoint dp3(client, tok3.getId(), dummy);
		dp3.initialize();

		RESOURCE_DEFAULT_TEARDOWN();
		return true;
	}

	static bool testResourceThreatDecisionPoint() {

		RESOURCE_DEFAULT_SETUP(ceObj, dbObj, false);

		PlanDatabaseId db = dbObj.getId();
		ConstraintEngineId ce = ceObj.getId();
		DbClientId client = db->getClient();

		Reusable reusable(db, "Reusable", "myReusable", "ClosedWorldFVDetector", "IncrementalFlowProfile", 1, 1, 0);

		ReusableToken tok1(db, "Reusable.uses", IntervalIntDomain(1, 3), IntervalIntDomain(10, 12), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		ReusableToken tok2(db, "Reusable.uses", IntervalIntDomain(11, 13), IntervalIntDomain(15, 17), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		ReusableToken tok3(db, "Reusable.uses", IntervalIntDomain(11, 16), IntervalIntDomain(18, 19), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		CPPUNIT_ASSERT(ce->propagate());

		CPPUNIT_ASSERT(reusable.hasTokensToOrder());
		std::vector<InstantId> flawedInstants;
		reusable.getFlawedInstants(flawedInstants);
		CPPUNIT_ASSERT(flawedInstants.size() == 5);
		CPPUNIT_ASSERT(flawedInstants[0]->getTime() == 11);

		TiXmlElement dummy("");
		ResourceThreatDecisionPoint dp1(client, flawedInstants[0], dummy);

		dp1.initialize();

		CPPUNIT_ASSERT(dp1.getChoices().size() == 8);

		std::string noFilter = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"none\"/>";
		TiXmlElement* noFilterXml = initXml(noFilter);
		ResourceThreatDecisionPoint dp2(client, flawedInstants[0], *noFilterXml);
		dp2.initialize();
		CPPUNIT_ASSERT(dp2.getChoices().size() == 8);

		std::string predFilter = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"predecessorNot\"/>";
		TiXmlElement* predFilterXml = initXml(predFilter);
		ResourceThreatDecisionPoint dp3(client, flawedInstants[0], *predFilterXml);
		dp3.initialize();
		CPPUNIT_ASSERT(dp3.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp3.getChoices()[0].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp3.getChoices()[0].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp3.getChoices()[1].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp3.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp3.getChoices()[2].first->time() == tok2.end());
		CPPUNIT_ASSERT(dp3.getChoices()[2].second->time() == tok3.start());

		std::string sucFilter = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"successor\"/>";
		TiXmlElement* sucFilterXml = initXml(sucFilter);
		ResourceThreatDecisionPoint dp4(client, flawedInstants[0], *sucFilterXml);
		dp4.initialize();
		CPPUNIT_ASSERT(dp4.getChoices().size() == 5);
		CPPUNIT_ASSERT(dp4.getChoices()[0].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp4.getChoices()[0].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp4.getChoices()[1].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp4.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp4.getChoices()[2].first->time() == tok2.start());
		CPPUNIT_ASSERT(dp4.getChoices()[2].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp4.getChoices()[3].first->time() == tok2.end());
		CPPUNIT_ASSERT(dp4.getChoices()[3].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp4.getChoices()[4].first->time() == tok3.start());
		CPPUNIT_ASSERT(dp4.getChoices()[4].second->time() == tok2.start());

		std::string bothFilter = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\"/>";
		TiXmlElement* bothFilterXml = initXml(bothFilter);
		ResourceThreatDecisionPoint dp5(client, flawedInstants[0], *bothFilterXml);
		dp5.initialize();
		CPPUNIT_ASSERT(dp5.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp5.getChoices()[0].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp5.getChoices()[0].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp5.getChoices()[1].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp5.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp5.getChoices()[2].first->time() == tok2.end());
		CPPUNIT_ASSERT(dp5.getChoices()[2].second->time() == tok3.start());

		//the combination of ascendingKeyPredecessor and ascendingKeySuccessor have already been tested by now

		std::string earliestPred = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"earliestPredecessor\"/>";
		TiXmlElement* earliestPredXml = initXml(earliestPred);
		ResourceThreatDecisionPoint dp6(client, flawedInstants[0], *earliestPredXml);
		dp6.initialize();
		CPPUNIT_ASSERT(dp6.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp6.getChoices()[0].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp6.getChoices()[0].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp6.getChoices()[1].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp6.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp6.getChoices()[2].first->time() == tok2.end());
		CPPUNIT_ASSERT(dp6.getChoices()[2].second->time() == tok3.start());

		std::string latestPred = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"latestPredecessor\"/>";
		TiXmlElement* latestPredXml = initXml(latestPred);
		ResourceThreatDecisionPoint dp7(client, flawedInstants[0], *latestPredXml);
		dp7.initialize();
		CPPUNIT_ASSERT(dp7.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp7.getChoices()[0].first->time() == tok2.end());
		CPPUNIT_ASSERT(dp7.getChoices()[0].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp7.getChoices()[1].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp7.getChoices()[1].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp7.getChoices()[2].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp7.getChoices()[2].second->time() == tok3.start());


		std::string longestPred = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"successor\" order=\"longestPredecessor\"/>";
		TiXmlElement* longestPredXml = initXml(longestPred);
		ResourceThreatDecisionPoint dp8(client, flawedInstants[0], *longestPredXml);
		dp8.initialize();
		CPPUNIT_ASSERT(dp8.getChoices().size() == 5);
		CPPUNIT_ASSERT(dp8.getChoices()[0].first->time() == tok3.start());


		std::string shortestPred = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"shortestPredecessor\"/>";
		TiXmlElement* shortestPredXml = initXml(shortestPred);
		ResourceThreatDecisionPoint dp9(client, flawedInstants[0], *shortestPredXml);
		dp9.initialize();
		CPPUNIT_ASSERT(dp9.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp9.getChoices()[0].first->time() == tok2.start() ||
				dp9.getChoices()[0].first->time() == tok1.end());

		std::string descendingKeyPred = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"descendingKeyPredecessor\"/>";
		TiXmlElement* descendingKeyPredXml = initXml(descendingKeyPred);
		ResourceThreatDecisionPoint dp10(client, flawedInstants[0], *descendingKeyPredXml);
		dp10.initialize();
		CPPUNIT_ASSERT(dp10.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp10.getChoices()[0].first->time() == tok2.end());
		CPPUNIT_ASSERT(dp10.getChoices()[1].first->time() == tok1.end());


		std::string earliestSucc = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"earliestSuccessor\"/>";
		TiXmlElement* earliestSuccXml = initXml(earliestSucc);
		ResourceThreatDecisionPoint dp11(client, flawedInstants[0], *earliestSuccXml);
		dp11.initialize();
		CPPUNIT_ASSERT(dp11.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp11.getChoices()[0].second->time() == tok2.start() ||
				dp11.getChoices()[0].second->time() == tok3.start());

		std::string latestSucc = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"latestSuccessor\"/>";
		TiXmlElement* latestSuccXml = initXml(latestSucc);
		ResourceThreatDecisionPoint dp12(client, flawedInstants[0], *latestSuccXml);
		dp12.initialize();
		CPPUNIT_ASSERT(dp12.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp12.getChoices()[0].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp12.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp12.getChoices()[2].second->time() == tok2.start());


		std::string longestSucc = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"longestSuccessor\"/>";
		TiXmlElement* longestSuccXml = initXml(longestSucc);
		ResourceThreatDecisionPoint dp13(client, flawedInstants[0], *longestSuccXml);
		dp13.initialize();
		CPPUNIT_ASSERT(dp13.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp13.getChoices()[0].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp13.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp13.getChoices()[2].second->time() == tok2.start());

		std::string shortestSucc = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"shortestSuccessor\"/>";
		TiXmlElement* shortestSuccXml = initXml(shortestSucc);
		ResourceThreatDecisionPoint dp14(client, flawedInstants[0], *shortestSuccXml);
		dp14.initialize();
		CPPUNIT_ASSERT(dp14.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp14.getChoices()[0].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp14.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp14.getChoices()[2].second->time() == tok3.start());

		std::string descendingKeySucc = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"descendingKeySuccessor\"/>";
		TiXmlElement* descendingKeySuccXml = initXml(descendingKeySucc);
		ResourceThreatDecisionPoint dp15(client, flawedInstants[0], *descendingKeySuccXml);
		dp15.initialize();
		CPPUNIT_ASSERT(dp15.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp15.getChoices()[0].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp15.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp15.getChoices()[2].second->time() == tok2.start());


		std::string ascendingKeySucc = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" order=\"ascendingKeySuccessor\"/>";
		TiXmlElement* ascendingKeySuccXml = initXml(ascendingKeySucc);
		ResourceThreatDecisionPoint dp16(client, flawedInstants[0], *ascendingKeySuccXml);
		dp16.initialize();
		CPPUNIT_ASSERT(dp16.getChoices().size() == 3);
		CPPUNIT_ASSERT(dp16.getChoices()[0].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp16.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp16.getChoices()[2].second->time() == tok3.start());


		std::string leastImpact = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"successor\" order=\"leastImpact,earliestPredecessor,shortestSuccessor\"/>";
		TiXmlElement* leastImpactXml = initXml(leastImpact);
		ResourceThreatDecisionPoint dp17(client, flawedInstants[0], *leastImpactXml);
		dp17.initialize();
		CPPUNIT_ASSERT(dp17.getChoices().size() == 5);
		CPPUNIT_ASSERT(dp17.getChoices()[0].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp17.getChoices()[0].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp17.getChoices()[1].first->time() == tok1.end());
		CPPUNIT_ASSERT(dp17.getChoices()[1].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp17.getChoices()[2].first->time() == tok2.start());
		CPPUNIT_ASSERT(dp17.getChoices()[2].second->time() == tok3.start());
		CPPUNIT_ASSERT(dp17.getChoices()[3].first->time() == tok3.start());
		CPPUNIT_ASSERT(dp17.getChoices()[3].second->time() == tok2.start());
		CPPUNIT_ASSERT(dp17.getChoices()[4].first->time() == tok2.end());
		CPPUNIT_ASSERT(dp17.getChoices()[4].second->time() == tok3.start());

		std::string precedesOnly = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" constraint=\"precedesOnly\"/>";
		TiXmlElement* precedesOnlyXml = initXml(precedesOnly);
		ResourceThreatDecisionPoint dp18(client, flawedInstants[0], *precedesOnlyXml);
		dp18.initialize();
		ConstraintNameListener* nameListener = new ConstraintNameListener(dp18.getChoices()[0].first->time());
		dp18.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("precedes"));
		dp18.undo();
		delete (ConstrainedVariableListener*) nameListener;
		ce->propagate();
		nameListener = new ConstraintNameListener(dp18.getChoices()[1].first->time());
		dp18.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("precedes"));
		dp18.undo();
		ce->propagate();
		delete (ConstrainedVariableListener*) nameListener;

		std::string concurrentOnly = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" constraint=\"concurrentOnly\"/>";
		TiXmlElement* concurrentOnlyXml = initXml(concurrentOnly);
		ResourceThreatDecisionPoint dp19(client, flawedInstants[0], *concurrentOnlyXml);
		dp19.initialize();
		nameListener = new ConstraintNameListener(dp19.getChoices()[0].first->time());
		dp19.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("concurrent"));
		dp19.undo();
		delete (ConstrainedVariableListener*) nameListener;
		ce->propagate();
		nameListener = new ConstraintNameListener(dp19.getChoices()[1].first->time());
		dp19.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("concurrent"));
		dp19.undo();
		ce->propagate();
		delete (ConstrainedVariableListener*) nameListener;

		//pair first has already been implicitly tested and constraint first is necessary to make this easy anyway, so it'll get tested as well.

		std::string precedesFirst = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" constraint=\"precedesFirst\" iterate=\"constraintFirst\"/>";
		TiXmlElement* precedesFirstXml = initXml(precedesFirst);
		ResourceThreatDecisionPoint dp20(client, flawedInstants[0], *precedesFirstXml);
		dp20.initialize();
		nameListener = new ConstraintNameListener(dp20.getChoices()[0].first->time());
		dp20.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("precedes"));
		dp20.undo();
		ce->propagate();
		dp20.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("concurrent"));
		dp20.undo();
		ce->propagate();
		delete (ConstrainedVariableListener*) nameListener;
		//step once more to test creating a constraint on the next choice
		nameListener = new ConstraintNameListener(dp20.getChoices()[1].first->time());
		dp20.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("precedes"));
		dp20.undo();
		ce->propagate();
		delete nameListener;

		std::string concurrentFirst = "<FlawHandler component=\"ResourceThreatDecisionPoint\" filter=\"both\" constraint=\"concurrentFirst\" iterate=\"constraintFirst\"/>";
		TiXmlElement* concurrentFirstXml = initXml(concurrentFirst);
		ResourceThreatDecisionPoint dp21(client, flawedInstants[0], *concurrentFirstXml);
		dp21.initialize();
		nameListener = new ConstraintNameListener(dp21.getChoices()[0].first->time());
		dp21.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("concurrent"));
		dp21.undo();
		ce->propagate();
		dp21.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("precedes"));
		dp21.undo();
		ce->propagate();
		delete (ConstrainedVariableListener*) nameListener;
		//step once more to test creating a constraint on the next choice
		nameListener = new ConstraintNameListener(dp21.getChoices()[1].first->time());
		dp21.execute();
		ce->propagate();
		CPPUNIT_ASSERT(nameListener->getName() == LabelStr("concurrent"));
		dp21.undo();
		ce->propagate();
		delete nameListener;

		delete concurrentFirstXml;
		delete precedesFirstXml;
		delete concurrentOnlyXml;
		delete precedesOnlyXml;
		delete leastImpactXml;
		delete ascendingKeySuccXml;
		delete descendingKeySuccXml;
		delete shortestSuccXml;
		delete longestSuccXml;
		delete latestSuccXml;
		delete earliestSuccXml;
		delete descendingKeyPredXml;
		delete shortestPredXml;
		delete longestPredXml;
		delete latestPredXml;
		delete earliestPredXml;
		delete bothFilterXml;
		delete sucFilterXml;
		delete predFilterXml;
		delete noFilterXml;

		return true;
	}


	static bool testResourceThreatManager() {

		RESOURCE_DEFAULT_SETUP(ceObj, dbObj, false);

		PlanDatabaseId db = dbObj.getId();
		ConstraintEngineId ce = ceObj.getId();
		DbClientId client = db->getClient();

		Reusable reusable(db, "Reusable", "myReusable", "ClosedWorldFVDetector", "IncrementalFlowProfile", 1, 1, 0);

		ReusableToken tok1(db, "Reusable.uses", IntervalIntDomain(1, 3), IntervalIntDomain(10, 12), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		ReusableToken tok2(db, "Reusable.uses", IntervalIntDomain(11, 13), IntervalIntDomain(15, 17), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		ReusableToken tok3(db, "Reusable.uses", IntervalIntDomain(11, 16), IntervalIntDomain(18, 19), IntervalIntDomain(1, PLUS_INFINITY),
				IntervalDomain(1.0, 1.0), "myReusable");

		CPPUNIT_ASSERT(ce->propagate());

		std::vector<InstantId> instants;
		reusable.getFlawedInstants(instants);

		LabelStr explanation;
		std::string earliest = "<ResourceThreatManager order=\"earliest\"><FlawHandler component=\"ResourceThreatHandler\"/></ResourceThreatManager>";
		TiXmlElement* earliestXml = initXml(earliest);
		ResourceThreatManager earliestManager(*earliestXml);
		earliestManager.initialize(*earliestXml, db, SOLVERS::ContextId::noId(), SOLVERS::FlawManagerId::noId());
		CPPUNIT_ASSERT(earliestManager.betterThan(instants[0], instants[1], explanation)); //these are identical except for the time
		CPPUNIT_ASSERT(earliestManager.betterThan(instants[1], instants[2], explanation)); //these have different levels
		CPPUNIT_ASSERT(!earliestManager.betterThan(instants[0], instants[0], explanation));

		std::string latest = "<ResourceThreatManager order=\"latest\"><FlawHandler component=\"ResourceThreatHandler\"/></ResourceThreatManager>";
		TiXmlElement* latestXml = initXml(latest);
		ResourceThreatManager latestManager(*latestXml);
		latestManager.initialize(*latestXml, db, SOLVERS::ContextId::noId(), SOLVERS::FlawManagerId::noId());
		CPPUNIT_ASSERT(latestManager.betterThan(instants[3], instants[2], explanation));
		CPPUNIT_ASSERT(latestManager.betterThan(instants[2], instants[1], explanation));
		CPPUNIT_ASSERT(!latestManager.betterThan(instants[0], instants[0], explanation));

		std::string most = "<ResourceThreatManager order=\"most\"><FlawHandler component=\"ResourceThreatHandler\"/></ResourceThreatManager>";
		TiXmlElement* mostXml = initXml(most);
		ResourceThreatManager mostManager(*mostXml);
		mostManager.initialize(*mostXml, db, SOLVERS::ContextId::noId(), SOLVERS::FlawManagerId::noId());
		CPPUNIT_ASSERT(mostManager.betterThan(instants[0], instants[1], explanation));
		CPPUNIT_ASSERT(!mostManager.betterThan(instants[1], instants[0], explanation));
		CPPUNIT_ASSERT(!mostManager.betterThan(instants[1], instants[2], explanation));
		CPPUNIT_ASSERT(!mostManager.betterThan(instants[3], instants[4], explanation));

		std::string least = "<ResourceThreatManager order=\"least\"><FlawHandler component=\"ResourceThreatHandler\"/></ResourceThreatManager>";
		TiXmlElement* leastXml = initXml(least);
		ResourceThreatManager leastManager(*leastXml);
		leastManager.initialize(*leastXml, db, SOLVERS::ContextId::noId(), SOLVERS::FlawManagerId::noId());
		CPPUNIT_ASSERT(!leastManager.betterThan(instants[0], instants[1], explanation));
		CPPUNIT_ASSERT(leastManager.betterThan(instants[1], instants[0], explanation));
		CPPUNIT_ASSERT(!leastManager.betterThan(instants[3], instants[4], explanation));
		CPPUNIT_ASSERT(!leastManager.betterThan(instants[4], instants[3], explanation));

		//can't test upper/lower with reusables
		delete leastXml;
		delete mostXml;
		delete latestXml;
		delete earliestXml;
		return true;
	}
};

void ResourceModuleTests::cppSetup(void)
{
  setTestLoadLibraryPath(".");
}

void ResourceModuleTests::defaultSetupTests(void)
{
  DefaultSetupTest::test();
}

void ResourceModuleTests::profileTests(void)
{
  ProfileTest::test();
}

void ResourceModuleTests::ResourceTests(void)
{
  ResourceTest::test();
}

void ResourceModuleTests::ResourceSolverTests(void)
{
  ResourceSolverTest::test();
}
