package aver;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.LineNumberReader;
import java.lang.reflect.Method;

import antlr.CommonAST;
import antlr.RecognitionException;
import antlr.TokenStreamException;
import antlr.collections.AST;
import antlr.debug.misc.ASTFrame;

import net.n3.nanoxml.*;

public abstract class AverToXML extends AverHelper implements AverTokenTypes {
  private static final boolean tracePrint = false;
  private static final boolean viewTree = false;
  private static String fileName = "";
  private static LineNumberReader lineIn = null;

  public static void main(String [] args) throws AverParseException, AverRuntimeException {
    if(args.length != 2) {
      System.err.println("Usage: AverToXML <source> <dest>");
      System.exit(-1);
    }
    convertFile(args[0], args[1]);
  }

  public static IXMLElement convert(InputStream in) throws AverParseException, AverRuntimeException {
    try {
      AverParser parser = new AverParser(new AverLexer(in));
      parser.test_set();
      if(viewTree)
        (new ASTFrame("Foo", parser.getAST())).setVisible(true);
      IXMLElement tests = astToXml((CommonAST)parser.getAST());
      return tests;
    }
    catch(RecognitionException re) {
      throw new AverParseException(re);
    }
    catch(TokenStreamException tse) {
      throw new AverParseException(tse);
    }
  }
  public static void convertFile(String source, String dest) throws AverParseException, AverRuntimeException {
    File sourceFile = new File(source);
    if(!sourceFile.exists())
      throw new AverParseException("Test file '" + source + "' doesn't exist.");
    if(!sourceFile.canRead())
      throw new AverParseException("Can't read test file '" + source + "'.");

    fileName = sourceFile.getName();
    
    File destFile = new File(dest);
    try{destFile.createNewFile();}
    catch(Exception e){throw new AverParseException(e);}
    if(!destFile.canWrite())
      throw new AverParseException("Can't read test file '" + source + "'.");

    try {
      lineIn = new LineNumberReader(new FileReader(source));
      lineIn.mark(1);
      (new XMLWriter(new FileWriter(destFile))).write(convert(new FileInputStream(sourceFile)), true);
    }
    catch(FileNotFoundException fnfe) {
      throw new AverParseException(fnfe);
    }
    catch(IOException ioe) {
      throw new AverParseException(ioe);
    }
    if(lineIn == null)
      throw new AverParseException("Failed to open source file for reading.");
  }
  protected static IXMLElement astToXml(CommonAST ast) throws AverParseException, AverRuntimeException {
    return test(ast);
  }


  private static final String [] methodNames;
  private static final Class [] methodArgTypes = {CommonAST.class};
  private static final Class [] twoArgMethodArgTypes = {CommonAST.class, Object.class};
  
  static {
    methodNames = new String [AverTokenTypes.DUMMY + 1];
    methodNames[OBRACE] = "enumeratedDomain";
    methodNames[OBRACKET] = "intervalDomain";
    methodNames[NUMBER] = "evalNumber";
    methodNames[IN] = "boolIn";
    methodNames[OUT] = "boolOut";
    methodNames[EQ] = "boolEq";
    methodNames[NE] = "boolNe";
    methodNames[TOKENS] = "tokens";
    methodNames[STRING] = "evalString";
    methodNames[OBJECTS] = "objects";
    methodNames[TRANSACTIONS] = "transactions";
    methodNames[ENTITY] = "entity";
    methodNames[COUNT] = "count";
    methodNames[LT] = "boolLt";
    methodNames[GT] = "boolGt";
    methodNames[GE] = "boolGe";
    methodNames[LE] = "boolLe";
    methodNames[TEST] = "test";
    methodNames[INTERSECTS] = "boolIntersects";
    methodNames[AT] = "assertion";
    methodNames[STEP] = "step";
    methodNames[PROPERTY] = "property";
  }


  private static IXMLElement evaluate(CommonAST ast, Object arg) throws AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evalute a null element.");
    checkValidType(ast.getType(), "Attempted to evaluate invalid type " + ast.getType());
    try {
      Method evalMethod = AverToXML.class.getDeclaredMethod(methodNames[ast.getType()], twoArgMethodArgTypes);
      Object [] args = {ast, arg};
      long t1 = System.currentTimeMillis();
      IXMLElement retval = (IXMLElement) evalMethod.invoke(null, args);
      //System.err.println(evalMethod.getName() + ": " + (System.currentTimeMillis() - t1) +
      //                   "ms");
      return retval;
      //return (IXMLElement) evalMethod.invoke(null, args);
    }
    catch(Exception e) {
      throw new AverRuntimeException(e);
    }
  }

  private static IXMLElement evaluate(CommonAST ast) throws AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evalute a null element.");
    checkValidType(ast.getType(), "Attempted to evaluate invalid type " + ast.getType() +
                   "\nAt ", ast);
    try {
      Method evalMethod = AverToXML.class.getDeclaredMethod(methodNames[ast.getType()], methodArgTypes);
      Object [] args = {ast};
      long t1 = System.currentTimeMillis();
      IXMLElement retval = (IXMLElement) evalMethod.invoke(null, args);
      //System.err.println(evalMethod.getName() + ": " + (System.currentTimeMillis() - t1) +
      //                   "ms");
      return retval;
      //return (IXMLElement) evalMethod.invoke(null, args);
    }
    catch(Exception e) {
      throw new AverRuntimeException(e);
    }
  }

  private static IXMLElement test(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Test' element.");
    checkValidType(ast.getType(), TEST, "Attempted to create test element for type " + ast.getType());
    
    IXMLElement retval = new XMLElement("Test");
    CommonAST child = (CommonAST) ast.getFirstChild();
    String testName = child.getText();
    testName = testName.substring(1, testName.length()-1);
    retval.setAttribute("name", testName);
    while((child = (CommonAST) child.getNextSibling()) != null) {
      if(child.getType() == TEST)
        test(child, retval);
      else if(child.getType() == AT)
        assertion(child, retval);
      else
        throw new AverParseException("Attempted to create non-assertion and non-test.");
      //retval.addChild(evaluate(child));
    }
    return retval;
  }

  private static IXMLElement test(CommonAST ast, IXMLElement parent) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Test' element.");
    checkValidType(ast.getType(), TEST, "Attempted to create test element for type " + ast.getType());
    
    IXMLElement retval = new XMLElement("Test");
    CommonAST child = (CommonAST) ast.getFirstChild();
    String testName = child.getText();
    testName = testName.substring(1, testName.length()-1);
    retval.setAttribute("name", testName);
    while((child = (CommonAST) child.getNextSibling()) != null) {
      if(child.getType() == TEST)
        test(child, retval);
      else if(child.getType() == AT)
        assertion(child, retval);
      else
        throw new AverParseException("Attempted to create non-assertion and non-test.");
      //retval.addChild(evaluate(child));
    }
    parent.addChild(retval);
    return retval;
  }

  private static IXMLElement enumeratedDomain(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null enumerated domain.");
    checkValidType(ast.getType(), OBRACE, "Attempted to evaluate an enumerated domain with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("EnumeratedDomain");
    CommonAST child = (CommonAST) ast.getFirstChild();
    while(child != null) {
      retval.addChild(evaluate(child, Boolean.TRUE));
      child = (CommonAST) child.getNextSibling();
    }
    return retval;
  }

  private static IXMLElement intervalDomain(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null interval domain.");
    checkValidType(ast.getType(), OBRACKET, "Attempted to evaluate an interval domain with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("IntervalDomain");
    IXMLElement lb = new XMLElement("LowerBound");
    IXMLElement ub = new XMLElement("UpperBound");
    retval.addChild(lb);
    retval.addChild(ub);
    CommonAST child = (CommonAST) ast.getFirstChild();
    lb.addChild(evaluate(child, Boolean.TRUE));
    child = (CommonAST) child.getNextSibling();
    ub.addChild(evaluate(child, Boolean.TRUE));
    return retval;
  }

  private static IXMLElement [] getBinaryChildren(CommonAST ast) throws AverParseException, AverRuntimeException {
    IXMLElement [] retval = new XMLElement [2];
    CommonAST child1 = (CommonAST) ast.getFirstChild();
    CommonAST child2 = (CommonAST) child1.getNextSibling();
    retval[0] = evaluate(child1);
    retval[1] = evaluate(child2);
    return retval;
  }

  //change this to void if other assertion variant goes away
  private static IXMLElement assertion(CommonAST ast, IXMLElement parent) throws AverParseException, AverRuntimeException {
    CommonAST step = (CommonAST) ast.getFirstChild();
    IXMLElement stepElem = evaluate(step);
    String lineText = getSourceLine(((LineNumberAST)step).getLine());
    String stepLine = lineText.split(":")[0].trim();
    CommonAST bool = step;
    while((bool = (CommonAST) bool.getNextSibling()) != null) {
      IXMLElement assnElem = new XMLElement("At");
      assnElem.setAttribute("cond", stepLine);
      assnElem.setAttribute("file", fileName);
      int lineNo = ((LineNumberAST)bool).getLine();
      assnElem.setAttribute("lineNo", Integer.toString(lineNo));
      //String lineText = getSourceLine(lineNo);
      assnElem.setAttribute("lineText", lineText.trim());
      assnElem.addChild(stepElem);
      assnElem.addChild(evaluate(bool));
      parent.addChild(assnElem);
    }
    return null;
  }
    
  private static IXMLElement assertion(CommonAST ast) throws AverParseException, AverRuntimeException {
    IXMLElement retval = new XMLElement("At");
    retval.setAttribute("file", fileName);
    int lineNo = ((LineNumberAST)ast).getLine();
    retval.setAttribute("lineNo", Integer.toString(lineNo));
    String lineText = "";
    lineText = getSourceLine(lineNo);
    retval.setAttribute("lineText", lineText.trim());
    CommonAST step = (CommonAST) ast.getFirstChild();
    retval.addChild(evaluate(step));
    CommonAST bool = (CommonAST) step.getNextSibling();
    retval.addChild(evaluate(bool));
    return retval;
  }

  private static String getSourceLine(final int lineNo) throws AverParseException {
    String retval = "";
    //System.err.println("Getting source line " + fileName + ", line " + lineNo);
    try {
      if(lineIn != null) {
        if(lineIn.getLineNumber() > lineNo) {
          //System.err.println("Past line " + lineNo + ", resetting...");
          lineIn.reset();
          lineIn.mark(1);
        }
        //System.err.println("Starting at line " + lineIn.getLineNumber());
        for(int i = lineIn.getLineNumber(); i < lineNo-1; i++) {
          lineIn.readLine();
          //System.err.println("Discarding line " + i + ": '" + lineIn.readLine() + "'");
        }
        retval = lineIn.readLine();
      }
    }
    catch(IOException ioe) {
      throw new AverParseException(ioe);
    }
    if(retval.trim().equals(""))
      throw new AverParseException("Got empty source line at " + fileName + ", line " + lineNo);
    //System.err.println("Returning line " + lineIn.getLineNumber() + ": '" + retval + "'");
    return retval;
  }

  private static IXMLElement step(CommonAST ast) throws AverParseException, AverRuntimeException {
    IXMLElement retval = new XMLElement("step");
    String qualifier = "each";
    String operator = ">";
    IXMLElement domain = new XMLElement("EnumeratedDomain");
    IXMLElement value = new XMLElement("Value");
    domain.addChild(value);
    value.setAttribute("type", "number");
    value.setContent("-1");

    CommonAST child = (CommonAST) ast.getFirstChild();
    while(child != null) {
      switch(child.getType()) {
      case FIRST:
        qualifier = "first";
        break;
      case LAST:
        qualifier = "last";
        break;
      case ANY:
        qualifier = "any";
        break;
      case EACH:
        qualifier = "each";
        break;
      case ALL:
        qualifier = "all";
        break;
      case EQ:
        operator = "=";
        break;
      case LT:
        operator = "<";
        break;
      case GT:
        operator = ">";
        break;
      case GE:
        operator = ">=";
        break;
      case LE:
        operator = "<=";
        break;
      case NE:
        operator = "!=";
        break;
      case IN:
        operator = "in";
        break;
      case OUT:
        operator = "out";
        break;
      default:
        domain = evaluate(child);
        break;
      }
      child = (CommonAST) child.getNextSibling();
    }
    retval.setAttribute("qualifier", qualifier);
    retval.setAttribute("operator", operator);
    retval.addChild(domain);
    return retval;
  }

  private static IXMLElement boolFunc(CommonAST ast, String name) throws AverParseException, AverRuntimeException {
    IXMLElement retval = new XMLElement(name);
    retval.setAttribute("file", fileName);
    int lineNo = ((LineNumberAST)ast).getLine();
    retval.setAttribute("lineNo", Integer.toString(lineNo));
    String lineText = "";
    try {
      if(lineIn != null) {
        if(lineIn.getLineNumber() > lineNo)
          lineIn.reset();
        for(int i = lineIn.getLineNumber(); i < lineNo; i++)
          lineText = lineIn.readLine();
      }
    }
    catch(IOException ioe) {
      throw new AverParseException(ioe);
    }
    retval.setAttribute("lineText", lineText);
    IXMLElement [] children = getBinaryChildren(ast);
    retval.addChild(children[0]);
    retval.addChild(children[1]);
    return retval;
  }

  private static IXMLElement boolIn(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'in' element.");
    checkValidType(ast.getType(), IN, "Attempted to evaluate an 'in' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "in");
  }

  private static IXMLElement boolOut(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'out' element.");
    checkValidType(ast.getType(), OUT, "Attempted to evaluate an 'out' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "out");
  }

  private static IXMLElement boolEq(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null '==' element.");
    checkValidType(ast.getType(), EQ, "Attempted to evaluate an '==' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "eq");
  }

  private static IXMLElement boolLt(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null '<' element.");
    checkValidType(ast.getType(), LT, "Attempted to evaluate an '<' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "lt");
  }

  private static IXMLElement boolGt(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null '>' element.");
    checkValidType(ast.getType(), GT, "Attempted to evaluate an '>' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "gt");
  }

  private static IXMLElement boolGe(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null '>=' element.");
    checkValidType(ast.getType(), GE, "Attempted to evaluate an '>=' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "ge");
  }

  private static IXMLElement boolLe(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null '<=' element.");
    checkValidType(ast.getType(), LE, "Attempted to evaluate an '<=' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "le");
  }

  private static IXMLElement boolNe(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null '!=' element.");
    checkValidType(ast.getType(), NE, "Attempted to evaluate an '!=' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "ne");
  }

  private static IXMLElement boolIntersects(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'intersects' element.");
    checkValidType(ast.getType(), INTERSECTS, "Attempted to evaluate an 'intersects' element with invalid type " +
                   ast.getType());
    return boolFunc(ast, "intersects");
  }

  private static IXMLElement evalString(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evalute null 'string' element.");
    checkValidType(ast.getType(), STRING, "Attempted to evaluate a 'string' element with invalid type " +
                   ast.getType());
    
    IXMLElement retval = new XMLElement("EnumeratedDomain");
    IXMLElement value = new XMLElement("Value");
    value.setAttribute("type", "string");
    retval.addChild(value);
    IXMLElement cont = value.createPCDataElement();
    value.addChild(cont);
    cont.setContent(ast.getText().substring(1, ast.getText().length() - 1));
    return retval;
  }
  
  private static IXMLElement evalString(CommonAST ast, Object arg) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evalute null 'string' element.");
    checkValidType(ast.getType(), STRING, "Attempted to evaluate a 'string' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Value");
    retval.setAttribute("type", "string");
    IXMLElement cont = retval.createPCDataElement();
    retval.addChild(cont);
    cont.setContent(ast.getText().substring(1, ast.getText().length() - 1));
    return retval;
  }
  private static IXMLElement evalNumber(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evalute null 'number' element.");
    checkValidType(ast.getType(), NUMBER, "Attempted to evaluate a 'number' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("EnumeratedDomain");
    IXMLElement value = new XMLElement("Value");
    value.setAttribute("type", "number");
    retval.addChild(value);
    IXMLElement cont = value.createPCDataElement();
    value.addChild(cont);
    cont.setContent(ast.getText());
    return retval;
  }

  private static IXMLElement evalNumber(CommonAST ast, Object arg) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evalute null 'number' element.");
    checkValidType(ast.getType(), NUMBER, "Attempted to evaluate a 'number' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Value");
    retval.setAttribute("type", "number");
    IXMLElement cont = retval.createPCDataElement();
    retval.addChild(cont);
    cont.setContent(ast.getText());
    return retval;
  }

  private static IXMLElement count(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Count' element.");
    checkValidType(ast.getType(), COUNT, "Attempted to evaluate a 'Count' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Count");
    retval.addChild(evaluate((CommonAST) ast.getFirstChild()));
    return retval;
  }

  private static IXMLElement count(CommonAST ast, Object arg) throws AverParseException, AverRuntimeException {
    return count(ast);
  }

  private static IXMLElement entity(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Entity' element.");
    checkValidType(ast.getType(), ENTITY, "Attempted to evaluate a 'Entity' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Entity");
    IXMLElement index = new XMLElement("Index");
    retval.addChild(index);
    CommonAST child = (CommonAST) ast.getFirstChild();
    index.addChild(evaluate(child));
    IXMLElement domain = new XMLElement("Domain");
    retval.addChild(domain);
    child = (CommonAST) child.getNextSibling();
    domain.addChild(evaluate(child));
    return retval;
  }
  
  private static IXMLElement entity(CommonAST ast, Object arg) throws AverParseException, AverRuntimeException {
    return entity(ast);
  }
  private static IXMLElement objects(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Objects' element.");
    checkValidType(ast.getType(), OBJECTS, "Attempted to evaluate a 'Objects' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Objects");
    CommonAST child = (CommonAST) ast.getFirstChild();
    boolean foundStep = false;
    boolean foundName = false;
    OBJECT_CHILD_LOOP: while(child != null) {
      IXMLElement childElem = null;
      switch(child.getType()) {
      case STEP:
        if(foundStep)
          throw new AverParseException("The 'Objects' function can have only one 'step' predicate.");
        foundStep = true;
        childElem = new XMLElement("step");
        CommonAST sub = (CommonAST) child.getFirstChild();
        CommonAST at = (CommonAST) sub.getNextSibling();
        if(sub.getType() == EQ && (at.getType() == FIRST || at.getType() == LAST || at.getType() == ANY ||
                                   at.getType() == EACH || at.getType() == ALL)) {
          retval.addChild(childElem);
          childElem.setAttribute("operator", sub.getText());
          childElem.setAttribute("at", at.getText());
          child = (CommonAST) child.getNextSibling();
          continue OBJECT_CHILD_LOOP;
        }
        break;
      case NAME:
        if(foundName)
          throw new AverParseException("The 'Objects' function can have only one 'name' predicate.");
        foundName = true;
        childElem = new XMLElement("name");
        break;
      case VARIABLE:
        childElem = new XMLElement("variable");
        retval.addChild(childElem);
        CommonAST varName = (CommonAST) child.getFirstChild();
        CommonAST varVal = (CommonAST) varName.getNextSibling();

        IXMLElement nameElem = new XMLElement("name");
        childElem.addChild(nameElem);
        CommonAST subName = (CommonAST) varName.getFirstChild();
        nameElem.setAttribute("operator", subName.getText());
        subName = (CommonAST) subName.getNextSibling();
        nameElem.addChild(evaluate(subName));
        
        IXMLElement valueElem = new XMLElement("value");
        childElem.addChild(valueElem);
        CommonAST subValue = (CommonAST) varVal.getFirstChild();
        valueElem.setAttribute("operator", subValue.getText());
        subValue = (CommonAST) subValue.getNextSibling();
        valueElem.addChild(evaluate(subValue));
        child = (CommonAST) child.getNextSibling();
        continue OBJECT_CHILD_LOOP;
      default:
        throw new AverRuntimeException("Attempted to evaluate 'Objects' function with invalid argument type " +
                                           child.getType() + " at " + ast.toStringList());
      }
      retval.addChild(childElem);
      CommonAST sub = (CommonAST) child.getFirstChild();
      childElem.setAttribute("operator", sub.getText());
      sub = (CommonAST) sub.getNextSibling();
      childElem.addChild(evaluate(sub));
      child = (CommonAST) child.getNextSibling();
    }
    return retval;
  }

  private static IXMLElement property(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Property' element.");
    checkValidType(ast.getType(), PROPERTY, "Attempted to evaluate a 'Property' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Property");
    CommonAST child = (CommonAST) ast.getFirstChild();
    retval.setAttribute("index", child.getText());
    child = (CommonAST) child.getNextSibling();
    retval.addChild(evaluate(child));
    return retval;
  }

  private static IXMLElement tokens(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Tokens' element.");
    checkValidType(ast.getType(), TOKENS, "Attempted to evaluate a 'Tokens' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Tokens");
    CommonAST child = (CommonAST) ast.getFirstChild();
    boolean foundStep = false;
    boolean foundStart = false;
    boolean foundEnd = false;
    boolean foundStatus = false;
    boolean foundPredicate = false;
    boolean foundPath = false;
    String multipleError = "The 'Tokens' function can have only one ";
    TOKEN_CHILD_LOOP: while(child != null) {
      IXMLElement childElem = null;
      switch(child.getType()) {
      case STEP:
        if(foundStep)
          throw new AverParseException(multipleError + "'step' predicate.");
        foundStep = true;
        childElem = new XMLElement("step");
        CommonAST sub = (CommonAST) child.getFirstChild();
        CommonAST at = (CommonAST) sub.getNextSibling();
        if(sub.getType() == EQ && (at.getType() == FIRST || at.getType() == LAST || at.getType() == ANY ||
                                   at.getType() == EACH || at.getType() == ALL)) {
          retval.addChild(childElem);
          childElem.setAttribute("operator", sub.getText());
          childElem.setAttribute("at", at.getText());
          child = (CommonAST) child.getNextSibling();
          continue TOKEN_CHILD_LOOP;
        }
        break;
      case START:
        if(foundStart)
          throw new AverParseException(multipleError + "'start' predicate.");
        foundStart = true;
        childElem = new XMLElement("start");
        break;
      case END:
        if(foundEnd)
          throw new AverParseException(multipleError + "'end' predicate.");
        foundEnd = true;
        childElem = new XMLElement("end");
        break;
      case STATUS:
        if(foundStatus)
          throw new AverParseException(multipleError + "'status' predicate.");
        foundStatus = true;
        childElem = new XMLElement("status");
        break;
      case PREDICATE:
        if(foundPredicate)
          throw new AverParseException(multipleError + "'predicate' predicate.");
        foundPredicate = true;
        childElem = new XMLElement("predicate");
        break;
      case PATH:
        throw new AverParseException("Error: 'path' not currently supported.");
//         if(foundPath)
//           throw new AverParseException(multipleError + "'path' predicate.");
//         foundPath = true;
//         childElem = new XMLElement("path");
//         retval.addChild(childElem);
//         CommonAST path = (CommonAST) child.getFirstChild();
//         path = (CommonAST) path.getNextSibling();
//         childElem.setAttribute("path", path.getText().replace('\'', ' '));
//         child = (CommonAST) child.getNextSibling();
//         continue TOKEN_CHILD_LOOP;
      case VARIABLE:
        childElem = new XMLElement("variable");
        retval.addChild(childElem);
        CommonAST varName = (CommonAST) child.getFirstChild();
        CommonAST varVal = (CommonAST) varName.getNextSibling();
        //CommonAST varType = (CommonAST) varVal.getNextSibling();

        IXMLElement nameElem = new XMLElement("name");
        childElem.addChild(nameElem);
        CommonAST subName = (CommonAST) varName.getFirstChild();
        nameElem.setAttribute("operator", subName.getText());
        subName = (CommonAST) subName.getNextSibling();
        nameElem.addChild(evaluate(subName));

        IXMLElement valueElem = new XMLElement("value");
        childElem.addChild(valueElem);
        CommonAST subValue = (CommonAST) varVal.getFirstChild();
        valueElem.setAttribute("operator", subValue.getText());
        subValue = (CommonAST) subValue.getNextSibling();
        valueElem.addChild(evaluate(subValue));

//         IXMLElement typeElem = new XMLElement("type");
//         childElem.addChild(typeElem);
//         CommonAST subType = (CommonAST) varType.getFirstChild();
//         typeElem.setAttribute("type", subType.getText());
        

        child = (CommonAST) child.getNextSibling();
        continue TOKEN_CHILD_LOOP;
      default:
        throw new AverRuntimeException("Attempted to evaluate 'Tokens' function with invalid argument type " + 
                                           child.getType() + "\nAt " + ast.toStringList());
      }
      retval.addChild(childElem);
      CommonAST sub = (CommonAST) child.getFirstChild();
      childElem.setAttribute("operator", sub.getText());
      sub = (CommonAST) sub.getNextSibling();
      childElem.addChild(evaluate(sub));
      child = (CommonAST) child.getNextSibling();
    }
    return retval;
  }

  private static IXMLElement transactions(CommonAST ast) throws AverParseException, AverRuntimeException {
    if(ast == null)
      throw new AverRuntimeException("Attempted to evaluate null 'Transactions' element.");
    checkValidType(ast.getType(), TRANSACTIONS, "Attempted to evaluate a 'Transactions' element with invalid type " +
                   ast.getType());
    IXMLElement retval = new XMLElement("Transactions");
    CommonAST child = (CommonAST) ast.getFirstChild();
    boolean foundStep = false;
    boolean foundName = false;
    boolean foundType = false;
    String multipleError = "The 'Transactions' function can have only one ";
    TRANSACTION_CHILD_LOOP: while(child != null) {
      IXMLElement childElem = null;
      switch(child.getType()) {
      case STEP:
        if(foundStep)
          throw new AverParseException(multipleError + "'step' predicate.");
        foundStep = true;
        childElem = new XMLElement("step");
        CommonAST sub = (CommonAST) child.getFirstChild();
        CommonAST at = (CommonAST) sub.getNextSibling();
        if(sub.getType() == EQ && (at.getType() == FIRST || at.getType() == LAST || at.getType() == ANY ||
                                   at.getType() == EACH || at.getType() == ALL)) {
          retval.addChild(childElem);
          childElem.setAttribute("operator", sub.getText());
          childElem.setAttribute("at", at.getText());
          child = (CommonAST) child.getNextSibling();
          continue TRANSACTION_CHILD_LOOP;
        }
        break;
      case NAME:
        if(foundName)
          throw new AverParseException(multipleError + "'name' predicate.");
        foundName = true;
        childElem = new XMLElement("name");
        break;
      case TYPE:
        if(foundType)
          throw new AverParseException(multipleError + "'type' predicate.");
        foundType = true;
        childElem = new XMLElement("type");
        break;
      default:
        throw new AverRuntimeException("Attempted to evaluate 'Transactions' function with invalid argument type " +
                                           child.getType() + "\nAt " + ast.toStringList()); 
      }
      retval.addChild(childElem);
      CommonAST sub = (CommonAST) child.getFirstChild();
      childElem.setAttribute("operator", sub.getText());
      sub = (CommonAST) sub.getNextSibling();
      childElem.addChild(evaluate(sub));
      child = (CommonAST) child.getNextSibling();
    }
    return retval;
  }


}
