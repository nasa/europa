/**
 * @mainpage Plan Database Services for Planning and Scheduling Applications 
 * @section intro Introduction
 * EUROPA 2 provides efficient, customizable Plan Database Services that
 * can be combined and aggregated build a wide variety of planning and scheduling
 * applications.  EUROPA 2 was built as an implementation of the
 * Constraint-Based Planning with Resources (CAPR) paradigm described in the
 * adjunct paper: EUROPA 2: Plan Database Services for Planning and
 * Scheduling Applications submitted to ICAPS 05.  Briefly, CAPR formalizes
 * planning domain descriptions in terms of variables, constraints, and
 * resources, and allows reasoning in the lifted case.  EUROPA 2 provides
 * both general and specific reasoning techniques to solve planning and
 * scheduling problems that can be formulated as sets of variables and constraints.
 * 
 * This first release of EUROPA 2 includes the following modules and
 * capabilities:
 * @li @ref ConstraintEngine "Constraint Engine":  manages the sets of
 * changes to variables and constraints and propagates the changes through
 * the network.  Variables are connected to other variables that are
 * constrained via the same constraint, forming a network.  This network
 * may change by either adding or removing variables or constraints.  The
 * constraint engine provides the capability to update incrementally based
 * on these changes.  It also provides the capability to create specialized
 * constraints and propagators.  
 * @li @ref TemporalNetwork "Temporal Network": manages the sets of changes
 * to temporal variables and constraints and propagates the changes through
 * the temporal network.  The temporal network is implemented as a
 * specialized propagator. 
 * @li @ref PlanDatabase "Plan Database": manages the plan or schedule
 * using state machines as its internal representation.   An Object is a
 * state machine described by the set of Tokens it contains.  We provide
 * two specialized Object classes: Timelines for Objects that accept a
 * single state at any one time and can therefore be described only by
 * totally ordered tokens; and Resources for Objects whose state is
 * determined by numerical quantities.  We also provide ObjectFactories so
 * users are able to define and register their own objects based on their
 * specific needs.  
 * @li @ref Resource "Resource": we provide a mosule dedicated to
 * Resources because Resources are a common type of object that requires
 * special reasoning. This module includes basic  resource definitions,
 * reasoning, decision support, and modeling support  for resources.  A
 * limited form of resource reasoning is done to determine the resource
 * envelope.  
 * @li @ref RulesEngine "RulesEngine": manages subgoaling relationships.
 * EUROPA 2 supports a set of pre-specified subgoaling relationships based
 * on the Allen Relationships.  We also provide means to specify and
 * register customized subgoaling rules. 
 * @li @ref CBPlanner "CBPlaner": we provide a basic chronological
 * backtracking planner.  It provides the ability to run to completion or
 * step through.  It also provides the capability to identify the set of
 * flaws that the planner needs to resolve in order to find a plan.  By
 * default, the planner willl resolve all but the temporal variable
 * assignments. 
 * @li @ref HSTS "HSTS": an extension of CBPlanner that includes heuristics
 * that replicate the behavior of EUROPA for backwards compatibility.  It
 * also includes a reader of EUROPA heuristics and a translator for EUROPA
 * 2 syntax of HSTS heuristics. 
 * @li @ref AgentDatabase "Agent Database": an extension of the Plan
 * Database that supports multi-threaded access to the plan database by
 * multiple agents.  We provide the capability to connect, disconnect, lock
 * and unloack the database. 
 * @li @ref NDDL "New Domain Description Language (NDDL)": a new domain
 * description language to facilitate describing domains that can be reason
 * with using EUROPA 2.  It includes constructs for Objects, Prediates,
 * Rules, Constraints, Resources, Variables of basic types and named
 * enumerations. NDDL comes with a parser and a compiler that generates C++
 * code from the domain descriptions.  These C++ classes are linked with
 * EUROPA 2 code to produce a runtime for the domain.  Initial states can
 * be written as transaction files which can be loaded during
 * initialization. 
 * @li @ref Utils "Utilities": general utilities for memory management,
 * debugging, managing errors, managing regression tests.
 * @li @ref System "System": includes a standard assembly to illustrate a
 * particular configuration of the system, Aver the test language,
 * system-level regression tests, and other system-level support files. You
 * can also find a user guide in the documentation subdirectory.
 *
 * @section motive Motivation
 * There are a number of perceived problems with the EUROPA which motivated
 * this new release.
 * @li Performance  - couldn't scale well to hundreds of objects and
 * thousands of tokens.
 * @li Configurability - couldn't pick and choose components to solve
 * particular problems.
 * @li Extendibility  - couldn't extend the code without changing most of
 * the code in a module.
 * @li Maintainability - couldn't diagnose and fix problems easily.
 * @li Safety - couldn't restrict access to prohibit unacceptable behavior
 * which users constantly took advantage of.
 *
 * Given this set of problems we came up with the following design goals:
 * The key design strategy is a highly decentralized architecure with small
 * interfaces and a small collaboration  model defining the core of the
 * framework, leaving room for localized, compact extension
 * throughout. Interfaces  and base class implementations enforce the
 * contracts of the collaboration.  The following are some specific design
 * goals: 
 * @li Restrict interfaces to allow required behaviour and prohibit
 * unacceptable behavior, key to achieving safety.
 * @li Allow flexible configuration and control of propagation, subgoaling,
 * and decision management, key to configurability and extendibility.
 * @li Allow registration of external constraints, propagators, rules, decision
 * filtering conditions, etc, key to extendibility and maintainability.
 * @li Provide fast implementations fo primitive components by specializing
 * via inheritance, key to performance.
 * @li Require construction of specializations to be external to the core
 * in components,  to sidestep changes to the core, key to achieving
 * extendibility and maintainability. 
 * @li Create a robust message passing mechanism to synchronize plan
 * database services, key to maintainability and performance.
 * 
 */
