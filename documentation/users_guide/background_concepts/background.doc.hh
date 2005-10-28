/**
 * @page background Background Concepts
 * In this chapter we review some of the core concepts employed in the @ref europa "EUROPA" 2 Constraint-based Planning 
 * Paradigm. The sections are:
 * @li @ref planning
 * @li @ref csp
 * @li @ref stp
 * @li @ref planRep
 * @li @ref partialPlan
 * @li @ref planDb
 * @li @ref problemSolving
 * @li @ref summary
 *
 * @section planning Planning, Scheduling, and Automated Planning
 * Planning, for our purposes, can be thought of as determining all the small tasks that must be carried out in order to accomplish a goal.
 * Let's say your goal is to buy a gallon of milk. It may sound like a simple task, but if you break it down, there are many small tasks 
 * involved: obtain keys, obtain wallet, start car, drive to store, find and obtain milk, purchase milk, etc. Planning also takes into account
 * rules, called constraints, which control when certain tasks can or cannot happen.  Two of the many constraints in this example are, you must
 * obtain your keys and wallet before driving to the store and you must obtain the milk before purchasing it. 
 *
 * Here is what a simple plan for buying milk at the store might look like:
 * @image html timelinePurchaseMilk.jpg
 *
 * Scheduling can be thought of as determining whether adequate resources are available to carry out the plan. Two resources that scheduling would 
 * have to take into account for our example above are fuel and time. If it takes two gallons of gas to get to the store and back and your car only has 
 * one gallon, you must develop a plan that includes a stop at the gas station. If it takes 5 minutes to drive to the store, the store closes at 10:00, 
 * and it is currently 9:30, you must also take that time constraint into account when scheduling your task.
 *
 * The automated planning community provides techniques for representing the actions that an agent may take in the world and the goals 
 * it wants to achieve together. These are complemented with computer software for automatically generating a plan that is composed
 *  of actions that when executed will obtain the goals of the agent. 
 *
 * In our above example, we as users would describe each of the actions available to us together with the goal of having milk at home. 
 * The automated planning software would generate the timeline with the plan for going out a purchasing the milk.  
 *
 * The automated planning process may also involve the user in a mixed-initiative form of interaction where they user can specify portions 
 * of the plan that the automated planner should complete or the automated planner can ask the user for guidance on planning decisions such 
 * as which actions to use for a particular goal. 
 *
 * There is a wealth of literature on automated planning. The <a href="http://ic.arc.nasa.gov/tech/groups/index.php?gid=8&&ta=2">Planning and Scheduling Group</a> at NASA Ames 
 * NASA Ames' Planning and Scheduling group provides a good entry point for applications related to spacecraft. The annual 
 * <a href="http://www.icaps-conference.org">International Conference on Automated Planning and Scheduling</a> provides a excellent entry point into the broader research community.
 *
 * @section csp Constraint Satisfaction Problems
 * @ref europa "EUROPA" is based on translating a representation of a plan into a graph of constraints and variables. Solving a planning
 * problem thus borrows heavily from methods of solving constraint satisfaction problems, and leverages algorithms for
 * propagating consequences of changes to variables through constraints to detect inconsistencies and improve search. 
 *
 * A Constraint Satisfaction problem consists of a set of @em Variables:
 * @li @em speed @f$\epsilon@f$ [1 10] i.e. the variable @em speed has a value in the range from 1 to 10.
 * @li @em distance @f$\epsilon@f$ [40 100] i.e. the variable @em distance has a value in the range from 1 40 to 100.
 * @li @em time @f$\epsilon@f$  [0 @f$\infty@f$] i.e. the variable @em time 
 * has a value in the range from 0 to @f$\infty@f$.
 * @li @em location1 @f$\epsilon@f$  [20 25] i.e. the variable @em location1 has a value in the range 20 to 25.
 * @li @em location2 @f$\epsilon@f$  [80 200] i.e. the variable @em location2 has a value in the range 80 200.
 *
 * and a set of @em Constraints:
 * @li C0: @em speed == @em distance * @em time
 * @li C1: @em location1 + @em distance == @em location2
 *
 * A @em Solution to a constraint satisfaction problem (@em CSP) is where each variable has a single value and all constraints
 * are satisifed. For example, the following is a solution to the above @em CSP:
 * @li @em speed=10; @em distance=70; @em time=700; @em location1=25; @em location2=95.
 *
 * In this problem, note that no soution is possible which contains the value 0 for @em time. Such a value would make constraint @em C0 inconsistent. Similarly, the value @f$\infty@f$ is excluded from all solutions. Constraints can be @em propagated to remove values from the domain of variable that cannot participate in a solution to the @em CSP. The principal solution techniques available for solving a @em CSP are:
 * @li @em Heuristic @em Search to select a value for each variable from the remaining values in its domain.
 * @li @em Propagation to prune infeasible values to reduce backtracking, and to detect a dead-end where no further refinement of the variables is a solution.
 * 
 * Solving a @em CSP is NP-Hard (i.e. totally intractable!) in theory, but often very efficient in practice using the 
 * above methods. @ref europa "EUROPA" uses an extended version of the basic ideas of a @em CSP called a @em Dynamic @em Constraint @em 
 * Satisfaction @em Problem (@em DCSP). A @em DCSP permits addition of new variables and constraints to the problem which
 * is essential as plans evolve with new activities and states and relations between them being created.
 * @section stp Simple Temporal Problems
 * The notion of time is central to temporal planning. @ref europa "EUROPA" uses @em variables to explicitly represent 
 * @em timepoints for plan activities and states. @em Constraints among @em timepoints provide a natural way to express
 * domain axioms. For example, in order to state that activity @em A must occur before activity @em B we can say that
 * the end @em timepoint of @em A is @f$<=@f$ the start @em timepoint of @em B.  @ref Dechter91 proposed that 
 * @em constraints 
 * among @em timepoints can be grouped together to form a Simple Temporal Network (@em STN). Such a network can be
 * transformed into a Distance Graph (@em DG) where the outward arc from a node represents the maximum distance from 
 * the source node to the target node. The diagram below illustrates a simple @em STN with just 2 variables and a 
 * single constraint. It also
 * shows the resulting @em DG.
 * @image html stn.png
 * @ref Dechter91 also showed that shortest path algorithms could be used to propagate values in the network and discover
 * a @em negative @em cycle. A @em negative @em cycle is a path from a node to itself that has a path length less than 0.
 * If such a cycle exists, the network is inconsistent. It was further shown that a single-source shortest path algorithm
 * was sufficient to detect a negative cycle and provide sufficient propagation to yield a backtrack-free search. Thus we
 * have an efficient and complete algorithm for propagating an @em STN. These results build on the already established
 * notion of a @em CSP and are naturally incorporated into the general representation and propagation scheme used in 
 * @ref europa "EUROPA".
 * @section planRep Plan Representation
 * Here we introduce 2 major abstractions which build upon the framework of @ref csp and the related work of @ref stp. The
 * first is the notion of an @em Object and the second is the notion of a @em Token. To explain these concepts, we use
 * an example of a satellite which has a camera on board to take pictures. It also has an attitude control
 * system that is responsible for holding a position and altering the position from one attitude to another in order to
 * take the next picture. Before getting into that however, we must first spend some more time discussing the particulars of @em variables in EUROPA which tie directly in to their representation in @ref csp but which have some particulars (one might say quirks) which must be understood.
 * @subsection variables Variables
 * Talk about base, specified and derived domains. This really confuses people!
 * @subsection objects Objects
 * The things we wish to describe and refer to in a domain are considered @em Objects. As in the case with object-oriented
 * analysis and design, one can seek out the nouns in any domain description to find likely objects. In our example,
 * we might consider the @em satellite, the @em camera, and the @em attitude @em controller to be @em objects. Objects
 * have @em state and @em behavior. For example, a @em camera can be:
 * @li @em off
 * @li @em ready
 * @li @em taking @em a @em picture.
 *
 * An @em attitude @em controller can be:
 * @li @em pointing at a position
 * @li @em turning from one position to another.
 *
 * An @em object is an instance of a @em class. In @ref europa "EUROPA" we model using the abstraction of a class to speak about all instances having certain properties of state and behavior. In order to describe such state and behavior we build on the formalism of @em first @em order @em logic.
 *
 * @subsection tqa Temporally Qualified Predicates
 * A predicate defines a relation between objects and properties. In @ref europa "EUROPA", we define such relations between @em variables
 * whose domains are @em sets of @em objects and @em sets of @em properties. For example, we might use a predicate @em Pointing(a,p) to  indicate that the @em attitude @em controller @em a is @em pointing at position @em p. Note that @em a is a @em
 * variable which may have a number of possible values in a problem with multiple satellites. Similarly, @em p is
 * a @em variable whose values are the set of possible @em positions. In a @ref groundedPlan "grounded plan", single values will be
 * specified for each variable as we saw in the case of a @em solved @ref csp. In general, is is not sufficient to state
 * that a predicate is true without giving it some temporal extent over which it holds. Predicates that are always true
 * can be thought to hold from the beginning to the end of time. However, in practice, the temporal extent of interest 
 * must be defined with @em timepoints to represent its @em start and @em end. So we might write:  @em Pointing(a,s,e,p)
 * to indicate that the @em attitude @em controller @em a is @em pointing at position @em p from time @em s to time
 * @em e. In fact, this pattern of using such predicates to describe both state and behavior of objects is so prevalent
 * in @ref europa "EUROPA" that we have introduced a special construct called a @em Token which has the built in @em
 * variables to indicate the @em object to which the statement principally applies and the @em timepoints over which 
 * it holds.
 * @subsection tokens Tokens
 * A @em Token is an instance of a @em predicate defined over a temporal extent delineated with a @em start and @em end 
 * time. In a @ref groundedPlan "grounded plan" each @em Token applies to a specific @em Object, reflecting the intuition that we are using @em Tokens to describe some aspect of an @em Object (i.e. its state or behavior) in time. However, in a @em partial @em plan, the commitment to a specific @em object may not yet have been made. The set of objects to which a @em token might apply are captured in a built-in @em object @em variable for each @em token.
 *
 * @image html token.std.png
 * 
 * The intrinsic token variables of @em object, @em start and @em end are further complemented by a @em state variable captring the current state of the @em token and its reachable states trhough further restriction. The diagram above presents the states and transitions of a token.
 * @li @em Open. Basically an internal state while the @em token is being constructed in the Plan Database.
 * @li @em Close. The operation to @em close a @em token indicates it has been constructed.
 * @li @em Inactive. Semantically, an @em inactive @em token represents a @em state or @em action that has been introduced explicitly be an external agent (e.g. submitting a goal as part of an initial state) or implicitly through inference in the PlanDatabase based on model rules (e.g. in order to @em takePicture the satellite must be @em pointing at the target). However, just because a @em token has been introduced into the PlanDatabase does not mean that it has been decided how it should be satisfied. In this way, an @inactive @em token is a request that must be satisfied.
 * @li @em Activate. One of the ways to satisfy an @em inactive @em token is to decide that it should be inserted into the @em partial @em plan. This is accomplished by the @em activate operation.
 * @li @em Active. An @em active @em token is a state or action in the @em partial plan. As such, it may impact the set of @em objects and other @em tokens. It may also be used to @em support outstanding @em requests (i.e. @em inactive @em tokens). The operation @em deactivate simply reverses the consequences of @em activate.
 * @li @em Merge. Another way to satisfy an @em inactive @em token is to designate an @em active @em token that can fulfill its requirements. For example, suppose that we wish to take 2 pictures of a given target. Each @em takePicture action requires the @em attitude @em controller to be @em pointing at the given target. It is entirely possible that a plan can be devised where the pictures can both be taken while the satellite is in position. This would be accomplished by @em merging the pointing requirements of one token with those of another. In this case, such a plan is possibly more efficient since it limits the amount of turning required and thus conserves time and fuel. The operation @em split simply reverses the consequences of @em merge
 * @li @em Merged. Once @em Merged, the requirements of the given token are passed onto the designated @em active @em token in the @em partial @em plan and the @em merged token and all its @em variables and @em constraints are removed from further consideration.
 * @li @em Reject. It may be that a problem is formulated as an oversubscription problem where more goals are presented than can be satisifed. In such cases it can be useful to represent a @em rejectable @em goal i.e. one that can be rejected if it cannot be otherwise resolved. The operation to @em reject an @em inactive @em token supports this behavior. It is reversed by @em reinstate.
 * @li @em Rejected. A @em rejected @em token has no impact on the @em partial @em plan.
 *
 * The values in the @em state @em variable impact the @em actual state of the token and the @em reachable states of the token. The @ref baseDomain "base domain" of the <em>state variable</em> is <em>{INACTIVE, ACTIVE, MERGED, REJECTED}</em>. For a state to be @em reachable, its value must be a member of the domain of values of the <em>state variable</em>. Here are the interpretations mapping the actual states to the variable values:
 * @li State Variable={INACTIVE, ACTIVE, MERGED, REJECTED}. State="OPEN".
 * @li State Variable={ACTIVE, MERGED, REJECTED}. State="INACTIVE".
 * @li State Variable={ACTIVE, MERGED}. State="INACTIVE".
 * @li State Variable={ACTIVE, REJECTED}. State="INACTIVE".
 * @li State Variable={ACTIVE}. State="ACTIVE".
 * @li State Variable={MERGED}. State="MERGED".
 * @li State Variable={REJECTED}. State="REJECTED".
 * @subsection object Objects
 * As we have noted, @em Tokens describe some aspect of an @em Object in time. @em Objects thus may have many @em Tokens in a plan in order to describe their state and behaviour throughout all points in time of the plan. Within this general framework, we note a few particulars:
 * @li @ref staticFacts "Static Facts". @em Classes without @em predicates lead to @em Objects without @em Tokens. This arises where an @em objects state or behaviour is independent of time. For example, a domain may have a set of @em locations and/or @em paths for which there is nothing more to say than that they exist.
 * @li @ref timelines "Timelines". Often @em objects in a domain must be described by exactly one @em token for every given 
 * @em timepoint in the plan. Such objects are so common that we provide a special construct in @ref europa "EUROPA" to extend these 
 * semantics to derived classes. Any instances of a class derived from a @em Timeline will induce ordering 
 * requirements among its @em tokens in order to ensure no temporal overlap may occur among them.
 * @li @ref resources "Resources". Metric resources, e.g. the energy of a battery or the capacity of a cargo hold, are @em objects
 * with an explicit quantitative state in time and with a circumscribed range of changes that can occur to impact that
 * state i.e. @em produce, @em consume, @em use, @em change. These changes are captured as @em tokens. @em Resources 
 * are such a common requirement for @ref europa "EUROPA" users that special constructs are also provided for them. 
 * Instances of @em classes derived from a @em Resource will induce ordering requirements on their @em Tokens in order to ensure that the @em level of the @em resource remains within specified @em limits.
 *
 * @section dynamicObjects Dynamic Objects
 * Under construction
 * @section partialPlan Partial Plans
 * A @em partial @em plan is a plan that is @em incomplete or @em partially @em specified. @ref europa "EUROPA" represents @em partial @em plans in terms of @em Objects, @em Tokens, @em Variables, and @em Constraints. To make this more concrete consider a simple example from our toy satellite domain. We start with the model fragment included below.
 * @include satellite.1.nddl
 * Note that in the model we deal with the abstractions of @em classes, @em predicates and @em rules. In a @em partial @em plan these are intsantiated as @em objects, @em tokens, and @ref ruleInstanceDef "rule instances" respectively. The particulars of the syntax and semantics are not important at this time, but will be dealt with in detail in @ref nddl. However, a few points are worth noting here:
 * @li The set of possible positions will be defined by the instances of class @em Position. This exemplifies the notion of a @ref staticFacts "static fact". Details of how position is encoded are omitted in this example.
 * @li The @em AttitudeController explicitly extends @em Timeline to indicate that there can be no gaps (i.e. timepoints in the @ref horizonDef "plan horizon" that are not covered by one token) and no overlaps (i.e. tokens assigned to this object with necessarily overlapping temporal extents).
 * @li @em Predicates are scoped by class. They permit 0 or more paramaters to be explicitly defined in the model. They implicitly include variables for the set of @em objects the predicate may be assigned to, and the @em start and @em end times delimiting its temporal extent.
 * @li @em Rules are defined to qualify the relationships that must hold between tokens of the predicate in question (i.e. @em Turning). Such relationships are expressed through constraints among variables. Short-hand notation is used to both introduce a @ref subgoalDef "subgoal" and indicate a required temporal relationship. For example, @em met_by indicates that the @em end time of @em this @em token is @em concurrent with the @em start time of the newly introduced @ref subgoalDef "subgoal" @em p0.
 *
 * Within a domain described by such a model, a @em partial @em plan of the form depicted below would be allowed. There we see @em ac1,  an instance of an @em AttitudeController, which contains 3 tokens. Given our model fragment above, we might imagine that the @em Turning token gave rise to each of its adjacent @em Pointing tokens. Note the constraints equating timepoints and parameters. 'D12' and 'Ast' are instances of the class @em position. The @em partial @em plan shown can be described as: "ac1 is pointing at D12 for some time between 0 and 15, whereafter it turns to Ast. It will be pointing at Ast for some time between 2 and 60."
 *
 * @image html partialPlan.1.png
 * @section planDb The Idea of a Plan Database
 * @image html applications.png
 * @section problemSolving Problem Solving
 * @ref europa "EUROPA" is designed first and foremost to solve planning and scheduling problems. In classical planning a planning problem is presented in terms of an @em initial @em state and a @em goal @em state. It is the role of a @em planner to derive the appropriate actions and orderings to flesh out a legal plan to go from one to the other. This is referred to as @em state @em space @em planning since the nodes in the search towards a solution vary in the states they result in based on planning operators selected. In contrast, @ref europa "EUROPA" formulates a planning problem as an initial @em partial @em plan which is @em refined through a series of refinement operations into a final plan that is complete with respect to the requirements of the planner. This approach is referred to as @ref refinementSearch "refinement search" and/or @em plan-space @em planning, since each node represents a different @em partial @em plan obtained by application of a particular refinement operator to a predecessor. In this section, we outline a general algorithm for @ref refinementSearch "refinement search". We then examine in more detail the nature of @ref flaws "flaws" and methods of @ref flaws "flaw resolution". For further details on the framework provided in @ref europa "EUROPA" for implementing custom problem-solvers, the reader is referred to @ref solvers.
 * @subsection refinementSearch Refinement Search
 * An algorithm for problem solving with @ref europa "EUROPA" is given below in pseudo-code. It takes as input a @em partial @em plan @em p and returns true if a complete and consistent extension of @em p could be found (or if @em p is initially complete and consistent), and false otherwise. This algorithm provides for a sound and complete search, assuming that no flaws or available refinement operators are pruned unnecessarily. It permits a heuristically controlled search by applying orderings for @em chooseFlaw and @em makeDecisionPoint. It results in a @em chronologically-backtracking, @em depth-first search. It should be emphasized that while this algorithm is commonly employed, it is only one of many that could be implemented with @ref europa "EUROPA".
 * @include RefinementSearch.cc
 * The key idea in this approach is that one can reliably compute:
 * @li the set of @em flaws for any given @em partial @em plan (i.e. @em chooseFlaw)
 * @li the set of possible refinement operations which can be employed to address a @em flaw (i.e. @em makeDecisionPoint)
 * @li when a dead-end has been reached (i.e. @em isInconsistent).
 *
 * These computations are the essential services that a PlanDatabase must provide to support problem solving. We have already seen that @ref csp "constraint propagation" addresses the latter requirement. In the next section we shall see how @em flaws are defined and addressed.
 * @subsection flaws Flaws and Flaw Resolution
 * There are 3 different types of flaws in @ref europa "EUROPA":
 * @li @anchor unboundVariable @em Unbound @em Variable. A @em variable in the @em partial @em plan whose domain is not a singleton. This @em flaw is the most transparently related to the @em CSP underlying a @em partial @em plan. @em Unbound @em Variables are resolved by speciifcation of a value from the domain of the variable.
 * @li @anchor openCondition @em Open @em Condition. An @em open @em condition is an @ref token "inactive token". A @em token, introduced through an agent external to the PlanDatabase, or through internal execution of a model rule, may not initially be @em inserted into the @em partial @em plan. In particular, it may be a planner decision to @em reject the @em token if it merely reflects an optional request, @em merge the @em token with an @em equaivalent @em token already existing in the @em partial @em plan, or to @em insert the @em token directly into the @em partial @em plan. @em Open @em Conditions are resolved by the operations of @em merge, @em activate, and @em reject. 
 * @li @anchor threat @em Threat. Once a @em token has been placed in the @em partial @em plan we consider its possible impact on the @em objects to which it may be assigned. Recall for example that a @em token may belong to objects which require a @em total @em order over their @em tokens. If any 2 @em tokens could possibly overlap (though not necessarily), then they pose a @em threat to each other in terms of achieving an extension of the current @em partial @em plan which is @em complete and @em consistent. Similarly, @em threats may arise where @em tokens share a common @em resource and their current state might yield extensions of the current @em partial @em plan which are @em inconsistent. @em Threats are esolved by imposing @em ordering @em constraints among @em tokens.
 *
 * The variety in types of @em flaws arises out of a desire to search at a higher-level of abstraction (i.e. in terms of @em objects and @em tokens) than that of a straight @em CSP encoding. This is advantageous when one applies heuristics for ordering choices since it provides a richer context in which to make decisions. Furthermore, it aids in reducing the amount of work done by a @em solver so that only the @em necessary @em refinements are made, otherwise leaving the @em partial @em plan with maximum flexibility. For example, one can omit @em unbound @em variables which are @em timepoints of tokens since @em threats will force a solver to impose restrictions on these variables based on the semantics of the @em objects to which their @em tokens apply. Thus we may end up with @em partially-ordered @em plans for which we can prove that all possible extensions are valid.
 * @section summary Summary
 * This background chapter has discussed the basics of @ref csp and @ref stp which together provide the core technolgoy for pruning invalid search paths and detecting inconsistency. We have also described the basics of @ref planRep in terms of @ref tqa. We further discussed the use of @ref tokens and @ref objects as key primitives in defining @ref partialPlans. Finally, we looked at the @ref problemSolving approach used with @ref refinementSearch, @ref flaws.
 */
