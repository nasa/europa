package nddl;

import java.io.File;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.Writer;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.StringWriter;
import java.io.IOException;
import java.io.Reader;
import java.util.Set;
import java.util.Set;
import java.util.HashSet;
import net.n3.nanoxml.StdXMLReader;
import net.n3.nanoxml.XMLParserFactory;
import net.n3.nanoxml.IXMLReader;
import net.n3.nanoxml.XMLWriter;
import net.n3.nanoxml.IXMLParser;
import net.n3.nanoxml.IXMLElement;
import net.n3.nanoxml.XMLElement;
import gnu.getopt.LongOpt;
import gnu.getopt.Getopt;

/**
 * Creation date: (5/11/2006 12:55:04 PM)
 * @author: Matthew E. Boyce
 * A replacement for nddl.Parse and nddl.NddlCompiler
 */
public class Nddl {

  static final String execName = "java nddl.Nddl";

  public static final int MAJOR_VERSION = 1;
  public static final int MINOR_VERSION = 2;

  // options
  static String debug = null;
  static Writer debugWriter = null;
  static boolean noxml = false;
  static boolean asNddlParser = false;
  static boolean asNddlCompiler = false;

  /**
   * Return the version of Nddl.
   * @return A version string.
   */
  public static String versionString() {
    return "Nddl "+ MAJOR_VERSION + "." + MINOR_VERSION;
  }

  public static double version() {
    return Double.parseDouble(MAJOR_VERSION + "." + MINOR_VERSION);
  }

  /**
   * Print a message and exit with a non-zero status.
   * @param message Message to print prior to dying.
   * @param usage print usage message if true.
   */
  private static void die(String message, boolean usage) {
    if(usage)
      printUsage();
    System.err.println(execName+": "+message);
    System.exit(1);
  }

  /**
   * Remove a specific extension from the end of a filename.
   */
  static String stripExt(String filename, String ext) {
    return filename.replaceAll("\\.(?:"+ext+")$","");
  }

  /**
   * Print a terse description of Nddl's available command line arguments.
   */
  private static void printUsage() {
    System.out.println(versionString());
    System.out.println("Usage: "+execName+" [options] file...");
    System.out.println("");
    System.out.println("  --ast <filename>              Print AST to file");
    System.out.println("  --noxml                       Don't output xml initial state");
    System.out.println("  --NddlParser                  Only do parse phase");
    System.out.println("  --NddlCompiler                Only do code generation phase");
    System.out.println("");
    System.out.println("  -C, --directory <directory>   Change to directory before processing the remaining arguments");
    System.out.println("  -d, --debug <message>         Print debug messages");
    System.out.println("  -h, --help                    Print this help");
    System.out.println("  -N, --config <filename>       XML NddlCompiler configuration file");
    System.out.println("  -q, --quiet                   Parse files quietly");
    System.out.println("  -v, --version                 Print the version");
    System.out.println("  -W, --warnings <warning>      Print warning messages");
  }

  /**
   * Parse through the options setting global variables and performing actions as
   * arguments are parsed.
   * @param args The arguments to a call as spilt by the JRE.
   * @return The arguments following all parsed arguments (should consist of files).
   */
  private static String[] getOptions(String[]args) {
    LongOpt[]longopt = new LongOpt[11];
    longopt[0]  = new LongOpt("ast",          LongOpt.REQUIRED_ARGUMENT, null,  1 );
    longopt[1]  = new LongOpt("noxml",        LongOpt.NO_ARGUMENT,       null,  2 );
    longopt[2]  = new LongOpt("NddlParser",   LongOpt.NO_ARGUMENT,       null,  3 );
    longopt[3]  = new LongOpt("NddlCompiler", LongOpt.NO_ARGUMENT,       null,  4 );
    longopt[4]  = new LongOpt("directory",    LongOpt.REQUIRED_ARGUMENT, null, 'C');
    longopt[5]  = new LongOpt("debug",        LongOpt.REQUIRED_ARGUMENT, null, 'd');
    longopt[6]  = new LongOpt("help",         LongOpt.NO_ARGUMENT,       null, 'h');
    longopt[7]  = new LongOpt("config",       LongOpt.REQUIRED_ARGUMENT, null, 'N');
    longopt[8]  = new LongOpt("quiet",        LongOpt.NO_ARGUMENT,       null, 'q');
    longopt[9]  = new LongOpt("version",      LongOpt.NO_ARGUMENT,       null, 'v');
    longopt[10] = new LongOpt("warnings",     LongOpt.REQUIRED_ARGUMENT, null, 'W');
    Getopt parser = new Getopt(execName, args, "C:d:hqvW:", longopt, true);
    int c;
    while((c = parser.getopt()) != -1)
      switch(c) {
        case 1: debug = parser.getOptarg(); break;
        case 2: noxml = true; break;
        case 3: asNddlParser = true; break;
        case 4: asNddlCompiler = true; break;
        case 'C': ModelAccessor.changeDirectory(parser.getOptarg()); break;
        case 'd': DebugMsg.enable(parser.getOptarg()); break;
        case 'h': printUsage(); System.exit(0);
        case 'q': DebugMsg.debugMsgEnabled = false; break;
        case 'N': ModelAccessor.setConfigFile(parser.getOptarg()); break;
        case 'v': System.out.print(versionString()); System.exit(0);
                  // later, when the compiler generates warnings....
        case 'W': NddlParser.warnings.add(parser.getOptarg()); break;
        case '?': die("Error processing arguments",true);
        default:  die("Option '"+(char)c+"' not implemented.",true);
      }
    int remainder = args.length - parser.getOptind();
    if(remainder < 1)
      die("no input files",true);
    String[] toRet = new String[remainder];
    for(int i=0; i<toRet.length; i++)
      toRet[i] = args[parser.getOptind()+i];
    return toRet;
  }

  /**
   * Call to start Nddl.
   * @see usage
   */
  public static void main(String [] args) {
    String[] models = getOptions(args);
    assert(DebugMsg.debugMsg(null,versionString()));
    assert(DebugMsg.debugMsg("DebugMsg:status",DebugMsg.staticToString()));

    if(debug!=null) {
      File debugFile = new File(debug);
      try {
        if(!debugFile.canWrite()&&!debugFile.exists())
          System.err.println("Cannot write to debug output file");
        else
          debugWriter = new BufferedWriter(new FileWriter(debugFile));
      }
      catch(IOException ex) {
        ex.printStackTrace();
        System.err.println(execName+": Error while opening debug output file +\""+debugFile.getAbsolutePath()+"\"");
      }
    }
    ModelAccessor.init();
    for(int i=0;i<models.length;i++)
      nddlModel(models[i]);
    assert(DebugMsg.debugMsg("Exiting "+execName));
  }

  /*
   * Method to be called to bypass main and parse only.
   * I'm sure code can be refactored so this is not a completely separate thing
   */
  public static String nddlToXML(Reader model, boolean exitOnError)
      throws IOException
  {
    ModelAccessor.init();

    IXMLElement xml = new XMLElement("nddl");
    try {
      NddlParser parser = NddlParser.eval(null, model);
      parser.getState().printWarnCount();
      if(parser.getState().printErrorCount()) {
        if(exitOnError) 
          System.exit(1);
        else
          throw new RuntimeException("Errors while parsing Nddl Model.");
      }
      NddlTreeParser treeParser = new NddlTreeParser(parser.getState());
      treeParser.nddl(parser.getAST(), xml);
    }
    catch (Exception e) {
      e.printStackTrace();
      if(exitOnError)
        System.exit(1);
      else
        throw new RuntimeException(e);
    }

    StringWriter result = new StringWriter();
    BufferedWriter writer = new BufferedWriter(result);
    XMLWriter xmlWriter = new XMLWriter(writer);
    xmlWriter.write(xml,true,2,true);
    writer.close();

    return result.toString();
  }

  /**
   * Parse and/or compile a model as defined by global options.
   * @param model A string which can be parsed as as a single filename.
   */
  public static void nddlModel(String model) {
    File modelFile =  ModelAccessor.generateIncludeFileName("",model);
    if(modelFile == null || !modelFile.canRead())
      die("Cannot read input model file \""+model+"\"",false);

    try {
      // Mode 1: As NddlParser (actually Parse)
      if(asNddlParser) {
        if(!model.endsWith(".xml"))
          nddlParse(modelFile);
        else
          System.err.println("Invalid extension for running in NddlParser mode: \""+model+"\"");
      }
      // Mode 2: As NddlCompiler
      else if(asNddlCompiler) {
        IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
        IXMLReader reader = StdXMLReader.fileReader(modelFile.getAbsolutePath());
        parser.setReader(reader);
        nddlCompile((IXMLElement) parser.parse(), modelFile);
      }
      // Mode 3: Normal mode, this automates the whole build process
      else {
        IXMLElement xml = null;
        if(!model.endsWith(".xml")) // if not xml, then parse
          xml = nddlParse(modelFile);
        else { // skip parse phase and instead read XML from file
          IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
          IXMLReader reader = StdXMLReader.fileReader(modelFile.getAbsolutePath());
          parser.setReader(reader);
          xml = (IXMLElement) parser.parse();
        }
        // run the NddlCompiler (actually, the NddlCompiler is completely subsumed by this exec)
        String cc = nddlCompile(xml, modelFile);
      }
    }
    catch (Exception ex) {
      ex.printStackTrace(); System.err.println(execName+": Skipping \""+model+"\" due to exception");
    }
    assert(DebugMsg.debugMsg(versionString()+": Finished with model \""+model+"\""));
  }

  /**
   * Perform only the parse phase on a model.  If noxml is not set, the XML resulting
   * from the parse will be written to disk.
   * @param modelFile A file containing valid NDDL.
   * @return XML AST representation of modelFile
   */
  public static IXMLElement nddlParse(File modelFile) {
    IXMLElement xml = new XMLElement("nddl");
    File outputFile = null;
    if(!noxml) {
      outputFile = new File(stripExt(modelFile.toString(),"nddl|e2") + ".xml");
      if(!outputFile.canWrite()&&outputFile.exists())
        die("Cannot write intermediate xml to file \""+outputFile+"\"",false);
    }
    try {
      assert(DebugMsg.debugMsg(versionString()+": Loading \""+modelFile+"\""));

      NddlParser parser = NddlParser.parse(modelFile,debugWriter);
      parser.getState().printWarnCount();
      if(parser.getState().printErrorCount())
        System.exit(1);
      NddlTreeParser treeParser = new NddlTreeParser(parser.getState());
      treeParser.nddl(parser.getAST(),xml);

      if(!noxml) {
        assert(DebugMsg.debugMsg(versionString()+": Writing \""+outputFile+"\""));

        BufferedWriter writer = new BufferedWriter(new FileWriter(outputFile));
        XMLWriter xmlWriter = new XMLWriter(writer);
        xmlWriter.write(xml,true,2,true);
        writer.close();
      }

      if (debugWriter != null)
        debugWriter.close();

    }
    catch (Exception e) {
      e.printStackTrace();
      die("Exception while parsing",false);
    }
    return xml;
  }

  /**
   * Perform only the compile phase on a model.
   * @param modelFile Used as a filename in generated comments and to create the filenames for generated code.
   * @return the path to the cc file generated.
   */
  public static String nddlCompile(IXMLElement el, File modelFile) throws IOException {
    String name = stripExt(modelFile.toString(),"nddl|xml|e2");
    File sourceFile = new File(name+".cc");
    File headerFile = new File(name+".hh");
    return NddlCompiler.compile(el, modelFile, sourceFile, headerFile);
  }

  // variables used by depricated function execCompiler(source)
  private static final String[]iDirs = {"HSTS/test",
    "NDDL", "NDDL/base", "NDDL/component",
    "RulesEngine", "RulesEngine/base", "RulesEngine/component",
    "Resource", "Resource/base", "Resource/component",
    "TemporalNetwork", "TemporalNetwork/base", "TemporalNetwork/component",
    "PlanDatabase", "PlanDatabase/base", "PlanDatabase/component",
    "ConstraintEngine", "ConstraintEngine/base", "ConstraintEngine/component",
    "Utils", "Utils/base", "Utils/component",
    "TinyXml", "TinyXml/base", "TinyXml/component"};

  private static String plasmaHome = getenv("PLASMA_HOME");
  private static String compiler = "g++";
  private static String compileflags = getenv("CXXFLAGS");
  private static String objFile = null;
  private static boolean nocompile = false;
  /*
   * Replacement for System.getenv to escape a problem with Java versions prior to 1.5
   * Note: This is only used by the following deprecated function
   */
  private static String getenv(String property) {
    String toReturn = null;
    try {
      toReturn = System.getenv(property);
    }
    catch(Throwable t) {
      toReturn = System.getProperty(property);
    }
    return toReturn;
  }
  /**
   * @Deprecated
   *   Tossed in favor of wrapping calls to nddl with either jam, make, or some script
   */
  private static int execCompiler(String source) throws IOException, InterruptedException {
    // if we have no PLASMA_HOME and it appears we are in the PLASMA root directory, use '.'
    if(plasmaHome == null || plasmaHome.equals("")) {
      for(int i=0;i<iDirs.length;i++)
        if(!new File(".",iDirs[i]).isDirectory()) {
          plasmaHome = ".";
          break;
        }
    }

    StringBuffer includes = new StringBuffer(1024);
    if(new File(plasmaHome).isDirectory()) {
      for(int i=0;i<iDirs.length;i++)
        includes.append(" -I").append(plasmaHome).append(File.separator).append(iDirs[i]);
    }

    assert(DebugMsg.debugMsg(compiler+": Compiling \""+source+"\""));
    /*
     * compiler:     command to run to compile c++ files (e.g. "g++")
     * "-c":         tell compiler not to link the output (assumes gcc, TODO: add option to use different flag**)
     * includes:     compiler flags for finding PLASMA header files (assumes gcc's -I, TODO: add option to use different flag**)
     * compileflags: user set flags to send
     * out:          set the output file (assumes gcc, TODO: add option to use different flag**)
     * source:       the C++ source file to compile
     *
     * **TODOs are to be completed when needed for users.
     */
    String out = "";
    if(objFile!=null)
      out = " -o "+objFile;
    Process compile = Runtime.getRuntime().exec(compiler+" -c "+includes+" "+compileflags+out+" "+source);
    StreamMerge.merge(new BufferedReader(new InputStreamReader(compile.getInputStream())),new OutputStreamWriter(System.out));
    StreamMerge.merge(new BufferedReader(new InputStreamReader(compile.getErrorStream())),new OutputStreamWriter(System.err));
    int toRet = compile.waitFor();
    if(toRet == 0) {
      if(objFile != null)
        assert(DebugMsg.debugMsg(compiler+": Output \""+objFile+"\""));
      else
        assert(DebugMsg.debugMsg(compiler+": Output \""+stripExt(new File(source).getName(),"cc")+".o\""));
    }
    return toRet;
  }
}
