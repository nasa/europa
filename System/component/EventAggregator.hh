#ifndef _H_EventAggregator
#define _H_EventAggregator

#include "Id.hh"
#include "DecisionManagerListener.hh"
#include "ConstraintEngineListener.hh"
#include "DbClientListener.hh"
#include "PlanDatabaseListener.hh"
#include "RulesEngineListener.hh"
//#include "TemporalNetworkListener.hh"
#include "EventDefs.hh"
#include "AggregateListener.hh"
#include "CBPlannerDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
//#include "TemporalNetworkDefs.hh"
#include "PlanDatabase.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "Object.hh"

#include <set>

namespace EUROPA {
  
#define publish(message) { \
  for(std::set<AggregateListenerId>::iterator it = m_listeners.begin(); \
      it != m_listeners.end(); ++it) { \
    if((*it).isValid()) {(*it)->message;} \
    else {m_listeners.erase(it);} \
  } \
}
  
#define publishTwo(message1, message2) { \
  for(std::set<AggregateListenerId>::iterator it = m_listeners.begin(); \
      it != m_listeners.end(); ++it) { \
    if((*it).isValid()) {(*it)->message1; (*it)->message2;} \
    else {m_listeners.erase(it);} \
  } \
}
  
#define DECLARE_EVENT(name, prototype, args...) void name(prototype) {publish(name(args))}
#define DECLARE_EVENT_TWO(name1, name2, prototype, args...) void name1(prototype) {publish(name1(args)) publish(name2(args))}
#define DECLARE_EVENT_ARG(name, prototype1, prototype2, args...) void name(prototype1, prototype2) {publish(name(args))}
  
  
  
  class EventAggregatorErr {
  public:
    DECLARE_ERROR(NotInstantiatedError);
  };
  

  class EventAggregator {
  public:
    
    static EventAggregatorId instance(const DecisionManagerId& dm, 
                                      const ConstraintEngineId& ce,
                                      const PlanDatabaseId& db,
                                      const RulesEngineId& re);
    static EventAggregatorId instance() {return s_instance;}
    static void remove(){delete (EventAggregator *) s_instance;}
    void addListener(const AggregateListenerId& el) {m_listeners.insert(el);}
    void removeListener(const AggregateListenerId& el);

  protected:
  private:
          
    PlanDatabaseListenerId m_dbl;
    DecisionManagerListenerId m_dml;
    ConstraintEngineListenerId m_cel;
    DbClientListenerId m_dcl;
    RulesEngineListenerId m_rel;
//    TemporalNetworkListenerId m_tnl;

    std::set<AggregateListenerId> m_listeners;
    EventAggregatorId m_id;

    EventAggregator(const DecisionManagerId& dm, const ConstraintEngineId& ce,
                    const PlanDatabaseId& db, const RulesEngineId& re);
    ~EventAggregator();
    EventAggregatorId getId(){return m_id;}

  public:
    /****From DecisionManagerListener****/
    DECLARE_EVENT(notifyConditionAdded, const ConditionId& cond, cond);
    DECLARE_EVENT(notifyConditionRemoved, const ConditionId& cond, cond);
    DECLARE_EVENT(notifyConditionsChanged,);
    DECLARE_EVENT_ARG(notifyPassed, const EntityId& entity, const ConditionId& cond, entity, cond);
    DECLARE_EVENT_ARG(notifyFailed, const EntityId& entity, const ConditionId& cond, entity, cond);
    DECLARE_EVENT(notifyNewUnitDecision, const DecisionPointId& dec, dec);
    DECLARE_EVENT(notifyNewDecision, const DecisionPointId& dec, dec);
    DECLARE_EVENT(notifyRemovedDecision, const EntityId& entity, entity);
    DECLARE_EVENT(notifyAssignNextStarted, const DecisionPointId& dec, dec);
    DECLARE_EVENT_TWO(notifyAssignNextFailed, notifyStep, const DecisionPointId& dec, dec);
    DECLARE_EVENT_TWO(notifyAssignNextSucceeded, notifyStep, const DecisionPointId& dec, dec);
    //DECLARE_EVENT(notifyAssignNextFailed, const DecisionPointId& dec, dec);
    //DECLARE_EVENT(notifyAssignNextSucceeded, const DecisionPointId& dec, dec);
    DECLARE_EVENT(notifyAssignCurrentStarted, const DecisionPointId& dec, dec);
    DECLARE_EVENT_TWO(notifyAssignCurrentFailed, notifyStep, const DecisionPointId& dec, dec);
    DECLARE_EVENT_TWO(notifyAssignCurrentSucceeded, notifyStep, const DecisionPointId& dec, dec);
    //DECLARE_EVENT(notifyAssignCurrentFailed, const DecisionPointId& dec, dec);
    //DECLARE_EVENT(notifyAssignCurrentSucceeded, const DecisionPointId& dec, dec);
    DECLARE_EVENT(notifyRetractStarted, const DecisionPointId& dec, dec);
    DECLARE_EVENT(notifyRetractFailed, const DecisionPointId& dec, dec);
    DECLARE_EVENT(notifyRetractSucceeded, const DecisionPointId& dec, dec);
    DECLARE_EVENT(notifySearchFinished,);
    DECLARE_EVENT(notifyPlannerTimeout,);

    /****From ConstraintEngineListener****/
    DECLARE_EVENT(notifyPropagationCommenced,);
    DECLARE_EVENT(notifyPropagationCompleted,);
    DECLARE_EVENT(notifyPropagationPreempted,);
    DECLARE_EVENT(notifyAdded, const ConstraintId& constr, constr);
    DECLARE_EVENT(notifyRemoved, const ConstraintId& constr, constr);
    DECLARE_EVENT(notifyExecuted, const ConstraintId& constr, constr);
    DECLARE_EVENT(notifyAdded, const ConstrainedVariableId& var, var);
    DECLARE_EVENT(notifyRemoved, const ConstrainedVariableId& var, var);
    void notifyChanged(const ConstrainedVariableId& var, const DomainListener::ChangeType& changeType);

    /****From DbClientListener****/
    DECLARE_EVENT(notifyObjectCreated, const ObjectId& obj, obj);
    DECLARE_EVENT_ARG(notifyObjectCreated, const ObjectId& obj, const std::vector<ConstructorArgument>& args, obj, args);
    DECLARE_EVENT(notifyClosed,);
    DECLARE_EVENT(notifyClosed, const LabelStr& objType, objType);
    DECLARE_EVENT(notifyTokenCreated, const TokenId& tok, tok);
    //void notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor)
    //{publish(notifyConstrained(object,token,successor));}
    //DECLARE_EVENT_ARG(notifyFreed, const ObjectId& obj, const TokenId& tok, obj, tok);
    //    DECLARE_EVENT(notifyActivated, const TokenId& tok, tok);
    DECLARE_EVENT_ARG(notifyMerged, const TokenId& tok, const TokenId& actTok, tok, actTok);
    //DECLARE_EVENT(notifyRejected, const TokenId& tok, tok);
    DECLARE_EVENT(notifyCancelled, const TokenId& tok, tok);
    DECLARE_EVENT(notifyConstraintCreated, const ConstraintId& constr, constr);
    DECLARE_EVENT(notifyVariableCreated, const ConstrainedVariableId& var, var);
    DECLARE_EVENT(notifyVariableSpecified, const ConstrainedVariableId& var, var);
    DECLARE_EVENT(notifyVariableClosed, const ConstrainedVariableId& var, var);
    DECLARE_EVENT(notifyVariableReset, const ConstrainedVariableId& var, var);

    /****From PlanDatabaseListener****/
    DECLARE_EVENT(notifyAdded, const ObjectId& obj, obj);
    DECLARE_EVENT(notifyRemoved, const ObjectId& obj, obj);
    DECLARE_EVENT(notifyAdded, const TokenId& obj, obj);
    DECLARE_EVENT(notifyRemoved, const TokenId& tok, tok);
    DECLARE_EVENT(notifyClosed, const TokenId& tok, tok);
    DECLARE_EVENT(notifyActivated, const TokenId& tok, tok);
    DECLARE_EVENT(notifyDeactivated, const TokenId& tok, tok);
    DECLARE_EVENT(notifyMerged, const TokenId& tok, tok);
    DECLARE_EVENT(notifySplit, const TokenId& tok, tok);
    DECLARE_EVENT(notifyRejected, const TokenId& tok, tok);
    DECLARE_EVENT(notifyReinstated, const TokenId& tok, tok);
    void notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor) {
      publish(notifyConstrained(object, token, successor));
    }
    DECLARE_EVENT_ARG(notifyFreed, const ObjectId& obj, const TokenId& tok, obj, tok);
    DECLARE_EVENT_ARG(notifyAdded, const ObjectId& obj, const TokenId& tok, obj, tok);
    DECLARE_EVENT_ARG(notifyRemoved, const ObjectId& obj, const TokenId& tok, obj, tok);

    /****From RulesEngineListener****/
    DECLARE_EVENT(notifyExecuted, const RuleInstanceId& rule, rule);
    DECLARE_EVENT(notifyUndone, const RuleInstanceId& rule, rule);

    /****From TemporalNetworkListener****/
//     DECLARE_EVENT_ARG(notifyTimepointAdded, const TempVarId& var, const TimepointId& timepoint,
//                       var, timepoint);
//     DECLARE_EVENT(notifyTimepointDeleted, const TimepointId& timepoint, timepoint);
//     void notifyBaseDomainConstraintAdded(const TempVarId& c, const TemporalConstraintId& constraint, Time lb, Time ub) {
//       publish(notifyBaseDomainConstraintAdded(c, constraint, lb, ub));
//     }
//     void notifyConstraintAdded(const ConstraintId c, const TemporalConstraintId& constraint, Time lb, Time ub) {
//       publish(notifyConstraintAdded(c, constraint, lb, ub));
//     }
//     DECLARE_EVENT_ARG(notifyConstraintDeleted, int key, const TemporalConstraintId& constraint, key, constraint);
//     void notifyBoundsRestricted(const TempVarId& v, Time newlb, Time newub) {
//       publish(notifyBoundsRestricted(v, newlb, newub));
//     }
//     DECLARE_EVENT_ARG(notifyBoundsSame, const TempVarId& v, const TimepointId& timepoint, v, timepoint);

    class DMListener : public DecisionManagerListener {
    public:
      DMListener(const DecisionManagerId& dm, const EventAggregatorId& ea) :
        DecisionManagerListener(dm), m_ea(ea){}
    protected:
    private:
      void notifyConditionAdded(const ConditionId& cond) {m_ea->notifyConditionAdded(cond);}
      void notifyConditionRemoved(const ConditionId& cond) {m_ea->notifyConditionRemoved(cond);}
      void notifyConditionsChanged() {m_ea->notifyConditionsChanged();}
      void notifyPassed(const EntityId& entity, const ConditionId& cond) {m_ea->notifyPassed(entity, cond);}
      void notifyFailed(const EntityId& entity, const ConditionId& cond) {m_ea->notifyFailed(entity, cond);}
      void notifyNewUnitDecision(const DecisionPointId& dec) {m_ea->notifyNewUnitDecision(dec);}
      void notifyNewDecision(const DecisionPointId& dec) {m_ea->notifyNewDecision(dec);}
      void notifyRemovedDecision(const EntityId& entity) {m_ea->notifyRemovedDecision(entity);}
      void notifyAssignNextStarted(const DecisionPointId& dec) {m_ea->notifyAssignNextStarted(dec);} // previous dec
      void notifyAssignNextFailed(const DecisionPointId& dec) {m_ea->notifyAssignNextFailed(dec);} // current dec
      void notifyAssignNextSucceeded(const DecisionPointId& dec) {m_ea->notifyAssignNextSucceeded(dec);} // current dec
      void notifyAssignCurrentStarted(const DecisionPointId& dec) {m_ea->notifyAssignCurrentStarted(dec);} // current dec
      void notifyAssignCurrentFailed(const DecisionPointId& dec) {m_ea->notifyAssignCurrentFailed(dec);} // current dec
      void notifyAssignCurrentSucceeded(const DecisionPointId& dec) {m_ea->notifyAssignCurrentSucceeded(dec);} // current dec
      void notifyRetractStarted(const DecisionPointId& dec) {m_ea->notifyRetractStarted(dec);} // current dec
      void notifyRetractFailed(const DecisionPointId& dec) {m_ea->notifyRetractFailed(dec);}  // current dec
      void notifyRetractSucceeded(const DecisionPointId& dec) {m_ea->notifyRetractSucceeded(dec);}  // current dec
      void notifySearchFinished() {m_ea->notifySearchFinished();}
      void notifyPlannerTimeout() {m_ea->notifyPlannerTimeout();}

      EventAggregatorId m_ea;
    };
    
    class CEListener : public ConstraintEngineListener {
    public:
      CEListener(const ConstraintEngineId& ce, const EventAggregatorId& ea) :
        ConstraintEngineListener(ce), m_ea(ea){}
    protected:
    private:
      void notifyPropagationCommenced() {m_ea->notifyPropagationCommenced();}
      void notifyPropagationCompleted() {m_ea->notifyPropagationCompleted();}
      void notifyPropagationPreempted() {m_ea->notifyPropagationPreempted();}
      void notifyAdded(const ConstraintId& constraint) {m_ea->notifyAdded(constraint);}
      void notifyRemoved(const ConstraintId& constraint) {m_ea->notifyRemoved(constraint);}
      void notifyExecuted(const ConstraintId& constraint) {m_ea->notifyExecuted(constraint);}
      void notifyAdded(const ConstrainedVariableId& variable) {m_ea->notifyAdded(variable);}
      void notifyRemoved(const ConstrainedVariableId& variable) {m_ea->notifyRemoved(variable);}
      void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType) 
      {m_ea->notifyChanged(variable, changeType);}

      EventAggregatorId m_ea;
    };
    
    class DBCListener : public DbClientListener {
    public:
      DBCListener(const DbClientId& dbc, const EventAggregatorId& ea) :
        DbClientListener(dbc), m_ea(ea){}
    protected:
    private:
      void notifyObjectCreated(const ObjectId& object) {m_ea->notifyObjectCreated(object);}
      void notifyObjectCreated(const ObjectId& object, const std::vector<ConstructorArgument>& arguments) 
      {m_ea->notifyObjectCreated(object, arguments);}
      void notifyClosed() {m_ea->notifyClosed();}
      void notifyClosed(const LabelStr& objectType) {m_ea->notifyClosed(objectType);}
      void notifyTokenCreated(const TokenId& token) {m_ea->notifyTokenCreated(token);}
      void notifyMerged(const TokenId& token, const TokenId& activeToken) {m_ea->notifyMerged(token);}
      void notifyCancelled(const TokenId& token) {m_ea->notifyCancelled(token);}
      void notifyConstraintCreated(const ConstraintId& constraint) {m_ea->notifyConstraintCreated(constraint);}
      void notifyVariableCreated(const ConstrainedVariableId& variable) {m_ea->notifyVariableCreated(variable);}
      void notifyVariableSpecified(const ConstrainedVariableId& variable) {m_ea->notifyVariableSpecified(variable);}
      void notifyVariableClosed(const ConstrainedVariableId& variable) {m_ea->notifyVariableClosed(variable);}
      void notifyVariableReset(const ConstrainedVariableId& variable) {m_ea->notifyVariableReset(variable);}
      
      EventAggregatorId m_ea;
    };

    class PDListener : public PlanDatabaseListener {
    public:
      PDListener(const PlanDatabaseId& pdb, const EventAggregatorId& ea) :
        PlanDatabaseListener(pdb), m_ea(ea){}
    protected:
    private:
      void notifyAdded(const ObjectId& object) {m_ea->notifyAdded(object);}
      void notifyRemoved(const ObjectId& object) {m_ea->notifyRemoved(object);}
      void notifyAdded(const TokenId& token) {m_ea->notifyAdded(token);}
      void notifyRemoved(const TokenId& token) {m_ea->notifyRemoved(token);}
      void notifyClosed(const TokenId& token) {m_ea->notifyClosed(token);}
      void notifyActivated(const TokenId& token) {m_ea->notifyActivated(token);}
      void notifyDeactivated(const TokenId& token) {m_ea->notifyDeactivated(token);}
      void notifyMerged(const TokenId& token) {m_ea->notifyMerged(token);}
      void notifySplit(const TokenId& token) {m_ea->notifySplit(token);}
      void notifyRejected(const TokenId& token) {m_ea->notifyRejected(token);}
      void notifyReinstated(const TokenId& token) {m_ea->notifyReinstated(token);}
      void notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor)
      {m_ea->notifyConstrained(object, token, successor);}
      void notifyFreed(const ObjectId& object, const TokenId& token) {m_ea->notifyFreed(object, token);}
      void notifyAdded(const ObjectId& object, const TokenId& token) {m_ea->notifyAdded(object, token);}
      void notifyRemoved(const ObjectId& object, const TokenId& token) {m_ea->notifyRemoved(object, token);}

      EventAggregatorId m_ea;
    };
    
    class REListener : public RulesEngineListener {
    public:
      REListener(const RulesEngineId& re, const EventAggregatorId& ea) :
        RulesEngineListener(re), m_ea(ea){}
    protected:
    private:
      void notifyExecuted(const RuleInstanceId &rule) {m_ea->notifyExecuted(rule);}
      void notifyUndone(const RuleInstanceId &rule) {m_ea->notifyUndone(rule);}
      
      EventAggregatorId m_ea;
    };
    
//     class TNListener : public TemporalNetworkListener {
//     public:
//       TNListener(const TemporalPropagatorId& tn, const EventAggregatorId& ea) :
//         TemporalNetworkListener(tn), m_ea(ea){}
//     protected:
//     private:
//       void notifyTimepointAdded(const TempVarId& var, const TimepointId& timepoint) 
//       {m_ea->notifyTimepointAdded(var, timepoint);}
//       void notifyTimepointDeleted(const TimepointId& timepoint) {m_ea->notifyTimepointDeleted(timepoint);}
//       void notifyBaseDomainConstraintAdded(const TempVarId& c, const TemporalConstraintId& constraint, Time lb, Time ub)
//       {m_ea->notifyBaseDomainConstraintAdded(c, constraint, lb, ub);}
//       void notifyConstraintAdded(const ConstraintId c, const TemporalConstraintId& constraint, Time lb, Time ub)
//       {m_ea->notifyConstraintAdded(c, constraint, lb, ub);}
//       void notifyConstraintDeleted(int key, const TemporalConstraintId& constraint)
//       {m_ea->notifyConstraintDeleted(key, constraint);}
//       void notifyBoundsRestricted(const TempVarId& v, Time newlb, Time newub)
//       {m_ea->notifyBoundsRestricted(v, newlb, newub);}
//       void notifyBoundsSame(const TempVarId& v,  const TimepointId& timepoint)
//       {m_ea->notifyBoundsSame(v, timepoint);}

//       EventAggregatorId m_ea;
//     };
    
    static EventAggregatorId s_instance;
  };
}

#undef DECLARE_EVENT
#undef DECLARE_EVENT_ARG

#endif
