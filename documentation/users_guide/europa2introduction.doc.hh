/**
 * @page europa2introduction EUROPA2
 *
 * @section whatiseuropa Objective
 *
 * EUROPA2 is a fast, flexible, extensible, and resusable technology platform for building planning and scheduling applications for space exploration. Its predecessor, EUROPA, 
 * has been deployed as the core planning technology for a varity of NASA research and mission applications. A notable example is MAPGEN, the ground-based
 * activity planning system for the Mars Exploration Rover (MER) mission. EUROPA2 is reimplementation of EUROPA that is significantly faster and easily exstended through a focus on state-of-the art software enginnering.  
 * 
 * EUROPA2 (Extensible Universal Remote Operations Architecture) is a component-based sofware library for representing and reasoning with plans. It is analogious to 
 * component-based sofware in over areas like CLARATy for robotoics or ILOG for scheduling.  
 *
 *@section usecases Use-Cases
 * There are three major use-cases for deploying EUROPA2. We first need to define two core concepts. A <b>planner</b> is the software Europa2
 * provides for generating plans. A <b>plan database</b> is the software Europa2 provides for storing goals and plans. The plan database is 
 * modified during the planning process as a plan is developed.
 * @image html Applications.jpg Uses Cases
 * <ul>
 * <li><b>Batch Planner</b>. The system is passed a partial plan that may include just the goals the planner is to plan for or it could also 
 * include some actions that must be included in the plan. This information is stored in a plan database. The planner then operates on the plan 
 * database to produce the plan. Planning is an incremental process so it may be interrupted at anytime and a partially completed plan can be 
 * recovered.
 * <li><b>Mixed-Initiative Planner</b>. The planner is used in concert with a user. The user may introduce goals into a plan, make planning decisions, 
 * and change or undo decisions previously made by the planner. 
 * <li><b>On-Board Planner</b>. The planner is deployed on board an agent (space craft, robot) and is connected to an execution executive. 
 * The executive executes the plan produced by the planner. If the world changes in unexpected ways or the execution of plan steps fails, 
 * the executive calls back to the planner to request that it re-plan to work around the changes and produce a new plan for achieving the goals. 
 * </ul>
 * 
 */
