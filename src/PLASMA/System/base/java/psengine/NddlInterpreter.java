package psengine;

import java.io.BufferedOutputStream;
import java.io.BufferedWriter;
import java.io.EOFException;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.io.StringReader;
import java.io.StringWriter;

import nddl.ModelAccessor;
import nddl.NddlParser;
import nddl.NddlParserState;
import nddl.NddlTreeParser;

import net.n3.nanoxml.IXMLElement;
import net.n3.nanoxml.XMLElement;
import net.n3.nanoxml.XMLWriter;

public class NddlInterpreter 
{
  NddlParserState persistantState_ = null;
  PrintStream errorStream_ = System.err;
  PSEngine psengine_;

  public NddlInterpreter(PSEngine pse) 
  {
    ModelAccessor.init();
    persistantState_ = new NddlParserState(null);
    psengine_ = pse;
  }
  
  public void setErrorStream(PrintStream s)
  {
      errorStream_ = s;
  }

  private boolean execute(NddlParser parser) 
  {
    IXMLElement xml = new XMLElement("nddl");

    if(parser.getState().getErrorCount() == 0) {
      try {
        NddlTreeParser treeParser = new NddlTreeParser(parser.getState());
        treeParser.nddl(parser.getAST(),xml);
      }
      catch(Exception ex) {
        return false;
      }
    }
    else
      return false;

    try {
      StringWriter string = new StringWriter();
      new XMLWriter(new BufferedWriter(string)).write(xml);
      psengine_.executeScript("nddl-xml",string.toString(), false /*isFile*/);
      persistantState_ = parser.getState();
    }
    catch(IOException ex) {
        return false;
    }
    catch(PSException ex) {
      errorStream_.print(ex.getFile());
      errorStream_.print(":");
      errorStream_.print(ex.getLine());
      errorStream_.print(": error: ");
      errorStream_.println(ex.getMessage());
      return false;
    }
    
    return true;
  }

  public void source(String filename) 
  {
    try {
      File modelFile =  ModelAccessor.generateIncludeFileName("",filename);
      if(modelFile == null)
        modelFile = new File(filename);
      if(modelFile == null) {
        errorStream_.println("error: Failed to find file named \"" + filename + "\"");
        return;
      }
      NddlParser parser = NddlParser.parse(persistantState_, modelFile, null);
      execute(parser);
    }
    catch(Exception ex) {
      errorStream_.print("error: ");
      ex.printStackTrace(errorStream_);
    }
  }

  // returns true if parse didn't encounter a premature end of string.
  public boolean eval(String toEval) 
  {
    try {
      NddlParser parser = NddlParser.eval(persistantState_, new StringReader(toEval));
      execute(parser);
    }
    catch(EOFException ex) {
      return false;
    }
    catch(Exception ex) {
      errorStream_.print("error: ");
      ex.printStackTrace(errorStream_);
    }
    return true;
  }
}
