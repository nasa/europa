package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

class RuleWriter {
  private static final Set defaultFilter = NddlUtil.immutableSet(new String[]{"if", "invoke", "subgoal"});
  public static String delimiter = "$";
  private static Set s_rules = new TreeSet(); /* The set of all rules built up. Used to generate rules for system initialization. */
  private static int s_counter = 0; /* Used to ensure rule names are unique when you have more than one rule for the same predicate */
  private static String s_activeRuleClass = "";
  private static int s_slaveCount = 0;

  /**
   * Top level call to produce a rule. Rules declaration and definitions are all
   * created in the output .cc file since they do not need to be referenced externally.
   * @param writer Output destination
   * @param rule XMLElement for compat
   */
  public static void generateRule(IndentWriter writer, IXMLElement rule) throws IOException {
    XMLUtil.checkExpectedNode("compat", rule);

    SharedWriter.generateFileLocation(writer, rule);

    // Nomenclature for each rule uses the predicate name and mangles it with
    // a prefix, the nddl klass on which it applies, and an index.
    // e.g. Rover::Going{..} will become 'Rule_Rover_Going_3' where 3 is an arbitrary
    // counter used since there may be more than one rule declared for a predicate.
    String name = XMLUtil.getAttribute(rule, "name"); 
    String klass = XMLUtil.getAttribute(rule, "class");
    String ruleInstanceName = klass + delimiter + name + delimiter + allocateCounter();

    // Set the scope for variable resolution
    ModelAccessor.setCurrentObjectType(klass);
    ModelAccessor.setCurrentPredicate(name);

    // Top-level call to generate necessary class declaration
    generateDeclaration(writer, ruleInstanceName , rule.getFirstChildNamed("group"), new HashSet(), 0);

    // Construct a rule class for this which can allocate rule instances.
    // The rule is declared with a hook to source code for rerference.
    writer.write("DECLARE_AND_DEFINE_RULE(Rule" + delimiter + ruleInstanceName + 
                 ", " + ruleInstanceName + delimiter + "0" + ", " + klass +
                 "." + name + ", \"" + XMLUtil.locationString(rule, ',') + "\");\n\n");

    // Top level call to output the implementations of functions. All of these functions
    // map to the ::handleExecute method.
    generateExecution(writer, ruleInstanceName, rule.getFirstChildNamed("group"), new HashSet(), 0);

    // Store rule name for later generation
    s_rules.add("Rule" + delimiter + ruleInstanceName); 

    // Reset the scope
    ModelAccessor.resetCurrentPredicate();
    ModelAccessor.resetCurrentObjectType();
  }

  /**
   * Outputs the class declaration for the rule instance. This is a recursive function
   * since rule instances have nested structures according to the use of guards.
   * @param writer The Output destination
   * @param ruleInstanceClass The name of the rule instance to use if written out
   * @param ruleNode The node in the compat tree that we are currently on.
   * @param guards A delimited string of guards accumulated in this scope
   * @param index The offset of child rule for the parent.
   * @param true if a child rule was declared, otherwise false
   */
  private static boolean generateDeclaration(IndentWriter writer, 
                                             String ruleInstanceClass, 
                                             IXMLElement ruleNode,
                                             Set guards,
                                             int index) throws IOException {
    assert(DebugMsg.debugMsg("RuleWriter:generateDeclaration","Rule Instance is " + ruleInstanceClass + " with current guards = " + guards));

    // Establish default values for filtering child nodes for recurive invocation of this
    // method
		Set filter = defaultFilter;

    boolean result = true;

    // Set the scope for resolving variables to be the current rule instance class
    ModelAccessor.setCurrentRule(ruleInstanceClass);

    // Iterate over children to register slaves and 
    // Recurse to children to introduce new declarations for each possible child rule
    Vector children = ruleNode.getChildren();
    int childRuleIndex = 0;
    for (int i=0; i< children.size(); i++) {
      IXMLElement child = (IXMLElement) children.elementAt(i);
      if(child.getName().equals("var")) // Register it as we go to allow evaluation for implied guards
        ModelAccessor.registerVariable(child);
      else if(child.getName().equals("subgoal"))// Register it as we go to allow for evaluation for implied goals
        ModelAccessor.registerSlave(child);
    }

    // Process declarations for this node. Only update ruleInstanceClass if we declare a class

    // CASE 0: The root of the rule
    if(ruleNode.getName().equals("group") && ruleNode.getParent().getName().equals("compat")) {
      ruleInstanceClass += delimiter + index;
      assert(DebugMsg.debugMsg("RuleWriter:generateDeclaration","Unguarded " + ruleInstanceClass));
      declareRuleInstance(writer, ruleInstanceClass); // Unguarded
    }
    else if(ModelAccessor.hasImplicitGuard(ruleNode, guards)) { // Implicitly guarded rule
      ruleInstanceClass += delimiter + index;
      filter = Collections.singleton("group");
      Set newGuards = ModelAccessor.getOutstandingGuards(ruleNode, guards);
      newGuards.addAll(guards);
      assert(DebugMsg.debugMsg("RuleWriter:generateDeclaration","implicit  " + ruleInstanceClass));
      declareGuardedRuleInstance(writer, ruleInstanceClass, ruleNode, newGuards);
    }
    else if(ModelAccessor.hasExplicitGuard(ruleNode)){
      ruleInstanceClass += delimiter + index;
      filter = Collections.singleton("group");
      Set newGuards = ModelAccessor.getOutstandingGuards(ruleNode, guards);
      newGuards.addAll(guards);
      assert(DebugMsg.debugMsg("RuleWriter:generateDeclaration","explicit  " + ruleInstanceClass));
      declareGuardedRuleInstance(writer, ruleInstanceClass, ruleNode, newGuards);
    }
    else // No new declaration introduced
      result = false;

    // Iterate over children to introduce new declarations for each possible child rule
    children = ruleNode.getChildren();
    childRuleIndex = 0;
    for (int i=0; i< children.size(); i++) {
      IXMLElement child = (IXMLElement) children.elementAt(i);
      if(filter.contains(child.getName()) &&
         generateDeclaration(writer, ruleInstanceClass, child, guards, childRuleIndex)){
        childRuleIndex++;
      }
    }

    ModelAccessor.resetCurrentRule();
    return result;
  }

  /**
   * Provides the implementation of the handleExecute method for this rule. This function
   * is recursive as it descends through child rule instances arising from implicit and
   * explicit guards.
   * @param true if we defined an implementation for this node. Otherwise false.
   */
  private static boolean generateExecution(IndentWriter writer, 
                                           String ruleInstanceClass, 
                                           IXMLElement ruleNode,
                                           Set guards,
                                           int index) throws IOException {
    assert(DebugMsg.debugMsg("RuleWriter:generateExecution","Rule Instance class is " + ruleInstanceClass + " with guards = " + guards));

    ModelAccessor.setCurrentRule(ruleInstanceClass);

    Set filter = defaultFilter;
    boolean result = true;

    if(ruleNode.getName().equals("group") && ruleNode.getParent().getName().equals("compat")) {
      ruleInstanceClass += delimiter + index;
      generateExecuteImplementation(writer, ruleNode, ruleInstanceClass, guards);
    }
    else if (ModelAccessor.hasImplicitGuard(ruleNode, guards)) {
      ruleInstanceClass += delimiter + index;
      Set newGuards = ModelAccessor.getOutstandingGuards(ruleNode, guards);
      newGuards.addAll(guards);
      generateExecuteImplementation(writer, ruleNode, ruleInstanceClass, newGuards);

      // Reset current rule on exit
      ModelAccessor.resetCurrentRule();
      return true; // Terminates on leaf node
    }
    else if (ModelAccessor.hasExplicitGuard(ruleNode)) {
      ruleInstanceClass += delimiter + index;
      filter = Collections.singleton("group");
      guards = new HashSet(guards);
      guards.add(ModelAccessor.getExplicitGuard(ruleNode));
      generateExecuteImplementation(writer, ruleNode.getFirstChildNamed("group"), ruleInstanceClass, guards);
    }
    else
      result = false; // No execution implementation generated.

    // Recurse to children to introduce new declarations
    Vector children = ruleNode.getChildren();
    int childRuleIndex = 0;
    for (int i=0; i< children.size(); i++) {
      IXMLElement child = (IXMLElement) children.elementAt(i);
      if(filter.contains(child.getName()) &&
         generateExecution(writer, ruleInstanceClass, child, guards, childRuleIndex)) {
        childRuleIndex++;
      }
    }

    ModelAccessor.resetCurrentRule();
    return result;
  }

  /**
   * Provides actual implementation code for the handleExecute method of a
   * RuleInstance
   * @param writer Output destination
   * @param ruleBody The XML node from which to derive the rule
   * @param ruleInstanceClass The RuleInstanceClass for which this method is defined.
   * @param guards An enumerated set of guard variable indetifiers
   */
  private static void generateExecuteImplementation(IndentWriter writer, 
                                                    IXMLElement ruleBody, 
                                                    String ruleInstanceClass,
                                                    Set guards) throws IOException {
    assert(DebugMsg.debugMsg("RuleWriter:generateExecutionImplementation","Generating execution implementation for rule instance " + ruleInstanceClass + " with current guards = " + guards));
    ModelAccessor.setCurrentRule(ruleInstanceClass);
    writer.write("void " + ruleInstanceClass + "::handleExecute() {\n");
    writer.indent();

    if(ruleBody.getName().equals("group")) {

      // Declare local variables
      addLocalVariables(writer, ruleBody, ruleInstanceClass);

      // Introduce slaves first, as they add to the scope
      generateSubgoals(writer, ruleBody, ruleInstanceClass); 

      // Apply filters, now that the full scope is in place
      applyVariableFilters(writer, ruleBody, ruleInstanceClass);

      // Process constraints. Write out those for which there are no implied guards only
      generateUnguardedConstraints(writer, ruleBody, ruleInstanceClass, guards);

      // Handle the foreach behavior
      generateLoopCode(writer, ruleBody, ruleInstanceClass);

      // Create child rules arising from implicit or explicit guards
      generateChildRules(writer, ruleBody, ruleInstanceClass, guards);
    }
    else if (ruleBody.getName().equals("invoke")) {
      ConstraintWriter.generateConstraint(writer, ruleBody, "rule", ruleInstanceClass);
    }
    else if (ruleBody.getName().equals("subgoal")) {
      // Introduce slaves first, as they add to the scope
      generateSubgoals(writer, ruleBody, ruleInstanceClass);
    }
    else if (ruleBody.getName().equals("if")) { // Implied guards on conditional
      Set guardScope = ModelAccessor.getOutstandingGuards(ruleBody, guards);
      writer.write("addChildRule(new "+ ruleInstanceClass + 
                   "$0(m_id, getVariables(\"" + NddlUtil.toDelimitedString(guardScope, ":") + "\"), true));\n");
    }

    writer.unindent();
    writer.write("}\n");

    ModelAccessor.resetCurrentRule();
  }

  public static Set getRules(){
    return s_rules;
  }

  private static int allocateCounter(){
    return s_counter++;
  }

  private static void addLocalVariables(IndentWriter writer, IXMLElement ruleBody, String klass) throws IOException {
    Vector localVariables = ruleBody.getChildrenNamed("var");
    for(int i=0;i<localVariables.size();i++) {
      IXMLElement variable = (IXMLElement) localVariables.elementAt(i);
      ModelAccessor.registerVariable(variable);

      String guarded = "false";
      String type = ModelAccessor.getVariableType(variable);
      String name = ModelAccessor.getVariableName(variable);

      if(ModelAccessor.isGuarded(name, ruleBody))
        guarded = "true";

      // If the type is not a class, then we must introduce a local variable, which may be specified
      if(!ModelAccessor.isClass(type)) {
        String extraCall = "";
        if(ModelAccessor.isEnumeration(type))
          extraCall = "BaseDomain";
        writer.write("localVar(" + ModelAccessor.getDomain(type) + extraCall + "(), "+name+", "+guarded+");\n");
      }
      else { // Now we are handling a class variable, so lets declare it
        boolean leaveOpen = false;
        if(variable.hasAttribute("filterOnly") && XMLUtil.getAttribute(variable, "filterOnly").equals("true"))
          leaveOpen = true;
        writer.write("objectVar("+ModelAccessor.getCppClass(type)+", "+name+", "+guarded+", "+leaveOpen+");\n");
      }
    }
  }

  private static void applyVariableFilters(IndentWriter writer, IXMLElement ruleBody, String klass) throws IOException {
    Vector localVariables = ruleBody.getChildrenNamed("var");
    for(int i=0;i<localVariables.size();i++) {
      IXMLElement variable = (IXMLElement) localVariables.elementAt(i);
      String type = XMLUtil.getAttribute(variable, "type");

      // If it is a class, permit filters to be applied
      if(ModelAccessor.isClass(type))
        ConstraintWriter.generateConstraints(writer, variable, "rule", klass);
    }
  }

  /**
   * @Brief Generate subgoals defined in a compatability. This is the implementation generator
   * for the fire method of a rule. The format we expect in nddl is: <relation>([<prefix>.]<predicate>[ name]);
   *
   * Correct Examples:
   * 1. meets(Going g);
   * 2. meets(object.Going g);
   * 3. meets(this.object.Going g);
   * 4. meets(foo.Bas b); // Predicate parameter variable is a set of objects.
   * 5. meets(foo.bar.Bax b); // Multi-level reference (not implemented).
   * 6. a meets b;
   * 7. a meets(Going g);
   *
   * Error cases:
   * 1. Invlaid relation
   * 2. Slave name collision
   * 3. Invalide predicate match
   */
  private static void generateSubgoals(IndentWriter writer, IXMLElement ruleBody, String klass) throws IOException {
    Vector subgoals = ruleBody.getChildrenNamed("subgoal");

    Set allSlaves = new HashSet(); // Used to detect duplicates. Should be depreceated when semantic processor is in place.

    for(int i=0; i<subgoals.size(); i++) {
      IXMLElement subgoal = (IXMLElement) subgoals.elementAt(i);
      String relation = XMLUtil.getAttribute(subgoal, "relation");

      // Process for errors

      // Test for invalid relation
      if(!ModelAccessor.isSlaveRelation(relation)) {
        XMLUtil.reportError(writer, "Invalid slave relation - " + relation);
        continue;
      }

      String origin = "this"; // By default, the implied relation
      String target = "ERROR";
      // If there is an explicit origin i.e. origin meets target;
      if(subgoal.hasAttribute("origin"))
        origin = XMLUtil.getAttribute(subgoal, "origin");

      // There is always a target. It may be an existing
      boolean slaveExists = subgoal.hasAttribute("target");

      // If the slave exists, update the target with existing slave name
      if(slaveExists)
        target = XMLUtil.getAttribute(subgoal, "target");
      else {
        target = allocateSlave(writer, subgoal, relation);
        if(allSlaves.contains(target)) //  A violation, name already used
          XMLUtil.reportError(writer, "Slave already created for the name " + target+"\n");
        else
          allSlaves.add(target);
      }

      // Now generate the necessary relations.
      if(origin == target) // Modelling bug
        writer.write("#error NddlCompiler Error: Origin and target must be different for temporal relations.");
      else if(!relation.equals("any")) // Generate temporal relation constraint if necessary
        writer.write(relation + "(" + origin + ", " + target + ");\n");
      else if(!origin.equals("this")) // Modelling bug!
        writer.write("#error NddlCompiler Error: Can only use 'any' relation with 'this' as the origin");
    } // End for
  } // End generateSubGoals

  /**
   * Generate code to introduce a new slave.
   * @return Returns the slave name introduced
   */
  private static String allocateSlave(IndentWriter writer, IXMLElement subgoal, String relation) throws IOException {
    XMLUtil.checkExpectedNode("subgoal", subgoal);

    IXMLElement slave = subgoal.getFirstChildNamed("predicateinstance");

    // A default name will be provided if the slave is unnamed
    String slaveName = "slave"+s_slaveCount++;
    if(slave.hasAttribute("name"))
      slaveName = XMLUtil.getAttribute(slave, "name");

    // Get the slaveType from XML node.
    String slaveType = XMLUtil.getAttribute(slave, "type");

    // Test for erroneous use of 'this
    int index = slaveType.indexOf("this");
    if( index == 0) {
      XMLUtil.reportError(writer, "Cannot use 'this' to designate a same object constraint. Use 'object.' instead.");
      return "ERROR";
    }

    boolean constrainObjectVariable = ModelAccessor.isConstrained(slave);
    String predicateObjectType = ModelAccessor.getSlaveObjectType(slave);
    // The predicate name is the unqualified slaveType.
    String predicate = slaveType.substring(slaveType.lastIndexOf(".") + 1);

    predicate = predicate.substring(predicate.lastIndexOf(".") + 1);

    String nddlClass = predicateObjectType + "." + predicate;

    String cppClass;
    if(ModelAccessor.isPredefinedClass(nddlClass))
      cppClass = ModelAccessor.getCppClass(nddlClass);
    else
      cppClass = ModelAccessor.getCppClass(predicateObjectType) + "::" + predicate;

    // If on the same object, use a special form to allocate a local slave that uses the type
    // of the current object and also constrain the object variable.
    if(slaveType.indexOf(".") == -1 || (slaveType.indexOf("object.") == 0 && slaveType.lastIndexOf(".") == 6))
      writer.write("localSlave(" + predicate + ", " + slaveName + ", \"" + relation + "\");\n");
    else 
      writer.write("slave(" + cppClass + ", " + nddlClass + ", " + slaveName + ", LabelStr(\"" + relation + "\"));\n");

    // Constrain the object variable if necessary
    if(constrainObjectVariable) {
      String suffix = "object";

      // If it has a delimiter, extract the prefix
      if(slaveType.indexOf(".") > -1) 
        suffix = slaveType.substring(0, slaveType.lastIndexOf("."));

      if(suffix.indexOf(".") < 0)
        writer.write("sameObject(" + suffix + ", " + slaveName + ");\n");
      else {
        String prefix = suffix.substring(0, suffix.indexOf("."));
        suffix = suffix.substring(suffix.indexOf(".") + 1, suffix.length());
        writer.write("constrainObject(" + prefix + " ," + suffix + "," + slaveName + ");\n");
      }
    }

    return slaveName;
  }

  /**
   * Cases of Intereset:
   * 1. Implicit guard by accessing field of an object variable in a constraint
   * 2. Explicit singleton guard
   * 3. Explicit conditional expression
   * 4. Anything else - just ignore
   */
  private static void generateChildRules(IndentWriter writer, 
                                         IXMLElement ruleBody, 
                                         String ruleInstanceClass,
                                         Set guards) throws IOException{
    Vector children = ruleBody.getChildren();
    int childIndex = 0;
    for (int i=0;i<children.size();i++) {
      IXMLElement child = (IXMLElement) children.elementAt(i);

      // Process child rule with implied guard(s) on constraint arguments
      if(ModelAccessor.hasImplicitGuard(child, guards)) {
        Set guardScope = ModelAccessor.getOutstandingGuards(child, guards);
        writer.write("addChildRule(new "+ ruleInstanceClass + 
            delimiter + childIndex++ + "(m_id, getVariables(\"" + NddlUtil.toDelimitedString(guardScope, ":") + "\"),true));\n");
      }
      else if(child.getName().equals("if")) {
        if(ModelAccessor.guardedBySingleton(child.getChildAtIndex(0))) {
          String variable = SharedWriter.processId(child.getChildAtIndex(0), ruleInstanceClass);
          writer.write("addChildRule(new " + ruleInstanceClass +  
              delimiter + childIndex++ + "(m_id, makeScope("+variable+"),true));\n");
        }
        else {
          XMLUtil.checkExpectedNode("equals:nequals", child.getChildAtIndex(0));
          IXMLElement guardCondition = (IXMLElement) child.getChildAtIndex(0);
          String variable = SharedWriter.processId(guardCondition.getChildAtIndex(0), ruleInstanceClass);
          // We might have a sinlge value, an interval, or a set. Have to handle all
          String value = ModelAccessor.makeDomainFromConstant(guardCondition.getChildAtIndex(1));
          if(guardCondition.getName().equals("equals")) {
            writer.write("addChildRule(new " + ruleInstanceClass +  
                delimiter + childIndex++ +"(m_id, "+variable+", " +value+", true));\n");
          }
          else {
            writer.write("addChildRule(new " + ruleInstanceClass +  
                delimiter + childIndex++ +"(m_id, "+variable+", " +value+", false));\n");
          }
        }
      }
    }
  }

  /**
   * Iterates over all constraints in a group and generates the constraint in cases where it is
   * unguarded.
   */
  private static void generateUnguardedConstraints(IndentWriter writer, 
                                                   IXMLElement ruleBody, 
                                                   String ruleInstanceClass,
                                                   Set guards) throws IOException {
    XMLUtil.checkExpectedNode("group", ruleBody);
    Vector children = ruleBody.getChildrenNamed("invoke");
    for (int i=0;i<children.size();i++) {
      IXMLElement constraint = (IXMLElement) children.elementAt(i);
      if(!ModelAccessor.hasImplicitGuard(constraint, guards))
        ConstraintWriter.generateConstraint(writer, constraint, "rule", ruleInstanceClass);
    }
  }

  /**
   * The following NDDL sample illustrates the type of code we must handle:
   * --------------------------
   * foreach (t in trackable) {
   *  Trackable t_other;
   *  neq(t_other, t); // Treat as a local loop variable
   *  contains(Target.Tracking target);
   *  eq(target.object, t.target);
   * };
   */
  private static void generateLoopCode(IndentWriter writer, IXMLElement ruleBody, String ruleInstanceClass) throws IOException {
    Vector loops = ruleBody.getChildrenNamed("loop");
    for(int i = 0; i<loops.size(); i++) {
      IXMLElement loop = (XMLElement) loops.elementAt(i);
      IXMLElement block = loop.getFirstChildNamed("group");
      // loops without bodies are ignored.
      if(block == null) continue;

      ModelAccessor.registerVariable(loop);

      // Enforce assumption that loops cannot have child rules - so no guards
      Vector childRules = block.getChildrenNamed("if");
      if(childRules.size() > 0) {
        System.err.println("Error in intermediate code: NO GUARDS SUPPORTED WITHIN 'foreach'");
        writer.write("#error NddlCompiler Error: NO GUARDS SUPPORTED WITHIN 'foreach'\n");
      }

      // Obtain the loop information
      String loopVariableName = XMLUtil.getAttribute(loop, "value");
      String loopVariableType = 
        ModelAccessor.getTypeForScopedVariable(ModelAccessor.getCurrentScope() + "." + loopVariableName);

      if(loopVariableType == null)
        loopVariableType = ModelAccessor.getClass(loopVariableName);

      if(loopVariableType == null) {
        writer.write("#error NddlCompiler Error: Only works to loop over local variables. '" + 
            loopVariableName + "' is not a local variable\n");
        return;
      }

      String objectId = XMLUtil.getAttribute(loop, "name");
      writer.write("{\n");
      writer.indent();
      writer.write("// Create a local domain based on the " + loopVariableName + " objects\n");
      writer.write("const ObjectDomain& foreach_" + loopVariableName + " =\n");
      writer.indent();
      writer.write("static_cast<const ObjectDomain&>(var(getId(), std::string(\""+
                   loopVariableName+"\"))->derivedDomain()); \n\n");
      writer.unindent();

      writer.write("if(!foreach_"+loopVariableName +".isEmpty()){\n");
      writer.indent();
      writer.write("// Post a locking constraint on "+loopVariableName+"\n");
      writer.write("{\n");
      writer.indent();
      writer.write("std::vector<ConstrainedVariableId> loop_vars;\n");
      writer.write("loop_vars.push_back(var(getId(), std::string(\""+loopVariableName+"\")));\n");
      writer.write("loop_vars.push_back(ruleVariable(foreach_"+loopVariableName+"));\n");
      writer.write("rule_constraint(Lock, loop_vars);\n");
      writer.unindent();
      writer.write("}\n");

      writer.write("std::list<double> foreach_"+loopVariableName+"_values;\n");
      writer.write("foreach_"+loopVariableName+".getValues(foreach_"+loopVariableName+"_values);\n");

      writer.write("// Translate into a set ordered by key to ensure reliable ordering across runs\n");
      writer.write("ObjectSet foreach_"+loopVariableName+"_valuesByKey;\n");
      writer.write("for(std::list<double>::iterator it=foreach_"+loopVariableName+"_values.begin();\n");
      writer.indent();
      writer.write("it!=foreach_"+loopVariableName+"_values.end(); ++it){\n");
      writer.write(loopVariableType+"Id t = *it;\n");
      writer.write("foreach_"+loopVariableName+"_valuesByKey.insert(t);\n");
      writer.unindent();
      writer.write("}\n");

      writer.write("// Process slaves\n");
      writer.write("for(ObjectSet::const_iterator it=foreach_"+
                   loopVariableName+"_valuesByKey.begin();\n");
      writer.indent();
      writer.write("it!=foreach_"+loopVariableName+"_valuesByKey.end(); ++it){\n");
      writer.write(loopVariableType+"Id "+objectId+" = *it;\n");
      writer.write("check_error("+objectId+".isValid());\n");
      writer.write("// Allocate a local variable for this singleton object\n");
      writer.write("loopVar(" + loopVariableType + ", " + objectId + ");\n");

      // Now we should extract the inner group of the loop, and try to generate appropriate
      // subgoals and constraints.
      IXMLElement innerGroup = (IXMLElement) loop.getFirstChildNamed("group");
      addLocalVariables(writer, innerGroup, ruleInstanceClass);
      generateSubgoals(writer, innerGroup, ruleInstanceClass); // Sub-goal before posting constraints since it impacts scope
      applyVariableFilters(writer, innerGroup, ruleInstanceClass);
      // This is not quite ready, but nested foreach should be legal soon.
      //generateLoopCode(writer, innerGroup, ruleInstanceClass);
      ConstraintWriter.generateConstraints(writer, innerGroup, "rule", ruleInstanceClass);

      writer.write("clearLoopVar(\"" + objectId + "\");\n");
      writer.unindent();
      writer.write("}\n");
      writer.unindent();
      writer.write("}\n");
      writer.unindent();
      writer.write("}\n");

    }
  }

  private static Vector getPredicateVariables(IXMLElement rule) {
    String predicateName = XMLUtil.getAttribute(rule, "class") + "." + XMLUtil.getAttribute(rule, "name");
    IXMLElement predicate = ModelAccessor.getPredicate(predicateName);

    if(predicate == null)
      throw new RuntimeException("Failed to find predicate " + predicateName + ".");

    Vector variables = new Vector();
    variables = predicate.getChildrenNamed("var");
    return variables;
  }

  private static void declareRuleInstance(IndentWriter writer, String ruleInstanceClass) throws IOException {
    // Declare the root context
    assert(DebugMsg.debugMsg("RuleWriter:declareRuleInstance","Generating declaration for " + ruleInstanceClass));

    writer.write("class " + ruleInstanceClass + ": public RuleInstance {\n");
    writer.write("public:\n");
    writer.indent();
    writer.write(ruleInstanceClass + "(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)\n");
    writer.write(": RuleInstance(rule, token, planDb){}\n");
    writer.write("void handleExecute();\n");
    writer.unindent();
    writer.write("};\n\n");
  }

  private static void declareGuardedRuleInstance(IndentWriter writer, 
                                                 String ruleInstanceClass,
                                                 IXMLElement node,
                                                 Set guards) throws IOException {
    XMLUtil.checkExpectedNode("if:invoke", node);

    assert(DebugMsg.debugMsg("RuleWriter:declareGuardedRuleInstance","Generating guarded declaration for " + ruleInstanceClass + " with guards " + guards));

    ModelAccessor.setCurrentRule(ruleInstanceClass);

    // Declare the root context
    writer.write("class " + ruleInstanceClass + ": public RuleInstance {\n");
    writer.write("public:\n");
    writer.indent();

    if((node.getName().equals("invoke") && ModelAccessor.hasImplicitGuard(node, guards)) || 
       ModelAccessor.guardedBySingleton(node.getChildAtIndex(0))) {
      writer.write(ruleInstanceClass + "(const RuleInstanceId& parent, const std::vector<ConstrainedVariableId>& vars, const bool positive)\n");
      writer.write(": RuleInstance(parent, vars, positive){}\n");
    }
    else { // Must be guarded by a membership test
      writer.write(ruleInstanceClass + "(const RuleInstanceId& parent, const ConstrainedVariableId& var, const AbstractDomain& domain, const bool positive)\n");
      writer.write(": RuleInstance(parent, var, domain, positive){}\n");
    }

    writer.write("void handleExecute();\n");
    writer.unindent();
    writer.write("};\n\n");

    ModelAccessor.resetCurrentRule();
  }

  /**
   * TODO: Extend for different types
   */
  private static String getValue(IXMLElement value) {
    String result = ModelAccessor.getValue(value);
    if (value.getName().equals("symbol") || value.getAttribute("type", "").equals("string"))
      return "LabelStr(\"" + XMLUtil.escapeQuotes(result) + "\")";

    if(!value.getName().equals("id") || result.equals("true") || result.equals("false"))
      return result;

    return "#error NddlCompiler Error: Bad Value in rule for " + result;
  }

  /**
   * Obtain the guard variable for an 'if' statement
   */
  static private String getExplicitGuard(IXMLElement ruleNode){
    XMLUtil.checkExpectedNode("if", ruleNode);
    IXMLElement guardVar = (ruleNode.getChildAtIndex(0).getName().equals("equals") ?
                             ruleNode.getChildAtIndex(0).getChildAtIndex(0) :
                             ruleNode.getChildAtIndex(0));
    return XMLUtil.getAttribute(guardVar, "name");
  }
}
