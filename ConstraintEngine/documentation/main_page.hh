/**
 * @mainpage Constraint Engine
 * @section intro Introduction
 * This module is a prototype for a new Constraint Engine Framework. A framework is a set of interfaces and components for
 * building a range of particular constraint engines instances. Please use this page as a guide for reviewing the prototype
 *
 * @section motive Motivation
 * There are a number of perceived problems with the current Constraint Network which motivate this prototype:
 * @li Performance - a major component of cost in Europa lies in propagation. Improvements in propagation
 * and repropagation are expected to have a strong impact. Furthermore, difficulties in integrating external
 * propagators such as the Resource Manager lead to inefficient scheduling of propagation. The mechanism
 * to integrate the CNET with the temporal network involves synchronization of shadowed data which might
 * be possible to avoid if we had control over specialized implementations of variables. It is not clear how big a deal this is.
 * @li Extendability - we cannot specialize implementations of variables since variables are created interanlly. We are limited in the
 * types of specialized propagation control we want based on what is already encoded in the propagation algorithm
 * of the constraint network.
 * @li Maintainability - the special cases of propagation are current handled by a single centralized algorithm, which maintains
 * multiple agendas based on the Constraint Network, has special cased code for construction and integartion of Constraints. It is generally
 * believed that a much smaller implementation is possible. Also, there is some direct coupling among modules that is inappropriate. For example,
 * use of ObjectId in Value class, use of TokenNetwork and Token references, in the Constraint Network Module. Direct dependence between
 * the constraint network and the temporal Network.
 * @li Safety - access is provided to functions that should not be called. In particular, one can get the derived domain of the variable
 * during execution of a constraint.
 *
 * @section goals Imperatives for new Design
 * The prototype must address the deficiencies outlined above. Some concrete design directions seem clear:
 * @li Restrict interfaces to allow required behaviour and prohibit unacceptable behavior.
 * @li Allow flexible configuration/control of the Propagation Scheduling
 * @li Allow registration of external constraints without requiring static linking. Have not put much effort into this, but would investigate
 * introducing a Library of some kind that can be extended and would be explciitly initialized in some way.
 * @li Provide fast implementations fo primitive components of propagation i.e. Domains
 * @li Require construction of actual objects to be external to the constraint network to
 * allow specialization of implementations without change to the CNet core. This is key to extendibility and maintainability.
 *
 * We also keep in mind a concrete instantiation of the framework that is guided by the Europa usage scenario. Thus we can
 * assume certain components would have to be integrated in an effective fashion:
 * @li A Resource Manager will be required to handle all propagation of Resoure Constraints
 * @li We want to be able to treat equality constraint propagation in a block to take advantage of
 * equivalence class formulation.
 * @li A temporal Network will be required to handle all propagtion of Temporal Constraints.
 * @li We want to handle Unary Constraints first, and call only once unless a retraction is required.
 *
 * @section strategy Design Strategy
 * The key design strategy is a highly decentralized architecure with small interfaces and a small collaboration 
 * model defining the core of the framework, leaving room for localized, compact extension throughout. Interfaces 
 * and base class implementations enforce the contracts of the collaboration.
 *
 * The prototype has focussed on:
 * @li New Domain Object Model - allows specialization of domains in a variety of ways to provide
 * large performance imporvements on primitive operations.
 * @li Introduction of Propagators that can be registered with the Constraint Engine. This is highly
 * extendible. Propagator implementations would include Procedural Constraint, UnaryConstraint, Resource Constraint, Temporal Constraint
 * and EqualityConstraint propagators.
 * @li New Variable Object Model - allows for specialization of Variables to accomodate the new Domain Model. Also
 * provides for a restricted view of Variables as an adpater class which limits access appropriate for the Constraint Network,
 * the Constraint base class, and any derived classes.
 * @li New Event Model - provides a message based model of control among the pieces rather than a centralized control model. Also
 * provides higher-fidelity events with data to expedite processing.
 * @li Integration Model for Backward Compatibility - demonstrated easy translation from old domains to new ones and back to allow
 * exploration of incremental changes to the core as we go.
 * @see Consraint, AbstractDomain, Propagator, ConstrainedVariable, Variable
 *
 * @section results Performance Results
 * A number of performance tests have been conducted. There are parallel versions of each test in DomainTest.cc and EuropaDomainTest.cc,
 * reflecting the new constraint engine paradigm and the exiting Europa code respoectively. The tests are as follows:
 * @li testIntersection - compares the cost for domain creation and intersection for numeric intreval domains.
 * @li testEquate - compares the cost of domain creation, and mutual intersection of 2 domains for LabelSet domains.
 * @li testLabelSetPerformance - compares the cost of creation of 10 variables, creation of 9 equality constraints placing all variables in an
 * equivalence class, and then looping over successive domain reductions to obtain a singleton value. This test is done with LabelSet based
 * variables.
 * @li testIntervalPerofmance - same basic setup as testLabelSetPerformance except in this case we use interval integer domains.
 * 
 * All results are based on the following:
 * @li run on murphys (linux box)
 * @li Europa is compiled with: make EUROPA_VERSION=_EUROPA_FAST_VERSION_ OPTIMIZE=3
 * @li ConstraintEngineDefs.hh includes: #define #define _PLASMA_FAST_VALUE_
 * @li ConstraintEngine makefile has: CC_FLAGS=-O3
 * 
 * Results are shown below:
 * @verbatim

TEST                     ConstraintEngine     Bitset version  Europa         Factor Improvement
=================================================================================================
testEquate                   2.82             [2.02]           7.58              2.7 [3.75]
testIntersection             0.35                              2.26              7.5
testLabelSetPerformance      0.30             [0.25]           1.48              4.9 [5.92]
testIntervalPerformance      0.19                              1.03              5.4 @endverbatim
 * 
 * Note that the above results do not explore any possible improvements in algorithms or handling of propagation events.
 * @section guide Readers Guide
 * @li module-tests.cc provides test cases for the system which are also intstuctive for seeing its use. In particular, look at
 * methods in the Constrainttest class.
 * @li DomainTest.cc shows the performance testing done on the new domains.
 * @li EuropaDomainTests.cc shows the equivalent tests condutced on old Europa domains.
 * @li EUROPA::ConstraintEngine is the core of the framework.
 * @li EUROPA::ConstrainedVariable shows the ConstraintEngines perspective on what a variable is.
 * @li EUROPA::Variable shows the external users view of what a variable is.
 * @li EUROPA::AbstractDomain and derived classes show the alternate domain implementation.
 * @li EUROPA::DomainListener defines the events that can be tracked on a domain.
 * @li EUROPA::Constraint class plays the same role it plays in Europa, though it is integarted to the ConstraintEngine and event model
 * a little differently.
 * @li EUROPA::Propagator class basically handles agenda management.
 *
 * @section build Build Instructions
 * Some of the basics are:
 * @li To build code, be sure to set the EUROPA_ROOT in the makefile.
 * @li make doc generates the html you are looking at :-)
 * @li make will run the module tests
 *
 * To run performance tests:
 * @li first make under NewPlan as follows: make EUROPA_VERSION=_EUROPA_FAST_VERSION_ OPTIMIZE=3.
 * @li with Europa built correctly, change CC_FLAGS in the local makefile to CC_FLAGS=-03.
 * @li define _EUROPA_FAST_VALUE_ in ConstraintEngineDefs.hh
 * @li make domain-tests will execute the new domain version of one of the tests (check the code to see which one.
 * @li make europa-domain-tests wille execute the europa domain version which should be comparable in functionality.
 *
 * @section issues Further Issues
 * @li Propagation Control - need to work out the range of propagation control schemes we want to have. Should a Constraint
 * be allowed to operate immediately? Can it mask messages, do we want to force a pre-emption of all local control under certain
 * circumstances.
 * @li Backward Compatibility - key interfaces are with the Temporal Network, Procedural Contraints and the TokenNetwork, Flaw Cache and
 * PlanInfo components that use domains. We could have the option to fix or leave Domain interfaces though the consequences for performance
 * are not clear since it would involve more domain copying. Still, it would provide for leveraging of prior test code, and a risk mitigation
 * approach for changes i.e. incremental delivery. One big question here is preserving the investemnt in tests for procedural and other constraints
 * and minimizing porting costs.
 * @li Additional Requirements - this can be fleshed out with the broader EUROPA team.
 * @todo Define a ConstraintEngineListener interface and implement publish and subscribe support.
 * @todo Add support to gather statistics on performance and general instrumentation - probably through event listeners.
 * @todo Examine how the system could be extended to obtain explanations - for example, the path to change, or inconsistency.
 */
