#include "ResourceFlawDecisionPoint.hh"
#include "DecisionPoint.hh"
#include "Resource.hh"
#include "PlanDatabase.hh"
#include "Transaction.hh"
#include "Utils.hh"
#include <list>

//#define NOISE 10

// Uncomment this to allow "push beyond horizon" option for resolving flaws
// #define PUSH2HORIZON

namespace EUROPA {

#define collectFromSet(into,from,getProducers)                       \
  for ( std::set<TransactionId>::iterator trans_it = from.begin();	\
	trans_it != from.end(); ++trans_it ) {				\
    TransactionId tx = *trans_it;					\
    if ( getProducers && tx->canProduce() ||				\
	 !getProducers && tx->canConsume() ) {				\
      into.insert( tx );						\
    }									\
  }


  ResourceFlawDecisionPoint::ResourceFlawDecisionPoint(const ResourceId& resource) : DecisionPoint(resource->getPlanDatabase()->getClient(), resource), m_resource(resource) { m_choiceIndex = 0; }

  ResourceFlawDecisionPoint::~ResourceFlawDecisionPoint() {}

  void ResourceFlawDecisionPoint::initializeChoices() {
      // Get flaws from our resource
      std::list<ResourceFlawId> allFlaws;
      m_resource->getResourceFlaws( allFlaws );
      check_error( allFlaws.size()>0 );

      // Let's always work on the first flaw
      // @todo Is it possible that we cannot do anything about this flaw,
      // but can fix some later one? Scenario: Drive to INF with [-50,0] 
      ResourceFlawId& currFlaw = allFlaws.front();
#if (NOISE>5)
      std::cout<<"Working on flaw "<<currFlaw<<" ";
      currFlaw->getInstant()->print( std::cout );
      std::cout<<std::endl;
#endif

      // For now we only support two types of flaws
      if ( currFlaw->getType() != ResourceProblem::LevelTooLow &&
	   currFlaw->getType() != ResourceProblem::LevelTooHigh ) {
	check_error( ALWAYS_FAILS, "Currently supporting only LevelTooLow and LevelTooHigh flaws" );
      }

      std::set<TransactionId> toBeBefore;
      std::set<TransactionId> toBeAfter;

      // Collect all transactions that can fix the flaw:
      // producers for LevelTooLow and consumers for LevelTooHigh
      std::set<TransactionId> allResTransactions;
      m_resource->getTransactions(allResTransactions);
      bool getProducers = (currFlaw->getType() == ResourceProblem::LevelTooLow);
 
      collectFromSet( toBeBefore, allResTransactions, getProducers );

      // Get the list of dangerous transactions intersecting the flawed instant:
      // consumers for LevelTooLow and producers for LevelTooHigh
      collectFromSet( toBeAfter, currFlaw->getInstant()->getTransactions(), !getProducers );

      // there should be some dangerous transactions, or there would be no flaw 
      check_error( toBeAfter.size()>0 );

      check_error(m_id.isValid());

      // Idea:
      // Order the latest (max getErliest()) dangerous transaction intersecting with the
      // flawed instant after the ealirest (min getLatest()) fixing transaction which 
      // is not already before our bad guy
      // 
      // After this first and the most likely choice, put all other orderings
      // for completeness sake. I am still not sure we need it though.

      // Select the latest bad guy
      TransactionId badGuy;
      for ( std::set<TransactionId>::iterator cons_it = toBeAfter.begin();
	    cons_it != toBeAfter.end(); ++cons_it ) {
	TransactionId tx = *cons_it;
	// the new consumer is better than the current one
	if ( badGuy==TransactionId::noId() ||
	     badGuy->getEarliest() < tx->getEarliest() ) {
	  badGuy=tx;
	}
      }

      // The bad guy should be valid, because
      // somebody should be causing all the problems
      check_error( badGuy.isValid() );

      // Select the earliest transaction that could possibly resolve the flaw
      TransactionId goodGuy;
      for ( std::set<TransactionId>::iterator prod_it = toBeBefore.begin();
	    prod_it != toBeBefore.end(); ++prod_it ) {
	TransactionId tx = *prod_it;

	// if the good guy is already before the bad guy, forget about it
	if ( tx->getLatest() <= badGuy->getEarliest() ) {
	  continue;
	}

	// the new good guy is better than the current one
	if ( goodGuy==TransactionId::noId() ||
	     goodGuy->getLatest() > tx->getLatest() ) {
	  goodGuy = tx;
	}
      }

      // Best choice is good guy before the bad guy.
      m_choices.push_back(std::make_pair<TransactionId,TransactionId>(goodGuy, badGuy ));

      // Now create all other choices:
      // order each of the dangerous transactions intersecting the
      // first flawed instant before each of the fixing transactions, or push
      // the flaw beyond horizon

      for ( std::set<TransactionId>::iterator cons_it = toBeAfter.begin();
	    cons_it != toBeAfter.end(); ++cons_it ) {
	TransactionId badGuy = *cons_it;

	for ( std::set<TransactionId>::iterator prod_it = toBeBefore.begin();
	      prod_it != toBeBefore.end(); ++prod_it ) {
	  TransactionId goodGuy = *prod_it;

	  // if goodGuy is already before badGuy, forget about it
	  // forget also if we already selected this pair as the best choice above
	  if ( goodGuy->getLatest() <= badGuy->getEarliest() ||
	       goodGuy == m_choices[0].first && badGuy == m_choices[0].second) {
	    continue;
	  }

	  m_choices.push_back(std::make_pair<TransactionId,TransactionId>(goodGuy, badGuy ));
	}
      }
  }

  const bool ResourceFlawDecisionPoint::assign() { 
    check_error(m_choiceIndex <= m_choices.size(), "m_choiceIndex exceeded number of choices");

    if (m_choiceIndex == 0) initializeChoices();

    TransactionId after = m_choices[m_choiceIndex].second;
    // Put before constraint between transaction on the resource... 
    if ( m_choices[m_choiceIndex].first ==TransactionId::noId() ) {
#ifndef PUSH2HORIZON
      check_error( ALWAYS_FAILS, "ResourceFlawDecisionPoint::assign: Do not push beyond horizon any more" );
#endif

#if (NOISE>6)
      std::cout << " Pushing ";
      after->print( std::cout );
      std::cout << " after horizon "<<std::endl;
#endif
      
      // VERY VERY BAD HACK! Get horizon instead!
      int safelyAway = after->getLatest();
      after->setEarliest( safelyAway );
    } 
    else {
      // should get ID of the costraint here and save it
      // within the Choice so that we can retract it later
      m_dbClient->constrain( m_resource, m_choices[m_choiceIndex].first, after);
    }
    m_choiceIndex++;

    return true;
  }

  const bool ResourceFlawDecisionPoint::retract() {
    check_error( ALWAYS_FAILS, "ResourceFlawDecisionPoint::retract is not implemented" );
    return true; // to make compiler stop complaining
  }

  const bool ResourceFlawDecisionPoint::hasRemainingChoices() {
    if (m_choiceIndex == 0) return true; // we have never assigned this decision  or initialized choices
    return (! m_choiceIndex < m_choices.size());
  }

  void ResourceFlawDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    os << "(" << getKey() << ") Resource (" << m_entityKey << ") ";
    os << " Current Choice: ";
    if (m_choiceIndex == 0) os << "noId";
    else {
      if (m_choices[m_choiceIndex-1].first.isNoId())
	os << "HORIZON";
      else m_choices[m_choiceIndex-1].first->print(os);
      os << " before ";
      m_choices[m_choiceIndex-1].second->print(os);
    }
    os << " Discarded: " << m_choiceIndex << "  ";
  }

  std::ostream& operator <<(std::ostream& os, const Id<ResourceFlawDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
