/*
 * NddlCompiler.java
 */
package nddl;

import java.io.File;
import java.io.Writer;
import java.io.OutputStreamWriter;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import net.n3.nanoxml.StdXMLReader;
import net.n3.nanoxml.XMLParserFactory;
import net.n3.nanoxml.IXMLReader;
import net.n3.nanoxml.XMLWriter;
import net.n3.nanoxml.IXMLParser;
import net.n3.nanoxml.IXMLElement;
import gnu.getopt.LongOpt;
import gnu.getopt.Getopt;

class NddlCompiler {

  public static final String execName = "java nddl.NddlCompiler";

  static String output = null;

  // returns the path to the cc file generated.
  public static String compile(IXMLElement el, File modelFile, File sourceFile, File headerFile) throws IOException {
    if(!headerFile.canWrite()&&headerFile.exists())
      throw new IOException("Cannot generate headerfile to compile \""+headerFile.getAbsolutePath()+"\"");

    ModelAccessor.setModelName(modelFile.getName().replaceAll("\\.[^.]+$",""));

    assert(DebugMsg.debugMsg(Nddl.versionString() + ": Generating \""+headerFile+"\""));	
    IndentWriter header = new IndentWriter(new BufferedWriter(new FileWriter(headerFile)));
    header.write("// "+modelFile.getName()+"\n\n");
    // write class and function prototypes
    new HeaderGenerator(header).generate(el);
    header.flush();
    header.close();

    if(!sourceFile.canWrite()&&sourceFile.exists())
      throw new IOException("Cannot generate sourcefile to compile \""+sourceFile.getAbsolutePath()+"\"");
    assert(DebugMsg.debugMsg(Nddl.versionString() + ": Generating \""+sourceFile+"\""));	

    IndentWriter implementation = new IndentWriter(new BufferedWriter(new FileWriter(sourceFile)));
    implementation.write("// "+modelFile.getName()+"\n\n");
    implementation.write("#include \""+headerFile.getName()+"\"\n");
    new ImplementationGenerator(implementation).generate(el);

    assert(DebugMsg.debugMsg(Nddl.versionString() + ": Writing Schema"));
    implementation.write("\n\n");
    SchemaWriter.generate(implementation);
    implementation.flush();
    implementation.close();

    return sourceFile.getAbsolutePath();
  }

  private static IXMLElement readXml(String filename) throws Exception {
    IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
    IXMLReader reader = StdXMLReader.fileReader(filename);
    parser.setReader(reader);
    return (IXMLElement) parser.parse();
  }

  private static void die(String message, boolean usage)
  {
    System.err.println(execName+": "+message);
    if(usage) printUsage();
    System.exit(1);
  }

  private static void printUsage() {
    System.out.println(Nddl.versionString());
    System.out.println("Usage: "+execName+" [OPTION]... [FILE]...");
    System.out.println("");
    System.out.println("  -C, --directory <directory>   Change to directory before processing the remaining arguments.");
    System.out.println("  -h, --help                    Print this help");
    System.out.println("  -N, --config <filename>       XML configuration file (defaults to NDDL.cfg)");
    System.out.println("  -o, --output <filename>       Print output to filename");
    System.out.println("  -q, --quiet                   Compile files quietly");
    System.out.println("  -v, --version                 Print the version");
    //System.out.println("  -W, --warnings <warning>   Print warning messages");
    System.out.println("");
    System.out.println("Examples:");
    System.out.println(execName+" -o filename.cc filename.xml");
  }

  private static String[] getOptions(String[]args)
  { 
    LongOpt[]longopt = new LongOpt[7];
    // we no longer support the --arguments option.
    longopt[0] = new LongOpt("directory",    LongOpt.REQUIRED_ARGUMENT, null, 'C');
    longopt[1] = new LongOpt("help",         LongOpt.NO_ARGUMENT,       null, 'h');
    longopt[2] = new LongOpt("config",       LongOpt.REQUIRED_ARGUMENT, null, 'N');
    longopt[3] = new LongOpt("output",       LongOpt.REQUIRED_ARGUMENT, null, 'o');
    longopt[4] = new LongOpt("quiet",        LongOpt.NO_ARGUMENT,       null, 'q');
    longopt[5] = new LongOpt("version",      LongOpt.NO_ARGUMENT,       null, 'v');
    longopt[6] = new LongOpt("warnings",     LongOpt.REQUIRED_ARGUMENT, null, 'W');
    Getopt parser = new Getopt(execName, args, "ho:qvW:", longopt, true);
    int c;
    while((c = parser.getopt()) != -1) 
      switch(c)
      {
        case 'C': ModelAccessor.changeDirectory(parser.getOptarg()); break;
        case 'h': printUsage(); System.exit(0);
				case 'N': ModelAccessor.setConfigFile(parser.getOptarg()); break;
        case 'o': output = parser.getOptarg(); break;
        case 'q': DebugMsg.debugMsgEnabled = false; break;
        case 'v': System.out.println(Nddl.versionString()); System.exit(0);
                  // later, when the compiler generates warnings....
        case 'W': System.err.println("Compiler doesn't yet support warnings!"); break;
        case '?': die("Error processing arguments",true);
        default:  die("Option '"+(char)c+"' not implemented.",true);
      }     
    int remainder = args.length - parser.getOptind();
    if(remainder < 1) die("no input files",true);
    if(remainder > 1 && output != null) die("specified output file with more than 1 input file",true);
    String[] toRet = new String[remainder];
    for(int i=0; i<toRet.length; i++)
      toRet[i] = args[parser.getOptind()+i];
    return toRet;
  }

  public static void main(String [] args) throws Exception {
    String[] models = getOptions(args);
    assert(DebugMsg.debugMsg(Nddl.versionString()));
    ModelAccessor.init();

    if(output != null && models.length == 1) {
      File sourceFile = new File(output);
      File headerFile = new File(output.replaceAll("\\.[^.]+$",".hh"));
      File modelFile = new File(models[0]);
      compile(readXml(models[0]),modelFile,sourceFile,headerFile);
    }
    else
    {
      for(int i=0;i<models.length;i++)
      {
        String name = models[i].replaceAll("\\.(?:xml)$","");
        File sourceFile = new File(name + ".cc");
        File headerFile = new File(name + ".hh");
        File modelFile = new File(models[i]);
        compile(readXml(models[i]),modelFile,sourceFile,headerFile);
      }
    }
  }
}
