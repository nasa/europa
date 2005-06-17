/**
 * @page glossary Glossary
 * @li @anchor activeToken Active Token :- A token which has been activated in the @ref partialPlanDef "partial plan". See @ref tokens "the token state transition diagram" for further details.
 * @li @anchor baseDomain Base Domain :- The maximal set of values a variable may take.
 * @li @anchor closedWorldAssumption Closed World Assumption :- An assumption that no new objects can be inserted into the partial plan. This assumption is enforceable in EUROPA, and leads to stronger propagation. For more information, see @ref dynamicObjects.
 * @li @anchor compileTime Compile Time Data :- Those elements of a problem specification that are compiled prior to construction of a @ref planDatabaseDef "plan database", which remain static for the lifetime of a given plan database. Such data is typically considered the @em model. See @ref workFlow "the batch-solver process overview" for further clarification.
 * @li @anchor ddl DDL :- Domain Description Language. Developed by Nicola Muscettola for the HSTS planner.
 * @li @anchor europa EUROPA :- Extensible Reusable Remote Operations Planner. Refers to the planning technology platform consisting of a @em framework, a set of @em components, and a set of @em tools.
 * @li @anchor groundedPlan Grounded Plan :- a plan in which all variables have been specified to a singleton value.
 * @li @anchor horizonDef Plan Horizon :- the temporal extent of the plan.
 * @li @anchor inactiveToken Inactive Token :- A token which has been created in the @ref planDatabaseDef "plan database", either explicitly through an invocation by an external client, or implicitly through execution of a @ref ruleInstanceDef "model rule instance". However, such a token has not yet been activated, merged or rejected. See @ref tokens "the token state transition diagram" for further details.
 * @li @anchor intervalToken Interval Token :- A token which has a duration of at least 1 time unit. All tokens on a class derived from @ref timeline "Timeline" are @em interval @em tokens.
 * @li @anchor masterToken Master Token :- An @ref activeToken "active" token that has generated subgoals through @ref ruleInstanceDef "rule instance" execution.
 * @li @anchor mergedToken Merged Token :- A token which has been merged with an @ref activeToken "active token" in the @ref partialPlanDef "partial plan". See @ref tokens "the token state transition diagram" for further details.
 * @li @anchor orphanToken Orphan Token :- Any token that has been created by explicit request from a client external to the plan database. Such a token has no @ref master "master".
 * @li @anchor parameterConstraint Parameter Constraint :- A constraint declared in the body of a NDDL predicate declaration. It can only apply to the immediate context of the predicate (i.e. built-in varaibles and user-defined predicate parameters). Parameter constraints apply to @ref inactiveToken "inactive" and @ref activeToken "active" tokens. 
 * @li @anchor partialPlanDef Partial Plan :- a plan which is incomplete.
 * @li @anchor planDatabaseDef Plan Database :- an object responsible for management of all entities in a @ref partialPlanDef "partial plan". It provides operations for legal restrictions and relaxations to the partial plan and co-ordinates automated reasoning services to propagate consequences of such operations according to the domain-model and rules of inference of EUROPA.
 * @li @anchor rejectedToken Rejected Token :- A token which has been rejected from the @ref partialPlanDef "partial plan". See @ref tokens "the token state transition diagram" for further details.
 * @li @anchor ruleInstanceDef Rule Instance :- an instance of a model rule, scoped to a particular token and its subgoals.
 * @li @anchor runTime Run-time Data :- those elements of a problem specification that are created or deleted during the lifetime of a @ref planDatabaseDef "plan database".  See @ref workFlow "the batch-solver process overview" for further clarification.
 * @li @anchor slaveToken Slave Token :- Any token that has been created through @ref ruleInstanceDef "rule instance" execution.
 * @li @anchor solver Solver :- A problem solving agent which conducts refinements to a @ref partialPlanDef "partial plan" until it is completed, has exhausted all possibilities, or has exhausted all its allocated time. For more information see @ref solvers.
 * @li @anchor specifiedDomain Specified Domain :- indicates a subset of the @ref baseDomain "base domain" of a variable to which it has been @em specified by an external client.
 */
