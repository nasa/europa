/**
 * @page nddl Modeling with NDDL
 * @section helloWorld Hello World
 * In this section we explain the basics of using the out of the box @em solver to solve a simple planning problem. The main objective is to get you started using the build process to edit a model and initial state and observe the results. In the time honored tradition of programming language or system documentation, we provide a simple demonstration with a "Hello World" example.
 * @subsection workFlow The Process
 * Before actually trying out the example, it behoves us to describe in brief the basic setup used. The diagram below presents a workflow to integrate the user-defined files and thus customize the solver to the needs of the domain. The steps are as follows:
 * @li Parse the model. The user writes a domain model in one or more @ref nddl "NDDL" model files. These files define the types of the domain and the required relationships among instances of the types. The @em parser produces an xml docuemnt with the same suffix as the main model file. The input model file produced by our @ref makeProject "template project generator" is described in @ref model.
 * @li Compile the model. The xml version of the model is used to generate C++ code with type declarations output to the .hh file and all implementation code output to the .cc file. Thus we have a C++ version of the model initially input in @ref nddl "NDDL". It is recommended that the reader inspect this source code to gain a deeper understanding of the relationship betwen the semantics of @ref nddl "NDDL" and the @ref europa "EUROPA" C++ library.
 * @li Build the model binary. The generated C++ code is compiled and linked to produce either a static or shared library. It is recommended that a shared library be used to limit link times when the model changes.
 * @li Parse the initial state. The user must provide an initial state file which uses the @ref nddl "NDDL" transaction language to procedurally construct an initial partial plan. The input initial state file produced by our @ref makeProject "template project generator" is described in @ref initialState.
 * @li Load and run the problem. The solver is a batch planner built as a specific assembly of @ref europa "EUROPA" components. The internal component architecture is not represented in this view, and its details are irrelevant for the purposes of experimenting with @ref nddl "NDDL". The input system configuration file is produced by our @ref makeProject "template project generator". Its details are not pertinent to this chapter. For more information on this topic see @ref solverConfig.
 *
 * While the workflow might seem complex, do not be intimidated! A convenient set of build rules have been set up to make this process quite straightforward. To try it out for yourself, follow the instructions in @ref makeProject.

 * @image html build.process.png
 * @subsection makeProject Trying HelloWorld
 * @include ../HELLOWORLD
 * @subsection model The Model
 * @include HelloWorld-model.nddl
 * @subsection initialState The Initial State
 * @include HelloWorld-initial-state.nddl
 */
