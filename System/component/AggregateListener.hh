#ifndef _H_AggregateListener
#define _H_AggregateListener

#include "EventDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
//#include "TemporalNetworkDefs.hh"
#include "CBPlannerDefs.hh"
#include "Object.hh"

namespace EUROPA {

#define DECLARE_EVENT(name, prototype) virtual void name(prototype) {}
#define DECLARE_EVENT_ARG(name, prototype1, prototype2) virtual void name(prototype1, prototype2) {}
  
  class AggregateListener {
  public:
    AggregateListener();
    virtual ~AggregateListener();
    
    /****Utility methods****/
    virtual void notifyStep(const DecisionPointId& dec) {m_step++;}
    DECLARE_EVENT(notifyRelaxed, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyReset, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyEmptied, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyUpperBoundDecreased, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyLowerBoundIncreased, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyBoundsRestricted, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyValueRemoved, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyRestrictToSingleton, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifySet, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifySetToSingleton, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyClosed, const ConstrainedVariableId& var);

    /****From DecisionManagerListener****/
    DECLARE_EVENT(notifyConditionAdded, const ConditionId& cond);
    DECLARE_EVENT(notifyConditionRemoved, const ConditionId& cond);
    DECLARE_EVENT(notifyConditionsChanged,);
    DECLARE_EVENT_ARG(notifyPassed, const EntityId& entity, const ConditionId& cond);
    DECLARE_EVENT_ARG(notifyFailed, const EntityId& entity, const ConditionId& cond);
    DECLARE_EVENT(notifyNewUnitDecision, const DecisionPointId& dec);
    DECLARE_EVENT(notifyNewDecision, const DecisionPointId& dec);
    DECLARE_EVENT(notifyRemovedDecision, const EntityId& entity);
    DECLARE_EVENT(notifyAssignNextStarted, const DecisionPointId& dec);
    DECLARE_EVENT(notifyAssignNextFailed, const DecisionPointId& dec);
    DECLARE_EVENT(notifyAssignNextSucceeded, const DecisionPointId& dec);
    DECLARE_EVENT(notifyAssignCurrentStarted, const DecisionPointId& dec);
    DECLARE_EVENT(notifyAssignCurrentFailed, const DecisionPointId& dec);
    DECLARE_EVENT(notifyAssignCurrentSucceeded, const DecisionPointId& dec);
    DECLARE_EVENT(notifyRetractStarted, const DecisionPointId& dec);
    DECLARE_EVENT(notifyRetractFailed, const DecisionPointId& dec);
    DECLARE_EVENT(notifyRetractSucceeded, const DecisionPointId& dec);
    DECLARE_EVENT(notifySearchFinished,);
    DECLARE_EVENT(notifyPlannerTimeout,);

    /****From ConstraintEngineListener****/
    DECLARE_EVENT(notifyPropagationCommenced,);
    DECLARE_EVENT(notifyPropagationCompleted,);
    DECLARE_EVENT(notifyPropagationPreempted,);
    DECLARE_EVENT(notifyAdded, const ConstraintId& constr);
    DECLARE_EVENT(notifyRemoved, const ConstraintId& constr);
    DECLARE_EVENT(notifyExecuted, const ConstraintId& constr);
    DECLARE_EVENT(notifyAdded, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyRemoved, const ConstrainedVariableId& var);
    DECLARE_EVENT_ARG(notifyChanged, const ConstrainedVariableId& var, const DomainListener::ChangeType& changeType);

    /****From DbClientListener****/
    DECLARE_EVENT(notifyObjectCreated, const ObjectId& obj);
    DECLARE_EVENT_ARG(notifyObjectCreated, const ObjectId& obj, const std::vector<ConstructorArgument>& args);
    DECLARE_EVENT(notifyClosed,);
    DECLARE_EVENT(notifyClosed, const LabelStr& objType);
    DECLARE_EVENT(notifyTokenCreated, const TokenId& tok);
    DECLARE_EVENT_ARG(notifyMerged, const TokenId& tok, const TokenId& actTok);
    DECLARE_EVENT(notifyCancelled, const TokenId& tok);
    DECLARE_EVENT(notifyConstraintCreated, const ConstraintId& constr);
    DECLARE_EVENT(notifyVariableCreated, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyVariableSpecified, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyVariableClosed, const ConstrainedVariableId& var);
    DECLARE_EVENT(notifyVariableReset, const ConstrainedVariableId& var);

    /****From PlanDatabaseListener****/
    DECLARE_EVENT(notifyAdded, const ObjectId& obj);
    DECLARE_EVENT(notifyRemoved, const ObjectId& obj);
    DECLARE_EVENT(notifyAdded, const TokenId& obj);
    DECLARE_EVENT(notifyRemoved, const TokenId& tok);
    DECLARE_EVENT(notifyClosed, const TokenId& tok);
    DECLARE_EVENT(notifyActivated, const TokenId& tok);
    DECLARE_EVENT(notifyDeactivated, const TokenId& tok);
    DECLARE_EVENT(notifyMerged, const TokenId& tok);
    DECLARE_EVENT(notifySplit, const TokenId& tok);
    DECLARE_EVENT(notifyRejected, const TokenId& tok);
    DECLARE_EVENT(notifyReinstated, const TokenId& tok);
    virtual void notifyConstrained(const ObjectId& object, const TokenId &token, const TokenId& successor){}
    DECLARE_EVENT_ARG(notifyFreed, const ObjectId& obj, const TokenId& tok);
    DECLARE_EVENT_ARG(notifyAdded, const ObjectId& obj, const TokenId& tok);
    DECLARE_EVENT_ARG(notifyRemoved, const ObjectId& obj, const TokenId& tok);

    /****From RulesEngineListener****/
    DECLARE_EVENT(notifyExecuted, const RuleInstanceId& rule);
    DECLARE_EVENT(notifyUndone, const RuleInstanceId& rule);

    /****From TemporalNetworkListener****/
//     DECLARE_EVENT_ARG(notifyTimepointAdded, const TempVarId& var, const TimepointId& timepoint);
//     DECLARE_EVENT(notifyTimepointDeleted, const TimepointId& timepoint);
//     virtual void notifyBaseDomainConstraintAdded(const TempVarId& c, 
//                                                  const TemporalConstraintId& constraint, 
//                                                  Time lb, Time ub){}
//     virtual void notifyConstraintAdded(const ConstraintId c, const TemporalConstraintId& constraint, 
//                                        Time lb, Time ub) {}
//     DECLARE_EVENT_ARG(notifyConstraintDeleted, int key, const TemporalConstraintId& constraint);
//     virtual void notifyBoundsRestricted(const TempVarId& v, Time newlb, Time newub) {}
//     DECLARE_EVENT_ARG(notifyBoundsSame, const TempVarId& v, const TimepointId& timepoint);
    
  protected:
    virtual int currentStep(){return m_step;}
  private:
    int m_step;
    AggregateListenerId m_id;
  };

#undef DECLARE_EVENT
#undef DECLARE_EVENT_ARG
}
#endif
