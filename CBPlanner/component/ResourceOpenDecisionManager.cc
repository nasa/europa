#include "ResourceDefs.hh"
#include "Choice.hh"
#include "TokenChoice.hh"
#include "ResourceOpenDecisionManager.hh"
#include "Resource.hh"
#include "DMResourceListener.hh"
#include "ResourceFlawDecisionPoint.hh"

namespace PLASMA {

  ResourceOpenDecisionManager::ResourceOpenDecisionManager(const DecisionManagerId& dm) : DefaultOpenDecisionManager(dm) {
  }

  // The destructor is not virtual
  ResourceOpenDecisionManager::~ResourceOpenDecisionManager() {
    cleanup(m_resDecs);
  }

  /**
   * If a new Resource has been created, create and hook up a Listener for it.
   * Otherwise do nothing.
   */
  void ResourceOpenDecisionManager::add(const ObjectId& object) {
	
    // if the object is Resource, set up a listener for it
    if ( ResourceId::convertable(object) ) {
      const ResourceId& resource = object;
      resource->add( (new DMResourceListener( m_id, resource ))->getId() );
    } 
	
    // Do not chain call DefaultOpenDecisionManager, because it always fails here
    // DefaultOpenDecisionManager::add( object );
  }

  
  /**
   * This method is called if there was a change in status AND if there was no change.
   * Basic idea: 
   *  if the resource is not flawed and we have an open DP for it, remove the DP
   *  if the resource is flawed and we do not have an open DP for it, create one
   * In any case, we do not care if there are any DPs for this resource sitting in 
   * the closed decisions list.
   */
  void ResourceOpenDecisionManager::notifyResourceState( const DMResourceListenerId& rl, bool isFlawed ) {
    if (loggingEnabled())
      std::cout<<"RODM notified of change of status of resource "<<rl->getResource()->getKey()<<" to "<<isFlawed<<std::endl;

    // The current decision point. If it is still in our list, it is open.
    std::map<int,ResourceFlawDecisionPointId>::iterator dp_it = m_resDecs.find( rl->getResource()->getKey() );

    // Creating a new DP if there is one open
    if ( isFlawed && dp_it==m_resDecs.end() ) {
      // create a new decision point
      ResourceFlawDecisionPointId dp = (new ResourceFlawDecisionPoint(rl->getResource()))->getId();
      check_error(dp->getEntityKey() == rl->getResource()->getKey());
      m_resDecs.insert(std::pair<int,ResourceFlawDecisionPointId>(dp->getEntityKey(),dp));
      publishNewDecision(dp);
    } 

    // Remove the open DP if we do not need it any more
    if ( !isFlawed && dp_it!=m_resDecs.end() ) {
      // remove the decision point
      delete (DecisionPoint*) dp_it->second;
      m_resDecs.erase( dp_it );
      // I hope this does not conflict with ObjectDPs. Resource IS an Object.
      publishRemovedDecision( rl->getResource() );
    }
  }

  const int ResourceOpenDecisionManager::getNumberOfDecisions() {
    /// std::cout<<"RODM is asked getNumberOfDecisions"<<std::endl;
    return m_objDecs.size() + m_unitVarDecs.size() + m_nonUnitVarDecs.size() + m_tokDecs.size() 
      + m_resDecs.size();
  }

  DecisionPointId ResourceOpenDecisionManager::getNextDecision() {
    //// std::cout<<"RODM "<<m_id<<" asked getNextDecision"<<std::endl;

    if(!m_objDecs.empty())
      m_curDec = m_objDecs.begin()->second;
    else if (!m_sortedUnitVarDecs.empty())
      m_curDec = *m_sortedUnitVarDecs.begin();
    else if (!m_sortedTokDecs.empty()) 
      m_curDec = *m_sortedTokDecs.begin();
    else if (!m_sortedNonUnitVarDecs.empty()) 
      m_curDec = *m_sortedNonUnitVarDecs.begin();
    else if (!m_resDecs.empty()) {
      m_curDec = m_resDecs.begin()->second;
      // kick it out now
      // this works IFF getNextDecision is only called to actually 
      // get a decision to work on
      m_resDecs.erase( m_resDecs.begin() );
      publishRemovedDecision( m_curDec );
    }
    else m_curDec = DecisionPointId::noId();

    // Shold be able to require that current choices are empty
    check_error(m_curDec.isNoId() || m_curDec->getCurrentChoices().empty());

    return m_curDec;
  }

  void ResourceOpenDecisionManager::getOpenDecisions(std::list<DecisionPointId>& decisions) {
    // collect from parent
    DefaultOpenDecisionManager::getOpenDecisions( decisions );
    // add resource flaw DPs
    std::map<int,ResourceFlawDecisionPointId>::iterator oit = m_resDecs.begin();
    for (; oit != m_resDecs.end(); ++oit) {
      decisions.push_back(oit->second);
    }
  }

  void ResourceOpenDecisionManager::printOpenDecisions(std::ostream& os) {
    DefaultOpenDecisionManager::printOpenDecisions( os );
    std::multimap<int,ResourceFlawDecisionPointId>::iterator oit = m_resDecs.begin();
    for (; oit != m_resDecs.end(); ++oit) {
      os << oit->second << std::endl;
    }
  }

}
