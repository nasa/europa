package testLang;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.util.Iterator;
import java.util.LinkedList;

import net.n3.nanoxml.*;

public class TestLangXMLToCpp extends TestLangHelper {

  public static void main(String [] args) {
    if(args.length == 0)
      System.err.println("Usage: TestLangXMLToCpp <file>");
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

  public static void convertTestFile(String source) throws TestLangParseException, TestLangRuntimeException {
    TestLangToXML.convertFile(source, source + ".xml");
    convertFile(source + ".xml");
  }

  public static void convertFile(String source) throws TestLangParseException, TestLangRuntimeException {
    File sourceFile = new File(source);
    if(!sourceFile.exists())
      throw new TestLangParseException("Test XML file '" + source + "' doesn't exist.");
    if(!sourceFile.canRead())
      throw new TestLangParseException("Can't read test XML file '" + source + "'.");
    try {
      IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
      IXMLReader reader = StdXMLReader.fileReader(source);
      parser.setReader(reader);
      (new TestLangXMLToCpp((IXMLElement) parser.parse(), sourceFile.getName(), 
                            sourceFile.getParent())).convert();
    }
    catch(Exception e) {
      throw new TestLangParseException(e);
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

  public TestLangXMLToCpp(IXMLElement xml, String sourceName, String destPath) throws TestLangRuntimeException {
    checkValidType(xml.getName(), "Test", "Attempted to convert non-test '" + xml.getName() +"'.");
    this.destPath = destPath;
    testTree = xml;
    header = new CppFile();
    impl = new CppFile();
    uniqueName = 0;
    String nonXmlName = sourceName.substring(0, sourceName.lastIndexOf('.'));
    headerName = "Test_" + nonXmlName + ".hh";
    implName = "Test_" + nonXmlName + ".cc";
    rootTestName = null;
  }

  public void convert() throws TestLangParseException, TestLangRuntimeException {
    header.addLine("#ifndef _H_" + headerName.substring(0, headerName.lastIndexOf('.')));
    header.addLine("#define _H_" + headerName.substring(0, headerName.lastIndexOf('.')));
    header.addLine("#include \"EventAggregator.hh\"");
    header.addLine("#include \"AggregateListener.hh\"");    
    header.addLine("#include \"Test.hh\"");
    header.addLine("#include \"TestSet.hh\"");
    header.addLine("#include \"Assertion.hh\"");
    header.addLine("#include \"StringDomain.hh\"");
    header.addLine("namespace Prototype {");
    header.indent();

    impl.addLine("#include \"" + headerName + "\"");
    impl.addLine("#include <iostream>");
    impl.addLine("namespace Prototype {");
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
    header.addLine("static Prototype::" + rootTestName + "* test;");
    header.unindent();
    header.addLine("}");
    header.addLine("void testLangInit(const Prototype::PlanDatabaseId& db, const Prototype::DecisionManagerId& dm = Prototype::DecisionManagerId::noId(),");
    header.indent();
    header.addLine("\tconst Prototype::ConstraintEngineId& ce = Prototype::ConstraintEngineId::noId(),");
    header.addLine("\tconst Prototype::RulesEngineId& re = Prototype::RulesEngineId::noId()) {");
    header.addLine("Prototype::EventAggregator::instance(dm, ce, db, re);");
    header.addLine(rootTestName + "_TestHidden::test = new Prototype::" + rootTestName + "(db);");
    header.unindent();
    header.addLine("}");
    header.addLine("void testLangDeinit() {");
    header.indent();
    header.addLine(rootTestName + "_TestHidden::test->dumpResults();");
    header.addLine("delete " + rootTestName + "_TestHidden::test;");
    header.addLine("Prototype::EventAggregator::remove();");
    header.unindent();
    header.addLine("}");

    header.addLine("#endif");

    try {
      System.err.println("Writing header '" + (new File(destPath + System.getProperty("file.separator") + headerName)).getAbsolutePath() + "'");
      header.write(new FileOutputStream(new File(destPath + System.getProperty("file.separator") + headerName)));
      System.err.println("Writing header '" + (new File(destPath + System.getProperty("file.separator") + implName)).getAbsolutePath() + "'");
      impl.write(new FileOutputStream(new File(destPath + System.getProperty("file.separator") + implName)));
    }
    catch(IOException ioe){ioe.printStackTrace(); System.exit(-1);}
  }

  private void convertTestSet(IXMLElement xml) throws TestLangParseException, TestLangRuntimeException {
    checkValidType(xml.getName(), "Test", "Attempted to convert non-test'" + xml.getName() + "'.");
    String name = xml.getAttribute("name", null);
    if(name == null)
      throw new TestLangParseException("Test with no name");
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
        throw new TestLangParseException("Invalid child of test: " + elem.getName());
    }
  }
}
