package aver;

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
  private int objsNum;
  private int propNum;
  private int enumDomNum;
  private int intDomNum;
  private int entNum;

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
    evalMap.put("Objects", "evalObjects");
    evalMap.put("EnumeratedDomain", "evalEnumDomain");
    evalMap.put("IntervalDomain", "evalIntDomain");
    evalMap.put("Transactions", "evalTransactions");
    evalMap.put("Property", "evalProperty");
    evalMap.put("Entity", "evalEntity");
    
    transactionNameMap = new HashMap();
    transactionNameMap.put("OBJECT_CREATED", "void notifyAdded(const ObjectId&)");
    transactionNameMap.put("OBJECT_DELETED", "void notifyRemoved(const ObjectId&)");
    transactionNameMap.put("TOKEN_CREATED", "void notifyAdded(const TokenId&)");
    transactionNameMap.put("TOKEN_ADDED_TO_OBJECT","void notifyAdded(const ObjectId&, const TokenId&)");
    transactionNameMap.put("TOKEN_ACTIVATED", "void notifyActivated(const TokenId&)");
    transactionNameMap.put("TOKEN_DEACTIVATED", "void notifyDeactivated(const TokenId&)");
    transactionNameMap.put("TOKEN_MERGED", "void notifyMerged(const TokenId&)");
    transactionNameMap.put("TOKEN_SPLIT", "void notifySplit(const TokenId&)");
    transactionNameMap.put("TOKEN_REJECTED", "void notifyRejected(const TokenId&)");
    transactionNameMap.put("TOKEN_REINSTATED", "void notifyReinstated(const TokenId&)");
    transactionNameMap.put("TOKEN_CLOSED", "void notifyClosed(const TokenId&)");
    transactionNameMap.put("TOKEN_DELETED", "void notifyDeleted(const TokenId&)");
    transactionNameMap.put("TOKEN_REMOVED", "void notifyRemoved(const ObjectId&, const TokenId&)");
    transactionNameMap.put("TOKEN_FREED", "void notifyFreed(const ObjectId&, const TokenId&)");
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
    transactionNameMap.put("VAR_DOMAIN_RESTRICTED", "void notifyBoundsRestricted(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_VALUE_REMOVED", "void notifyValueRemoved(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_RESTRICT_TO_SINGLETON", "void notifyRestrictToSingleton(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_SET", "void notifySet(const ConstrainedVariableId&)");
    transactionNameMap.put("VAR_DOMAIN_SPECIFIED", "void notifySet(const ConstrainedVariableId&)");
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
    transactionNameMap.put("PROPAGATION_COMMENCED", "void notifyPropagationCommenced()");
    transactionNameMap.put("PROPAGATION_COMPLETED", "void notifyPropagationCompleted()");
    transactionNameMap.put("PROPAGATION_PREEMPTED", "void notifyPropagationPreempted()");
    

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
    //specificationList.add("VAR_DOMAIN_SPECIFIED");
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

  public Assertion(String name, IXMLElement assertion) throws AverRuntimeException {
    this.name = name;
    tokensNum = objsNum = propNum = enumDomNum = intDomNum = entNum = 0;
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

  private void toImpl() throws AverRuntimeException {
    addConstructorImpl();
    addCheckImpl();
    addExecuteImpl();
  }

  private void addConstructorImpl() throws AverRuntimeException {
    String constructorDecl = name + "::" + name + 
      "(const PlanDatabaseId& planDb) : Assertion(planDb, \"" + fileName + "\", " + lineNo +
      ", \"" + lineText + "\"), m_stepDomain(";
    //impl.addLine(name + "::" + name + "(const PlanDatabaseId& planDb) : Assertion(planDb, \"" +
    //fileName + "\", " + lineNo + ", \"" + lineText + "\") {");
    //impl.indent();
    String stepDom = "m_stepDomain = ";
    IXMLElement dom = null;
    if((dom = step.getFirstChildNamed("IntervalDomain")) != null) {
      //      stepDom = stepDom + " IntervalIntDomain((int)" + getLowerBound(dom) + ", (int)" + getUpperBound(dom) + ");";
      stepDom = stepDom + " IntervalDomain(" + getLowerBound(dom) + ", " + getUpperBound(dom) + ");";
      //      header.addLine("IntervalIntDomain m_stepDomain;");
      header.addLine("IntervalDomain m_stepDomain;");
      constructorDecl = constructorDecl + ") {";
      impl.addLine(constructorDecl);
      impl.indent();
    }
    else if((dom = step.getFirstChildNamed("EnumeratedDomain")) != null) {
      constructorDecl = constructorDecl + "true, EnumeratedDomain::getDefaultTypeName().c_str()) {";
      impl.addLine(constructorDecl);
      impl.indent();
      String enumDomName = "temp";
      //impl.addLine("EnumeratedDomain " + enumDomName + "(true);");
      impl.addLine("EnumeratedDomain " + enumDomName + "(true, \"REAL_ENUMERATION\");");
      addEnumInserts(dom, enumDomName, impl);
      stepDom = stepDom + enumDomName + ";";
      header.addLine("EnumeratedDomain m_stepDomain;");
    }
    impl.addLine(stepDom);
    impl.addLine("m_stepDomain.close();");
    impl.unindent();
    impl.addLine("}");
  }

  private void addEnumInserts(IXMLElement dom, String domName, CppFile file) 
    throws AverRuntimeException {
    for(Iterator it = dom.getChildren().iterator(); it.hasNext();)
      file.addLine(domName + ".insert((double)" + evalValue((IXMLElement)it.next()) + ");");
  }

  private String getLowerBound(IXMLElement intDomain) throws AverRuntimeException {
    return getBound(intDomain, "LowerBound");
  }

  private String getUpperBound(IXMLElement intDomain) throws AverRuntimeException {
    return getBound(intDomain, "UpperBound");
  }

  private String getBound(IXMLElement intDomain, String bound) 
    throws AverRuntimeException {
    return evalValue(intDomain.getFirstChildNamed(bound).getFirstChildNamed("Value"));
  }

  private String evalValue(IXMLElement value) throws AverRuntimeException {
    String type = value.getAttribute("type", "");
    if(type.equals("number"))
      return "(double)" + value.getContent().trim();
    else if(type.equals("string"))
      return "LabelStr(\"" + value.getContent().trim() + "\")";
    else
      throw new AverRuntimeException("Invalid value '" + value.getName() + "'");
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
        "(EnumeratedDomain((double)currentStep(), true, EnumeratedDomain::getDefaultTypeName().c_str()), m_stepDomain);";
      impl.addLine(check);
    }
    else if(Q.equals("any")) {
      String check = "bool retval = m_status != PASSED && " + 
        (String) opToFuncMap.get(step.getAttribute("operator", "")) +
        "(EnumeratedDomain((double)currentStep(), true, EnumeratedDomain::getDefaultTypeName().c_str()), m_stepDomain);";
      impl.addLine(check);
    }
    else if(Q.equals("all")) {
      System.err.println("Error: 'all' not currently implemented.");
      System.exit(-1);
    }
    impl.addLine("return retval;");
    impl.unindent();
    impl.addLine("}");
  }
  private void addExecuteImpl() throws AverRuntimeException {
    LinkedList funcImpls = new LinkedList();
    impl.addLine("Test::Status " + name + "::execute() {");
    evaluateBoolean(funcImpls);
    impl.addLine("}");
    for(Iterator it = funcImpls.iterator(); it.hasNext();)
      impl.add((CppFile)it.next());
  }

  private void evaluateBoolean(LinkedList funcImpls) throws AverRuntimeException {
    String Q = step.getAttribute("qualifier", "");
    impl.indent();
    String domStart = "const AbstractDomain& dom";
    for(int i = 0; i < 2; i++) {
      if(asstn.getChildAtIndex(i).getName().equals("EnumeratedDomain") &&
         asstn.getChildAtIndex(i).getChildren().size() > 1) {
        impl.addLine(evalEnumDomain(asstn.getChildAtIndex(i), funcImpls));
        impl.addLine(domStart + (i+1) + " = " + getCurrentEnumName() + ";");
      }
      else {
        impl.addLine(domStart + (i+1) + " = " + 
                     evaluate(asstn.getChildAtIndex(i), funcImpls) + ";");
      }
    }
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
      //System.err.println(xml.getName() + " : " + evalMap.get(xml.getName()));
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

  //assumes singleton.  this should be fixed.
  private String evalProperty(IXMLElement xml, LinkedList funcImpls) 
    throws AverRuntimeException {
    String funcName = "property_" + propNum++ + "()";
    header.addLine("const AbstractDomain& " + funcName + ";");
    CppFile propFunc = new CppFile();
    propFunc.addLine("const AbstractDomain& " + name + "::" + funcName + "{");
    propFunc.indent();
    IXMLElement func = null;
    if((func = xml.getFirstChildNamed("Objects")) != null) {
      propFunc.addLine("EnumeratedDomain entities = " + 
                       evalObjects(func, funcImpls) + ";");
      propFunc.addLine("return (ObjectId(entities.getSingletonValue()))->getVariable(LabelStr(\"" + xml.getAttribute("index", "") + "\"))->lastDomain();");
    }
    else if((func = xml.getFirstChildNamed("Tokens")) != null) {
      propFunc.addLine("EnumeratedDomain entities = " +
                       evalTokens(func, funcImpls) + ";");
      propFunc.addLine("const std::vector<ConstrainedVariableId>& vars = " +
                       "(TokenId(entities.getSingletonValue()))->getVariables();");
      propFunc.addLine("for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end(); ++it) {");
      propFunc.indent();
      propFunc.addLine("ConstrainedVariableId var = *it;");
      propFunc.addLine("if(var->getName() == LabelStr(\"" + xml.getAttribute("index", "") + "\"))");
      propFunc.indent();
      propFunc.addLine("return var->lastDomain();");
      propFunc.unindent();
      propFunc.unindent();
      propFunc.addLine("}");
      propFunc.addLine("handle_error(ALWAYS_FAIL, \"Failed to find a variable with name '" + xml.getAttribute("index", "") + "'.\");");
      propFunc.addLine("return *(new EnumeratedDomain(false, \"ErrorDomain\"));");
    }
    else if((func = xml.getFirstChildNamed("Entity")) != null) {
      propFunc.addLine("EnumeratedDomain entity = " +
                       evalEntity(func, funcImpls) + ";");
      propFunc.addLine("double id = entity.getSingletonValue();");
      IXMLElement idType = null;
      if((idType = func.getFirstChildNamed("Domain").getFirstChildNamed("Tokens")) 
         != null) {
        propFunc.addLine("const std::vector<ConstrainedVariableId>& vars = " +
                         "(TokenId(id))->getVariables();");
        propFunc.addLine("for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end(); ++it) {");
        propFunc.indent();
        propFunc.addLine("ConstrainedVariableId var = *it;");
        propFunc.addLine("if(var->getName() == LabelStr(\"" + xml.getAttribute("index", "") + "\"))");
        propFunc.indent();
        propFunc.addLine("return var->lastDomain();");
        propFunc.unindent();
        propFunc.unindent();
        propFunc.addLine("}");
        propFunc.addLine("handle_error(ALWAYS_FAIL, \"Failed to find a variable with name '" + xml.getAttribute("index", "") + "'.\");");
      }
      else if((idType = func.getFirstChildNamed("Domain").getFirstChildNamed("Objects"))
              != null) {
        propFunc.addLine("return (ObjectId(id))->getVariable(LabelStr(\"" + xml.getAttribute("index", "") + "\"))->lastDomain();");
      }
      else
        throw new AverRuntimeException("'Entity' functions must contain a call to either 'Objects' or 'Tokens'.");
    }
    else
      throw new AverRuntimeException("'Property' functions must contain a call to either 'Objects' or 'Tokens'.");
    propFunc.unindent();
    propFunc.addLine("}");
    funcImpls.add(propFunc);
    return funcName;
  }

  private String evalEntity(IXMLElement xml, LinkedList funcImpls) 
    throws AverRuntimeException {
    String funcName = "entity_" + entNum++ + "()";
    header.addLine("EnumeratedDomain " + funcName + ";");
    CppFile entFunc = new CppFile();
    entFunc.addLine("EnumeratedDomain " + name + "::" + funcName + "{");
    entFunc.indent();
    String indexDom = evalEnumDomain(xml.getFirstChildNamed("Index").getChildAtIndex(0),
                                     funcImpls);
    String indexName = 
      getCurrentDomainName(xml.getFirstChildNamed("Index").getChildAtIndex(0));
    entFunc.addLine("EnumeratedDomain " + indexName + " = " + indexDom + ";");
    entFunc.addLine("EnumeratedDomain ents = " + 
                    evaluate(xml.getFirstChildNamed("Domain").getChildAtIndex(0),
                             funcImpls) + ";");
    entFunc.addLine("std::list<double> entities;");
    entFunc.addLine("ents.getValues(entities);");
    entFunc.addLine("std::list<double>::iterator index = entities.begin();");
    entFunc.addLine("for(int i = 0; i < (int)" + indexName + ".getSingletonValue(); i++, index++){}");
    entFunc.addLine("return EnumeratedDomain(*index, true, EnumeratedDomain::getDefaultTypeName().c_str());");
    entFunc.unindent();
    entFunc.addLine("}");
    funcImpls.add(entFunc);
    return funcName;
  }

  private String evalObjects(IXMLElement xml, LinkedList funcImpls) 
    throws AverRuntimeException {
    String funcName = "objects_" + objsNum++ + "()";
    header.addLine("EnumeratedDomain " + funcName + ";");
    CppFile objFunc = new CppFile();
    objFunc.addLine("EnumeratedDomain " + name + "::" + funcName + "{");
    objFunc.indent();
    objFunc.addLine("ObjectSet temp(m_db->getObjects());");
    for(Iterator it = xml.getChildren().iterator(); it.hasNext();) {
      IXMLElement child = (IXMLElement) it.next();
      addTrimElement(child, "Objects", objFunc, funcImpls);
    }
    objFunc.addLine("return objectSetToDomain(temp);");
    objFunc.unindent();
    objFunc.addLine("}");
    funcImpls.add(objFunc);
    return funcName;
  }

  private String evalTokens(IXMLElement xml, LinkedList funcImpls) 
    throws AverRuntimeException {
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
    if(elem.getFirstChildNamed("Value").getAttribute("type", "").equals("number"))
      return "EnumeratedDomain";
    else
      return "StringDomain";
  }

  private void addTrimElement(IXMLElement elem, String type, CppFile func, 
                              LinkedList funcImpls) throws AverRuntimeException {
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
                                  "m_db->getSchema()->getEnumFromMember(" + evalValue(value.getChildAtIndex(0).getChildAtIndex(0)) + ").c_str()");
      String valDomName = getCurrentDomainName(value.getChildAtIndex(0));
      func.addLine(enumDomainType(name.getChildAtIndex(0)) + " " + nameDomName + " = " + nameDomain);
      if(value.getChildAtIndex(0).getName().equals("IntervalDomain"))
        func.addLine("IntervalDomain " + valDomName + " = " + valDomain);
      else
        func.addLine(enumDomainType(value.getChildAtIndex(0)) + " " + valDomName + " = " + valDomain);
      func.addLine("trim" + setType + "By" + trimType + "(temp, " +
                   nameOp + ", " + nameDomName + ", " + valOp + ", " +
                   valDomName + ");");
    }
    else if(elem.getName().equals("path")) {
      func.addLine("std::vector<int> path;");
      String [] elems = elem.getAttribute("path", "").split("\\.");
      for(int i = 0; i < elems.length; i++)
        func.addLine("path.push_back(" + elems[i] + ");");
      func.addLine("trimTokensByPath(temp, path);");
    }
    else {
      String domain = evaluate(elem.getChildAtIndex(0), funcImpls);
      if(domain.indexOf("\n") != -1) //this is bad
        func.addLine(domain);
      else {
        if(elem.getChildAtIndex(0).getName().equals("IntervalDomain"))
          func.addLine("IntervalDomain " + 
                       getCurrentDomainName(elem.getChildAtIndex(0)) + " = " + domain);
        else
          func.addLine(enumDomainType(elem.getChildAtIndex(0)) + " " + getCurrentDomainName(elem.getChildAtIndex(0)) + " = " + domain);
      }
      func.addLine("trim" + setType + "By" + trimType + "(temp, " + 
                   opToFuncMap.get(elem.getAttribute("operator", "")).toString() + ", " +
                   getCurrentDomainName(elem.getChildAtIndex(0)) + ");");
    }
  }

  private String evalEnumDomain(IXMLElement xml, LinkedList funcImpls) 
    throws AverRuntimeException {
    if(xml.getChildren().size() == 1) {
      getNextEnumName();
      String val = evalValue(xml.getChildAtIndex(0));
      if(val.indexOf("LabelStr(\"") != -1)
        return "StringDomain((double)" + val + ");";
      else
        return "EnumeratedDomain(" + val + ", true, EnumeratedDomain::getDefaultTypeName().c_str());";
    }
    CppFile enumBuild = new CppFile();
    if(xml.getChildAtIndex(0).getAttribute("type", "").equals("number"))
      enumBuild.addLine("EnumeratedDomain " + getNextEnumName() + "(true, EnumeratedDomain::getDefaultTypeName().c_str());");
    else
      enumBuild.addLine("StringDomain " + getNextEnumName() + ";");
    for(Iterator it = xml.getChildren().iterator(); it.hasNext();) {
      IXMLElement child = (IXMLElement) it.next();
      enumBuild.addLine(getCurrentEnumName() + ".insert(" +evalValue(child) + ");");
    }
    enumBuild.addLine(getCurrentEnumName() + ".close();");
    return enumBuild.toString();
  }

  private String evalEnumDomain(IXMLElement xml, LinkedList funcImpls, String type) 
    throws AverRuntimeException {
    if(xml.getChildren().size() == 1) {
      getNextEnumName();
      String val = evalValue(xml.getChildAtIndex(0));
      if(val.indexOf("LabelStr(\"") != -1)
        return "StringDomain((double)" + val + ", " + type + ");";
        //return "StringDomain((double)" + val + ", DomainListenerId::noId(), " + type + ");";
      else
        return "EnumeratedDomain(" + val + ", " + type + ");";
        //return "EnumeratedDomain(" + val + ", DomainListenerId::noId(), " + type + ");";
    }
    CppFile enumBuild = new CppFile();
    if(xml.getChildAtIndex(0).getAttribute("type", "").equals("number"))
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

  private String evalIntDomain(IXMLElement xml, LinkedList funcImpls) 
    throws AverRuntimeException {
    String lb = evalValue(xml.getFirstChildNamed("LowerBound").getChildAtIndex(0));
    String ub = evalValue(xml.getFirstChildNamed("UpperBound").getChildAtIndex(0));
    getNextIntName();
    return "IntervalDomain(" + lb + ", " + ub + ");";
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

  private String evalTransactions(IXMLElement elem, LinkedList funcImpls) throws AverRuntimeException {
    IXMLElement spec = elem.getChildAtIndex(0);
    String info = spec.getFirstChildNamed("EnumeratedDomain").getFirstChildNamed("Value").getContent().trim();
    header.unindent();
    header.addLine("public:");
    header.indent();
    if(spec.getName().equals("name"))
      addTransaction(info);
    else if(spec.getName().equals("type")) {
      if(transactionTypeMap.get(info) == null)
        throw new AverRuntimeException("'" + info + "' is not a valid transaction type.");
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
    countGenerator.addLine("EnumeratedDomain retval(true, EnumeratedDomain::getDefaultTypeName().c_str());");
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
  private void addTransaction(String info) throws AverRuntimeException {
    if(transactionNameMap.get(info) == null)
      throw new AverRuntimeException("'" + info + "' is not a valid transaction name.");
    String function = (String) transactionNameMap.get(info);
    header.addLine(function + "{if(check()){m_transactionCounter++;}};");
  }
}
