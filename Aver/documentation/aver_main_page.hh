/**
 * @defgroup Aver
 * <A NAME="aver_overview"></A>
 * <pre><b>aver</b> (a*ver') tr. v.: 
 *    1. To affirm positively; declare.
 *    2. <i>Law</i>
 *      a. To assert formally as a fact.
 *      b. To justify or prove.
 * </pre>
 * -The American Heritage Dictionary of the English Language, Fourth Edition

 * Aver is a test-definition language intended for the automated verification of planners, models, or the automatic behavior of a plan 
 * database.  It provides facilities for specifying the attributes of a partial or complete plan and the events that ocurred during planning
 * that constitude "correct" planning and plan database operation.  An assertion is a boolean statement that examines a particular aspect of
 * a plan (how many "Foo" tokens exist) or the planning behavior (whether or not a "backtrack" message occurred) and asserts something about
 * it. Assertions are preceeded by a specification of when the assertion must be true. They are grouped into tests that can be further 
 * organized into super-tests.  Files containing collections of tests and assertions are converted into XML and then compiled into Aver 
 * instructions which are executed at run-time.
 * <A NAME="aver_to_xml"><H2>Aver Parser</H2></A>
 * Aver is converted from it's common textual format to an XML format offline before it is compiled and executed at run-time.  The converter
 * from Aver to XML is available as a standalone tool.
 * <H3>Design</H3>
 * The Aver to XML converter uses <a href="http://www.antlr.org/>ANTLR</a> as a parser which produces an abstract syntax tree from Aver 
 * statements.  This abstract syntax tree is converted into an XML structure that is written to disk.  The XML is manipulated using
 * <a href="http://nanoxml.sourceforge.net/orig/>NanoXML</a>.
 * <H3>Command-line interface</H3>
 * Aver is intended to be used as part of a suite of tests, complementing a system's unit tests.  As such, the typical interface to the 
 * XML converter is through special rules created for the <a href=http://public.perforce.com/public/jam/index.html>Jam</a> build management 
 * tool.  However, the Aver to XML converter has a command-line interface which is built as part of aver.jar when the Aver module is built.
 * The converter also requires two third-party libraries:  antlr.jar and nanoxml.jar.  A typical invocation would look like this:
 * <pre>java -cp <path to nanoxml.jar>:<path to antlr.jar>:<path to aver.jar> aver.AverToXml <aver file> <output file></pre>
 * <H2>Aver Compiler/Interpreter</H2>
 * At run-time, the XML is used by the Aver interpreter to model the structure of its Tests and Assertions and the individual assertions are
 * compiled into Aver instructions.  Once planning begins, a set of AssertionExecutors listen for the circumstances under which their 
 * associated Assertions should be executed and execute them if appropriate.<br>
 * It is possible to set the behavior of Assertions upon failure.  The three behaviors are:
 * <ul>
 * <li>Fail fast - report and abort on the first failed Assertion.</li>
 * <li>Warn fast - report immediately on each failed Assertion, but continue execution.</li>
 * <li>Warn wait - store a report for each failed Assertion, continue execution, and report on termination.</li>
 * </ul>
 * <A NAME="aver_entry"><H3>Compiler/Interpreter Interface</H3></A>
 * Because compilation and interpretation occurs at run-time, the only interface to the compiler/interpreter is in code, through the 
 * AverInterp class.  AverInterp::init() takes as arguments the name of the XML file, the plan database, and the EUROPA2 components
 * whose events should be listened for.  It compiles the XML, instantiates the Test, Assertion, and AssertionExecutor objects, and registers
 * the appropriate listeners.  AverInterp::terminate() tears down the structure and prints a report on the status of the Assertions.
 * <A NAME="aver_use"><H2>Using the Aver API</H2></A>
 * Each of the tests in PLASMA/Aver/test/converter display the full usage of on of the Aver language's features.<p>
 * PLASMA/Aver/test/modtest is a fairly comprehensive test that actually executes.  <p>
 * PLASMA/Aver/test/runTransactions.cc demonstrates how to use the Aver compiler/interpreter in your code. <p>
 */
