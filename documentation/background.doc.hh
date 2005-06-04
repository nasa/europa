/**
 * @page background Background
 * Wherein we review some of the core concepts and technologies employed in the EUROPA 2 Constraint-based Planning 
 * Paradigm.
 *
 * @section csp Constraint Satisfaction Problems
 * EUROPA is based on translating a representation of a plan into a graph of constraints and variables. Solving a planning
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
 * In this problem, note that no soution is possible which contains the value 0 for @em time. Such a value would make constraint @em C0 inconsistent. Similarly, the value @f$\infty@f$ is excluded from all solutions. Constraints cab be @em propagated to remove values from the domain of variable that cannot participate in a solution to the @em CSP. The principal solution techniques available for solving a @em CSP are:
 * @li Heuristic Search to select a value for each variable from the remaining values in its domain.
 * @li Propagation to prune infeasible values to reduce backtracking, and to detect a dead-end where no further refinement
 * of the variables is a solution.
 * 
 * Solving a @em CSP is NP-Hard (i.e. totally intractable!) in theory, but often very efficient in practice using the 
 * above methods. EUROPA uses an extended version of the basic ideas of a @CSP called a @em Dynamic @em Constraint @em 
 * Satisfaction @em Problem (@em DCSP). A @em DCSP permits addition of new variables and constraints to the problem which
 * is essential as plans evolve with new activities and states and relations between them being created.
 * @section stp Simple Temporal Problems
 * The notion of time is central to temporal planning. EUROPA uses @em variables to explicitly represent 
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
 * EUROPA.
 * @section planrep Plan Representation
 * Here we introduce 2 major abstractions which build upon the framework of @ref csp and the related work of @ref stp. The
 * first is the notion of an @em Object and the second is the notion of an @em Token. To explain these concepts, we use
 * an example of a satellite which has a camera on board to take pictures. It also has an attitude control
 * system that is responsible for holding a position and altering the position from one attitude to another in order to
 * take the next picture.
 *
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
 * @li @em slewing from one position to another.
 *
 * An @em object is an instance of a @em class, as is the case in most object-oriented paradigms. And we model using the
 * abstraction of a class to speak about all instances having certain properties of state and behavior. In order to
 * describe such state and behavior we turn build on the formalism of @em first @em order @em logic.
 *
 * @subsection tokens Predicates in Time
 * A predicate defines a relation between objects and properties.In EUROPA, we define such relations between variables
 * whose domains are sets of objects and sets of properties. For example, we might use a predicate @em Pointing(a,p) to 
 * indicate that the @em attitude @em controller @em a is @em pointing at position @em p. Note that @em a is a @em
 * variable which may have a number of possible values in a problem with multiple satellites. Similarly, @em p is
 * a @em variable whose values are the set of possible @em positions. In a fully grounded plan, single values will be
 * specified for each variable as we saw in the case of a @em solved @ref csp. In general, is is not sufficient to state
 * that a predicate is true without giving it some temporal extent over which it holds. Predicates that are always true
 * can be thought to hold from the beginning to the end of time. However, in practice, the temporal extent of interest 
 * must be defined with @em timepoints to represent its @em start and @em end. So we might write:  @em Pointing(a,s,e,p)
 * to indicate that the @em attitude @em controller @em a is @em pointing at position @em p from time @em s to time
 * @em e. In fact, this pattern of using such predicates to describe both state and behavior of objects is so prevalent
 * in temporal planning that we have introduced a special construct called a @em Token which has the built in @em
 * variables to indicate the @em object to which the statement principally aplies and the @em timepoints over which 
 * it holds.
 * @subsection objecttoken Objects And Their Tokens
 * @section problemrep Problem Representation
 * @section problemsolving Problem Solving
 */
