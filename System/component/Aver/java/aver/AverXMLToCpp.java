package aver;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.Iterator;
import java.util.LinkedList;

import net.n3.nanoxml.*;

public class AverXMLToCpp extends AverHelper {

  public static void main(String [] args) {
    if(args.length == 0)
      System.err.println("Usage: AverXMLToCpp <file>");
    try {
      if(args[0].endsWith(".xml"))
        convertFile(args[0]);
      else
        convertTestFile(args[0]);
    }
    catch(Exception e) {
      e.printStackTrace();
      System.exit(-1);
    }
  }

  public static void convertTestFile(String source) throws AverParseException, AverRuntimeException {
    AverToXML.convertFile(source, source + ".xml");
    convertFile(source + ".xml");
  }

  public static void convertFile(String source) throws AverParseException, AverRuntimeException {
    File sourceFile = new File(source);
    if(!sourceFile.exists())
      throw new AverParseException("Test XML file '" + source + "' doesn't exist.");
    if(!sourceFile.canRead())
      throw new AverParseException("Can't read test XML file '" + source + "'.");
    try {
      IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
      IXMLReader reader = StdXMLReader.fileReader(source);
      parser.setReader(reader);
      (new AverXMLToCpp((IXMLElement) parser.parse(), sourceFile.getName(), 
                            sourceFile.getParent())).convert();
    }
    catch(Exception e) {
      throw new AverParseException(e);
    }
  }

  private CppFile header;
  private CppFile impl;
  private int uniqueName;
  private IXMLElement testTree;
  private String headerName;
  private String implName;
  private String rootTestName;
  private String destPath;

  public AverXMLToCpp(IXMLElement xml, String sourceName, String destPathArg) throws AverRuntimeException {
    checkValidType(xml.getName(), "Test", "Attempted to convert non-test '" + xml.getName() +"'.");
    if(destPathArg == null)
      throw new AverRuntimeException("Destination can't be null.");
    this.destPath = destPathArg;
    testTree = xml;
    header = new CppFile();
    impl = new CppFile();
    uniqueName = 0;
    String nonXmlName = sourceName.substring(0, sourceName.lastIndexOf('.'));
    headerName = nonXmlName + ".hh";
    implName = nonXmlName + ".cc";
    rootTestName = null;
  }

  public void convert() throws AverParseException, AverRuntimeException {
    header.addLine("#ifndef _H_" + headerName.substring(0, headerName.lastIndexOf('.')));
    header.addLine("#define _H_" + headerName.substring(0, headerName.lastIndexOf('.')));
    header.addLine("#include \"EventAggregator.hh\"");
    header.addLine("#include \"AggregateListener.hh\"");    
    header.addLine("#include \"Test.hh\"");
    header.addLine("#include \"TestSet.hh\"");
    header.addLine("#include \"Assertion.hh\"");
    header.addLine("#include \"StringDomain.hh\"");
    header.addLine("namespace EUROPA {");
    header.indent();

    impl.addLine("#include \"" + headerName + "\"");
    impl.addLine("#include <iostream>");
    impl.addLine("namespace EUROPA {");
    impl.indent();

    convertTestSet(testTree);

    AssertionGenerator.toCpp(header, impl);
    TestSetGenerator.toCpp(header);


    impl.unindent();
    impl.addLine("}");

    header.unindent();
    header.addLine("}");

    header.addLine("namespace " + rootTestName + "_TestHidden {");
    header.indent();
    header.addLine("static EUROPA::" + rootTestName + "* test;");
    header.unindent();
    header.addLine("}");
    header.addLine("void averInit(const EUROPA::PlanDatabaseId& db, const EUROPA::DecisionManagerId& dm = EUROPA::DecisionManagerId::noId(),");
    header.indent();
    header.addLine("\tconst EUROPA::ConstraintEngineId& ce = EUROPA::ConstraintEngineId::noId(),");
    header.addLine("\tconst EUROPA::RulesEngineId& re = EUROPA::RulesEngineId::noId()) {");
    header.addLine("EUROPA::EventAggregator::instance(dm, ce, db, re);");
    header.addLine(rootTestName + "_TestHidden::test = new EUROPA::" + rootTestName + "(db);");
    header.unindent();
    header.addLine("}");
    header.addLine("void averDeinit() {");
    header.indent();
    header.addLine(rootTestName + "_TestHidden::test->dumpResults();");
    header.addLine("delete " + rootTestName + "_TestHidden::test;");
    header.addLine("EUROPA::EventAggregator::remove();");
    header.unindent();
    header.addLine("}");

    header.addLine("#endif");

    try {
      System.err.println("Writing header '" + (new File(destPath + System.getProperty("file.separator") + headerName)).getAbsolutePath() + "'");
      header.write(new FileOutputStream(new File(destPath + System.getProperty("file.separator") + headerName)));
      System.err.println("Writing implementation '" + (new File(destPath + System.getProperty("file.separator") + implName)).getAbsolutePath() + "'");
      impl.write(new FileOutputStream(new File(destPath + System.getProperty("file.separator") + implName)));
    }
    catch(IOException ioe){ioe.printStackTrace(); System.exit(-1);}
  }

  private void convertTestSet(IXMLElement xml) throws AverParseException, AverRuntimeException {
    checkValidType(xml.getName(), "Test", "Attempted to convert non-test'" + xml.getName() + "'.");
    String name = xml.getAttribute("name", null);
    if(name == null)
      throw new AverParseException("Test with no name");
    name = TestSetGenerator.addSet(xml.getAttribute("name", null));
    if(rootTestName == null)
      rootTestName = name;
    for(Iterator it = xml.getChildren().iterator(); it.hasNext();) {
      IXMLElement elem = (IXMLElement) it.next();
      if(elem.getName().equals("Test")) {
        TestSetGenerator.addSet(name, elem.getAttribute("name", null));
        convertTestSet(elem);
      }
      else if(elem.getName().equals("At"))
        TestSetGenerator.addAssertion(name, AssertionGenerator.addAssertion(name, elem));
      else
        throw new AverParseException("Invalid child of test: " + elem.getName());
    }
  }
}
