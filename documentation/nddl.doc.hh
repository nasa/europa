/**
 * @page nddl Modeling with NDDL
 *
 * In this chapter we review the capabilities of the NDDL modeling language for describing problem domains and constructing partial plans. NDDL is an acronym for New Domain Description Language which reflects its origins in @ref ddl "DDL" which established many of the semantics and constructs found in NDDL. NDDL is an object-oriented language designed to support expression of problem domain elements and axioms in a manner consistent with the paradigm outlined in @ref planRep. The reader is strongly advised to review the material in @ref background prior to working through this chapter. The material contains:
 * @li @ref varsAndConstraints
 * @li @ref classes
 * @li @ref rules
 * @li @ref nddlTx
 * @section varsAndConstraints Introducing Variables and Constraints
 * Variables and constraints are the basic building blocks of EUROPA. They can be introduced in a number of ways, and in a range of scopes. The following basic forms for variable declaration are supported:
 * @li @em type @em label; Declares a variable of type @em type with the name @em label and allocates a default domain as the @ref baseDomain "base domain" based on the type. The available primitive types are: @em int, @em float, @em bool, and @em string. In addition, a type can be a user-defined @em enumeration or @em class. The default base domain for @em int and @em float is <em>[-inf +inf]</em>, i.e. from negative to positive infinity. Variables of type @em bool have a base domain of <em>{false, true}</em>. Variables of type @em string are a special case requiring a singleton value on declaration.
 * @li @em type @em label = @em value; Declares a variable of type @em type with the name @em label and sets the @ref baseDomain "base domain" to the singleton value given by @em value.
 * @li @em type @em label = @em domain; Declares a variable of type @em type with the name @em label and sets the @ref baseDomain "base domain" to @em domain. For @em int and @em float the domain is described as an interval.
 *
 * The particulars of a constraint are not part of NDDL. Instead, we permit constraints to be drawn from a system or user-defined library of available constraints which are registered under a constraint name. Such names cannot be reserved words in NDDL, but they may otherwise be any alhpa-numeric string. The semantics of the relationship imposed by a registered constraint are not defined within NDDL. Rather, these semantics must be defined by the constraint provider. The general form for introducing a constraint is:
 * @li <em> constraintName(arg0, arg1[, arg2[, ..argN]]);</em> Allocates an instance of a constraint registered under the name @em constraintName. A constraint has 2 or more arguments. An argument can be one of: @em label, @em value, or @em domain.
 *
 * The example below illustrates the use of NDDL to formulate a @ref csp "Constraint Satisfaction Problem" over 4 integer variables.
 * @include ./NDDL/test/compiler/csp.0.nddl
 *
 * If one were to run this problem, the output would be as shown below. First we see the variables <em>a, b, c, </em> and @em d and their values. Note the additional unnamed variables. They arise since constraint arguments that are not references to existing variables result in implict variable creation behind the scenes i.e. a variable is created for each @em non-label constraint argument.
 *
 * @include ./NDDL/test/compiler/RUN_csp.0_g_rt.csp.0.xml.output
 *
 * In this next example, we demonstrate the use of <em>user-defined enumerations</em> and allocate variables of these types.
 *
 * @include ./NDDL/test/compiler/csp.1.nddl
 *
 * As previously mentioned, a variable can be created to refer to a set of instances of a class i.e. @em objects. In this next example, we declare a class @em Foo using the most trivial form of class declaration. We then use NDDL commands to allocate instances. These commands are procedurally executed and will introduce objects into the Plan Database. The first example of variable declaration introduces the variable @em allFoo. This variable will refer to all instances of the class @em Foo that exist in the database once it is @em closed. For more details on @em closure see @ref dynamicObjects "Dynamic Objects". The example also illustrates variable declaration with initial domains allocated to new singleton values. Furthermore, the example demonstrates initialization of one variable with the contents of another. Note that this does not maintain an equality relation between these variables. Instead, it has copy semantics.
 *
 * @anchor csp_2 @include ./NDDL/test/compiler/csp.2.nddl
 *
 * The output of the PlanDatabase for the above example is quite instructive:
 * @li The first three objects are anonymous, and so names are automatically generated to distinguish them in the PlanDatabase. The names are <em> Foo:$OBJECT[0], Foo:$OBJECT[1], Foo:$OBJECT[2]</em>.
 * @li The second three objects are given the names of the variables to which they are assigned e.g. <em>f1, f2, f3</em>
 * @li The domain of @em allFoo contains all 6 instances even though it was declared before <em>f1, f2, f3</em> were allocated. This is because the final set of values in the @ref baseDomain "Base Domain" for a variable containing @em objects is not defined until the type is @em closed e.g. through the statement <em>Foo.close()</em>.
 * @li The contents of @em allFoo and @em f5 differ despite the initial assignment. This demonstrates the @em copy semantics of initial assignment. The differences arise due to constraints posted.
 *
 * @include ./NDDL/test/compiler/RUN_csp.2_g_rt.csp.2.xml.output
 * @section classes Classes
 * The @em class is an abstract data type typically representing a @em noun in the description of a planning problem. The purposes of a class in NDDL are two-fold:
 * @li To introduce a structured data type. For example, a @em path may be a static relation between 2 @em locations.
 * @li To permit manipulation of domain elements at the abstract level of sets of things of a common type which may or may not be enumerable at <em>compile time</em>. For example, we may wish to only go from one location to another if there is a path between them. We may wish to state this in a domain model without actually knowing or caring what paths or locations exist.
 *
 *  In this section we review NDDL's comprehensive support for class declaration which make it a powerful object-oriented modeling language, covering:
 * @li @ref classesAndMembers
 * @li @ref predicates
 * @li @ref inheritance
 * @li @ref composition
 * @li @ref rules
 * @li @ref idioms
 * @li @ref nddlTx
 * @li @ref nddlConfig
 *
 * @subsection classesAndMembers Classes and Members
 * Consider the statement:
 * @verbatim class Path {} @endverbatim
 * This creates a type for which instances can be created in the PlanDatabase and for which variables may be declared and populated based on the allocation of objects of this type in the database. At this point, @em Path has no further structure to it. Nor does it possess any means by which we can describe anything about it. As such, it is a rather limited and useless creature akin to the example seen in @ref csp_2 "basic variable declaration". However, if we now introduce some additional structure a useful abstract data type emerges.
 * @include ./NDDL/test/compiler/classes.0.nddl
 * The above model illustrates:
 * @li Declaration of member variables of a class. The same syntax already seen for variable declaration can be used within the scope of a <em>class declaration</em>. This syntax permits declaration where the default domain can be derived by type as is the case for the @em Location member variables, or based on explicit assignment of a default value as is the case for @em cost.
 * @li Constructors can be defined and overloaded to accomplish different initialization patterns. The rules for variable assignment in a class constructor are the same as those already seen in @ref csp_2 "basic variable declaration", where the scope in this case is defined by variables passed in as arguments to the constructor, or available as memebrs of the class.
 * @li If no constructor is provided, a default constructor will be assumed and it will allocate variables based strictly on how they are declared in the class. If a constructor is provided, then all constructors must be explicitly declared. For example, given the prior example, we could not write: <em>Path p = new Path();</em> since no constructor is declared without arguments.
 *
 * In order to emphasze the distinction between @ref compileTime "compile-time" and @ref runTime "run-time" problem specification, the NDDL code to allocate instances into a @em running @ref planDatabaseDef "plan database" (i.e. run-time data) has been separated into a file which includes the model (i.e. compile-time data) as shown below.
 * @include ./NDDL/test/compiler/classes.0.tx.nddl
 * Execution of the above yields:
 * @include ./NDDL/test/compiler/RUN_classes.0.tx_g_rt.classes.0.tx.xml.output
 *
 * Here we can see the contents of each object printed out. Note the nomenclature for these variables. They are scoped by the parent object name. An important caveat should be noted: <em>member variables should be initialized to singletons</em>. This is a restriction arising from a limitation in the implementation of @ref existentialQuantification "existential quantification"
 * @subsection predicates Predicate Declaration
 * Thus far, the discussion of @em classes has been limited to problem domain elements without any temporal context. This does not get us very far in tackling planning problems! Now we turn to the use of @em predicates in class declarations to provide the means to describe temporally qualified state and action of @em objects.
 *
 * In this next example, consider a @em Navigator with the simple behavior of being @em At a location or @em Going from one location to another. We can use predicates to represent both the @em state of being @em At a location and the @em action of @em Going from one location to another:
 * @include ./NDDL/test/compiler/classes.1.nddl
 * Note that declaration of arguments to a predicate follow patterns of variable declaration and allocation already described. Note also that there may be cases where relationships exist among predicate arguments and we wish to capture these relationships in the declaration. In our example, we prohibit going to a location one is presently at by posting a @ref parameterConstraint "parameter constraint" in the body of the predicate declaration which imposes the relation that its arguments are not equal (@em neq for short).
 * To demonstrate construction of a simple partial plan we use NDDL to construct an initial state with:
 * @li A set of @em Locations
 * @li A single instance of a @em Navigator
 * @li 2 @em tokens i.e. instances of @em predicates.
 *
 * The commands to produce this partial plan are shown below:
 * @include ./NDDL/test/compiler/classes.1.tx.nddl
 *
 * Notice that we are introducing a number of new commands which are included in the @ref nddlTx "NDDL transaction language" (and further described therein):
 * @li @em close. Close the plan database thereafter prohibiting any new objects from being created. We may now exploit the @ref closedWorldAssumption "Closed World Assumption".
 * @li @em goal. Creates a new token in the plan database and activates it immediately.
 * @li @em specify. Sets the @ref specifiedDomain "specified domain" of a variable to a given @em value or @em domain.
 *
 * The contents of the plan database obtained by execution of the above commands is given in the listing below:
 * @include ./NDDL/test/compiler/RUN_classes.1.tx_g_rt.classes.1.tx.xml.output
 * A few points of interest are worthy of comment:
 * @li Observe that 2 @em active tokens are present in the partial plan. This corresponds with @em t0 and @em t1.
 * @li Details of each token indicate the @ref derivedDomain "derived domains" of each token variable. The @em start times of each token are singletons as expected.
 * @li The @em end variables of each token have been propagated to reflect the @em temporalDistance constraint and the built in requirement that @ref intervalToken "interval tokens" have a @em duration of at least 1.
 * @li The tokens have no @ref masterToken "master token". This is because these tokens have been introduced explicitly by an external client (i.e. an initial state loader), rather than implicitly through @ref ruleInstanceDef "rule instance" execution. Tokens without a @em master are referred to as @ref orphanToken "orphans".
 * @subsection inheritance Inheritance
 * In the @em Navigator example, we really desire that tokens for @em At and @em Going may not overlap in time. To obtain this behavior, we change the definition of @em Navigator such that it extends the built-in @ref timeline "Timeline" class and thus inherits the desired semantics. The revision to the previous example is shown below which uses the @em extends keyword to declare inheritance.
 * @include ./NDDL/test/compiler/classes.2.nddl
 * This particular example of inheritance shows extension of a class that maps to a special implementation in the plan database. For more on this important mechanism of customization and specialization of NDDL, see @ref nddlConfig. Since we are for now relying on commands in the initial state to construct the final partial plan for our examples, we must also add and additional command to explicitly assign the tokens to the @em nav1 object. This is accomplished with the additional statement: @verbatim nav1.constrain(t0, t1);@endverbatim The new partial plan resulting indicates the active tokens are now sequenced on the @em Navigator instance.
 * @include ./NDDL/test/compiler/RUN_classes.2.tx_g_rt.classes.2.tx.xml.output
 * @subsection composition Composition
 * @section rules Model Rules
 * @subsection allenRelations Subgoals and Allen Relations
 * @subsection conditionalSubGoals Conditional Subgoals
 * @subsection existentialQuantification Existential Quantification
 * @subsection universalQuantification Universal Quantification
 * @section idioms Useful Modeling Idioms
 * @section nddlTx The NDDL transaction language
 * @section nddlConfig NDDL Compiler Configuration
 */
