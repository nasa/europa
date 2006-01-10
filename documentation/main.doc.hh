/**
 * @mainpage EUROPA
 * E2 is a constraint-based planning and scheduling framework to embed plan generation and manipulation capabilities within a host application. It is designed to be expressive, efficient, extendable and configurable. It  provides:
 * @li A <b>Plan Database</b>.  The technology cornerstone of E2 for storage and manipulation of plans as they are initialized and refined. The E2 Plan Database integrates a rich representation for actions, states, objects and constraints with powerful algorithms for automated reasoning, propagation, querying and manipulation.
 * @li A <b>Problem Solver</b>.  A core solver to automatically find and fix flaws in the plan database. It can be configured to plan, schedule or both. It can be easily customized to integrate specialized heuristics and resolution operations.
 * @li A <b>Tool Box</b>. E2 includes a debugger for instrumentation and visualization of applications. It also includes a very high-level, declarative modeling language for describing problem domains and partial-plans. Finally, it is accompanied with a scripting language to facilitate automated regression testing of models and solvers as part of the application development effort.
 *
 * @image html Applications.png "E2 components embedded in different application scenarios"

 * E2 is unconventional in providing a separate Plan Database that can integrate in a wide variety of applications. This reflects the common needs for representation and manipulation of plan data in different application contexts and different problem solving approaches. The examples above indicate:
 * @li A batch planning application where an initial state is input and a final plan is output without any interaction with other actors.
 * @li A mixed-initiative planning application where human users interact directly with a plan database but also employ an automated problem solver to work on parts of the planning problem in an interleaved fashion.
 * @li An autonomous execution system where the plan database stores the plan data as it evolves in time, being updated from data in the environment, commitments from the executive, and the accompanying automated solver which plans ahead and fixes plans when they break. 
 *
 * @section structure Documentation Structure
 * <ul> 
 * <li>A <a href="../install_guide/install_guide.pdf">Installation Guide</a> has been prepared to set up an installation and a simple project. 
 * <li>A @ref rovertutorial "Simple Sample Application" is described to get an overview of the capabilities of EUROPA. 
 * <li>A more comprehensive @ref usersGuide  is also available which covers background material, tutorials, case studies and technical references.
 *</ul>
 * @section projects Projects
 * EUROPA has been used in a wide variety of projects which illustrate some of these scenarios, including:
 * @li <b>MER Tactical Activity Planning</B>. EUROPA is the core planning technology behind MAPGEN, a decision support tool for generating detailed activity plans on a daily basis for the MER robotic mission to Mars.
 * @li <b>On-board Planning and Plan Execution</b>. EUROPA was the core planning technolgoy for deliberative and reactive planning on-board a variety of mobile robots. It has been fielded in the Atacama Desert and was the cornerstone of a 2005 milestone of human-robotic collaboration for the Collaborative Decision Systems program.
 * @li <b>Mission Simulation</B>. EUROPA was used to simulate a prospective robotic mission (LORAX) to the Antarctic for the purposes of system design evaluation.
 *
 * @section contactandsupport Contacts and Support
 *
 * @subsection loggingbugs Logging Bugs and Feature Requests
 *
 * To report a bug in EUROPA2 or to request new functionality please e-mail your request to the following address: <a href="mailto:europa-help@nx.arc.nasa.gov">europa-help@nx.arc.nasa.gov</a>.
 *
 * @subsection deprecation Deprecation Policy
 * Under construction
 *
 * @subsection release Release Policy
 * Under construction
 *
 * @subsection contribute Contributing to the Library
 * Under construction
 *
 * @section ack Acknowledgements
 * EUROPA 2 is a culmination of many years of research, development and deployment of constraint-based planning
 * technology. The precursor to EUROPA was HSTS, designed and developed by Nicola Muscettola. HSTS set out the
 * initial domain description language and essentials of the planning paradigm that became the starting point for
 * EUROPA, under the leadership of Ari Jonsson. Ari's team included Jeremy Frank, Paul Morris and Will Edgington, who all
 * made valuable contributions to the development of EUROPA. EUROPA 2 is a further evolution of this line of work,
 * targeted mainly at making the technology easier to use, more efficient, easier to integrate and easier to extend. The
 * development of EUROPA 2 has been lead by Conor McGann in collaboration with Andrew Bachmann, Tania Bedrax-Weiss, 
 * Patrick Daley, Will Edgington, Jeremy Frank, Michael Iatauro, Peter Jarvis, Ari Jonsson, Sailesh Ramakrishnan and 
 * Will Taylor. Funding for this work has been provided by the NASA Intelligent Systems and Collaborative Decision Systems Programs.
 */
