/**
 * @defgroup NDDL
 * <A NAME="nddl_overview"><H3>NDDL Overview</H3></A>
 *
 * The NDDL module encompasses a couple of standalone tools
 * for parsing NDDL and compiling the resulting XML into
 * C++ code.  The module also includes domain-independent 
 * C++ code which is used by the generated code.
 *
 * <A NAME="nddl_parser"><H3>NDDL Parser</H3></A>
 *
 * The NDDL Parser transforms NDDL into XML.  It performs
 * some analysis and semantic checking during the 
 * transformation.  It is available as a standalone tool.
 *
 * <H4>Design</H4>
 *
 * The NDDL Parser is based on <a href="http://www.antlr.org/>ANTLR</a>.
 * There are two passes to the parse process.  The first pass
 * parses the input NDDL input an abstract syntax tree.  The
 * second pass parses the abstract syntax tree into an in-memory
 * XML structure.  This structure is passed to a class which
 * processes it and determines type information.  Finally, the
 * XML is passed to a semantic checker, before being written out.
 * The XML is manipulated using <a href="http://nanoxml.sourceforge.net/orig/>NanoXML</a>.
 *
 * <H4>Command-line interface</H4>
 *
 * The NDDL Parser has a command line interface which is built
 * as part of nddl.jar when the NDDL module is built.  The
 * parser also requires the third-party libraries antlr.jar and
 * nanoxml.jar.  An example of how to invoke the parser is in
 * the root cvs directory in a script called "nddlparse".
 *
 * Generally the parser is invoked to parse a single NDDL file
 * into xml.  An example of how to do this is shown when you run
 * the "nddlparse" executable.  It looks similar to this:
 *
 * <PRE>nddlparse -o filename.xml filename.nddl</PRE>
 *
 * In the above example, the program looks for a file called
 * "filename.nddl" in the current directory and passes it to
 * the NDDL parsing routines.  After parsing, adding types, and
 * checking the semantics, the program writes the resulting
 * xml to a file called "filename.xml" in the current directory.
 *
 * <A NAME="nddl_compiler"><H3>NDDL Compiler</H3></A>
 *
 * The NDDL Compiler transforms XML into C++ code.  It
 * performs a semantic check on the XML and then generates
 * a header file and an implementation file.  It is available
 * as a standalone tool.
 *
 * <H4>Design</H4>
 *
 * First, the NDDL Compiler reads in the XML for all input files.
 * Next, it passes the XML through the semantic checker.  If it
 * passes the semantic check, it processes the input using
 * the header generator to write the header (.hh) file.  Finally,
 * it processes the input using the implementation generator to
 * write the implementation (.cc) file.
 *
 * The NDDL Compiler makes use of a number of classes for writing
 * specific constructs, such as enumerations, predicates, rules.
 * It also uses several utility classes that provide services such
 * as indented printing and common xml traversals for models.
 *
 * <H4>Command-line interface</H4>
 *
 * The NDDL Compiler has a command line interface which is built
 * as part of nddl.jar when the NDDL module is built.  The
 * compiler also requires the third-party library nanoxml.jar.  An
 * example of how to invoke the compiler is in the root cvs
 * directory in a script called "nddlcompile".
 *
 * Generally the parser is invoked to parse a single XML file
 * into a header file and implementation file.  An example of
 * how to do this is shown when you run the "nddlcompile"
 * executable.  It looks similar to this:
 *
 * <PRE>nddlcompile -o filename.cc filename.xml</PRE>
 *
 * In the above example, the program looks for a file called
 * "filename.xml" in the current directory and passes it to
 * the XML loading routines.  After loading and checking the
 * semantics of the XML, it generates the header and then the
 * body, in files called "filename.hh" and "filename.cc",
 * which are both written to the current directory.
 *
 * <A NAME="nddl_support"><H3>NDDL Support Code</H3></A>
 *
 * Aside from the above two tools the NDDL module wraps up
 * some additional NDDL support code.  This is mostly to
 * simplify the work of the compiler by predefining some
 * macros and other utilities.
 *
 * A very important class for use outside of the NDDL module
 * is the StandardAssembly, which provides a facade for the
 * major system components.  Also important are the nddl
 * files that are defined here:
 * @li PlannerConfig.nddl
 * @li Plasma.nddl
 * These need to be included in your model to get the basic
 * NDDL functionality.
 *
 * The C++ code in this module shows a number of examples
 * of how to augment system behavior through inheritance.
 * @li NddlToken from IntervalToken
 * @li NddlResource from Resource
 * @li Factory from ConcreteTokenFactory
 * @li intTypeFactory from IntervalIntTypeFactory
 * @li ...
 *
 *
 * Briefly describe the functionality
 * <A NAME="nddl_entry"><H3>NDDL API</H3></A>
 * Indicate the main entry points to the API
 * <A NAME="nddl_use"><H3>Using the NDDL API</H3></A>
 * Indicate tests that provide examples of usage
 */
