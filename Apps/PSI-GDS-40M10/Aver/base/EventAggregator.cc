#include "EventAggregator.hh"

namespace EUROPA {

  EventAggregatorId EventAggregator::s_instance = EventAggregatorId::noId();

  
  //EventAggregator::EventAggregator(const DecisionManagerId& dm,
  EventAggregator::EventAggregator(const SOLVERS::SolverId& dm,
                                   const ConstraintEngineId& ce,
                                   const PlanDatabaseId& db,
                                   const RulesEngineId& re) :
    m_dbl(PlanDatabaseListenerId::noId()), m_dml(SOLVERS::SearchListenerId::noId()),
    m_cel(ConstraintEngineListenerId::noId()), m_dcl(DbClientListenerId::noId()),
    m_rel(RulesEngineListenerId::noId()), /*m_tnl(TemporalNetworkListenerId::noId()),*/ m_id(this) {
    if(dm.isValid())
      m_dml = (new PlannerListener(dm, m_id))->getId();
    if(ce.isValid()) {
      m_cel = (new CEListener(ce, m_id))->getId();
//       const PropagatorId& prop = ce->getPropagatorByName(LabelStr("Temporal"));
//       if(prop.isValid())
//         m_tnl = (new TNListener((TemporalPropagatorId&) prop, m_id))->getId();
    }
    if(db.isValid()) {
      m_dbl = (new PDListener(db, m_id))->getId();
      m_dcl = (new DBCListener(db->getClient(), m_id))->getId();
    }
    if(re.isValid())
      m_rel = (new REListener(re, m_id))->getId();
  }

  EventAggregator::~EventAggregator() {
    if(m_dbl.isValid())
      delete (PlanDatabaseListener*) m_dbl;
    if(m_dml.isValid())
      delete (PlannerListener*) m_dml;
    if(m_cel.isValid())
      delete (ConstraintEngineListener*) m_cel;
    if(m_dcl.isValid())
      delete (DbClientListener*) m_dcl;
    if(m_rel.isValid())
      delete (RulesEngineListener*) m_rel;
    //if(m_tnl.isValid())
    //  delete (TemporalNetworkListener*) m_tnl;
    m_id.remove();
  }

  EventAggregatorId EventAggregator::instance(const SOLVERS::SolverId& dm, 
                                              const ConstraintEngineId& ce,
                                              const PlanDatabaseId& db,
                                              const RulesEngineId& re) {
    if(!s_instance.isNoId())
      return s_instance;
    s_instance = (new EventAggregator(dm, ce, db, re))->getId();
    return s_instance;
  }
  
  void EventAggregator::removeListener(const AggregateListenerId& el) {
    std::set<AggregateListenerId>::iterator it = m_listeners.find(el);
    if(it != m_listeners.end())
      m_listeners.erase(it);
  }

  void EventAggregator::notifyChanged(const ConstrainedVariableId& var, 
                                      const DomainListener::ChangeType& changeType) {
    publishMessage(notifyChanged(var, changeType));
    switch(changeType) {
    case DomainListener::RELAXED:
      publishMessage(notifyRelaxed(var));
      break;
    case DomainListener::RESET:
      publishMessage(notifyReset(var));
      break;
    case DomainListener::VALUE_REMOVED:
      publishMessage(notifyValueRemoved(var));
      break;
    case DomainListener::BOUNDS_RESTRICTED:
      publishMessage(notifyBoundsRestricted(var));
      break;
    case DomainListener::LOWER_BOUND_INCREASED:
      publishMessage(notifyLowerBoundIncreased(var));
      break;
    case DomainListener::UPPER_BOUND_DECREASED:
      publishMessage(notifyUpperBoundDecreased(var));
      break;
    case DomainListener::RESTRICT_TO_SINGLETON:
      publishMessage(notifyRestrictToSingleton(var));
      break;
    case DomainListener::SET_TO_SINGLETON:
      publishMessage(notifySetToSingleton(var));
      break;
    case DomainListener::EMPTIED:
      publishMessage(notifyEmptied(var));
      break;
    case DomainListener::CLOSED:
      publishMessage(notifyClosed(var));
      break;
    default:
      break;
    }
  }
}
