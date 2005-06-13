/**
 * @page nddl Modeling with NDDL
 *
 * In this chapter we review the capabilities of the NDDL modeling language for describing problem domains and constructing partial plans. NDDL is an acronym for New Domain Description Language which reflects its origins in @ref ddl "DDL" which established many of the semantics and constructs found in NDDL. NDDL is an object-oriented language designed to support expression of problem domain elements and axioms in a manner consistent with the paradigm outlined in @ref planRep. The reader is strongly advised to review the material in @ref background prior to working through this chapter. The material contains:
 * @li @ref varsAndConstraints
 * @li @ref basicClasses
 * @li @ref basicPredicates
 * @li @ref dynamicObjects
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
 * @include ./NDDL/test/compiler/csp.2.nddl
 *
 * The output of the PlanDatabase for the above example is quite instructive:
 * @li The first three objects are anonymous, and so names are automatically generated to distinguish them in the PlanDatabase. The names are <em> Foo:$OBJECT[0], Foo:$OBJECT[1], Foo:$OBJECT[2]</em>.
 * @li The second three objects are given the names of the variables to which they are assigned e.g. <em>f1, f2, f3</em>
 * @li The domain of @em allFoo contains all 6 instances even though it was declared before <em>f1, f2, f3</em> were allocated. This is because the final set of values in the @ref baseDomain "Base Domain" for a variable containing @em objects is not defined until the type is @em closed e.g. through the statement <em>Foo.close()</em>.
 * @li The contents of @em allFoo and @em f5 differ despite the initial assignment. This demonstrates the @em copy semantics of initial assignment. The differences arise due to constraints posted.
 *
 * @include ./NDDL/test/compiler/RUN_csp.2_g_rt.csp.2.xml.output
 * @section basicClasses Class Declaration
 * @section basicPredicates Predicate Declaration
 * @section dynamicObjects Dynamic Objects
 */
