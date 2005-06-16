/**
 * @page nddl Modeling with NDDL
 * In this chapter we review the capabilities of the NDDL modeling language for describing problem domains and constructing partial plans. NDDL is an acronym for New Domain Description Language which reflects its origins in @ref ddl "DDL" which established many of the semantics and constructs found in NDDL. NDDL is an object-oriented language designed to support expression of problem domain elements and axioms in a manner consistent with the paradigm outlined in @ref planRep. The reader is strongly advised to review the material in @ref background prior to working through this chapter. The material contains:
 * @li @ref varsAndConstraints
 * @li @ref classes
 * @li @ref nddlTx
 * @li @ref rules
 * @li @ref idioms
 * @li @ref nddlConfig
 * @li @ref nddlKeywords
 *
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
 * @anchor csp_0 @include NDDL/test/compiler/csp.0.nddl
 *
 * If one were to run this problem, the output would be as shown below. First we see the variables <em>a, b, c, </em> and @em d and their values. Note the additional unnamed variables. They arise since constraint arguments that are not references to existing variables result in implict variable creation behind the scenes i.e. a variable is created for each @em non-label constraint argument.
 *
 * @include NDDL/test/compiler/RUN_csp.0_g_rt.csp.0.xml.output
 *
 * In this next example, we demonstrate the use of <em>user-defined enumerations</em> and allocate variables of these types.
 *
 * @include NDDL/test/compiler/csp.1.nddl
 *
 * As previously mentioned, a variable can be created to refer to a set of instances of a class i.e. @em objects. In this next example, we declare a class @em Foo using the most trivial form of class declaration. We then use NDDL commands to allocate instances. These commands are procedurally executed and will introduce objects into the Plan Database. The first example of variable declaration introduces the variable @em allFoo. This variable will refer to all instances of the class @em Foo that exist in the database once it is @em closed. For more details on @em closure see @ref dynamicObjects "Dynamic Objects". The example also illustrates variable declaration with initial domains allocated to new singleton values. Furthermore, the example demonstrates initialization of one variable with the contents of another. Note that this does not maintain an equality relation between these variables. Instead, it has copy semantics.
 *
 * @anchor csp_2 @include NDDL/test/compiler/csp.2.nddl
 *
 * The output of the PlanDatabase for the above example is quite instructive:
 * @li The first three objects are anonymous, and so names are automatically generated to distinguish them in the PlanDatabase. The names are <em> Foo:$OBJECT[0], Foo:$OBJECT[1], Foo:$OBJECT[2]</em>.
 * @li The second three objects are given the names of the variables to which they are assigned e.g. <em>f1, f2, f3</em>
 * @li The domain of @em allFoo contains all 6 instances even though it was declared before <em>f1, f2, f3</em> were allocated. This is because the final set of values in the @ref baseDomain "Base Domain" for a variable containing @em objects is not defined until the type is @em closed e.g. through the statement <em>Foo.close()</em>.
 * @li The contents of @em allFoo and @em f5 differ despite the initial assignment. This demonstrates the @em copy semantics of initial assignment. The differences arise due to constraints posted.
 *
 * @include NDDL/test/compiler/RUN_csp.2_g_rt.csp.2.xml.output
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
 *
 * @subsection classesAndMembers Classes and Members
 * Consider the statement:
 * @verbatim class Path {} @endverbatim
 * This creates a type for which instances can be created in the PlanDatabase and for which variables may be declared and populated based on the allocation of objects of this type in the database. At this point, @em Path has no further structure to it. Nor does it possess any means by which we can describe anything about it. As such, it is a rather limited and useless creature akin to the example seen in @ref csp_2 "basic variable declaration". However, if we now introduce some additional structure a useful abstract data type emerges.
 * @include NDDL/test/compiler/classes.0.nddl
 * The above model illustrates:
 * @li Declaration of member variables of a class. The same syntax already seen for variable declaration can be used within the scope of a <em>class declaration</em>. This syntax permits declaration where the default domain can be derived by type as is the case for the @em Location member variables, or based on explicit assignment of a default value as is the case for @em cost.
 * @li Constructors can be defined and overloaded to accomplish different initialization patterns. The rules for variable assignment in a class constructor are the same as those already seen in @ref csp_2 "basic variable declaration", where the scope in this case is defined by variables passed in as arguments to the constructor, or available as memebrs of the class.
 * @li If no constructor is provided, a default constructor will be assumed and it will allocate variables based strictly on how they are declared in the class. If a constructor is provided, then all constructors must be explicitly declared. For example, given the prior example, we could not write: <em>Path p = new Path();</em> since no constructor is declared without arguments.
 *
 * In order to emphasze the distinction between @ref compileTime "compile-time" and @ref runTime "run-time" problem specification, the NDDL code to allocate instances into a @em running @ref planDatabaseDef "plan database" (i.e. run-time data) has been separated into a file which includes the model (i.e. compile-time data) as shown below.
 * @include NDDL/test/compiler/classes.0.tx.nddl
 * Execution of the above yields:
 * @include NDDL/test/compiler/RUN_classes.0.tx_g_rt.classes.0.tx.xml.output
 *
 * Here we can see the contents of each object printed out. Note the nomenclature for these variables. They are scoped by the parent object name. An important caveat should be noted: <em>member variables should be initialized to singletons</em>. This is a restriction arising from a limitation in the implementation of @ref existentialQuantification "existential quantification"
 * @subsection predicates Predicate Declaration
 * Thus far, the discussion of @em classes has been limited to problem domain elements without any temporal context. This does not get us very far in tackling planning problems! Now we turn to the use of @em predicates in class declarations to provide the means to describe temporally qualified state and action of @em objects.
 *
 * In this next example, consider a @em Navigator with the simple behavior of being @em At a location or @em Going from one location to another. We can use predicates to represent both the @em state of being @em At a location and the @em action of @em Going from one location to another:
 * @include NDDL/test/compiler/classes.1.nddl
 * Note that declaration of arguments to a predicate follow patterns of variable declaration and allocation already described. Note also that there may be cases where relationships exist among predicate arguments and we wish to capture these relationships in the declaration. In our example, we prohibit going to a location one is presently at by posting a @ref parameterConstraint "parameter constraint" in the body of the predicate declaration which imposes the relation that its arguments are not equal (@em neq for short).
 * To demonstrate construction of a simple partial plan we use NDDL to construct an initial state with:
 * @li A set of @em Locations
 * @li A single instance of a @em Navigator
 * @li 2 @em tokens i.e. instances of @em predicates.
 *
 * The commands to produce this partial plan are shown below:
 * @include NDDL/test/compiler/classes.1.tx.nddl
 *
 * Notice that we are introducing a number of new commands which are included in the @ref nddlTx "NDDL transaction language" (and further described therein):
 * @li @em close. Close the plan database thereafter prohibiting any new objects from being created. We may now exploit the @ref closedWorldAssumption "Closed World Assumption".
 * @li @em goal. Creates a new token in the plan database and activates it immediately.
 * @li @em specify. Sets the @ref specifiedDomain "specified domain" of a variable to a given @em value or @em domain.
 *
 * The contents of the plan database obtained by execution of the above commands is given in the listing below:
 * @include NDDL/test/compiler/RUN_classes.1.tx_g_rt.classes.1.tx.xml.output
 * A few points of interest are worthy of comment:
 * @li Observe that 2 @em active tokens are present in the partial plan. This corresponds with @em t0 and @em t1.
 * @li Details of each token indicate the @ref derivedDomain "derived domains" of each token variable. The @em start times of each token are singletons as expected.
 * @li The @em end variables of each token have been propagated to reflect the @em temporalDistance constraint and the built in requirement that @ref intervalToken "interval tokens" have a @em duration of at least 1.
 * @li The tokens have no @ref masterToken "master token". This is because these tokens have been introduced explicitly by an external client (i.e. an initial state loader), rather than implicitly through @ref ruleInstanceDef "rule instance" execution. Tokens without a @em master are referred to as @ref orphanToken "orphans".
 * @subsection inheritance Inheritance
 * Inheritance is a method of re-use in NDDL designed to make models more compact, and modeling less laborious and error-prone. The goal is that NDDL libraries may be developed that can be applied on many applications in a common domain (e.g. robotic control).
 *
 * @par Making the @em Navigator a @em Timeline
 * In the @em Navigator example, we really desire that tokens for @em At and @em Going may not overlap in time. To obtain this behavior, we change the definition of @em Navigator such that it extends the built-in @ref timeline "Timeline" class and thus inherits the desired semantics. The revision to the previous example is shown below which uses the @em extends keyword to declare inheritance.
 * @include NDDL/test/compiler/classes.2.nddl
 * This particular example of inheritance shows extension of a class that maps to a special implementation in the plan database. For more on this important mechanism of customization and specialization of NDDL, see @ref nddlConfig. Since we are for now relying on commands in the initial state to construct the final partial plan for our examples (i.e. we are not running a @em solver to fill out a final plan), we must also add an additional command to explicitly assign the tokens to the @em nav1 object. This is accomplished with the additional statement: @verbatim nav1.constrain(t0, t1);@endverbatim The new partial plan resulting indicates the active tokens are now sequenced on the @em Navigator instance.
 * @include NDDL/test/compiler/RUN_classes.2.tx_g_rt.classes.2.tx.xml.output
 * @par Inheritance, Types and Sets
 * The @em Navigator example provides a first look at the mechanism of inheritance i.e. the @em extends keyword. Before delving further into the mechanics of this language feature, we will consider the relationship between inheritance, types and sets of objects in the partial plan. This is very important since we deal extensively in sets of objects so the interaction with inheritance must be well understood. To understand this, consider the diagram below which depicts an inheritance tree where an arrow indicates an @em is-A relationship. For example, a <em>Bar is-A Foo</em>.
 * @image html inheritance.0.png
 * When we create instances of these classes in the plan database, each instance will fall into one of the sets identified in the Venn Diagram below according to its type. For example, @verbatim Foo f = new Foo();@endverbatim will insert f into the set of all instances of type @em foo.
 * @image html inheritance.1.png
 * The NDDL code below creates this class hierarchy (@em Object and @em Timeline are re-used from Plasma.nddl). For this example, the classes are trivial. The code also allocates instances of each class and declares variables, each of which will be populated with the set of objects of the type of the variable.
 * @include NDDL/test/compiler/inheritance.0.nddl
 * The resulting plan database illustrates precisely which objects are placed in which sets in the afore mentioned Venn Diagram. The contents of variables <em>allObjects, allTimelines, allFoo, allBar, allBaz, and allBing</em> indicate the membership of the sets of objects of type <em>Object, Timeline, Foo, Bar, Baz, and Bing</em> respectively.
 * @include NDDL/test/compiler/RUN_inheritance.0_g_rt.inheritance.0.xml.output
 * @par Superclass Constructors
 * We have seen that NDDL permits the declaration of 0 or more constructors for any given class. Similarly, one may declare 0 or more constructors in a subclass. Since constructors can be @em overloaded (i.e. more than one version), we must be able to unambiguously refer to a specific constructor of the immediate superclass from within the constructor of a derived class. For this, the keyword @em super is introduced.
 * @include NDDL/test/compiler/inheritance.1.nddl
 * The output is shown below:
 * @include NDDL/test/compiler/RUN_inheritance.1_g_rt.inheritance.1.xml.output
 * @par Predicate Inheritance
 * So far, we have seen examples of inheritance which extend the set of member variables in the derived class. Predicates defined in a base class are also inherited. In a derived class one can:
 * @li inherit all predicate definitions from super classses.
 * @li extend the set of arguments to a predicate defined in a super class
 * @li extend the set of predicates defined in the derived class
 * @li add additional constraints to the arguments of a predicate defined in a super class
 * @li add additional rules relating to the compatibility of a predicate first defined in a super class with other predicates. This capability will be discussed in the chapter on @ref rules "Model Rules".
 *
 * The example below demonstrates these feautures.
 * @include NDDL/test/compiler/inheritance.2.nddl
 * The output is shown below.
 * @include NDDL/test/compiler/RUN_inheritance.2_g_rt.inheritance.2.xml.output
 * @subsection composition Composition
 * We have already seen that classes can compose variables of primitive data types. We have also seen one mechansim for re-use through @em inheritance. NDDL also supports re-use of classes by @em composition, and it achieves this by permitting member variables to be of a class. For example, consider a domain with a @em rover and a @em crew-carrier which both have a common behavior for navigation. Specifically, they both can be @em At a location or @em Going from one location to another. However, one can easily imagine that other details of the @em rover and @em crew-carrier might be very different from one another. We can represent the @em rover and @em crew-carrier as classes which both contain a @em Navigator. This permits the modeler to re-use the model-component and integrate it into additional needs of the containing object. The NDDL code below gives a trivial example of class composition which illustrates the method of declaration following that already established for variables in general. It also illustrates different methods of assignment, including passing an object into a constructor, thereby allowing objects to be shared. Alternatively, an object can be allocated within the class constructor.
 * @include NDDL/test/compiler/composition.0.nddl
 * The output is shown below.
 * @include NDDL/test/compiler/RUN_composition.0_g_rt.composition.0.xml.output
 * @section nddlTx The NDDL transaction language
 * Now that we have explained the type structures available in NDDL, we can present the NDDL commands to operate on the plan database and thus initialize and/or modify a partial plan. The idea of the NDDL transaction language is to provide syntax and demantics closely related to the use of NDDL elsewhere for class, predicate and rule declaration. However, the NDDL transaction language pertains exclusively to @ref runTime "run-time" data.
 *
 * In @ref varsAndConstraints we covered variables and constraint creation using the NDDL transaction language. We showed:
 * @li Declaration of a variable with a default base domain - e.g. <em>int i; Colors colors;</em>
 * @li Declaration of a variable with an explicit and restricted base domain - e.g. <em> int i = 6; int j = [10 40]; Colors colors = Blue;</em>
 * @li Declaration of an @em object variable and allocation of an object - e.g. <em>Foo f = new Foo();</em>
 * @li Declaration of one variable by assignment of the contents of another - e.g. <em>Foo b = f;</em>
 * @li Allocation of a constraint - e.g. <em>eq(i, j); neq(j, k);</em>
 *
 * In @ref classes we introduced @em predicates, @em composition and @em inheritance. There we learned that constructors with arguments can be invoked explicitly , passing in values, domains or variable references - e.g. <em>Bar b = new Bar(f);</em>. We also learned that tokens can be created using the @em goal keyword. We used the @em constrain keyword to asssign tokens to an object and impose an ordering, and used the @em specify keyword to set values of a variable. We will revisit some of these operations in the remainder of this section as well as reviewing others.
 * @par Creating Tokens
 * There are 2 ways to introduce a token into the plan database using NDDL transactions:
 * @li The @em goal keyword. It has the form <em>goal(class.predicate [label]);</em> where @em class designates the set of objects to which this token can be assigned and @em predicate designates the particular predicate to be created. A @em label is optionally used if later NDDL statements wish to refer to the instance to be allocated. For example, consider the statement: @verbatim goal(Navigator.At);@endverbatim This statement results in a new token in the plan database which will be in the @em active state. The object variable of the new token will be populated with the set of all instances in the Navigator class present in the database <em>at the time of token creation</em>.
 * @subsection ssec3 Closure
 * @subsection ssec4 Specifying and Resetting Variables
 * @subsection ssec5 Operations on Tokens
 * @section rules Model Rules
 * So far in our discussion of NDDL we have managed to avoid any real planning context, since we have not yet mentioned any means of expressing interactions among tokens other than through statements see in examples of using the @ref nddlTx. In this section we will describe the facilities in NDDL for describing relationships that must or must not exist between tokens and their variables. The components of this section are:
 * @li @ref basicRules
 * @li @ref allenRelations
 * @li @ref conditionalSubgoals
 * @li @ref existentialQuantification
 * @li @ref universalQuantification
 * @li @ref idioms
 * @subsection basicRules Basic Rule Definition
 * Let us return to our navigation example. For the careful reader, it should be clear that the proper relationships betwen @em At tokens and @em Going tokens are absent. Though we have extended a Timeline to give an indication of a total order, we have not precluded the seemingly magical movement from being <em>At(Lander)</em> and immediately switching to <em>At(Rock)</em> without actually @em Going anywhere. To rectify this, we define 2 model rules. The first defines appopriate relations that must hold for an @em active instance of the @em At predicate. The second addresses the same for @em Going.
 * @include NDDL/test/compiler/rules.0.nddl
 * @include NDDL/test/compiler/rules.0.tx.nddl
 * @include NDDL/test/compiler/RUN_rules.0.tx_g_rt.rules.0.tx.xml.output
 * @subsection allenRelations The Allen Relations
 * @subsection conditionalSubgoals Conditional Subgoals
 * @subsection existentialQuantification Existential Quantification
 * @subsection universalQuantification Universal Quantification
 * @subsection idioms Useful Modeling Idioms
 * @section nddlConfig NDDL Compiler Configuration
 * @section nddlKeywords Summary of NDDL keywords
 * class
 * extends
 * int
 * float
 * bool
 * string
 * enum
 * meets
 * met_by
 * any
 * overlaps
 * contains
 * contained_by
 * starts
 * ends
 * if
 * foreach
 * in
 * ==
 * true
 * false
 */
