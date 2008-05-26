package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

class ClassWriter {
	public static final Set primitives = NddlUtil.immutableSet(new String[]{"int", "float", "bool"});

  public static void writeTypedefs(IndentWriter writer, IXMLElement klass) throws IOException {
    String name = ModelAccessor.getClassName(klass);

    if(!ModelAccessor.isPredefinedClass(name)){
      SharedWriter.writeTypedefs(writer, name);
      writer.write("typedef ObjectDomain " + name + "Domain;\n\n");
    }
  }

  public static void generateImplementation(IndentWriter writer, IXMLElement klass) throws IOException {
    String superClass = ModelAccessor.getSuperClass(klass);
    String name = XMLUtil.getAttribute(klass,"name");

    if(ModelAccessor.isPredefinedClass(name)){
	writer.write("// SKIPPING IMPLEMENTATION FOR BUILT-IN CLASS " + name + "\n\n");
	return;
    }

    ModelAccessor.setCurrentObjectType(name);

    String longname = XMLUtil.qualifiedName(klass);

    boolean extendsBuiltIn = ModelAccessor.isPredefinedClass(superClass);

    String superCppClass = ModelAccessor.getCppClass(superClass);

    writer.write("\n");
    SharedWriter.generateFileLocation(writer, klass);
    writer.write(longname+"::"+name+"(const PlanDatabaseId& planDatabase, const LabelStr& name)\n");
    if(extendsBuiltIn)
      writer.write(" : "+superCppClass+"(planDatabase, \""+name+"\", name, true)");
    else
      writer.write(" : "+superCppClass+"(planDatabase, \""+name+"\", name)");
    writer.write(" {\n");
    writer.write("}\n");

    writer.write(longname+"::"+name+"(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name)\n");
    if(extendsBuiltIn)
      writer.write(" : "+superCppClass+"(planDatabase, type, name, true)");
    else
      writer.write(" : "+superCppClass+"(planDatabase, type, name)");
    writer.write(" {\n");
    writer.write("}\n");

    writer.write(longname+"::"+name+"(const ObjectId& parent, const LabelStr& name)\n");
    if(extendsBuiltIn)
      writer.write(" : "+superCppClass+"(parent, \""+name+"\", name, true)");
    else
      writer.write(" : "+superCppClass+"(parent, \""+name+"\", name)");
    writer.write(" {}\n");

    writer.write(longname+"::"+name+"(const ObjectId& parent, const LabelStr& type, const LabelStr& name)\n");
    if(extendsBuiltIn)
      writer.write(" : "+superCppClass+"(parent, type, name, true)");
    else
      writer.write(" : "+superCppClass+"(parent, type, name)");
    writer.write(" {}\n");
   
    Vector classVariables = klass.getChildrenNamed("var");
    SharedWriter.defineHandleDefaults(writer, klass, classVariables);

    generateChildren(writer, klass);
    ModelAccessor.resetCurrentObjectType();
  }

    private static void generateChildren(IndentWriter writer, IXMLElement parent) throws IOException {
	for (Enumeration e = parent.enumerateChildren() ; e.hasMoreElements() ; ){
	    IXMLElement element = (IXMLElement)e.nextElement();
	    if (element.getName().equals("enum")) {
		EnumerationWriter.generateImplementation(writer, element);
	    } else if (element.getName().equals("predicate")) {
		PredicateWriter.generateImplementation(writer, element);
	    } else if (element.getName().equals("var")) {
	    } else if (element.getName().equals("constructor")) {
		defineConstructor(writer, element);
		generateFactory(writer, element);
	    } else {
		System.err.println("generateImplementation:"); XMLUtil.dump(element);
	    }
	}
    }

  static public void declareConstructor(IndentWriter writer, IXMLElement constructor) throws IOException {
    IXMLElement klass = constructor.getParent();
    SharedWriter.generateFileLocation(writer, klass);
    if (klass == null) {
      throw new RuntimeException("constructor not contained in a class");
    }
    String name = XMLUtil.getAttribute(klass,"name");
    writer.write("virtual void ");
    generateSignature(writer, constructor);
    writer.write(";\n");
  }

  public static void defineConstructor(IndentWriter writer, IXMLElement constructor) throws IOException {
    assert(DebugMsg.debugMsg("ClassWriter","Begin ClassWriter::defineConstructor"));
    IXMLElement klass = constructor.getParent();
    SharedWriter.generateFileLocation(writer, klass);

    if (klass == null) {
      throw new RuntimeException("missing class for constructor");
    }

    String longname = XMLUtil.nameOf(klass);
    writer.write("void "+longname+"::");
    generateSignature(writer, constructor);
    writer.write(" {\n");
    writer.indent();

    // If the constructor has a call to the super class, invoke it.
    assert(DebugMsg.debugMsg("ClassWriter","ClassWriter::defineConstructor -  getting children named 'super'"));
    Vector superCalls = constructor.getChildrenNamed("super");
    assert(DebugMsg.debugMsg("ClassWriter","ClassWriter::defineConstructor -  " + superCalls.size() + " children named retrieved"));
    if(! superCalls.isEmpty()){
	assert(DebugMsg.debugMsg("ClassWriter","ClassWriter::defineConstructor -  calling super"));

      if(superCalls.size() > 1)
	writer.write("!ERROR: AT MOST ONE CALL TO SUPER ALLOWED\n");

      IXMLElement superCall = (IXMLElement) superCalls.elementAt(0);
      String superClass = ModelAccessor.getParentClassName(klass);
      String superCppClass = ModelAccessor.getCppClass(superClass);
      writer.write(superCppClass + "::constructor(");
      String comma = "";
      Enumeration arguments = superCall.enumerateChildren();
      while(arguments.hasMoreElements()){
	IXMLElement argument = (IXMLElement) arguments.nextElement();
	String value = ModelAccessor.getValue(argument);
	if(argument.getName().equals("value") && XMLUtil.getAttribute(argument, "type").equals("string"))
	    writer.write(comma + "\"" + value + "\"");
	else
	    writer.write(comma + value);
	comma = ", ";
      }
      writer.write(");\n");
    }

    Set allocatedMemberVariables = new HashSet(); // Store names as we go for reference

    /* Now capture names of constructor arguments */
    Set constructorArguments = new HashSet(); // Store for names of constructor arguments
    {
	Vector args = constructor.getChildrenNamed("arg");
	assert(DebugMsg.debugMsg("ClassWriter","ClassWriter::defineConstructor -  getting " + args.size() + " arguments"));
	for(int i=0;i<args.size();i++){
	    IXMLElement arg = (IXMLElement)args.elementAt(i);
	    String argName = XMLUtil.getAttribute(arg,"name");
	    constructorArguments.add(argName);
	}
    }

    Vector constructorAssignments = constructor.getChildrenNamed("assign");
    Vector classVariables = klass.getChildrenNamed("var");

    //Use the set below  to track when an assignment is being made more than once for same variable
    Set assignmentsMade = new HashSet(); 

    for(int i=0;i<constructorAssignments.size(); i++){
      IXMLElement element = (IXMLElement)constructorAssignments.elementAt(i);
      String target = XMLUtil.getAttribute(element,"name");

      if(assignmentsMade.contains(target)){
	  writer.write("!ERROR: Duplicate assignment for " + target + "\n");
      }
      else {
	  assignmentsMade.add(target);
	  //String type = getVariableType(classVariables, target);
		//trust in the parser, for it will assign the correct types
		String type = XMLUtil.getAttribute(element,"type");
	  IXMLElement sourceElement = element.getChildAtIndex(0);

	  if (sourceElement.getName().equals("new")) { // Handle object allocation
	      assert(DebugMsg.debugMsg("ClassWriter","ClassWriter::defineConstructor -  allocating object for " + target));
	      String sourceType = XMLUtil.typeOf(sourceElement);
	      writer.write(target+" = addVariable(");
	      writer.write(sourceType+"Domain((new "+sourceType+"(m_id, \""+target+"\"))->getId(), \""+sourceType+"\")");
	      writer.write(", " + makeObjectVariableNameString(target) + ");\n");
	      writer.write("Id<" + type + ">(singleton("+target+"))->constructor(" + 
			   buildArguments(classVariables, 
					  constructorArguments, 
					  allocatedMemberVariables, 
					  sourceElement) +");\n"); // Invoke initialization
	      writer.write("Id<" + type + ">(singleton("+target+"))->handleDefaults();\n"); // Default variable setup
	  }
	  else { // Handle variable allocation
	      String value = ModelAccessor.getValue(sourceElement);

	      if(sourceElement.getName().equals("id") && type.equals(NddlXmlStrings.x_string))
		  value = XMLUtil.escapeQuotes(value);
	      else if(sourceElement.getName().equals(NddlXmlStrings.x_symbol) || XMLUtil.getAttribute(sourceElement,"type").equals(NddlXmlStrings.x_string))
		  value = "LabelStr(\""+XMLUtil.escapeQuotes(value)+"\")";
	      else if(ModelAccessor.isNumericPrimitive(type) && !type.equals(NddlXmlStrings.x_boolean)) {
		  // Set both bounds to singleton value
		  value = value + ", " + value;
	      }

	      writer.write(target+" = addVariable(");

	      if(allocatedMemberVariables.contains(value)) // If we are assigning one member to another, we obtain the base domain for it
		  writer.write(value+"->baseDomain()");
	      else
		  writer.write(ModelAccessor.getDomain(type) + "("+value +", \""+type+"\")");

	      writer.write(", "+ makeObjectVariableNameString(target)+");\n");
	  }
	  // Add member to the set
	  allocatedMemberVariables.add(target);
      }
    }

    writer.unindent();
    writer.write("}\n");
    assert(DebugMsg.debugMsg("ClassWriter","End ClassWriter::defineConstructor"));
  }

    /**
     * Test if the given variable name is listed in the vector
     * of variable xml elements
     */
    static private boolean isMember(Vector classVariables, String name){
	for(int i=0;i<classVariables.size();i++){
	    IXMLElement variable = (IXMLElement) classVariables.elementAt(i);
	    if(XMLUtil.nameOf(variable).equals(name))
		return true;
	}
	return false;
    }

  static private String getVariableType(Vector classVariables, String name){
    String type = "!ERROR";
    for(int i=0;i<classVariables.size();i++){
      IXMLElement variable = (IXMLElement) classVariables.elementAt(i);
      if(XMLUtil.nameOf(variable).equals(name)){
	type = XMLUtil.getAttribute(variable, "type");
	String _class = XMLUtil.getAttribute(variable.getParent(), "name");
	if(ModelAccessor.isEnumeration(_class + "::" + type))
	   type = _class + "::" + type;
	break;
      }
    }
    return type;
  }

    /**
     * @brief Generates the signature for the class constructors
     */
  static private void generateSignature(IndentWriter writer, IXMLElement constructor) throws IOException  {
    assert(DebugMsg.debugMsg("ClassWriter","Begin ClassWriter::generateSignature"));
    writer.write("constructor(");
    boolean comma = false;
    for (Enumeration e = constructor.enumerateChildren() ; e.hasMoreElements() ; ) {
      IXMLElement element = (IXMLElement)e.nextElement();
      if (!element.getName().equals("arg")) {
	break;
      }
      String argType = XMLUtil.typeOf(element);
      String argName = XMLUtil.getAttribute(element,"name");
      if (comma) {
	writer.write(", ");
      }

      // Argument are structured as: <arg name="argName" type="argType"/>
      if(argType.equals(NddlXmlStrings.x_int))
	writer.write("int "+argName);
      else if(argType.equals(NddlXmlStrings.x_float))
	writer.write("float "+argName);
      else if(argType.equals(NddlXmlStrings.x_boolean))
	writer.write("bool "+argName);
      else if(argType.equals(NddlXmlStrings.x_string) || argType.equals(NddlXmlStrings.x_symbol)
	      || ModelAccessor.isEnumeration(argType))
	writer.write("const LabelStr& "+argName);
      else
	  writer.write("const "+argType+"Id& "+argName);

      comma = true;
    }
    writer.write(")");
    assert(DebugMsg.debugMsg("ClassWriter","End ClassWriter::generateSignature"));
  }

    /**
     * Builds the argument invocation string for calling a constructor
     * @param classVariables The collection of member variables as XMLElements
     * @param constructorArguments The set of names for constructor arguments
     * @param allocatedMemberVariables The set of member variables already allocated
     * @param element The specific 'new' xml element
     */
    static private String buildArguments(Vector classVariables, 
					 Set constructorArguments, 
					 Set allocatedMemberVariables, 
					 IXMLElement element) throws IOException  {
    XMLUtil.checkExpectedNode("new", element);

    boolean comma = false;
    String arguments = "";
    for (Enumeration e = element.enumerateChildren() ; e.hasMoreElements() ; ) {
      IXMLElement argument = (IXMLElement)e.nextElement();
      String argType = ModelAccessor.getValueType(argument);
      String value = ModelAccessor.getValue(argument);

      /* Exclude cases where we do nothing to the value */
      if(!primitives.contains(argType) && !value.equals("true") && !value.equals("false")){
	// CASE 0: Translate 'this'
	if (value.equalsIgnoreCase("this")){
	  value = "m_id";
	}

	// CASE 1: It is a singleton member variable which must be mapped to the singleton
	else if(allocatedMemberVariables.contains(value)){
	    String singletonType = getVariableType(classVariables, value);
	    if(primitives.contains(singletonType)){
		value = "(" + singletonType + ") singleton(" + value + ")";
	    }
	    else 
		value = "singleton("+value+")";
	}

	// CASE 2: It is a constructor argument
	else if(constructorArguments.contains(value)){ // Do nothing, just use the string as is.
	}

	// CASE 3: Test for error - using a member before initializing it
	else if(!allocatedMemberVariables.contains(value) && isMember(classVariables,value)){
	    value = "!ERROR: Cannot use " + value + " without prior initialization.";
	}

	// Otherwise it is a string or enumerataion
	else if(argType.equals("string") || // It is an enumeartion of some kind
		ModelAccessor.isEnumeration(argType) ||
		argType.equals("symbol")){
	    value = "LabelStr(\"" + XMLUtil.escapeQuotes(value) + "\")";
	}

	// If we fall through to here, there is an error.
	else value = "!ERROR:BAD ASSIGNMENT";
      }

      // CASE 3: It is a true or false value
      if (comma)
	arguments = arguments + ", ";
 
      arguments = arguments + value;
      comma = true;
    }

    return arguments;
  }
  

  static public void declareDefaultConstructor(IndentWriter writer, String superClass) throws IOException {
    // If it is a built in class then define the default 'constructor' call
    if(ModelAccessor.isPredefinedClass(superClass))
      writer.write("virtual void constructor(); // default constructoror\n");
  }

  public static void generateDefaultFactory(IndentWriter writer, IXMLElement klass) throws IOException {
    SharedWriter.generateFileLocation(writer, klass);

    if (klass == null) {
      throw new RuntimeException("missing class for factory");
    }

    String longname = XMLUtil.nameOf(klass) + "Factory" + s_factoryCounter++;
    String klassName = XMLUtil.nameOf(klass);

    writer.write("DECLARE_DEFAULT_OBJECT_FACTORY("+longname+", "+klassName+");\n");

    // Generate all appropriate registration information
    SchemaWriter.addFactory("REGISTER_OBJECT_FACTORY("+longname+", "+klassName+");\n");
  }

  public static void generateFactory(IndentWriter writer, IXMLElement constructor) throws IOException {
    IXMLElement klass = constructor.getParent();
    SharedWriter.generateFileLocation(writer, klass);

    if (klass == null)
      throw new RuntimeException("missing class for factory");

    String longname = XMLUtil.nameOf(klass) + "Factory" + s_factoryCounter++;

    writer.write("class "+longname+": public ConcreteObjectFactory {\n");
    writer.write("public:\n");
    writer.indent();
    writer.write(longname + "(const LabelStr& name): ConcreteObjectFactory(name){}\n");
    writer.unindent();
    writer.write("private:\n");
    writer.indent();
    writer.write("ObjectId createInstance(const PlanDatabaseId& planDb,\n"); 
    writer.write("                        const LabelStr& objectType, \n");
    writer.write("                        const LabelStr& objectName,\n");
    writer.write("                        const std::vector<const AbstractDomain*>& arguments) const {\n");
    writer.indent();

    Vector constructorAssignments = constructor.getChildrenNamed("arg");
    String klassName = XMLUtil.nameOf(klass);
    String constructorArguments = "";
    String comma = "";

    // Do some type checking - at least the size must match!
    writer.write("check_error(arguments.size() == " + constructorAssignments.size() + ");\n");

    // Process arguments, defining local variables and giving them initial values
    for(int i=0;i<constructorAssignments.size(); i++){
      IXMLElement element = (IXMLElement)constructorAssignments.elementAt(i);
      String target = XMLUtil.getAttribute(element,"name");
      String type = XMLUtil.getAttribute(element, "type");
      writer.write("check_error(AbstractDomain::canBeCompared(*arguments["+i+"], \n");
      writer.write("                                          planDb->getConstraintEngine()->getTypeFactoryMgr()->baseDomain(\""+type+"\")), \n");
      writer.write("            \"Cannot convert \" + arguments["+i+"]->getTypeName().toString() + \" to "+type+"\");\n");
      writer.write("check_error(arguments["+i+"]->isSingleton());\n");

      String localVarType = makeArgumentType(type); // Some conversion may be required for Id's or strings or enumerations

      // Declare local variable for current argument
      writer.write(localVarType + " " + target + "(("+localVarType + ")arguments["+i+"]->getSingletonValue());\n\n");
      constructorArguments = constructorArguments + comma + target;
      comma = ", ";
    }

    // Now do the declaration and allocation of the instance
    writer.write(klassName + "Id instance = (new " + klassName + "(planDb, objectType, objectName))->getId();\n");
    writer.write("instance->constructor("+constructorArguments+");\n");
    writer.write("instance->handleDefaults();\n");
    writer.write("return instance;\n");
    writer.unindent();
    writer.write("}\n");
    writer.unindent();
    writer.write("};\n");

    // Generate all appropriate registration informationLabelStr
    Vector factoryNames = makeFactoryNames(constructor);
    for(int i=0;i<factoryNames.size();i++){
      String factoryName = (String) factoryNames.elementAt(i);
      SchemaWriter.addFactory("REGISTER_OBJECT_FACTORY("+longname+", "+factoryName+");\n");
    }
  }

  /**
     * Helper method to generate the signatures for binding the factory. Must account for
     * the inheritance relationships for any arguments that are classes. Ths, each argument can potentially
     * result in a range of values for type, and we have to handle the cross product of all of these.
     */
  static private Vector makeFactoryNames(IXMLElement constructor){
    String factoryName = XMLUtil.nameOf(constructor.getParent());
    Vector allPermutations = new Vector();
    for (Enumeration e = constructor.enumerateChildren() ; e.hasMoreElements() ; ) {
      IXMLElement element = (IXMLElement)e.nextElement();

      if (!element.getName().equals("arg")) {
	break;
      }

      String argType = XMLUtil.typeOf(element);
      Vector argTypeAlternates = new Vector();
      argTypeAlternates.add(argType);
      
      if(ModelAccessor.isClass(argType)){
	List ancestors = ModelAccessor.getAncestors(argType);
	if(ancestors != null)
	  argTypeAlternates.addAll(ancestors);
      }

      allPermutations.add(argTypeAlternates);
    }

    // Generate all the signatures
    Vector factoryNames = new Vector();
    makeFactoryNames(factoryName, allPermutations, 0, factoryNames);
    return factoryNames;
  }

  /**
	 * Tail-recursive algorithm to generate all factory signatures
	 */
  static private void makeFactoryNames(String prefix, Vector arguments, int startIndex, Vector results){

    // if startIndex == arguments.size, then simply add the prefix to the results
    if(startIndex == arguments.size()){
      if(s_allFactorySignatures.contains(prefix))
	throw new RuntimeException("Constructor signature collision detected: Factory already registered with prefix " + prefix);

      results.add(prefix);
      assert(DebugMsg.debugMsg("ClassWriter","makeFactoryNames:adding:"+prefix));
      s_allFactorySignatures.add(prefix);
      return;
    }

    // Otherwise, iterate over each element of arguments, and call recursively with adjusted input
    Vector currentAlternates = (Vector) arguments.elementAt(startIndex);
    for(int i=0;i<currentAlternates.size();i++){
      makeFactoryNames(prefix + ":" + (String) currentAlternates.elementAt(i), arguments, startIndex + 1, results);
    }
  }

  static private String makeObjectVariableNameString(String name){
    String result = "\"" + name + "\"";
    return result;
  }

  static private String makeArgumentType(String argType){
    if(primitives.contains(argType))
      return argType;
    else if(argType.equals("string") || ModelAccessor.isEnumeration(argType))
      return "LabelStr";
    else
      return argType+"Id";
  }


  /**
   * Returns the type for the given member name. We must handle cases of inheritance.
   * @param klass The klass context in which it arises
   * @param memberName The variable name in the class (or parent class
   */
  static public String getMemberVariableType(String klass, String memberName){
    return null;
  }

  // Store names of enumerations as they are declared
  static private int s_factoryCounter = 0; // Used to differentitate factory names
  static private Set s_allFactorySignatures = new HashSet();
}
