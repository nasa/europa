#include "EventAggregator.hh"

namespace Prototype {

  EventAggregatorId EventAggregator::s_instance = EventAggregatorId::noId();

  
  EventAggregator::EventAggregator(const DecisionManagerId& dm,
                                   const ConstraintEngineId& ce,
                                   const PlanDatabaseId& db,
                                   const RulesEngineId& re) :
    m_dbl(PlanDatabaseListenerId::noId()), m_dml(DecisionManagerListenerId::noId()),
    m_cel(ConstraintEngineListenerId::noId()), m_dcl(DbClientListenerId::noId()),
    m_rel(RulesEngineListenerId::noId()), /*m_tnl(TemporalNetworkListenerId::noId()),*/ m_id(this) {
    if(dm.isValid())
      m_dml = (new DMListener(dm, m_id))->getId();
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
      delete (DecisionManagerListener*) m_dml;
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

  EventAggregatorId EventAggregator::instance(const DecisionManagerId& dm, 
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
    publish(notifyChanged(var, changeType));
    switch(changeType) {
    case DomainListener::RELAXED:
      publish(notifyRelaxed(var));
      break;
    case DomainListener::RESET:
      publish(notifyReset(var));
      break;
    case DomainListener::VALUE_REMOVED:
      publish(notifyValueRemoved(var));
      break;
    case DomainListener::BOUNDS_RESTRICTED:
      publish(notifyBoundsRestricted(var));
      break;
    case DomainListener::LOWER_BOUND_INCREASED:
      publish(notifyLowerBoundIncreased(var));
      break;
    case DomainListener::UPPER_BOUND_DECREASED:
      publish(notifyUpperBoundDecreased(var));
      break;
    case DomainListener::RESTRICT_TO_SINGLETON:
      publish(notifyRestrictToSingleton(var));
      break;
    case DomainListener::SET:
      publish(notifySet(var));
      break;
    case DomainListener::SET_TO_SINGLETON:
      publish(notifySetToSingleton(var));
      break;
    case DomainListener::EMPTIED:
      publish(notifyEmptied(var));
      break;
    case DomainListener::CLOSED:
      publish(notifyClosed(var));
      break;
    default:
      break;
    }
  }
}
