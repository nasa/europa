#ifndef _H_EventAggregator
#define _H_EventAggregator

/**
 * @file   EventAggregator.hh
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @brief Defines a class that aggregates EUROPA events.
 * @date   Thu Jan 13 14:25:08 2005
 * @ingroup System
 */

#include "Id.hh"
//#include "DecisionManagerListener.hh"
#include "SearchListener.hh"
#include "ConstraintEngineListener.hh"
#include "DbClientListener.hh"
#include "PlanDatabaseListener.hh"
#include "RulesEngineListener.hh"
//#include "TemporalNetworkListener.hh"
#include "EventDefs.hh"
#include "AggregateListener.hh"
//#include "CBPlannerDefs.hh"
#include "SolverDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
//#include "TemporalNetworkDefs.hh"
#include "PlanDatabase.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "Object.hh"
#include "Solver.hh"

#include <set>

namespace EUROPA {

  /**
   *@def publishMessage
   *@brief A small macro for publishing a message to all available listeners.
   *@param messsage the message to publish.
   */
#define publishMessage(message) { \
  for(std::set<AggregateListenerId>::iterator it = m_listeners.begin(); \
      it != m_listeners.end(); ++it) { \
    if((*it).isValid()) {(*it)->message;} \
    else {m_listeners.erase(it);} \
  } \
}
  
  /**
   *@def publishTwo
   *@brief A macro for publishing two messages to all available listeners.
   *@param message1 the first message to publish
   *@param message2 the second message to publish
   */
#define publishTwo(message1, message2) { \
  for(std::set<AggregateListenerId>::iterator it = m_listeners.begin(); \
      it != m_listeners.end(); ++it) { \
    if((*it).isValid()) {(*it)->message1; (*it)->message2;} \
    else {m_listeners.erase(it);} \
  } \
}
  
  /**
   *@def DECLARE_EVENT
   *@brief A macro for declaring a publishable message.
   *@param name the name of the message.
   *@param prototype the single- or no-argument prototype of the message function.
   *@param args optional arguments to the message published.
   */
#define DECLARE_EVENT(name, prototype, args...) void name(prototype) {publishMessage(name(args))}

  /**
   *@def DECLARE_EVENT_TWO
   *@brief A macro for declaring an event that publishes two messages with identical prototypes and arguments.
   *@param name1 the name of the first message to publish.
   *@param name2 the name of the second message to publish.
   *@param prototype the single- or no-argument prototype of the message function.
   *@param args optional arguments to the messages published.
   */
#define DECLARE_EVENT_TWO(name1, name2, prototype, args...) void name1(prototype) {publishMessage(name1(args)) publishMessage(name2(args))}


  /**
   *@def DECLARE_EVENT_ARG
   *@brief A macro for declaring a publishable message with two arguments.
   *@param name the name of the message.
   *@param prototype1 the prototype for the first argument to the message.
   *@param prototype2 the prototype for the second argument to the message.
   *@param args optional arguments to the message published.
   */
#define DECLARE_EVENT_ARG(name, prototype1, prototype2, args...) void name(prototype1, prototype2) {publishMessage(name(args))}
  
  
  
  class EventAggregatorErr {
  public:
    DECLARE_ERROR(NotInstantiatedError);
  };
  

  /**
   *@class EventAggregator
   *@brief Aggregates EUROPA2 events.
   *
   * The EventAggregator class is a singleton that registers listeners with the various components of EUROPA2 and redispatches them
   * to AggregateListeners.  This greatly simplifies any class that needs to listen to more than one component of EUROPA2.
   */
  class EventAggregator {
  public:
    
    /**
     *@brief Singleton "constructor".  Does nothing if called twice without an interstitial call to EventAggregator::remove().
     *@param dm the DecisionManager to listen to.
     *@param ce the ConstraintEngine to listen to.
     *@param db the PlanDatabase to listen to.
     *@param re the RulesEngine to listen to.
     *@return the Id for the pre-existing EventAggregator or the newly instantiated one.
     */
    static EventAggregatorId instance(const SOLVERS::SolverId& dm, //const DecisionManagerId& dm, 
                                      const ConstraintEngineId& ce,
                                      const PlanDatabaseId& db,
                                      const RulesEngineId& re);

    /**
     *@brief Singleton accessor.
     *@return the Id for the existing EventAggregator.
     */
    static EventAggregatorId instance() {return s_instance;}

    /**
     *@brief Deletes the current EventAggregator.
     */
    static void remove(){delete (EventAggregator *) s_instance;}

    /**
     *@brief Add an AggregateListener to the EventAggregator.
     *@param el the listener to add.
     */
    void addListener(const AggregateListenerId& el) {m_listeners.insert(el);}

    /**
     *@brief Remove an AggregateListener from the EventAggregator.
     *@param el the listener to remove.
     */
    void removeListener(const AggregateListenerId& el);

  protected:
  private:
          
    PlanDatabaseListenerId m_dbl; /*!< The internal listener for the PlanDatabase. */
    //DecisionManagerListenerId m_dml; /*!< The internal listener for the DecisionManager. */
    SOLVERS::SearchListenerId m_dml; /*!< The internal listener for the search. */
    ConstraintEngineListenerId m_cel; /*!< The internal listener for the ConstraintEngine. */
    DbClientListenerId m_dcl; /*!< The internal listener for the DbClient. */
    RulesEngineListenerId m_rel; /*!< The internal listener for the RulesEngine. */
//    TemporalNetworkListenerId m_tnl;

    std::set<AggregateListenerId> m_listeners; /*!< The set of listeners on this EventAggregator. */
    EventAggregatorId m_id; /*!< The Id for this EventAggregator. */

    /**
     *@brief Constructor.  Creates a listener for each component whose Id is valid.
     *@param dm the DecisionManager to listen to.  Can be a noId.
     *@param ce the ConstraintEngine to listen to.  Can be a noId.
     *@param db the PlanDatabase to listen to.  Can be a noId.
     *@param re the RulesEngine to listen to.  Can be a noId.
     */
    //EventAggregator(const DecisionManagerId& dm, const ConstraintEngineId& ce,
    EventAggregator(const SOLVERS::SolverId& dm, const ConstraintEngineId& ce,
                    const PlanDatabaseId& db, const RulesEngineId& re);

    /**
     *@brief Destructor.  Un-registers all internal listeners.
     */
    ~EventAggregator();

    /**
     *@brief Gets the Id for this EventAggregator.
     *@return the Id for this EventAggregator.
     */
    EventAggregatorId getId(){return m_id;}

  public:
    /**
     *@brief Notify listeners that a "step" has occured.
     *@param dp the decision that was made.
     */
    DECLARE_EVENT(notifyStep, const SOLVERS::DecisionPointId& dp = SOLVERS::DecisionPointId::noId(), dp);

    /****From DecisionManagerListener****/
    /**
     *@brief Notify listeners that a codition was added to the DecisionManager.
     *@param cond the condition that was added.
     */
    //DECLARE_EVENT(notifyConditionAdded, const ConditionId& cond, cond);
    
    /**
     *@brief Notify listeners that a Condition was removed from the DecisionManager.
     *@param cond the Condition that was removed.
     */
    //DECLARE_EVENT(notifyConditionRemoved, const ConditionId& cond, cond);

    /**
     *@brief Notify listeners that Conditions have changed.
     */
    //DECLARE_EVENT(notifyConditionsChanged,);

    /**
     *@brief Notify listeners that an Entity has passed a Condition.
     *@param entity the Entity that passed.
     *@param the Condition that was passed.
     */
    //DECLARE_EVENT_ARG(notifyPassed, const EntityId& entity, const ConditionId& cond, entity, cond);

    /**
     *@brief Notify listeners that an Entity has failed a Condition.
     *@param entity the Entity that failed.
     *@param the Condition that failed.
     */
    //DECLARE_EVENT_ARG(notifyFailed, const EntityId& entity, const ConditionId& cond, entity, cond);

    /**
     *@brief Notify listeners of a new Unit Decision.
     *@param dec the Unit Decision.
     */
    //DECLARE_EVENT(notifyNewUnitDecision, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners of a new DecisionPoint.
     *@param dec the new DecisionPoint.
     */
    DECLARE_EVENT(notifyNewDecision, const SOLVERS::DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners of an undone DecisionPoint.
     *@param dec the retracted DecisionPoint.
     */
    DECLARE_EVENT(notifyUndone, const SOLVERS::DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that an Entity has been removed from the set of decidable Entities.
     *@param entity the Entity that was removed.
     */
    DECLARE_EVENT(notifyRemovedDecision, const SOLVERS::DecisionPointId& entity, entity);

    /**
     *@brief Notify listeners that the planner has started assigning a new decision.
     *@param dec the new DecisionPoint.
     */
    //DECLARE_EVENT(notifyAssignNextStarted, const DecisionPointId& dec, dec);
    
    /**
     *@brief Notify listeners that the choice made led to an inconsistency.
     *@param dec the failed DecisionPoint.
     */
    //DECLARE_EVENT_TWO(notifyAssignNextFailed, notifyStep, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that the choice made was successful.
     *@param dec the DecisionPoint that was made.
     */
    //DECLARE_EVENT_TWO(notifyAssignNextSucceeded, notifyStep, const DecisionPointId& dec, dec);
    
    /**
     *@brief Notify listeners that the planner is making a new choice on a failed DecisionPoint.
     *@param dec the DecisionPoint being decided.
     */
    //DECLARE_EVENT(notifyAssignCurrentStarted, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that the choice made led to an inconsistency.
     *@param dec the failed DecisionPoint.
     */
    //DECLARE_EVENT_TWO(notifyAssignCurrentFailed, notifyStep, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that the choice made was successful.
     *@param dec the DecisionPoint that was made.
     */
    //DECLARE_EVENT_TWO(notifyAssignCurrentSucceeded, notifyStep, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that the planner has begun retracting a decision that was made.
     *@param dec the DecisionPoint being retracted.
     */
    //DECLARE_EVENT(notifyRetractStarted, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that the retraction failed.
     *@param dec the DecisionPoint that was being retracted.
     *@note This is bad.
     */
    //DECLARE_EVENT(notifyRetractFailed, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that retraction was successful.
     *@param dec the DecisionPoint that was retracted.
     */
    //DECLARE_EVENT(notifyRetractSucceeded, const DecisionPointId& dec, dec);

    /**
     *@brief Notify listeners that the search is complete--a plan was found.
     */
    DECLARE_EVENT(notifySearchFinished,);

    /**
     *@brief Notify listeners that the planner failed to find a plan due to timeout.
     */
    DECLARE_EVENT(notifyPlannerTimeout,);

    /**
     *@brief Notify listeners that the planner failed to find a plan due to
     *search space exhaustion.
     */
    DECLARE_EVENT(notifySearchExhausted,);

    /****From ConstraintEngineListener****/

    /**
     *@brief Notify listeners that constraint propagation has commenced.
     */
    DECLARE_EVENT(notifyPropagationCommenced,);
    
    /**
     *@brief Notify listeners that constraint propagation completed successfully.
     */
    DECLARE_EVENT(notifyPropagationCompleted,);
    
    /**
     *@brief Notify listeners that constraint propagation led to an inconsistency and so was preempted.
     */
    DECLARE_EVENT(notifyPropagationPreempted,);

    /**
     *@brief Notify listeners that a Constraint has been added to the ConstraintEngine.
     *@param constr the Constraint that was added.
     */
    DECLARE_EVENT(notifyAdded, const ConstraintId& constr, constr);

    /**
     *@brief Notify listeners that a Constraint has been removed from the ConstraintEngine.
     *@param constr the Constraint that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const ConstraintId& constr, constr);
    
    /**
     *@brief Notify listeners that a Constraint has executed.
     *@param constr the Constraint that executed.
     */
    DECLARE_EVENT(notifyExecuted, const ConstraintId& constr, constr);
    
    /**
     *@brief Notify listeners that a ConstrainedVariable has been added to the ConstraintEngine.
     *@param var the ConstrainedVariable that was added.
     */
    DECLARE_EVENT(notifyAdded, const ConstrainedVariableId& var, var);

    /**
     *@brief Notify listeners that a ConstrainedVariable has been removed from the ConstraintEngine.
     *@param var the ConstrainedVariable that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const ConstrainedVariableId& var, var);

    /**
     *@brief Notify listeners that the domain of a ConstrainedVariable has changed.
     *@param var the ConstrainedVarable whose domain has changed.
     *@param changeType the type of change.
     */
    void notifyChanged(const ConstrainedVariableId& var, const DomainListener::ChangeType& changeType);

    /****From DbClientListener****/

    /**
     *@brief Notify listeners that a new Object has been created in the PlanDatabase.
     *@param obj the Object that was created.
     */
    DECLARE_EVENT(notifyObjectCreated, const ObjectId& obj, obj);

    /**
     *@brief Notify listeners that a new Object has been created in the PlanDatabase with particular constructor arguments.
     *@param ob the Object that was created.
     *@param args the arguments to the Object's constructor.
     */
    DECLARE_EVENT_ARG(notifyObjectCreated, const ObjectId& obj, const std::vector<ConstructorArgument>& args, obj, args);

    /**
     *@brief Notify listeners that the PlanDatabase has been closed.
     */
    DECLARE_EVENT(notifyClosed,);

    /**
     *@brief Notify listeners that Objects of a particular type have been closed.
     *@param objType the type of Object that has been closed.
     */
    DECLARE_EVENT(notifyClosed, const LabelStr& objType, objType);

    /**
     *@brief Notify listeners that a new Token has been created in the PlanDatabase.
     *@param tok the Token that was created.
     */
    DECLARE_EVENT(notifyTokenCreated, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token has been merged with another Token.
     *@param tok the merged Token.
     *@param actTok the active Token.
     */
    DECLARE_EVENT_ARG(notifyMerged, const TokenId& tok, const TokenId& actTok, tok, actTok);

    /**
     *@brief Notify listeners that a Token has been cancelled.
     *@param tok the cancelled token.
     */
    DECLARE_EVENT(notifyCancelled, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a new Constraint has been created.
     *@param constr the new Constraint.
     */
    DECLARE_EVENT(notifyConstraintCreated, const ConstraintId& constr, constr);

    /**
     *@brief Notify listners that a new ConstrainedVariable has been created.
     *@param var the new ConstrainedVariable.
     */
    DECLARE_EVENT(notifyVariableCreated, const ConstrainedVariableId& var, var);

    /**
     *@brief Notify listeners that the domain of a ConstrainedVariable has been specified.
     *@param var the ConstrainedVariable whose domain has been specified.
     */
    DECLARE_EVENT(notifyVariableSpecified, const ConstrainedVariableId& var, var);

    /**
     *@brief Notify listeners that the domain of a ConstrainedVariable has been closed.
     *@param var the ConstrainedVariable whose domain has been closed.
     */
    DECLARE_EVENT(notifyVariableClosed, const ConstrainedVariableId& var, var);

    /**
     *@brief Notify listeners that the domain of a ConstrainedVariable has been reset.
     *@param var the ConstrainedVariable whose domain has been reset.
     */
    DECLARE_EVENT(notifyVariableReset, const ConstrainedVariableId& var, var);

    /****From PlanDatabaseListener****/

    /**
     *@brief Notify listeners that an Object has been added to the PlanDatabase.
     *@param obj the Object that was added.
     */
    DECLARE_EVENT(notifyAdded, const ObjectId& obj, obj);

    /**
     *@brief Notify listeners that an Object has been removed from the PlanDatabase.
     *@param obj the Object that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const ObjectId& obj, obj);
    
    /**
     *@brief Notify listeners that a Token was added to the PlanDatabase.
     *@param tok the Token that was added.
     */
    DECLARE_EVENT(notifyAdded, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token was removed from the PlanDatabase.
     *@param tok the Token that was removed.
     */
    DECLARE_EVENT(notifyRemoved, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token has been closed.
     *@param tok the Token that was closed.
     */
    DECLARE_EVENT(notifyClosed, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token has been activated.
     *@param tok the Token that was activated.
     */
    DECLARE_EVENT(notifyActivated, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token has been deactivated.
     *@param tok the Token that was deactivated.
     */
    DECLARE_EVENT(notifyDeactivated, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token was merged.
     *@param tok the Token that was merged.
     */
    DECLARE_EVENT(notifyMerged, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token was split.
     *@param tok the Token that was split.
     */
    DECLARE_EVENT(notifySplit, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token was rejected.
     *@param tok the Token that was rejected.
     */
    DECLARE_EVENT(notifyRejected, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token was reinstated.
     *@param tok the Token that was reinstated.
     */
    DECLARE_EVENT(notifyReinstated, const TokenId& tok, tok);

    /**
     *@brief Notify listeners that a Token was constrained to come before a successor Token on an Object.
     *@param object the Object to which the Token was constrained.
     *@param token the Token that was constrained.
     *@param successor the successor Token.
     */
    void notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor) {
      publishMessage(notifyConstrained(object, token, successor));
    }

    /**
     *@brief Notify listeners that a Token has been freed from a particular object.
     *@param obj the Object from which the Token was freed.
     *@param tok the Token that was freed.
     */
    DECLARE_EVENT_ARG(notifyFreed, const ObjectId& obj, const TokenId& tok, obj, tok);

    /**
     *@brief Notify listeners that a Token has been added to an Object.
     *@param obj the Object to which the Token was added.
     *@param tok the Token that was added.
     */
    DECLARE_EVENT_ARG(notifyAdded, const ObjectId& obj, const TokenId& tok, obj, tok);

    /**
     *@brief Notify listeners that a Token has been removed from an object.
     *@param obj the Object from which the token was removed.
     *@param tok the Token that was removed.
     */
    DECLARE_EVENT_ARG(notifyRemoved, const ObjectId& obj, const TokenId& tok, obj, tok);

    /****From RulesEngineListener****/
    /**
     *@brief Notify listeners that a Rule has been executed, creating a RuleInstance.
     *@param rule the RuleInstance that was created.
     */
    DECLARE_EVENT(notifyExecuted, const RuleInstanceId& rule, rule);

    /**
     *@brief Notify listeners that a Rule has been undone, removing a RuleInstance.
     *@param rule the RuleInstance that was removed.
     */
    DECLARE_EVENT(notifyUndone, const RuleInstanceId& rule, rule);


    /**
     *@class PlannerListener
     *@brief The internal listener class for the planner.  Redispatches all events to the EventAggregator.
     */
    class PlannerListener : public SOLVERS::SearchListener {
    public:
      PlannerListener(const SOLVERS::SolverId& s, const EventAggregatorId& ea)
        : SearchListener(), m_solver(s), m_ea(ea) {m_solver->addListener(getId());}
      ~PlannerListener(){m_solver->removeListener(getId());}
      void notifyCreated(SOLVERS::DecisionPointId& dp) {m_ea->notifyNewDecision(dp);}
      void notifyDeleted(SOLVERS::DecisionPointId& dp) {m_ea->notifyRemovedDecision(dp);}
      void notifyUndone(SOLVERS::DecisionPointId& dp) {m_ea->notifyUndone(dp);}
      void notifyStepSucceeded() {m_ea->notifyStep();}
      void notifyStepFailed() {m_ea->notifyStep();}
      void notifyCompleted() {m_ea->notifySearchFinished();}
      void notifyExhausted() {m_ea->notifySearchExhausted();}
      void notifyTimedOut() {m_ea->notifyPlannerTimeout();}
    protected:
    private:
      SOLVERS::SolverId m_solver;
      EventAggregatorId m_ea;
    };

    /**
     *@class DMListener
     *@brief The internal listener class for the DecisionManager.  Redispatches all events to the EventAggregator.
     */
//     class DMListener : public DecisionManagerListener {
//     public:
//       DMListener(const DecisionManagerId& dm, const EventAggregatorId& ea) :
//         DecisionManagerListener(dm), m_ea(ea){}
//     protected:
//     private:
//       void notifyConditionAdded(const ConditionId& cond) {m_ea->notifyConditionAdded(cond);}
//       void notifyConditionRemoved(const ConditionId& cond) {m_ea->notifyConditionRemoved(cond);}
//       void notifyConditionsChanged() {m_ea->notifyConditionsChanged();}
//       void notifyPassed(const EntityId& entity, const ConditionId& cond) {m_ea->notifyPassed(entity, cond);}
//       void notifyFailed(const EntityId& entity, const ConditionId& cond) {m_ea->notifyFailed(entity, cond);}
//       void notifyNewUnitDecision(const DecisionPointId& dec) {m_ea->notifyNewUnitDecision(dec);}
//       void notifyNewDecision(const DecisionPointId& dec) {m_ea->notifyNewDecision(dec);}
//       void notifyRemovedDecision(const EntityId& entity) {m_ea->notifyRemovedDecision(entity);}
//       void notifyAssignNextStarted(const DecisionPointId& dec) {m_ea->notifyAssignNextStarted(dec);} // previous dec
//       void notifyAssignNextFailed(const DecisionPointId& dec) {m_ea->notifyAssignNextFailed(dec);} // current dec
//       void notifyAssignNextSucceeded(const DecisionPointId& dec) {m_ea->notifyAssignNextSucceeded(dec);} // current dec
//       void notifyAssignCurrentStarted(const DecisionPointId& dec) {m_ea->notifyAssignCurrentStarted(dec);} // current dec
//       void notifyAssignCurrentFailed(const DecisionPointId& dec) {m_ea->notifyAssignCurrentFailed(dec);} // current dec
//       void notifyAssignCurrentSucceeded(const DecisionPointId& dec) {m_ea->notifyAssignCurrentSucceeded(dec);} // current dec
//       void notifyRetractStarted(const DecisionPointId& dec) {m_ea->notifyRetractStarted(dec);} // current dec
//       void notifyRetractFailed(const DecisionPointId& dec) {m_ea->notifyRetractFailed(dec);}  // current dec
//       void notifyRetractSucceeded(const DecisionPointId& dec) {m_ea->notifyRetractSucceeded(dec);}  // current dec
//       void notifySearchFinished() {m_ea->notifySearchFinished();}
//       void notifyPlannerTimeout() {m_ea->notifyPlannerTimeout();}

//       EventAggregatorId m_ea;
//     };
    
    /**
     *@class CEListener
     *@brief The internal listener class for the ConstraintEngine.  Redispatches all events to the EventAggregator.
     */
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
    
    /**
     *@class DBCListener
     *@brief The internal listener class for the DbClient.  Redispatches all events to the EventAggregator.
     */
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


    /**
     *@class PDListener
     *@brief The internal listener class for the PlanDatabase.  Redispatches all events to the EventAggregator.
     */
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
    

    /**
     *@class REListener
     *@brief The internal listener class for the RulesEngine.  Redispatches all events to the EventAggregator.
     */
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
    
    static EventAggregatorId s_instance; /*!< The singleton instance of EventAggregator. */
  };
}

#undef DECLARE_EVENT
#undef DECLARE_EVENT_ARG

#endif
