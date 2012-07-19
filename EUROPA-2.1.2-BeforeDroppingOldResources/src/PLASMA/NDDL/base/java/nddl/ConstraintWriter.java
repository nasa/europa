package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

class ConstraintWriter{

    /**
     * Generates the Implementation code for a single constraint. 
     * @param writer Destination to write code to.
     * @param constraint XML Element of type 'invoke'
     * @param qualifier rule or token context where constraint might arise
     * @param klass The rule or predicate C++ klass in which this arises
     */
  public static void generateConstraint(IndentWriter writer, IXMLElement constraint, String qualifier, String klass) throws IOException {
      XMLUtil.checkExpectedNode("invoke", constraint);

      // The singleton string is used when constraints require variables to be created since we don't support passing
      // constants to constraints.
      String allocator = "ruleVariable";
      if(qualifier.equals("token"))
	  allocator = "predicateVariable";

      String name = XMLUtil.getAttribute(constraint,"name");

      String macroName = "!ERROR: UNDEFINED";

      // We must build up the invocation commands as we go, since we may also have to allocate variables
      // as we go, and they must precede usage in constraints
      String command = qualifier + "_constraint";

      if (constraint.getChildrenCount() == 0)
	command = "!ERROR: Constraint must have at least one variable as an argument.\n";

      command += "("+ name + ", vars);\n";
      writer.write("{\n");
      writer.indent();
      String vars = "std::vector<ConstrainedVariableId> vars;\n" ;
      // Build up arguments and identify and allocate required filter variables as we go
      for (Enumeration e = constraint.enumerateChildren() ; e.hasMoreElements() ; ) {
	IXMLElement argument = (IXMLElement)e.nextElement();
	vars += "vars.push_back(" + generateArgument(argument, klass, allocator) + ");\n";
      }
      writer.write(vars);
      writer.write(command);
      writer.unindent();
      writer.write("}\n");
  }

  public static void generateConstraints(IndentWriter writer, IXMLElement container, String qualifier, String klass) throws IOException {
    Vector constraints = container.getChildrenNamed("invoke");

    for(int i=0;i<constraints.size();i++) {
	IXMLElement constraint = (IXMLElement) constraints.elementAt(i);
	generateConstraint(writer, constraint, qualifier, klass);
    }
  }

  /** 
   * This is what we are parsing:
   * <var line="51" column="7" name="p" type="Path">
   *  <invoke line="52" column="9" name="eq">
   *   <ident value="from"/>
   *   <id line="52" column="18" name="p.from"/>
   *  </invoke>
   *  <invoke line="53" column="9" name="eq">
   *   <ident value="to"/>
   *   <id line="53" column="16" name="p.to"/>
   *  </invoke>
   * </var>
   *    
   * This is what we would want to generate:
   *
   * declareFilter(p);
   * allocateFilterCondition(Path, p_filter, World::GoingId(tok("g"))->from, from, eq);
   * allocateFilterCondition(Path, p_filter, World::GoingId(tok("g"))->to, from, eq);
   * allocateFilterConstraint(p);
   */
  public static void generateFilter(IndentWriter writer, 
				    IXMLElement container, 
				    Vector constraints, 
				    String klass) throws IOException {
    XMLUtil.checkExpectedNode("var", container);
    String filteredVariable = XMLUtil.getAttribute(container, "name");
    String filteredType = XMLUtil.getAttribute(container, "type");
    writer.write("declareFilter("+filteredType +"," + filteredVariable + ");\n");

    // Now process all constraints. For each we are expecting it to be a binary
    // constraint, where one of the arguments is a field of the object being filtered.
    // We also restrict the possible filter operands to 'eq' for now.

    for(int i=0;i<constraints.size();i++) {
      IXMLElement constraint = (IXMLElement) constraints.elementAt(i);

      // Enforce restriction for 'eq' being the only relation supported.
      String name = XMLUtil.getAttribute(constraint,"name");
      if(!name.equals("eq")){
	writer.write("ERROR! Only 'eq' supported for filtering. " + name + " is not valid.\n");
	continue;
      }

      Vector filterConditions = constraint.getChildren();

      // Enforce restriction that constraint is binary
      if (filterConditions.size() != 2){
	writer.write("ERROR! Only binary equality constraints supported for filtering.\n");
	continue;
      }

      String fieldName = "!ERROR: Invalid field for filtering";
      String filterVar = "!ERROR: Invalid filter variable";
      for(int j=0;j<2;j++){
	// One of the children must conform to the for <filteredVariable>.<fieldName>. This will provide
	// the field index.
	IXMLElement argument = (IXMLElement) filterConditions.elementAt(j);
	assert(DebugMsg.debugMsg("ConstraintWriter","ConstraintWriter:generateFilter processing argument with filtered variable " + filteredVariable));
	if(argument.getName().equals("id") && XMLUtil.getAttribute(argument, "name").indexOf(filteredVariable) == 0 &&
	   XMLUtil.getAttribute(argument, "name").charAt(filteredVariable.length()) == '.') {
          // This gives us the field
	  fieldName = ModelAccessor.getSuffix(".", XMLUtil.getAttribute(argument, "name"));
	  assert(DebugMsg.debugMsg("ConstraintWriter","ConstraintWriter:generateFilter got field " + fieldName));
	  continue;
	}

	// So it must be a standard variable to filter against. All we need is the text for it
	filterVar = generateArgument(argument, klass, "ruleVariable");
	assert(DebugMsg.debugMsg("ConstraintWriter","ConstraintWriter:generateFilter Found filter var " + filterVar));
      }


      String filterVarType = ModelAccessor.getTypeForScopedVariable(filteredType + "." + fieldName);
      List ancestors = ModelAccessor.getAncestors(filteredType);
      int counter=0;
      while(filterVarType == null && counter < ancestors.size()){
	  String ancestor = (String) ancestors.get(counter++);

	  if(ModelAccessor.isPredefinedClass(ancestor))
	      filterVarType = "!ERROR: No field forr " + filteredType + "." + fieldName;

	  filterVarType = ModelAccessor.getTypeForScopedVariable(ancestor + "." + fieldName);
      }

      // allocateFilterCondition(Path, p, World::GoingId(tok("g"))->from, from, eq);
      writer.write("allocateFilterCondition("+filteredVariable+", "+filterVarType+", "+filterVar+", "+ fieldName+", eq);\n");
    }
    //  allocateFilterConstraint(p);
    writer.write("allocateFilterConstraint("+filteredVariable+");\n");
  }

  /**
   * Helper method to construct the text for an argument of a constraint
   */
  private static String generateArgument(IXMLElement argument, String klass, String allocator) throws IOException {
    // If it is an id, process it
    if (argument.getName().equals("id"))
      return SharedWriter.processId(argument, klass);

    // Otherwise, it is a constant, so make a domain out of it
    String domain = ModelAccessor.makeDomainFromConstant(argument);
    return allocator + "(" + domain + ")";
  }

  /**
   * Assumes we are searching from a container  to look ahead within the 
   * context of this rule to search for a node with a tag of <loop> and where the
   * name is of the loop variable is given by 'filteredVariable'. Tail-recursive implementation.
   */
  private static boolean usedInForEach(IXMLElement container, String filteredVariable){
    final Set canIgnoreFromHere = NddlUtil.immutableSet(new String[]{"subgoal", "invoke", "ident", "var"});

    // If we get a hit, return true
    if(container.getName().equals("loop") && XMLUtil.getAttribute(container, "value").equals(filteredVariable))
      return true;

    // If a node is in 'canIgnoreFromHere', we can ignore further exploration
    if(canIgnoreFromHere.contains(container.getName()))
      return false;

    // Otherwise, try exploring the children
    Vector children = container.getChildren();
    for(int i=0;i<children.size();i++){
	IXMLElement child = (IXMLElement) children.elementAt(i);
	if(usedInForEach(child, filteredVariable))
	    return true;
    }

    return false;
  }

}
