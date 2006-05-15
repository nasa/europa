#include "rs-flow-test-module.hh"
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
#include "SAVH_FlowProfile.hh"
#include "SAVH_ProfilePropagator.hh"

#include "Debug.hh"
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
#include "STNTemporalAdvisor.hh"

#include "Debug.hh"

#include "LockManager.hh"

#include <iostream>
#include <string>
#include <list>

#define RESOURCE_DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    SchemaId schema = Schema::instance();\
    schema->reset();\
    schema->addObjectType(LabelStr("Resource")); \
    schema->addObjectType(LabelStr("SAVHResource")); \
    schema->addPredicate(LabelStr("Resource.change"));\
    schema->addMember(LabelStr("Resource.change"), IntervalDomain().getTypeName(), LabelStr("quantity")); \
    PlanDatabase db(ce.getId(), schema); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new ResourcePropagator(LabelStr("Resource"), ce.getId(), db.getId()); \
    new TemporalPropagator(LabelStr("Temporal"), ce.getId()); \
    new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), ce.getId()); \
    db.setTemporalAdvisor((new STNTemporalAdvisor(ce.getPropagatorByName(LabelStr("Temporal"))))->getId()); \
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

class DummyDetector : public SAVH::FVDetector {
public:
  DummyDetector(const SAVH::ResourceId res) : FVDetector(res) {};
  bool detect(const SAVH::InstantId inst) {return false;}
  void initialize(const SAVH::InstantId inst) {}
  void initialize() {}
};

class FlowProfileTest
{
public:

  static bool test(){
    testAddAndRemove();
    testScenario0();
    testScenario1();
    testScenario2();
    testScenario3();
    testScenario4();
    testScenario5();
    testScenario6();
    testScenario7();
    testScenario8();
    return true;
  }
private:
  static bool verifyProfile( SAVH::Profile& profile, int instances, int times[], double lowerLevel[], double upperLevel[] ) {
    int counter = 0;

    SAVH::ProfileIterator ite( profile.getId() );

    while( !ite.done() ) {
      if( counter >= instances ) {
	debugMsg("ResourceTest:verifyProfile","Profile has more instances than expected, now at instance "
		 << counter << " and expected only "
		 << instances );

	return false;
      }

      if( times[counter] != ite.getTime() ) {
	debugMsg("ResourceTest:verifyProfile","Profile has no instant at time "
		 << times[counter] << " the nearest instant is at " << ite.getTime() );

	return false;
      }
      
      if( lowerLevel[counter] != ite.getLowerBound() ) {
	debugMsg("ResourceTest:verifyProfile","Profile has incorrect lower level at instant at time "
		 << times[counter] << " the level is " << ite.getLowerBound() << " and is supposed to be "
		 << lowerLevel[counter] );
	return false;

      }
	
      if( upperLevel[counter] != ite.getUpperBound() ) {
	debugMsg("ResourceTest:verifyProfile","Profile has incorrect upper level at instant at time "
		 << times[counter] << " the level is " << ite.getUpperBound() << " and is supposed to be "
		 << upperLevel[counter] );

	++counter;

	return false;
      }
      
      ++counter;
      
      ite.next();
    }
    
    return true;
  }

  static void executeScenario0( SAVH::Profile& profile, ConstraintEngine& ce ) {
    // no transactions
    profile.recompute();
  }

  static void executeScenario1( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * No explicit ordering between transactions
     *
     * Transaction1   <0-------(+1)-------10>
     * Transaction2    |                 <10--(-1)--15>
     * Transaction3    |       <5--------(-1)-------15>
     * Transaction4    |       <5--------(+1)-------15>
     *                 |        |         |          |
     *                 |        |         |          |
     * Max level       1        2         2          0
     * Min level       0       -1        -1          0
     *
     */
    const int nrInstances = 4;

    int itimes[nrInstances] = {0,5,10,15};
    double lowerLevels[nrInstances] = {0,-1,-1,0};
    double upperLevels[nrInstances] = {1,2,2,0};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain(10, 15), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 5, 15), true, "t3" );
    Variable<IntervalIntDomain> t4( ce.getId(), IntervalIntDomain( 5, 15), true, "t4" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 1), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 1), true, "q3" );
    Variable<IntervalDomain> q4( ce.getId(), IntervalDomain(1, 1), true, "q4" );

    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 
    SAVH::Transaction trans3( t3.getId(), q3.getId(), true); 
    SAVH::Transaction trans4( t4.getId(), q4.getId(), false);

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );
    profile.addTransaction( trans4.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario2( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * No explicit ordering between transactions
     *
     * Transaction1   <0-------(+1)-------10>
     * Transaction2   <0-------(-1)-------10>
     *                 |                   |
     *                 |                   |
     * Max level       1                   0
     * Min level      -1                   0
     *
     */

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {-1,0};
    double upperLevels[nrInstances] = {1,0};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 1), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );
    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario3( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * No explicit ordering between transactions
     *
     * Transaction1   <0-------[1,2]------10>
     * Transaction2                      <10>[-2,-1]
     * Transaction3                      <10------[1,2]-------20>
     *                 |                   |                   | 
     *                 |                   |                   |
     * Max level       2                   3                   3                
     * Min level       0                  -1                   0
     *
     */

    const int nrInstances = 3;

    int itimes[nrInstances] = {0,10,20};
    double lowerLevels[nrInstances] = {0,-1,0};
    double upperLevels[nrInstances] = {2,3,3};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain(  0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 10, 10), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 10, 20), true, "t3" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 2), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 2), true, "q3" );

    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 
    SAVH::Transaction trans3( t3.getId(), q3.getId(), false ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario4( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     *
     * Transaction1   <0---[1,2]---5>
     * Transaction2                    <10---[-2,-1]---15>
     * Transaction3                                        <20---[1,2]---25>
     *                 |           |    |               |   |             | 
     *                 |           |    |               |   |             | 
     * Max level       2           2    2               1   3             3   
     * Min level       0           1   -1              -1  -1             0
     *
     */

    const int nrInstances = 6;

    int itimes[nrInstances] = {0,5,10,15,20,25};
    double lowerLevels[nrInstances] = {0,1,-1,-1,-1,0};
    double upperLevels[nrInstances] = {2,2,2,1,3,3};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain(  0, 5), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 10, 15), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 20, 25), true, "t3" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 2), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 2), true, "q3" );

    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 
    SAVH::Transaction trans3( t3.getId(), q3.getId(), false ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario5( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * Transaction1 constrained to be at Transaction2
     *
     * Transaction1   <0-------(+1)-------10>
     * Transaction2   <0-------(-1)-------10>
     *                 |                   |
     *                 |                   |
     * Max level       0                   0
     * Min level       0                   0
     *
     */

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {0, 0};
    double upperLevels[nrInstances] = {0, 0};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );
    
    EqualConstraint c0(LabelStr("concurrent"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    ce.propagate();

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 1), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );
    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario6( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * No explicit ordering between transactions
     *
     * Transaction1   <0------[1,2]-------100>
     * Transaction2   <0-----[-1,-2]------100>
     * Transaction3   <0------[1,2]-------100>
     * Transaction4   <0-----[-1,-2]------100>
     *                 |                   |
     *                 |                   |
     * Max level       4                   2
     * Min level      -4                  -2
     *
     */
    std::cout << "    Case 1" << std::endl;

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,100};
    double lowerLevels[nrInstances] = {-4,-2};
    double upperLevels[nrInstances] = {4,2};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 100), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 100), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 0, 100), true, "t3" );
    Variable<IntervalIntDomain> t4( ce.getId(), IntervalIntDomain( 0, 100), true, "t4" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 2), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 2), true, "q3" );
    Variable<IntervalDomain> q4( ce.getId(), IntervalDomain(1, 2), true, "q4" );
    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 
    SAVH::Transaction trans3( t3.getId(), q3.getId(), false);
    SAVH::Transaction trans4( t4.getId(), q4.getId(), true ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );
    profile.addTransaction( trans4.getId() );

    profile.recompute();

    {
      bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );
      
      assertTrue( profileMatches );
    }

    /*!
     * Change the quantity of transaction 2 to a singleton 
     * (only lower levels needs to be recalculated, optimization possible)
     *
     * No explicit ordering between transactions
     *
     * Transaction1   <0------[1,2]-------100>
     * Transaction2   <0-------[-1]-------100>
     * Transaction3   <0------[1,2]-------100>
     * Transaction4   <0-----[-1,-2]------100>
     *                 |                   |
     *                 |                   |
     * Max level       4                   2
     * Min level      -3                  -1
     *
     */
    std::cout << "    Case 2" << std::endl;
   
    double postq2eq1LowerLevels[nrInstances] = {-3,-1};

    q2.restrictBaseDomain( IntervalDomain(1,1) );
    
    {
      bool profileMatches = verifyProfile( profile, nrInstances, itimes, postq2eq1LowerLevels, upperLevels );
      
      assertTrue( profileMatches );
    }

    /*!
     * Change the time of transaction 2 
     *
     * No explicit ordering between transactions
     *
     * Transaction1   <0------[1,2]-------100>
     * Transaction2       <10--[-1]-------100>
     * Transaction3   <0------[1,2]-------100>
     * Transaction4   <0-----[-1,-2]------100>
     *                 |   |               |
     *                 |   |               |
     * Max level       4   4               2
     * Min level      -2  -3              -1
     *
     */
    std::cout << "    Case 3" << std::endl;

    t2.restrictBaseDomain( IntervalIntDomain( 10, 100) );

    const int postt2to10NrInstances = 3;

    int postt2to10Itimes[postt2to10NrInstances] = {0,10,100};
    double postt2to10LowerLevels[postt2to10NrInstances] = {-2,-3,-1};
    double postt2to10UpperLevels[postt2to10NrInstances] = {4,4,2};

    {
      bool profileMatches = verifyProfile( profile, postt2to10NrInstances, postt2to10Itimes, postt2to10LowerLevels, postt2to10UpperLevels );
      
      assertTrue( profileMatches );
    }
    
    /*!
     * Change the time of transaction 2 a little more 
     * (this should skip the recalculation of the first Instant)
     *
     * No explicit ordering between transactions
     *
     * Transaction1   <0------[1,2]-------100>
     * Transaction2       <11--[-1]-------100>
     * Transaction3   <0------[1,2]-------100>
     * Transaction4   <0-----[-1,-2]------100>
     *                 |   |               |
     *                 |   |               |
     * Max level       4   4               2
     * Min level      -2  -3              -1
     *
     */
    std::cout << "    Case 4" << std::endl;

    t2.restrictBaseDomain( IntervalIntDomain( 11, 100) );

    const int postt2to11NrInstances = 3;

    int postt2to11Itimes[postt2to11NrInstances] = {0,11,100};
    double postt2to11LowerLevels[postt2to11NrInstances] = {-2,-3,-1};
    double postt2to11UpperLevels[postt2to11NrInstances] = {4,4,2};

    {
      bool profileMatches = verifyProfile( profile, postt2to11NrInstances, postt2to11Itimes, postt2to11LowerLevels, postt2to11UpperLevels );
      
      assertTrue( profileMatches );
    }

    /*!
     * Constrain Transaction3 and Transaction4 to be concurrent
     *
     * No explicit ordering between transactions
     *
     * Transaction1   <0------[1,2]-------100>
     * Transaction2       <11--[-1]-------100>
     * Transaction3   <0------[1,2]-------100>
     * Transaction4   <0-----[-1,-2]------100>
     *                 |   |               |
     *                 |   |               |
     * Max level       3   3               2
     * Min level      -1  -2              -1
     *
     */
    std::cout << "    Case 5" << std::endl;

    /*! BEING WORKED ON BY MICHAEL

    EqualConstraint c0(LabelStr("concurrent"), LabelStr("Temporal"), ce.getId() , makeScope( t3.getId(), t4.getId()));

    ce.propagate();
    
    profile.recompute();

    const int postt3eqt4NrInstances = 3;

    int postt3eqt4Itimes[postt3eqt4NrInstances] = {0,11,100};
    double postt3eqt4LowerLevels[postt3eqt4NrInstances] = {-1,-2,-1};
    double postt3eqt4UpperLevels[postt3eqt4NrInstances] = {3,3,2};

    {
      bool profileMatches = verifyProfile( profile, postt3eqt4NrInstances, postt3eqt4Itimes, postt3eqt4LowerLevels, postt3eqt4UpperLevels );
      
      assertTrue( profileMatches );
    }
    */
  }

  static void executeScenario7( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * Transaction1 constrained to be at Transaction2
     *
     * Transaction1   <0-------(+2)-------10>
     * Transaction2   <0-----[-1,-2]------10>
     *                 |                   |
     *                 |                   |
     * Max level       1                   1
     * Min level       0                   0
     *
     */

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {0, 0};
    double upperLevels[nrInstances] = {1, 1};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );
    
    EqualConstraint c0(LabelStr("concurrent"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    ce.propagate();

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(2, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 2), true, "q2" );

    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario8( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * Transaction1 constrained to be at Transaction2
     *
     * Transaction1   <0------[+1,+2]----10>
     * Transaction2   <0-------(-2)------10>
     *                 |                   |
     *                 |                   |
     * Max level       0                   0
     * Min level      -1                  -1
     *
     */

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {-1, -1};
    double upperLevels[nrInstances] = {0, 0};

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );
    
    EqualConstraint c0(LabelStr("concurrent"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    ce.propagate();

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(2, 2), true, "q2" );

    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }
  static bool testNoTransactions() {
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile(ce.getId(), detector.getId());

    profile.recompute();
    return true;
  }
  static bool testOnePositiveTransaction() {
    return true;
  }
  static bool testOneNegativeTransaction() {
    return true;
  }
  static bool testAddAndRemove(){
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain(10, 15), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 5, 15), true, "t3" );
    Variable<IntervalIntDomain> t4( ce.getId(), IntervalIntDomain( 5, 15), true, "t4" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 1), true, "q3" );
    Variable<IntervalDomain> q4( ce.getId(), IntervalDomain(1, 1), true, "q4" );

    std::set<int> times;

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 
    SAVH::Transaction trans3( t3.getId(), q3.getId(), true); 
    SAVH::Transaction trans4( t4.getId(), q4.getId(), false);

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );
    profile.addTransaction( trans4.getId() );

    trans1.quantity()->restrictBaseDomain( IntervalIntDomain( (int) trans1.quantity()->lastDomain().getLowerBound(), 1 ) );

    profile.removeTransaction( trans1.getId() );
    profile.removeTransaction( trans2.getId() );
    profile.removeTransaction( trans3.getId() );
    profile.removeTransaction( trans4.getId() );

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );
    profile.addTransaction( trans4.getId() );

    return true;
  }
  static bool testScenario0(){
    std::cout << "  Scenario 0" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario0( profile, ce );
    return true;
  }

  static bool testScenario1(){
    std::cout << "  Scenario 1" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario1( profile, ce );
    return true;
  }
  static bool testScenario2(){
    std::cout << "  Scenario 2" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario2( profile, ce );
    return true;
  }
  static bool testScenario3(){
    std::cout << "  Scenario 3" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario3( profile, ce );
    return true;
  }
  static bool testScenario4(){
    std::cout << "  Scenario 4" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario4( profile, ce );
    return true;
  }
  static bool testScenario5(){
    std::cout << "  Scenario 5" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario5( profile, ce );
    return true;
  }

  static bool testScenario6(){
    std::cout << "  Scenario 6" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario6( profile, ce );
    return true;
  }

  static bool testScenario7(){
    std::cout << "  Scenario 7" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario7( profile, ce );
    return true;
  }
  static bool testScenario8(){
    std::cout << "  Scenario 8" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::FlowProfile profile( db.getId(), detector.getId());

    executeScenario8( profile, ce );
    return true;
  }
  static bool testDeltaTime(){
    return true;
  }
  static bool testDeltaQuantity(){
    return true;
  }
  static bool testDeltaOrdering(){
    return true;
  }
};

void FlowProfileModuleTests::runTests( const std::string& path) {
  LockManager::instance().connect();
  LockManager::instance().lock();
  setTestLoadLibraryPath(path);  

  
  Schema::instance();
  initConstraintLibrary();
  runTestSuite(DefaultSetupTest::test);
  runTestSuite(FlowProfileTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  uninitConstraintLibrary();
}



