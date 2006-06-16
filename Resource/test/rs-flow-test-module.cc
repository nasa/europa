#include "rs-flow-test-module.hh"
#include "ResourceDefs.hh"
#include "Resource.hh"
#include "Transaction.hh"
#include "ResourceConstraint.hh"
#include "ResourcePropagator.hh"
#include "SAVH_ResourceDefs.hh"
#include "SAVH_Profile.hh"
#include "SAVH_FVDetector.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_ReusableFVDetector.hh"

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
#include "SAVH_Reusable.hh"
#include "SAVH_DurativeTokens.hh"

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
    schema->addObjectType(LabelStr("Reusable")); \
    schema->addPredicate(LabelStr("Reusable.uses")); \
  schema->addMember(LabelStr("Reusable.uses"), IntervalDomain().getTypeName(), LabelStr("quantity")); \
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
    std::cout << " FlowProfile " << std::endl;

    testAddAndRemove< EUROPA::SAVH::FlowProfile> ();
    testScenario0< EUROPA::SAVH::FlowProfile>();
    testScenario1< EUROPA::SAVH::FlowProfile>();
    testScenario2< EUROPA::SAVH::FlowProfile>();
    testScenario3< EUROPA::SAVH::FlowProfile>();
    testScenario4< EUROPA::SAVH::FlowProfile>();
    testScenario5< EUROPA::SAVH::FlowProfile>();
    testScenario6< EUROPA::SAVH::FlowProfile>();
    testScenario7< EUROPA::SAVH::FlowProfile>();
    testScenario8< EUROPA::SAVH::FlowProfile>();
    testScenario9< EUROPA::SAVH::FlowProfile>( 0, 0 );
    testScenario9< EUROPA::SAVH::FlowProfile>( 1, 1 );
    //testScenario10< EUROPA::SAVH::FlowProfile>();

    std::cout << " IncrementalFlowProfile " << std::endl;

    testAddAndRemove< EUROPA::SAVH::IncrementalFlowProfile> ();
    testScenario0< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario1< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario2< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario3< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario4< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario5< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario6< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario7< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario8< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario9< EUROPA::SAVH::IncrementalFlowProfile>( 0, 0 );
    testScenario9< EUROPA::SAVH::IncrementalFlowProfile>( 1, 1 );
    testScenario10< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario11< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario12< EUROPA::SAVH::IncrementalFlowProfile>();
    testScenario13< EUROPA::SAVH::IncrementalFlowProfile>();

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

  static void executeScenario1( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[] ) {

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

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain(10, 15), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 5, 15), true, "t3" );
    Variable<IntervalIntDomain> t4( ce.getId(), IntervalIntDomain( 5, 15), true, "t4" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 1), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 1), true, "q3" );
    Variable<IntervalDomain> q4( ce.getId(), IntervalDomain(1, 1), true, "q4" );

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

  static void executeScenario2( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 1), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario3( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {
    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain(  0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 10, 10), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 10, 20), true, "t3" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 2), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 2), true, "q3" );

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

  static void executeScenario4( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {
    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain(  0, 5), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 10, 15), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 20, 25), true, "t3" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 2), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 2), true, "q3" );

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

  static void executeScenario5( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );
    

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 1), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    EqualConstraint c0(LabelStr("concurrent"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    ce.propagate();

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

    /*! BEING WORKED ON BY MICHAEL */

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

  }

  static void executeScenario7( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {
    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );
    
    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(2, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 2), true, "q2" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    EqualConstraint c0(LabelStr("concurrent"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    ce.propagate();

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario8( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {
    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 0, 10), true, "t2" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(2, 2), true, "q2" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), false);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), true ); 

    EqualConstraint c0(LabelStr("concurrent"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    ce.propagate();

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario9( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 1, 3), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain(10, 12), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 9, 9), true, "t3" );
    Variable<IntervalIntDomain> t4( ce.getId(), IntervalIntDomain( 11, 11), true, "t4" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 1), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 1), true, "q3" );
    Variable<IntervalDomain> q4( ce.getId(), IntervalDomain(1, 1), true, "q4" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), true);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), false ); 
    SAVH::Transaction trans3( t3.getId(), q3.getId(), true); 
    SAVH::Transaction trans4( t4.getId(), q4.getId(), false );

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );
    profile.addTransaction( trans4.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario10( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {

    /*!
     * Transaction1 constrained to be at Transaction2
     *
     * Transaction1   [0]-3
     * Transaction2         [10]+3
     * Transaction3         [10]-2
     * Transaction4                               [100]+2
     * Transaction5             <11------[-3]------100>
     * Transaction6                  <12----(+3)---100>
     *                 |     |    |   |             |
     *                 |     |    |   |             |
     * Max level(5)    2     3    3   3             5
     * Min level(5)    2     3    0   0             5
     *
     */

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain(0,0), true, "t1" );
    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(3, 3), true, "q1" );
    SAVH::Transaction trans1( t1.getId(), q1.getId(), true);

    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain(10, 10), true, "t2" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(3, 3), true, "q2" );
    SAVH::Transaction trans2( t2.getId(), q2.getId(), false ); 

    LessThanEqualConstraint c0(LabelStr("precedes"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain(10, 10), true, "t3" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(2, 2), true, "q3" );
    SAVH::Transaction trans3( t3.getId(), q3.getId(), true); 

    Variable<IntervalIntDomain> t4( ce.getId(), IntervalIntDomain(100, 100), true, "t4" );
    Variable<IntervalDomain> q4( ce.getId(), IntervalDomain(2, 2), true, "q4" );
    SAVH::Transaction trans4( t4.getId(), q4.getId(), false );

    LessThanEqualConstraint c1(LabelStr("precedes"), LabelStr("Temporal"), ce.getId() , makeScope(t3.getId(), t4.getId()));

    Variable<IntervalIntDomain> t5( ce.getId(), IntervalIntDomain(11, 100), true, "t5" );
    Variable<IntervalDomain> q5( ce.getId(), IntervalDomain(3, 3), true, "q5" );
    SAVH::Transaction trans5( t5.getId(), q5.getId(), true); 

    Variable<IntervalIntDomain> t6( ce.getId(), IntervalIntDomain(12, 100), true, "t6" );
    Variable<IntervalDomain> q6( ce.getId(), IntervalDomain(3, 3), true, "q6" );
    SAVH::Transaction trans6( t6.getId(), q6.getId(), false );

    LessThanEqualConstraint c2(LabelStr("precedes"), LabelStr("Temporal"), ce.getId() , makeScope(t5.getId(), t6.getId()));

    LessThanEqualConstraint c3(LabelStr("precedes"), LabelStr("Temporal"), ce.getId() , makeScope(t2.getId(), t3.getId()));
    LessThanEqualConstraint c4(LabelStr("precedes"), LabelStr("Temporal"), ce.getId() , makeScope(t2.getId(), t5.getId()));

    ce.propagate();

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );
    profile.addTransaction( trans4.getId() );
    profile.addTransaction( trans5.getId() );
    profile.addTransaction( trans6.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario11( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {
    /*!
     * Transaction1 constrained to be before or at Transaction2
     *
     * Transaction1   <-inf---------[-inf,0]---------inf>
     * Transaction2   <-inf---------[0, inf]---------inf>
     *                 |                              |
     *                 |                              |
     * Max level      inf                            inf
     * Min level     -inf                           -inf
     *
     */

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( MINUS_INFINITY, PLUS_INFINITY ), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( MINUS_INFINITY, PLUS_INFINITY ), true, "t2" );


    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(0, PLUS_INFINITY), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(0, PLUS_INFINITY), true, "q2" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), true);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), false ); 

    LessThanEqualConstraint c1(LabelStr("precedes"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), t2.getId()));

    ce.propagate();

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static void executeScenario12( SAVH::Profile& profile, ConstraintEngine& ce ) {
    /*!
     * Transaction1 constrained to be before or at Transaction2
     *
     * Transaction1   <-inf---------[-inf,0]---------inf>
     * Transaction2   <-inf---------[0, inf]---------inf>
     *                 |                              |
     *                 |                              |
     * Max level      inf                            inf
     * Min level     -inf                           -inf
     *
     */

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( MINUS_INFINITY, PLUS_INFINITY ), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( MINUS_INFINITY, PLUS_INFINITY ), true, "t2" );


    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(0, PLUS_INFINITY), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(0, PLUS_INFINITY), true, "q2" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), true);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), false ); 


    Variable<IntervalIntDomain> distance( ce.getId(), IntervalIntDomain( 1, PLUS_INFINITY ), true, "distance" );
    AddEqualConstraint c1(LabelStr("temporalDistance"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), distance.getId(), t2.getId()));

    ce.propagate();

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );

    std::cout << "    Case 1" << std::endl;

    profile.recompute();

    {
      const int nrInstances = 2;
      
      int itimes[nrInstances] = {MINUS_INFINITY,PLUS_INFINITY};
      double lowerLevels[nrInstances] = {MINUS_INFINITY,MINUS_INFINITY};
      double upperLevels[nrInstances] = {PLUS_INFINITY,PLUS_INFINITY};
      
      bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );
      
      assertTrue( profileMatches );
    }

    std::cout << "    Case 2" << std::endl;
    
    t1.restrictBaseDomain( IntervalIntDomain( 0, PLUS_INFINITY ) );

    ce.propagate();

    profile.recompute();

    {
      const int nrInstances = 3;
      
      int itimes[nrInstances] = {0,1,PLUS_INFINITY};
      double lowerLevels[nrInstances] = {MINUS_INFINITY,MINUS_INFINITY,MINUS_INFINITY};
      double upperLevels[nrInstances] = {0,PLUS_INFINITY,PLUS_INFINITY};
      
      bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );
      
      assertTrue( profileMatches );
    }

  }

  static void executeScenario13( SAVH::Profile& profile, ConstraintEngine& ce, int nrInstances, int itimes[], double lowerLevels[], double upperLevels[]  ) {
    /*!
     * Transaction1 constrained to be [1,inf) before Transaction2
     *
     * Transaction1   <0---------(-3)---------9>
     * Transaction2         <1---------(+3)---------10>
     * Transaction2   [0](-3)
     *                 |     |                |      |
     *                 |     |                |      |
     * Max level      -3    -3               -3      0
     * Min level      -6    -6               -6     -3
     *
     */

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 9 ), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain( 1,10 ), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 0, 0 ), true, "t3" );


    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(3, 3), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(3, 3), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(3, 3), true, "q3" );

    SAVH::Transaction trans1( t1.getId(), q1.getId(), true);
    SAVH::Transaction trans2( t2.getId(), q2.getId(), false ); 
    SAVH::Transaction trans3( t3.getId(), q3.getId(), true ); 

    Variable<IntervalIntDomain> distance( ce.getId(), IntervalIntDomain( 1, PLUS_INFINITY ), true, "distance" );
    AddEqualConstraint c1(LabelStr("temporalDistance"), LabelStr("Temporal"), ce.getId() , makeScope(t1.getId(), distance.getId(), t2.getId()));

    ce.propagate();

    profile.addTransaction( trans1.getId() );
    profile.addTransaction( trans2.getId() );
    profile.addTransaction( trans3.getId() );

    profile.recompute();

    bool profileMatches = verifyProfile( profile, nrInstances, itimes, lowerLevels, upperLevels );

    assertTrue( profileMatches );
  }

  static bool testNoTransactions() {
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    SAVH::IncrementalFlowProfile profile(ce.getId(), detector.getId());

    profile.recompute();
    return true;
  }
  static bool testOnePositiveTransaction() {
    return true;
  }
  static bool testOneNegativeTransaction() {
    return true;
  }

  template< class Profile >
  static bool testAddAndRemove(){
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    Profile profile(db.getId(), detector.getId());

    Variable<IntervalIntDomain> t1( ce.getId(), IntervalIntDomain( 0, 10), true, "t1" );
    Variable<IntervalIntDomain> t2( ce.getId(), IntervalIntDomain(10, 15), true, "t2" );
    Variable<IntervalIntDomain> t3( ce.getId(), IntervalIntDomain( 5, 15), true, "t3" );
    Variable<IntervalIntDomain> t4( ce.getId(), IntervalIntDomain( 5, 15), true, "t4" );

    Variable<IntervalDomain> q1( ce.getId(), IntervalDomain(1, 2), true, "q1" );
    Variable<IntervalDomain> q2( ce.getId(), IntervalDomain(1, 1), true, "q2" );
    Variable<IntervalDomain> q3( ce.getId(), IntervalDomain(1, 1), true, "q3" );
    Variable<IntervalDomain> q4( ce.getId(), IntervalDomain(1, 1), true, "q4" );

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

    //MJI- commented out because this can't happen.  
//     profile.addTransaction( trans1.getId() );
//     profile.addTransaction( trans2.getId() );
//     profile.addTransaction( trans3.getId() );
//     profile.addTransaction( trans4.getId() );

    return true;
  }

  template< class Profile >
  static bool testScenario0(){
    std::cout << "  Scenario 0" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    Profile profile( db.getId(), detector.getId());

    executeScenario0( profile, ce );
    return true;
  }

  template< class Profile >
  static bool testScenario1(){
    std::cout << "  Scenario 1" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());

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

    Profile profile( db.getId(), detector.getId());

    const int nrInstances = 4;

    int itimes[nrInstances] = {0,5,10,15};
    double lowerLevels[nrInstances] = {0,-1,-1,0};
    double upperLevels[nrInstances] = {1,2,2,0};

    executeScenario1( profile, ce, nrInstances, itimes, lowerLevels, upperLevels );

    return true;
  }

  template< class Profile >
  static bool testScenario2(){
    std::cout << "  Scenario 2" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
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

    Profile profile( db.getId(), detector.getId());

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {-1,0};
    double upperLevels[nrInstances] = {1,0};

    executeScenario2( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );
    return true;
  }

  template< class Profile >
  static bool testScenario3(){
    std::cout << "  Scenario 3" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
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

    Profile profile( db.getId(), detector.getId());

    const int nrInstances = 3;

    int itimes[nrInstances] = {0,10,20};
    double lowerLevels[nrInstances] = {0,-1,0};
    double upperLevels[nrInstances] = {2,3,3};


    executeScenario3( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );
    return true;
  }

  template< class Profile >
  static bool testScenario4(){
    std::cout << "  Scenario 4" << std::endl;

    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());

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
    Profile profile( db.getId(), detector.getId());

    const int nrInstances = 6;

    int itimes[nrInstances] = {0,5,10,15,20,25};
    double lowerLevels[nrInstances] = {0,1,-1,-1,-1,0};
    double upperLevels[nrInstances] = {2,2,2,1,3,3};

    executeScenario4( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );
    return true;
  }

  template< class Profile >
  static bool testScenario5(){
    std::cout << "  Scenario 5" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());

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
    Profile profile( db.getId(), detector.getId());

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {0, 0};
    double upperLevels[nrInstances] = {0, 0};

    executeScenario5( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );
    return true;
  }

  template< class Profile >
  static bool testScenario6(){
    std::cout << "  Scenario 6" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    Profile profile( db.getId(), detector.getId());

    executeScenario6( profile, ce );
    return true;
  }

  template< class Profile >
  static bool testScenario7(){
    std::cout << "  Scenario 7" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
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
    Profile profile( db.getId(), detector.getId());

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {0, 0};
    double upperLevels[nrInstances] = {1, 1};


    executeScenario7( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );
    return true;
  }

  template< class Profile >
  static bool testScenario8(){
    std::cout << "  Scenario 8" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
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
    Profile profile( db.getId(), detector.getId());

    const int nrInstances = 2;

    int itimes[nrInstances] = {0,10};
    double lowerLevels[nrInstances] = {-1, -1};
    double upperLevels[nrInstances] = {0, 0};

    executeScenario8( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );
    return true;
  }

  template< class Profile >
  static bool testScenario9( int initialLowerLevel, int initialUpperLevel ){
    std::cout << "  Scenario 9, initial levels [" 
	      << initialLowerLevel << ","
	      << initialUpperLevel << "]" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    /*!
     * No explicit ordering between transactions
     *
     * Transaction1   <1----(-1)----3>
     * Transaction2    |            |            <10--(+1)--12>
     * Transaction3    |            |  <9(-1)>    |          |
     * Transaction4    |            |    |        | <11(+1)> |
     *                 |            |    |        |    |     |
     *                 |            |    |        |    |     |
     * Min level(1)   -1           -1   -2       -2   -1     0
     * Max level(1)    0           -1   -2       -1    0     0
     *
     */

    Profile profile( db.getId(), detector.getId(), initialLowerLevel, initialUpperLevel );

    const int nrInstances = 6;

    int itimes[nrInstances] =         { 1, 3, 9,10,11,12};
    double lowerLevels[nrInstances] = {-1 + initialLowerLevel,-1 + initialLowerLevel,-2 + initialLowerLevel,-2 + initialLowerLevel,-1 + initialLowerLevel, 0 + initialLowerLevel };
    double upperLevels[nrInstances] = { 0 + initialUpperLevel,-1 + initialUpperLevel,-2 + initialUpperLevel,-1 + initialUpperLevel, 0 + initialUpperLevel, 0 + initialUpperLevel };

    executeScenario9( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );

    return true;
  }

  template< class Profile >
  static bool testScenario10(){
    std::cout << "  Scenario 10" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());

    Profile profile( db.getId(), detector.getId(), 5, 5);

    /*!
     * Transaction1 constrained to be at Transaction2
     *
     * Transaction1   [0]-3
     * Transaction2         [10]+3
     * Transaction3         [10]-2
     * Transaction4                               [100]+2
     * Transaction5             <11------[-3]------100>
     * Transaction6                  <12----(+3)---100>
     *                 |     |    |   |             |
     *                 |     |    |   |             |
     * Max level(5)    2     3    3   3             5
     * Min level(5)    2     3    0   0             5
     *
     */

    const int nrInstances = 5;

    int itimes[nrInstances] = {0,10,11,12,100};
    double lowerLevels[nrInstances] = {2,3,0,0,5};
    double upperLevels[nrInstances] = {2,3,3,3,5};

    executeScenario10( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );

    return true;
  }

  template< class Profile >
  static bool testScenario11(){
    std::cout << "  Scenario 11" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());

    Profile profile( db.getId(), detector.getId(), 0, 0);

    /*!
     * Transaction1 constrained to be before or at Transaction2
     *
     * Transaction1   <-inf---------[-inf,0]---------inf>
     * Transaction2   <-inf---------[0, inf]---------inf>
     *                 |                              |
     *                 |                              |
     * Max level      inf                            inf
     * Min level     -inf                           -inf
     *
     */

    const int nrInstances = 2;


    int itimes[nrInstances] = {MINUS_INFINITY,PLUS_INFINITY};
    double lowerLevels[nrInstances] = {MINUS_INFINITY,MINUS_INFINITY};
    double upperLevels[nrInstances] = {PLUS_INFINITY,PLUS_INFINITY};

    executeScenario11( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );

    return true;
  }

  template< class Profile >
  static bool testScenario12(){
    std::cout << "  Scenario 12" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    Profile profile( db.getId(), detector.getId());

    executeScenario12( profile, ce );
    return true;
  }

  template< class Profile >
  static bool testScenario13(){
    std::cout << "  Scenario 13" << std::endl;
    RESOURCE_DEFAULT_SETUP(ce, db, true);
    DummyDetector detector(SAVH::ResourceId::noId());
    Profile profile( db.getId(), detector.getId());
    /*!
     * Transaction1 constrained to be [1,inf) before Transaction2
     *
     * Transaction1   <0---------(-3)---------9>
     * Transaction2         <1---------(+3)---------10>
     * Transaction2   [0](-3)
     *                 |     |                |      |
     *                 |     |                |      |
     * Max level      -3    -3               -3     -3
     * Min level      -6    -6               -6     -3
     *
     */
    const int nrInstances = 4;


    int itimes[nrInstances] = {0,1,9,10};
    double lowerLevels[nrInstances] = {-6,-6,-6,-3};
    double upperLevels[nrInstances] = {-3,-3,-3,-3};

    executeScenario13( profile, ce, nrInstances, itimes, lowerLevels, upperLevels  );

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

class FVDetectorTest {
public:
  static bool test() {
    runTest(testReusableDetector);
    return true;
  }
private:
  static bool testReusableDetector() {
    RESOURCE_DEFAULT_SETUP(ce, db, false);
    
    SAVH::Reusable res(db.getId(), LabelStr("Reusable"), LabelStr("res1"), LabelStr("ReusableFVDetector"), LabelStr("FlowProfile"),
		       1, 1, 0);
    
    //create a token that violates the limit (i.e. consumes 2)
    SAVH::ReusableToken tok1(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(1), IntervalIntDomain(10), 
			     IntervalIntDomain(9), IntervalDomain(2));
    assertTrue(!ce.propagate());
    tok1.discard(false);
    
    //create a token that doesn't
    SAVH::ReusableToken tok2(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(1, 3), IntervalIntDomain(10, 12), IntervalIntDomain(9),
		       IntervalDomain(1));
    assertTrue(ce.propagate());
    //create a token that doesn't, but must start during the previous token, causing a violation
    SAVH::ReusableToken tok3(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(9), IntervalIntDomain(11), IntervalIntDomain(2),
		       IntervalDomain(1));
    assertTrue(!ce.propagate());
    tok3.discard(false);
    assertTrue(ce.propagate());
    //create a token that doesn't, and may start afterwards, creating a flaw
    SAVH::ReusableToken tok4(db.getId(), LabelStr("Reusable.uses"), IntervalIntDomain(10, 13), IntervalIntDomain(15, 18), IntervalIntDomain(5),
		       IntervalDomain(1));
    assertTrue(ce.propagate());
    assertTrue(db.hasOrderingChoice(tok4.getId()));
    std::cout << "CREATING THE CONSTRAINT" << std::endl;
    res.constrain(tok2.getId(), tok4.getId());
    assertTrue(ce.propagate());
    assertTrue(!db.hasOrderingChoice(tok4.getId()));
    RESOURCE_DEFAULT_TEARDOWN();
    return true;
  }
};

void FlowProfileModuleTests::runTests( const std::string& path) {
  LockManager::instance().connect();
  LockManager::instance().lock();
  setTestLoadLibraryPath(path);  

  
  Schema::instance();
  initConstraintLibrary();
  REGISTER_PROFILE(EUROPA::SAVH::FlowProfile, FlowProfile);
  REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile);
  REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);

  runTestSuite(DefaultSetupTest::test);
  runTestSuite(FlowProfileTest::test);
  runTestSuite(FVDetectorTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  uninitConstraintLibrary();
}



