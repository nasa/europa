#ifndef _H_AggregateListener
#define _H_AggregateListener

/**
 * @file   AggregateListener.hh
 * @brief Defines a class for listening to various EUROPA2 events.
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @date   Thu Jan 13 16:41:12 2005
 * @ingroup System
 */

#include "EventDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
//#include "TemporalNetworkDefs.hh"
#include "CBPlannerDefs.hh"
#include "Object.hh"

namespace EUROPA {

  /*@def DECLARE_EVENT
   *@brief A small macro for defining a default message-reciept function with zero or one argument.
   *@param name the name of the message.
   *@param prototype the single- or no-argument prototype of the function.
   */
#define DECLARE_EVENT(name, prototype) virtual void name(prototype) {}

  /*@def DECLARE_EVENT_ARG
   *@brief A small macro for defining a default message-reciept function with two arguments.
   *@param name the name of the message.
   *@param prototype1 the prototype of the first argument.
   *@param prototype2 the prototype of the second argument.
   */
#define DECLARE_EVENT_ARG(name, prototype1, prototype2) virtual void name(prototype1, prototype2) {}
  

  /**
   *@class AggregateListener
   *@brief Listens for various EUROPA2 events.
   *
   *The AggregateListener class provides the capability to listen for events on a variety of EUROPA2's modules without
   *having to create a listener for each one or inherit multiply.  All methods have a default no-op definition so
   *listeners only have to implement the relevant ones.
   */
  class AggregateListener {
  public:
    /**
     *@brief Constructor.  Automatically adds the instance to the EventAggregator's list of listeners.
     */
    AggregateListener();

    /**
     *@brief Destructor.  Automatically removes the instance from the EventAggregator's list of listeners.
     */
    virtual ~AggregateListener();

    /**
     *@brief Returns the AggregateListener's Id.
     *@return the AggregateListener's Id.
     */
    AggregateListenerId getId() const {return m_id;}
    
    /****Utility methods****/

    /**
     *@brief Receive a notification that a "step" has occurred.
     *@param dec the DecisionPoint that was decided.
     */
    virtual void notifyStep(const DecisionPointId& dec) {m_step++;}

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been relaxed.
     *@param var the ConstrainedVariable whose domain has been relaxed.
     */
    DECLARE_EVENT(notifyRelaxed, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notifiaction that a ConstrainedVariable's domain has been reset.
     *@param var the ConstrainedVariable whose domain has been reset.
     */
    DECLARE_EVENT(notifyReset, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been emptied.
     *@param var the ConstrainedVariable whose domain has been emptied.
     */
    DECLARE_EVENT(notifyEmptied, const ConstrainedVariableId& var);
    
    /**
     *@brief Receive a notification that the upper bound of a ConstrainedVariable's domain has been decreased.
     *@param var the ConstrainedVariable whose domain has been restricted.
     */
    DECLARE_EVENT(notifyUpperBoundDecreased, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that the lower bound of a ConstrainedVariable's domain has been increased.
     *@param var the ConstrainedVariable whose domain has been restricted.
     */
    DECLARE_EVENT(notifyLowerBoundIncreased, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that the bounds of a ConstrainedVariable's domain have been restricted.
     *@param var the ConstrainedVariable whose domain has been restricted.
     */
    DECLARE_EVENT(notifyBoundsRestricted, const ConstrainedVariableId& var);
    
    /**
     *@brief Receive a notification that a value has been removed from a ConstrainedVarible's domain.
     *@param var the ConstrainedVariable whose domain has had an element removed.
     */
    DECLARE_EVENT(notifyValueRemoved, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been restricted to a singleton.
     *@param var the ConstrainedVariable whose domain has been restricted.
     */
    DECLARE_EVENT(notifyRestrictToSingleton, const ConstrainedVariableId& var);
    
    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been set.
     *@param var the ConstrainedVariable whose domain has been set.
     */
    DECLARE_EVENT(notifySet, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been set to a singleton.
     *@param var the ConstrainedVariable whose domain has been set to a singleton.
     */
    DECLARE_EVENT(notifySetToSingleton, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been closed.
     *@param var the ConstrainedVariable whose domain has been closed.
     */
    DECLARE_EVENT(notifyClosed, const ConstrainedVariableId& var);

    /****From DecisionManagerListener****/

    /**
     *@brief Receive a notification that a Condition has been added.
     *@param cond the Condition that was added.
     */
    DECLARE_EVENT(notifyConditionAdded, const ConditionId& cond);

    /**
     *@brief Receive a notification that a Condition has been removed.
     *@param cond the Condition that was removed.
     */
    DECLARE_EVENT(notifyConditionRemoved, const ConditionId& cond);
    
    /**
     *@brief Receive a notification that Conditions have changed.
     */
    DECLARE_EVENT(notifyConditionsChanged,);

    /**
     *@brief Receive a notification that an Entity has passed a Condition.
     *@param entity the Entity that passed.
     *@param cond the Condition that was passed.
     */
    DECLARE_EVENT_ARG(notifyPassed, const EntityId& entity, const ConditionId& cond);

    /**
     *@brief Receive a notification that an Entity has failed a Condition.
     *@param entity the Entity that failed.
     *@param cond the Condition that was failed.
     */
    DECLARE_EVENT_ARG(notifyFailed, const EntityId& entity, const ConditionId& cond);

    /**
     *@brief Receive a notification that a new unit decision exists.
     *@param dec the DecisionPoint.
     */
    DECLARE_EVENT(notifyNewUnitDecision, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that a new decision exists.
     *@param dec the new DecisionPoint.
     */
    DECLARE_EVENT(notifyNewDecision, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that a Decision has been removed.
     */
    DECLARE_EVENT(notifyRemovedDecision, const EntityId& entity);

    /**
     *@brief Receive a notification that the planner has started assigning a new decision.
     *@param dec the DecisionPoint being decided.
     */
    DECLARE_EVENT(notifyAssignNextStarted, const DecisionPointId& dec);
    
    /**
     *@brief Receive a notification that the decision that was made has led to an inconsistency
     *@param dec the DecisionPoint that was made.
     */
    DECLARE_EVENT(notifyAssignNextFailed, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that the decision was successfully made.
     *@param dec the DecisionPoint that was made.
     */
    DECLARE_EVENT(notifyAssignNextSucceeded, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that the planner has started assigning a new choice for the current decision.
     *@param dec the DecisionPoint being decided.
     */
    DECLARE_EVENT(notifyAssignCurrentStarted, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that the decision that was made has led to an inconsistency.
     *@param dec the DecisionPoint that was made.
     */
    DECLARE_EVENT(notifyAssignCurrentFailed, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that the decision was successfully made.
     *@param dec the DecisionPoint that was made.
     */
    DECLARE_EVENT(notifyAssignCurrentSucceeded, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that the planner has begun retracting a decision.
     *@param dec the DecisionPoint being retracted.
     */
    DECLARE_EVENT(notifyRetractStarted, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that the retraction led to an inconsistency or otherwise failed.
     *@param dec the DecisionPoint being retracted.
     */
    DECLARE_EVENT(notifyRetractFailed, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that the retraction was succesful.
     *@param dec the DecisionPoint that was retracted.
     */
    DECLARE_EVENT(notifyRetractSucceeded, const DecisionPointId& dec);

    /**
     *@brief Receive a notification that a plan was found.
     */
    DECLARE_EVENT(notifySearchFinished,);

    /**
     *@brief Receive a notification that the planner failed to find a plan.
     */
    DECLARE_EVENT(notifyPlannerTimeout,);

    /****From ConstraintEngineListener****/
    /**
     *@brief Receive a notification that constraint propagation has begun.
     */
    DECLARE_EVENT(notifyPropagationCommenced,);
    
    /**
     *@brief Receive a notification that constraint propagation completed.
     */
    DECLARE_EVENT(notifyPropagationCompleted,);

    /**
     *@brief Receive a notification that propagation was preempted.
     */
    DECLARE_EVENT(notifyPropagationPreempted,);

    /**
     *@brief Receive a notification that a Constraint was added to the ConstraintEngine.
     *@param constr the Constraint that was added.
     */
    DECLARE_EVENT(notifyAdded, const ConstraintId& constr);

    /**
     *@brief Receive a notification that a Constraint was removed from the ConstraintEngine.
     *@param constr the Constraint that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const ConstraintId& constr);

    /**
     *@brief Receive a notification that a Constraint was executed.
     *@param constr the Constraint that was executed.
     */
    DECLARE_EVENT(notifyExecuted, const ConstraintId& constr);

    /**
     *@brief Receive a notification that a ConstrainedVariable was added to the ConstraintEngine.
     *@param var the ConstrainedVariable that was added.
     */;
    DECLARE_EVENT(notifyAdded, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable was removed from the ConstraintEngine.
     *@param var the ConstrainedVariable that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const ConstrainedVariableId& var);
    
    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has changed.
     *@param var the ConstrainedVariable whose domain has changed.
     *@param changeType the type of domain change that occurred.
     */
    DECLARE_EVENT_ARG(notifyChanged, const ConstrainedVariableId& var, const DomainListener::ChangeType& changeType);

    /****From DbClientListener****/
    /**
     *@brief Receive a notification that an Object was created in the PlanDatabase.
     *@param obj the Object that was created.
     */
    DECLARE_EVENT(notifyObjectCreated, const ObjectId& obj);

    /**
     *@brief Receive a notification that an Object was created with particular constructor arguments.
     *@param obj the Object that was created.
     *@param args the arguments to the Object's constructor.
     */
    DECLARE_EVENT_ARG(notifyObjectCreated, const ObjectId& obj, const std::vector<ConstructorArgument>& args);

    /**
     *@brief Receive a notification that the PlanDatabase has been closed.
     */
    DECLARE_EVENT(notifyClosed,);

    /**
     *@brief Receive a notification that a particular Object type has been closed.
     *@param objType the Object type that has been closed.
     */
    DECLARE_EVENT(notifyClosed, const LabelStr& objType);

    /**
     *@brief Receive a notification that a Token has been created.
     *@param tok the Token that was created.
     */
    DECLARE_EVENT(notifyTokenCreated, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been merged.
     *@param tok the Token that was merged.
     *@param actTok the active Token with with tok has been merged.
     */
    DECLARE_EVENT_ARG(notifyMerged, const TokenId& tok, const TokenId& actTok);

    /**
     *@brief Receive a notification that a Token has been cancelled.
     *@param tok the Token that was cancelled.
     */
    DECLARE_EVENT(notifyCancelled, const TokenId& tok);

    /**
     *@brief Receive a notification that a Constraint has been created.
     *@param constr the Constraint that was created.
     */
    DECLARE_EVENT(notifyConstraintCreated, const ConstraintId& constr);

    /**
     *@brief Receive a notification that a ConstrainedVaraible has been created.
     *@param var the ConstrainedVariable that was created.
     */
    DECLARE_EVENT(notifyVariableCreated, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been specified.
     *@param var the ConstrainedVariable whose domain has been specified.
     */
    DECLARE_EVENT(notifyVariableSpecified, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been closed.
     *@param var the ConstrainedVariable whose domain has been closed.
     */
    DECLARE_EVENT(notifyVariableClosed, const ConstrainedVariableId& var);

    /**
     *@brief Receive a notification that a ConstrainedVariable's domain has been reset.
     *@param var the ConstrainedVariable whose domain has been reset.
     */
    DECLARE_EVENT(notifyVariableReset, const ConstrainedVariableId& var);

    /****From PlanDatabaseListener****/
    /**
     *@brief Receive a notification that an Object has been added to the PlanDatabase.
     *@param obj the Object that was added.
     */
    DECLARE_EVENT(notifyAdded, const ObjectId& obj);

    /**
     *@brief Receive a notification that an Object has been removed from the PlanDatabase.
     *@param obj the Object that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const ObjectId& obj);
    
    /**
     *@brief Receive a notification that a Token has been added to the PlanDatabase.
     *@param tok the Token that was added.
     */
    DECLARE_EVENT(notifyAdded, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been removed from the PlanDatabase.
     *@param tok the Token that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been closed.
     *@param tok the Token that was closed.
     */
    DECLARE_EVENT(notifyClosed, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been activated.
     *@param tok the Token that was activated.
     */
    DECLARE_EVENT(notifyActivated, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been deactivated.
     *@param tok the Token that was deactivated.
     */
    DECLARE_EVENT(notifyDeactivated, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been merged.
     *@param tok the Token that was merged.
     */
    DECLARE_EVENT(notifyMerged, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been split.
     *@param tok the Token that was split.
     */
    DECLARE_EVENT(notifySplit, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been rejected.
     *@param tok the Token that was rejected.
     */
    DECLARE_EVENT(notifyRejected, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been reinstated.
     *@param tok the Token that was reinstated.
     */
    DECLARE_EVENT(notifyReinstated, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been constrained to be a predecessor to another Token on a particular Object.
     *@param object the Object on which the constraining is occurring.
     *@param token the Token being contrained.
     *@param successor the successor Token.
     */
    virtual void notifyConstrained(const ObjectId& object, const TokenId &token, const TokenId& successor){}

    /**
     *@brief Receive a notification that a Token has been freed from an Object.
     *@param obj the Object from which the Token was freed.
     *@param tok the Token that was freed.
     */
    DECLARE_EVENT_ARG(notifyFreed, const ObjectId& obj, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token was added onto an Object.
     *@param obj the Object onto which the Token was added.
     *@param tok the Token that was added.
     */
    DECLARE_EVENT_ARG(notifyAdded, const ObjectId& obj, const TokenId& tok);

    /**
     *@brief Receive a notification that a Token has been removed from an Object.
     *@param obj the Object from which the Token was removed.
     *@param tok the Token that was removed.
     */
    DECLARE_EVENT_ARG(notifyRemoved, const ObjectId& obj, const TokenId& tok);

    /****From RulesEngineListener****/
    /**
     *@brief Receive a notification that a rule was executed.
     *@param rule the RuleInstance that was created.
     */
    DECLARE_EVENT(notifyExecuted, const RuleInstanceId& rule);

    /**
     *@brief Receive a notification that a rule was undone.
     *@param rule the RuleInstance that was removed.
     */
    DECLARE_EVENT(notifyUndone, const RuleInstanceId& rule);
    
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
