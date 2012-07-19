package nddl;

import java.io.File;
import java.io.Writer;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.util.Set;
import java.util.HashSet;
import net.n3.nanoxml.XMLWriter;
import net.n3.nanoxml.IXMLElement;
import net.n3.nanoxml.XMLElement;
import gnu.getopt.LongOpt;
import gnu.getopt.Getopt;

/**
 * Creation date: (1/26/2004 12:07:04 PM)
 * @author: Andrew Bachmann
 */
class Parse {

	// options
	static final String execName = "java nddl.NddlParser";
	static String debug = null;
	static String output = null;
	static String model = null;

	public Parse() {
		super();
	}
    
	private static String version() {return NddlParser.version();}

	private static void die(String message, boolean usage)
	{
		System.err.println(execName+": "+message);
		if(usage) printUsage();
		System.exit(1);
	}

	private static void printUsage() {
		System.out.println(version());
		System.out.println("Usage: "+execName+" [OPTION]... [FILE]...");
		System.out.println("");
		System.out.println("  -C, --directory <directory>   Change to directory before processing the remaining arguments.");
		System.out.println("  -d, --debug <filename>     Print debug information to filename");
		System.out.println("  -h, --help                 Print this help");
		System.out.println("  -o, --output <filename>    Print output to filename");
		System.out.println("  -q, --quiet                Parse files quietly");
		System.out.println("  -v, --version              Print the version");
		System.out.println("  -W, --warnings <warning>   Print warning messages");
		System.out.println("");
		System.out.println("Examples:");
		System.out.println(execName+" -o filename.xml filename.nddl");
	}
  
	private static void getOptions(String[]args)
	{
		LongOpt[]longopt = new LongOpt[7];
		// we no longer support the --arguments option.
		longopt[0] = new LongOpt("directory", LongOpt.REQUIRED_ARGUMENT, null, 'C');
		longopt[1] = new LongOpt("debug",     LongOpt.REQUIRED_ARGUMENT, null, 'd');
		longopt[2] = new LongOpt("help",      LongOpt.NO_ARGUMENT,       null, 'h');
		longopt[3] = new LongOpt("output",    LongOpt.REQUIRED_ARGUMENT, null, 'o');
		longopt[4] = new LongOpt("quiet",     LongOpt.NO_ARGUMENT,       null, 'q');
		longopt[5] = new LongOpt("version",   LongOpt.NO_ARGUMENT,       null, 'v');
		longopt[6] = new LongOpt("warnings",  LongOpt.REQUIRED_ARGUMENT, null, 'W');
		Getopt parser = new Getopt(execName, args, "d:ho:qvW:", longopt, true);
		int c;
		while((c = parser.getopt()) != -1)
			switch(c)
			{
				case 'C': ModelAccessor.changeDirectory(parser.getOptarg()); break;
				case 'd': debug = parser.getOptarg(); break;
				case 'h': printUsage(); System.exit(0);
				case 'o': output = parser.getOptarg(); break;
				case 'q': DebugMsg.debugMsgEnabled = false; break;
				case 'v': System.out.println(version()); System.exit(0);
				case 'W': NddlParser.warnings.add(parser.getOptarg()); break;
				case '?': die("Error processing arguments",true);
				default:  die("Option '"+(char)c+"' not implemented.",true);
			}
		if(parser.getOptind() != args.length -1)
			die("trailing arguments starting with \""+args[parser.getOptind()]+"\"",true);
		model = args[args.length-1];
		if(output == null)
			output = model+".xml";
	}

	public static void main(String [] args) {
		getOptions(args);
		assert(DebugMsg.debugMsg(version()));
		ModelAccessor.init();

		File modelFile =  ModelAccessor.generateIncludeFileName("",model);
		if(!modelFile.canRead()) die("Cannot read input model file \""+model+"\"",false);
		File outputFile = new File(output);
		if(!outputFile.canWrite()&&outputFile.exists())
			die("Cannot write to output file \""+output+"\"",false);
		try {

			File debugFile = null;
			Writer debugWriter = null;
			if(debug!=null)
			{
				debugFile = new File(debug);
				if(!debugFile.canWrite()&&!debugFile.exists())
					System.err.println("Cannot write to debug output file");
				else debugWriter = new BufferedWriter(new FileWriter(debugFile));
			}

			assert(DebugMsg.debugMsg("  Loading \""+model+"\""));

			IXMLElement xml = new XMLElement("nddl");
			NddlParser parser = NddlParser.parse(modelFile,debugWriter);
			parser.getState().printWarnCount();
			if(parser.getState().printErrorCount()) System.exit(1);
			NddlTreeParser treeParser = new NddlTreeParser(parser.getState());
			treeParser.nddl(parser.getAST(),xml);

			assert(DebugMsg.debugMsg("  Writing \""+output+"\""));

			BufferedWriter writer = new BufferedWriter(new FileWriter(outputFile));
			XMLWriter xmlWriter = new XMLWriter(writer);
			xmlWriter.write(xml,true,2,true);
			writer.close();

			if (debugWriter != null) debugWriter.close();

		} catch (Exception e) {
			e.printStackTrace();
			die("Exception while parsing",false);
		}
		assert(DebugMsg.debugMsg("Exiting "+NddlParser.version()));
	}
}
