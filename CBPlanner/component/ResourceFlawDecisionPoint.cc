#include "ResourceFlawDecisionPoint.hh"
#include "ResourceFlawChoice.hh"
#include "DecisionPoint.hh"
#include "Resource.hh"
#include "PlanDatabase.hh"
#include "Transaction.hh"
#include "Utils.hh"
#include <list>

//#define NOISE 10

// Uncomment this to allow "push beyond horizon" option for resolving flaws
// #define PUSH2HORIZON

namespace PLASMA {

  ResourceFlawDecisionPoint::ResourceFlawDecisionPoint(const ResourceId& resource) : DecisionPoint(resource->getPlanDatabase()->getClient(), resource), m_resource(resource) {}

  ResourceFlawDecisionPoint::~ResourceFlawDecisionPoint() {}

  const bool ResourceFlawDecisionPoint::assign(const ChoiceId& choice) { 
#if (NOISE>5)
    std::cout<<"ResourceFlawDecisionPoint::assign ";
    choice->print( std::cout );
    std::cout<<std::endl;
#endif

    check_error(choice.isValid());
    check_error(Id<ResourceFlawChoice>::convertable(choice));
    const Id<ResourceFlawChoice>& rfChoice = choice;
    // Put before constraint between transaction on the resource... 
    if ( rfChoice->getBefore()==TransactionId::noId() ) {
#ifndef PUSH2HORIZON
      check_error( ALWAYS_FAILS, "ResourceFlawDecisionPoint::assign: Do not push beyond horizon any more" );
#endif
#if (NOISE>6)
      std::cout << " Pushing ";
      rfChoice->getAfter()->print( std::cout );
      std::cout << " after horizon "<<std::endl;
#endif
      
      // VERY VERY BAD HACK! Get horizon instead!
      int safelyAway = rfChoice->getAfter()->getLatest();
      rfChoice->getAfter()->setEarliest( safelyAway );
    } 
    else {
      // should get ID of the costraint here and save it
      // within the Choice so that we can retract it later
      m_dbClient->constrain( m_resource,
			     rfChoice->getBefore(), 
			     rfChoice->getAfter()); 
    }

    // to satisfy check in DecisionPoint::assign
    m_current = ChoiceId::noId();
	
    return DecisionPoint::assign(choice);
  }

  const bool ResourceFlawDecisionPoint::retract() {
    check_error( ALWAYS_FAILS, "ResourceFlawDecisionPoint::retract is not implemented" );
    return true; // to make compiler stop complaining
    /*
      Remove constraints!!!

      check_error(Id<TokenChoice>::convertable(m_current));
      Id<TokenChoice> tChoice = m_current;
      m_dbClient->free(tChoice->getResourceFlaw(), m_token);
      return DecisionPoint::retract();
    */
  }

#define collectFromSet(into,from,getProducers)                       \
  for ( std::set<TransactionId>::iterator trans_it = from.begin();	\
	trans_it != from.end(); ++trans_it ) {				\
    TransactionId tx = *trans_it;					\
    if ( getProducers && tx->canProduce() ||				\
	 !getProducers && tx->canConsume() ) {				\
      into.insert( tx );						\
    }									\
  }


  std::list<ChoiceId>& ResourceFlawDecisionPoint::getUpdatedChoices() {
#if (NOISE>5)
    std::cout<<"ResourceFlawDecisionPoint::getUpdatedChoices"<<std::endl;
#endif
    return DecisionPoint::getUpdatedChoices();
  }

  std::list<ChoiceId>& ResourceFlawDecisionPoint::getChoices() {
#if (NOISE>5)
    std::cout<<"ResourceFlawDecisionPoint::getChoices"<<std::endl;
#endif

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
    cleanup(m_choices);
    m_choices.clear();

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

    // Now create the best choice: ...
    ResourceFlawChoiceId bestChoice;
    // ... either order the bad guy after the good guy...
    if ( goodGuy!=TransactionId::noId() ) {
      bestChoice = (new ResourceFlawChoice( m_id, goodGuy, badGuy ) )->getId();
    } else {
      // ... or, if there is no fixing transactions, push the flaw beyond horizon
#ifdef PUSH2HORIZON
      bestChoice = (new ResourceFlawChoice( m_id, badGuy ) )->getId();
#endif
    }
#if (NOISE>5)
    std::cout<<" Created the best choice "<<bestChoice<<std::endl;
#endif
    if ( bestChoice!=ResourceFlawChoiceId::noId() )
      m_choices.push_back( bestChoice );


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
	if ( goodGuy->getLatest() <= badGuy->getEarliest() ) {
	  continue;
	}

	// No "before" yet, so add it
	ResourceFlawChoiceId choice = (new ResourceFlawChoice( m_id, goodGuy, badGuy ) )->getId();
	if ( choice!=bestChoice ) {
#if (NOISE>5)
	  std::cout<<" Created a choice "<<choice<<std::endl;
#endif
	  m_choices.push_back( choice );
	}
      }

#ifdef PUSH2HORIZON
      // no try to push badGuy beyond horizon
      ChoiceId choice = (new ResourceFlawChoice( m_id, badGuy ) ) -> getId();
      if ( choice!=bestChoice ) {
#if (NOISE>5)
	std::cout<<" Created a choice "<<choice<<std::endl;
#endif
	m_choices.push_back( choice );
      }
#endif // PUSH2HORIZON
    }

    // This call takes care of removing discarded choices from the list
    return DecisionPoint::getChoices();	
  }

  void ResourceFlawDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    os << "(" << getKey() << ") Resource (" << m_entityKey << ") ";
    os << " Current Choice: " << m_current;
    os << " Discarded: " << m_discarded.size();
  }

  std::ostream& operator <<(std::ostream& os, const Id<ResourceFlawDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
