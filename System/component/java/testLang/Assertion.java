package testLang;

import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

import net.n3.nanoxml.*;

public class Assertion {
  private IXMLElement step;
  private IXMLElement asstn;
  private String name;
  private String lineText;
  private String fileName;
  private int lineNo;
  private CppFile header;
  private CppFile impl;
  private static final Map opToFuncMap;
  private static final Map evalMap;
  private static final Map transactionTypeMap;
  private static final Map transactionNameMap;
  private static final Map changeTypeMap;
  private static final Class [] funcArgs = {IXMLElement.class, LinkedList.class};
  private static final Class [] funcArgsStr = {IXMLElement.class, LinkedList.class, String.class};
  private int tokensNum;
  private int enumDomNum;
  private int intDomNum;

  static {
    opToFuncMap = new HashMap();
    opToFuncMap.put("=", "eq");
    opToFuncMap.put("==", "eq");
    opToFuncMap.put("!=", "ne");
    opToFuncMap.put(">", "gt");
    opToFuncMap.put("<", "lt");
    opToFuncMap.put(">=", "ge");
    opToFuncMap.put("<=", "le");
    opToFuncMap.put("in", "in");
    opToFuncMap.put("out", "out");
    opToFuncMap.put("intersects", "intersects");

    evalMap = new HashMap();
    evalMap.put("Count", "evalCount");
    evalMap.put("Tokens", "evalTokens");
    evalMap.put("EnumeratedDomain", "evalEnumDomain");
    evalMap.put("IntervalDomain", "evalIntDomain");
    evalMap.put("Transactions", "evalTransactions");
    
    transactionNameMap = new HashMap();
    transactionNameMap.put("OBJECT_CREATED", "void notifyAdded(const ObjectId&)");
    transactionNameMap.put("OBJECT_DELETED", "void notifyRemoved(const ObjectId&)");
    transactionNameMap.put("TOKEN_CREATED", "void notifyAdded(const TokenId&)");
    transactionNameMap.put("TOKEN_ADDED_TO_OBJECT","void notifyAdded(const ObjectId&, const TokenId&)");
    transactionNameMap.put("TOKEN_CLOSED", "void notifyClosed(const TokenId&)");
    transactionNameMap.put("TOKEN_DELETED", "void notifyDeleted(const TokenId&)");
    transactionNameMap.put("TOKEN_REMOVED", "void notifyRemoved(const ObjectId&, const TokenId&)");
    transactionNameMap.put("CONSTRAINT_CREATED", "void notifyAdded(const ConstraintId&)");
    transactionNameMap.put("CONSTRAINT_DELETED", "void notifyRemoved(const ConstraintId&)");
    transactionNameMap.put("CONSTRAINT_EXECUTED", "void notifyExecuted(const ConstraintId&)");
    transactionNameMap.put("VAR_CREATED", "void notifyAdded(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DELETED", "void notifyRemoved(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_RELAXED", "void notifyRelaxed(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_RESET", "void notifyReset(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_EMPTIED", "void notifyEmptied(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_UPPER_BOUND_DECREASED","void notifyUpperBoundDecreased(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_LOWER_BOUND_INCREASED", "void notifyLowerBoundIncreased(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_BOUNDS_RESTRICTED", "void notifyBoundsRestricted(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_VALUE_REMOVED", "void notifyValueRemoved(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_RESTRICT_TO_SINGLETON", "void notifyRestrictToSingleton(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_SET", "void notifySet(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_SET_TO_SINGLETON", "void notifySetToSingleton(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_CLOSED", "void notifyClosed(const ConstrainedVariableId&)");
    transactionNameMap.put("RULE_EXECUTED", "void notifyExecuted(const RuleInstanceId&)");
    transactionNameMap.put("RULE_UNDONE", "void notifyUndone(const RuleInstanceId&)");
    transactionNameMap.put("ASSIGN_NEXT_STARTED", "void notifyAssignNextStarted(const DecisionPointId&)");
    transactionNameMap.put("ASSIGN_NEXT_SUCCEEDED", "void notifyAssignNextSucceeded(const DecisionPointId&)");
    transactionNameMap.put("ASSIGN_NEXT_FAILED", "void notifyAssignNextFailed(const DecisionPointId&)");
    transactionNameMap.put("ASSIGN_CURRENT_STARTED", "void notifyAssignCurrentStarted(const DecisionPointId&)");
    transactionNameMap.put("ASSIGN_CURRENT_SUCCEEDED", "void notifyAssignCurrentSucceeded(const DecisionPointId&)");
    transactionNameMap.put("ASSIGN_CURRENT_FAILED", "void notifyAssignCurrentFailed(const DecisionPointId&)");
    transactionNameMap.put("RETRACT_STARTED", "void notifyRetractStarted(const DecisionPointId&)");
    transactionNameMap.put("RETRACT_SUCCEEDED", "void notifyRetractSucceeded(const DecisionPointId&)");
    transactionNameMap.put("RETRACT_FAILED", "void notifyRetractFailed(const DecisionPointId&)");
    

    transactionTypeMap = new HashMap();
    LinkedList creationList = new LinkedList();
    creationList.add("OBJECT_CREATED");
    creationList.add("TOKEN_CREATED");
    creationList.add("CONSTRAINT_CREATED");
    creationList.add("VAR_CREATED");
    transactionTypeMap.put("CREATION", creationList);

    LinkedList deletionList = new LinkedList();
    deletionList.add("OBJECT_DELETED");
    deletionList.add("TOKEN_DELETED");
    deletionList.add("CONSTRAINT_DELETED");
    deletionList.add("VAR_DELETED");
    transactionTypeMap.put("DELETION", deletionList);

    LinkedList additionList = new LinkedList();
    additionList.add("TOKEN_ADDED_TO_OBJECT");
    transactionTypeMap.put("ADDITION", additionList);

    LinkedList closureList = new LinkedList();
    closureList.add("TOKEN_CLOSED");
    closureList.add("VAR_DOMAIN_CLOSED");
    transactionTypeMap.put("CLOSURE", closureList);

    LinkedList removalList = new LinkedList();
    removalList.add("TOKEN_REMOVED");
    transactionTypeMap.put("REMOVAL", removalList);

    LinkedList executionList = new LinkedList();
    executionList.add("CONSTRAINT_EXECUTED");
    executionList.add("RULE_EXECUTED");
    transactionTypeMap.put("EXECUTION", executionList);

    LinkedList relaxationList = new LinkedList();
    relaxationList.add("VAR_DOMAIN_RELAXED");
    relaxationList.add("VAR_DOMAIN_RESET");
    transactionTypeMap.put("RELAXATION", relaxationList);

    LinkedList restrictionList = new LinkedList();
    restrictionList.add("VAR_DOMAIN_EMPTIED");
    restrictionList.add("VAR_DOMAIN_UPPER_BOUND_DECREASED");
    restrictionList.add("VAR_DOMAIN_LOWER_BOUND_INCREASED");
    restrictionList.add("VAR_DOMAIN_BOUNDS_RESTRICTED");
    restrictionList.add("VAR_DOMAIN_VALUE_REMOVED");
    restrictionList.add("VAR_DOMAIN_RESTRICT_TO_SINGLETON");
    transactionTypeMap.put("RESTRICTION", restrictionList);

    LinkedList specificationList = new LinkedList();
    specificationList.add("VAR_DOMAIN_SPECIFIED");
    specificationList.add("VAR_DOMAIN_SET");
    specificationList.add("VAR_DOMAIN_SET_TO_SINGLETON");
    transactionTypeMap.put("SPECIFICATION", specificationList);

    LinkedList undoList = new LinkedList();
    undoList.add("RULE_UNDONE");
    transactionTypeMap.put("UNDO", undoList);

    LinkedList assignmentList = new LinkedList();
    assignmentList.add("ASSIGN_NEXT_SUCCEEDED");
    assignmentList.add("ASSIGN_CURRENT_SUCCEEDED");
    transactionTypeMap.put("ASSIGNMENT", assignmentList);

    LinkedList retractionList = new LinkedList();
    retractionList.add("RETRACT_SUCCEEDED");
    transactionTypeMap.put("RETRACTION", retractionList);

    changeTypeMap = new HashMap();
    changeTypeMap.put("VAR_DOMAIN_RELAXED", "DomainListener::RELAXED");
    changeTypeMap.put("VAR_DOMAIN_RESET", "DomainListener::RESET");
    changeTypeMap.put("VAR_DOMAIN_VALUE_REMOVED", "DomainListener::VALUE_REMOVED");
    changeTypeMap.put("VAR_DOMAIN_BOUNDS_RESTRICTED", "DomainListener::BOUNDS_RESTRICTED");
    changeTypeMap.put("VAR_DOMAIN_LOWER_BOUND_INCREASED", "DomainListener::LOWER_BOUND_INCREASED");
    changeTypeMap.put("VAR_DOMAIN_UPPER_BOUND_DECREASED", "DomainListener::UPPER_BOUND_DECREASED");
    changeTypeMap.put("VAR_DOMAIN_RESTRICT_TO_SINGLETON", "DomainListener::RESTRICT_TO_SINGLETON");
    changeTypeMap.put("VAR_DOMAIN_SET", "DomainListener::SET");
    changeTypeMap.put("VAR_DOMAIN_SET_TO_SINGLETON", "DomainListener::SET_TO_SINGLETON");
    changeTypeMap.put("VAR_DOMAIN_EMPTIED", "DomainListener::EMPTIED");
    changeTypeMap.put("VAR_DOMAIN_CLOSED", "DomainListener::CLOSED");
  }

  public Assertion(String name, IXMLElement assertion) {
    this.name = name;
    tokensNum = enumDomNum = intDomNum = 0;
    for(Iterator it = assertion.getChildren().iterator(); it.hasNext();) {
      IXMLElement child = (IXMLElement) it.next();
      if(child.getName().equals("step"))
        step = child;
      else
        asstn = child;
    }
    lineText = assertion.getAttribute("lineText", "");
    fileName = assertion.getAttribute("file", "");
    lineNo = Integer.parseInt(assertion.getAttribute("lineNo", "-1"));
    header = new CppFile();
    impl = new CppFile();
    toHeader();
    toImpl();
    header.unindent();
    header.addLine("};");
  }
  
  public void toCpp(CppFile header, CppFile impl) {
    header.add(this.header);
    impl.add(this.impl);
  }

  private void toHeader() {
    header.addLine("class " + name + " : public Assertion {");
    header.addLine("public:");
    header.indent();
    header.addLine(name + "(const PlanDatabaseId& planDb);");
    addCheckDeclaration();
    header.addLine("Test::Status execute();");
    addListeners();
    header.unindent();
    header.addLine("private:");
    header.indent();
  }

  private void toImpl() {
    addConstructorImpl();
    addCheckImpl();
    addExecuteImpl();
  }

  private void addConstructorImpl() {
    impl.addLine(name + "::" + name + "(const PlanDatabaseId& planDb) : Assertion(planDb, \"" +
                 fileName + "\", " + lineNo + ", \"" + lineText +
                 "\") {");
    impl.indent();
    String stepDom = "m_stepDomain = ";
    IXMLElement dom = null;
    if((dom = step.getFirstChildNamed("IntervalDomain")) != null) {
      stepDom = stepDom + " IntervalIntDomain((int)" + getLowerBound(dom) + ", (int)" + getUpperBound(dom) + ");";
      header.addLine("IntervalIntDomain m_stepDomain;");
    }
    else if((dom = step.getFirstChildNamed("EnumeratedDomain")) != null) {
      String enumDomName = "temp";
      impl.addLine("EnumeratedDomain " + enumDomName + "(true);");
      addEnumInserts(dom, enumDomName, impl);
      stepDom = stepDom + enumDomName + ";";
      header.addLine("EnumeratedDomain m_stepDomain;");
    }
    impl.addLine(stepDom);
    impl.addLine("m_stepDomain.close();");
    impl.unindent();
    impl.addLine("}");
  }

  private void addEnumInserts(IXMLElement dom, String domName, CppFile file) {
    for(Iterator it = dom.getChildren().iterator(); it.hasNext();)
      file.addLine(domName + ".insert((double)" + evalValue((IXMLElement)it.next()) + ");");
  }

  private String getLowerBound(IXMLElement intDomain) {
    return getBound(intDomain, "LowerBound");
  }

  private String getUpperBound(IXMLElement intDomain) {
    return getBound(intDomain, "UpperBound");
  }

  private String getBound(IXMLElement intDomain, String bound) {
    return evalValue(intDomain.getFirstChildNamed(bound).getFirstChildNamed("Value"));
  }

  private String evalValue(IXMLElement value) {
    String type = value.getAttribute("type", "");
    if(type.equals("integer"))
      return "(double)" + value.getContent().trim();
    else if(type.equals("string"))
      return "LabelStr(\"" + value.getContent().trim() + "\")";
    return null;
  }

  private void addCheckDeclaration() {
    header.addLine("bool check();");
  }

  private void addCheckImpl() {
    String Q = step.getAttribute("qualifier", "");
    impl.addLine("bool " + name + "::check() {");
    impl.indent();
    if(Q.equals("first")) {
      impl.addLine("bool retval = m_status == INCOMPLETE && currentStep() == 1;");
    }
    else if(Q.equals("last")) {
      impl.addLine("bool retval = false;"); //directly executed in notifySearchFinished
      header.addLine("void notifySearchFinished(){execute();}");
    }
    else if(Q.equals("each")) {
      String check = "bool retval =  " + (String) opToFuncMap.get(step.getAttribute("operator", "")) +
        "(EnumeratedDomain((double)currentStep()), m_stepDomain);";
      impl.addLine(check);
    }
    else if(Q.equals("any")) {
      String check = "bool retval = m_status != PASSED && " + 
        (String) opToFuncMap.get(step.getAttribute("operator", "")) +
        "(EnumeratedDomain((double)currentStep()), m_stepDomain);";
      impl.addLine(check);
    }
    else if(Q.equals("all")) {
    }
    impl.addLine("return retval;");
    impl.unindent();
    impl.addLine("}");
  }
  private void addExecuteImpl() {
    LinkedList funcImpls = new LinkedList();
    impl.addLine("Test::Status " + name + "::execute() {");
    evaluateBoolean(funcImpls);
    impl.addLine("}");
    for(Iterator it = funcImpls.iterator(); it.hasNext();)
      impl.add((CppFile)it.next());
  }

  private void evaluateBoolean(LinkedList funcImpls) {
    String Q = step.getAttribute("qualifier", "");
    impl.indent();
    impl.addLine("const AbstractDomain &dom1 = " + evaluate(asstn.getChildAtIndex(0), funcImpls) + ";");
    impl.addLine("const AbstractDomain &dom2 = " + evaluate(asstn.getChildAtIndex(1), funcImpls) + ";");
    impl.addLine("if(" + asstn.getName() + "(dom1, dom2))");
    impl.indent();
    impl.addLine("m_status = Test::PASSED;");
    impl.unindent();
    impl.addLine("else");
    impl.indent();
    impl.addLine("m_status = Test::FAILED;");
    impl.unindent();
    if(Q.equals("each")) {
      impl.addLine("if(m_status == Test::FAILED)");
      impl.indent();
      impl.addLine("fail();");
    }
    impl.unindent();
    impl.addLine("return m_status;");
    impl.unindent();
  }

  private String evaluate(IXMLElement xml, LinkedList funcImpls) {
    String retval = "";
    try {
      Method evalMethod = getClass().getDeclaredMethod((String)evalMap.get(xml.getName()), funcArgs);
      Object [] args = {xml, funcImpls};
      retval = evalMethod.invoke(this, args).toString();
    }
    catch(Exception e) {
      e.printStackTrace();
      System.exit(-1);
    }
    return retval;
  }

  private String evaluate(IXMLElement xml, LinkedList funcImpls, String other) {
    String retval = "";
    try {
      Method evalMethod = getClass().getDeclaredMethod((String)evalMap.get(xml.getName()), funcArgsStr);
      Object [] args = {xml, funcImpls, other};
      retval = evalMethod.invoke(this, args).toString();
    }
    catch(Exception e) {
      e.printStackTrace();
      System.exit(-1);
    }
    return retval;
  }
  
  private String evalCount(IXMLElement xml, LinkedList funcImpls) {
    return "count(" + evaluate(xml.getChildAtIndex(0), funcImpls) + ")";
  }

  private String evalTokens(IXMLElement xml, LinkedList funcImpls) {
    String funcName = "tokens_" + tokensNum++ + "()";
    header.addLine("EnumeratedDomain " + funcName + ";");
    
    CppFile tokenFunc = new CppFile();
    tokenFunc.addLine("EnumeratedDomain " + name + "::" + funcName + "{");
    tokenFunc.indent();
    tokenFunc.addLine("TokenSet temp(m_db->getTokens());");
    for(Iterator it = xml.getChildren().iterator(); it.hasNext();) {
      IXMLElement child = (IXMLElement) it.next();
      addTrimElement(child, "Tokens", tokenFunc, funcImpls);
    }
    tokenFunc.addLine("return tokenSetToDomain(temp);");
    tokenFunc.unindent();
    tokenFunc.addLine("}");
    funcImpls.add(tokenFunc);
    return funcName;
  }

  private String enumDomainType(IXMLElement elem) {
    if(!elem.getName().equals("EnumeratedDomain"))
      throw new IllegalArgumentException();
    if(elem.getFirstChildNamed("Value").getAttribute("type", "").equals("integer"))
      return "EnumeratedDomain";
    else
      return "StringDomain";
  }

  private void addTrimElement(IXMLElement elem, String type, CppFile func, LinkedList funcImpls) {
    String setType = type.substring(0, 1).toUpperCase() + type.substring(1);
    String trimType = elem.getName().substring(0, 1).toUpperCase() + elem.getName().substring(1);
    if(elem.getName().equals("variable")) {
      IXMLElement name = elem.getFirstChildNamed("name");
      IXMLElement value = elem.getFirstChildNamed("value");
      String nameOp = (String) opToFuncMap.get(name.getAttribute("operator", ""));
      String nameDomain = evaluate(name.getChildAtIndex(0), funcImpls);
      String nameDomName = getCurrentDomainName(name.getChildAtIndex(0));
      String valOp = (String) opToFuncMap.get(value.getAttribute("operator", ""));
      String valDomain = evaluate(value.getChildAtIndex(0), funcImpls, 
                                  "m_db->getSchema()->getEnumFromMember(" + evalValue(value.getChildAtIndex(0).getChildAtIndex(0)) + ")");
      String valDomName = getCurrentDomainName(value.getChildAtIndex(0));
      func.addLine(enumDomainType(name.getChildAtIndex(0)) + " " + nameDomName + " = " + nameDomain);
      if(value.getChildAtIndex(0).getName().equals("IntervalDomain"))
        func.addLine("IntervalIntDomain " + valDomName + " = " + valDomain);
      else
        func.addLine(enumDomainType(value.getChildAtIndex(0)) + " " + valDomName + " = " + valDomain);
      func.addLine("trim" + setType + "By" + trimType + "(temp, " +
                   nameOp + ", " + nameDomName + ", " + valOp + ", " +
                   valDomName + ");");
    }
    else {
      String domain = evaluate(elem.getChildAtIndex(0), funcImpls);
      
      if(elem.getChildAtIndex(0).getName().equals("IntervalDomain"))
        func.addLine("IntervalIntDomain " + 
                     getCurrentDomainName(elem.getChildAtIndex(0)) + " = " + domain);
      else
        func.addLine(enumDomainType(elem.getChildAtIndex(0)) + " " + getCurrentDomainName(elem.getChildAtIndex(0)) + " = " + domain);
      func.addLine("trim" + setType + "By" + trimType + "(temp, " + 
                   opToFuncMap.get(elem.getAttribute("operator", "")).toString() + ", " +
                   getCurrentDomainName(elem.getChildAtIndex(0)) + ");");
    }
  }

  private String evalEnumDomain(IXMLElement xml, LinkedList funcImpls) {
    if(xml.getChildren().size() == 1) {
      getNextEnumName();
      String val = evalValue(xml.getChildAtIndex(0));
      if(val.indexOf("LabelStr(\"") != -1)
        return "StringDomain((double)" + val + ");";
      else
        return "EnumeratedDomain(" + val + ");";
    }
    CppFile enumBuild = new CppFile();
    if(xml.getChildAtIndex(0).getAttribute("type", "").equals("integer"))
      enumBuild.addLine("EnumeratedDomain " + getNextEnumName() + "();");
    else
      enumBuild.addLine("StringDomain " + getNextEnumName() + "();");
    for(Iterator it = xml.getChildren().iterator(); it.hasNext();) {
      IXMLElement child = (IXMLElement) it.next();
      enumBuild.addLine(getCurrentEnumName() + ".insert(" +evalValue(child) + ");");
    }
    enumBuild.addLine(getCurrentEnumName() + ".close();");
    return enumBuild.toString();
  }

  private String evalEnumDomain(IXMLElement xml, LinkedList funcImpls, String type) {
    if(xml.getChildren().size() == 1) {
      getNextEnumName();
      String val = evalValue(xml.getChildAtIndex(0));
      if(val.indexOf("LabelStr(\"") != -1)
        return "StringDomain((double)" + val + ", DomainListenerId::noId(), " + type + ");";
      else
        return "EnumeratedDomain(" + val + ", DomainListenerId::noId(), " + type + ");";
    }
    CppFile enumBuild = new CppFile();
    if(xml.getChildAtIndex(0).getAttribute("type", "").equals("integer"))
      enumBuild.addLine("EnumeratedDomain " + getNextEnumName() + "(" + type + ");");
    else
      enumBuild.addLine("StringDomain " + getNextEnumName() + "(" + type + ");");
    for(Iterator it = xml.getChildren().iterator(); it.hasNext();) {
      IXMLElement child = (IXMLElement) it.next();
      enumBuild.addLine(getCurrentEnumName() + ".insert(" +evalValue(child) + ");");
    }
    enumBuild.addLine(getCurrentEnumName() + ".close();");
    return enumBuild.toString();
  }

  private String evalIntDomain(IXMLElement xml, LinkedList funcImpls) {
    String lb = evalValue(xml.getFirstChildNamed("LowerBound").getChildAtIndex(0));
    String ub = evalValue(xml.getFirstChildNamed("UpperBound").getChildAtIndex(0));
    return "IntervalIntDomain((int)" + lb + ", (int)" + ub + ");";
  }

  private void addTokensDeclaration() {
    header.addLine("AbstractDomain& tokens();");
  }

  private String getCurrentDomainName(IXMLElement dom) {
    if(dom.getName().equals("IntervalDomain"))
      return getCurrentIntName();
    else if(dom.getName().equals("EnumeratedDomain"))
      return getCurrentEnumName();
    else
      throw new IllegalArgumentException();
  }

  private String getCurrentEnumName() {
    return "enum_" + enumDomNum;
  }
  private String getNextEnumName() {
    enumDomNum++;
    return getCurrentEnumName();
  }
  private String getCurrentIntName() {
    return "int_" + intDomNum;
  }
  private String getNextIntName() {
    intDomNum++;
    return getCurrentIntName();
  }
  private void addListeners() {
    header.addLine("void notifyStep(const DecisionPointId& dec) {AggregateListener::notifyStep(dec); if(check()){execute();}};");
  }

  private String evalTransactions(IXMLElement elem, LinkedList funcImpls) throws TestLangRuntimeException {
    IXMLElement spec = elem.getChildAtIndex(0);
    String info = spec.getFirstChildNamed("EnumeratedDomain").getFirstChildNamed("Value").getContent().trim();
    header.unindent();
    header.addLine("public:");
    header.indent();
    if(spec.getName().equals("name"))
      addTransaction(info);
    else if(spec.getName().equals("type")) {
      if(transactionTypeMap.get(info) == null)
        throw new TestLangRuntimeException("'" + info + "' is not a valid transaction type.");
      List functions = (List) transactionTypeMap.get(info);
      for(Iterator it = functions.iterator(); it.hasNext();)
        addTransaction((String)it.next());
    }
    header.unindent();
    header.addLine("private:");
    header.indent();
    header.addLine("EnumeratedDomain countGenerator();");
    CppFile countGenerator = new CppFile();
    countGenerator.addLine("EnumeratedDomain " + name + "::countGenerator() {");
    countGenerator.indent();
    countGenerator.addLine("EnumeratedDomain retval(true);");
    countGenerator.addLine("for(int i = 0; i < m_transactionCounter; i++)");
    countGenerator.indent();
    countGenerator.addLine("retval.insert((double)i);");
    countGenerator.unindent();
    countGenerator.addLine("retval.close();");
    countGenerator.addLine("return retval;");
    countGenerator.unindent();
    countGenerator.addLine("}");
    funcImpls.add(countGenerator);
    return "countGenerator()";
  }
  private void addTransaction(String info) throws TestLangRuntimeException {
    if(transactionNameMap.get(info) == null)
      throw new TestLangRuntimeException("'" + info + "' is not a valid transaction name.");
    String function = (String) transactionNameMap.get(info);
    header.addLine(function + "{if(check()){m_transactionCounter++;}};");
  }
}
