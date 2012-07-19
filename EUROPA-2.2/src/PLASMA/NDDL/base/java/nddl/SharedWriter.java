package nddl;

import java.io.*;
import java.util.*;
import net.n3.nanoxml.IXMLElement;

class SharedWriter {

  /**
   * Forward declarations and typedef for Id to allow for specialized access throughout
   */
  public static void writeTypedefs(IndentWriter writer, String name) throws IOException {
    writer.write("class "+name+";\n");
    writer.write("typedef Id<"+name+"> "+name+"Id;\n");
  }

  public static void registerBuiltInClass(IXMLElement element){
    String name = ModelAccessor.getClassName(element);
    String superClass = ModelAccessor.getSuperClass(element);
    String superCppClass = ModelAccessor.getCppClass(superClass);

    // Update the active scope in question
    if(ModelAccessor.isPredicate(element))
      ModelAccessor.setCurrentPredicate(name);
    else
      ModelAccessor.setCurrentObjectType(name);

    if (ModelAccessor.isPredicate(element)) { // Declare constructors for Tokens
      String parentName = ModelAccessor.getPredicateClassName(element);
      if(!ModelAccessor.isPredefinedClass(parentName))
          SchemaWriter.addCommand("addPredicate(\""+parentName + "." + name+"\")");
      ModelAccessor.registerPredicate(element);
    } else if(!name.equals(superClass) && !name.equals("Object") && !name.equals("Timeline")) { // Declare constructors for Objects
      if(!ModelAccessor.isPredefinedClass(name))
          SchemaWriter.addObjectType("addObjectType(\""+name+"\", \""+superClass+"\")");
      ModelAccessor.addParentChildRelation(superClass, name);
    }

    // Forward declare Typedefs
    for (Enumeration e = element.enumerateChildren() ; e.hasMoreElements() ; ) {
      IXMLElement child = (IXMLElement)e.nextElement();
      if (child.getName().equals("predicate")){
        registerBuiltInClass(child);
      }
      else if(child.getName().equals("var"))
        ModelAccessor.registerVariable(child);
    }

    if(ModelAccessor.isPredicate(element))
      ModelAccessor.resetCurrentPredicate();
    else
      ModelAccessor.resetCurrentObjectType();
  }

  /**
   * Class definition for predicates and objects. 
   */
  public static void writeClassDeclaration(IndentWriter writer,
                                           IXMLElement element) throws IOException {

    String name = ModelAccessor.getClassName(element);
    String superClass = ModelAccessor.getSuperClass(element);
    String superCppClass = ModelAccessor.getCppClass(superClass);

    if(ModelAccessor.isPredefinedClass(name)){
      registerBuiltInClass(element);
      writer.write("// SKIPPING DECLARATION FOR BUILT-IN CLASS " + name + "\n\n");
      return;
    }

    generateFileLocation(writer, element);

    // Update the active scope in question
    if(ModelAccessor.isPredicate(element))
      ModelAccessor.setCurrentPredicate(name);
    else
      ModelAccessor.setCurrentObjectType(name);

    writer.write("class "+name+" : public "+superCppClass+" {\n");
    writer.write("public:\n");
    writer.indent();

    if (ModelAccessor.isPredicate(element)) { // Declare constructors for Tokens
      writer.write(name+"(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable = false, bool isFact = false, bool close = false);\n");
      writer.write(name+"(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close = false);\n");
      String parentName = ModelAccessor.getPredicateClassName(element);
      if(!ModelAccessor.isPredefinedClass(parentName))
          SchemaWriter.addCommand("addPredicate(\""+parentName + "." + name+"\")");
    } else { // Declare constructors for Objects
      writer.write(name+"(const PlanDatabaseId& planDatabase, const LabelStr& name);\n");
      writer.write(name+"(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name);\n");
      writer.write(name+"(const ObjectId& parent, const LabelStr& name);\n");
      writer.write(name+"(const ObjectId& parent, const LabelStr& type, const LabelStr& name);\n");
      SchemaWriter.addObjectType("addObjectType(\""+name+"\", \""+superClass+"\")");
      ModelAccessor.addParentChildRelation(superClass, name);
    }

    // generate prototype for handling default variable initialization
    writer.write("void handleDefaults(bool autoClose = true); // default variable initialization\n");

    if (ModelAccessor.isPredicate(element)) // Declare token factory
      PredicateWriter.declareTokenFactory(writer, element);
    else if( element.getFirstChildNamed("constructor") == null) // Make default constructor
      ClassWriter.declareDefaultConstructor(writer, superClass);

    writer.write("\n");

    // Forward declare Typedefs
    for (Enumeration e = element.enumerateChildren() ; e.hasMoreElements() ; ) {
      IXMLElement child = (IXMLElement)e.nextElement();
      if (child.getName().equals("predicate")){
        ModelAccessor.registerPredicate(child);
        writeTypedefs(writer, ModelAccessor.getPredicateName(child));
      }
      else if(child.getName().equals("enum"))
        EnumerationWriter.generateHeader(writer, child);
    }

    // Process children
    for (Enumeration e = element.enumerateChildren() ; e.hasMoreElements() ; ) {
      IXMLElement child = (IXMLElement)e.nextElement();
      if (child.getName().equals("predicate")) // Recursive call
        writeClassDeclaration(writer, child);
      else if(child.getName().equals("var"))
        declareVariable(writer, child);
      else if(child.getName().equals("constructor"))
        ClassWriter.declareConstructor(writer, child);
    }

    writer.unindent();
    writer.write("};\n");

    if(ModelAccessor.isPredicate(element))
      ModelAccessor.resetCurrentPredicate();
    else
      ModelAccessor.resetCurrentObjectType();
  }

  static public void declareVariable(IndentWriter writer, IXMLElement variable) throws IOException {
    ModelAccessor.registerVariable(variable);

    String varName = ModelAccessor.getVariableName(variable);

    // Declare member in C++ class
    writer.write("ConstrainedVariableId " + varName+"; // SchemaWriter::declareVariable");

    writer.write("\n");
  }

  public static void defineHandleDefaults(IndentWriter writer,
                                          IXMLElement element,
                                          Vector variables) throws IOException {   // Generate default initialization code
    String longname = ModelAccessor.getQualifiedClassName(element);
    String name = ModelAccessor.getClassName(element);
    String superClass = ModelAccessor.getSuperClass(element);

    writer.write("// default initialization of member variables\n");
    writer.write("void "+longname+"::handleDefaults(bool autoClose) {\n");
    writer.indent();

    // If we have a superclass, call its default handler
    if(superClass != null && !ModelAccessor.isPredefinedClass(superClass))
      writer.write(superClass + "::handleDefaults(false);\n");

    for (Enumeration e = variables.elements() ; e.hasMoreElements() ; ) {
      IXMLElement variable = (IXMLElement)e.nextElement();
      String var_type = ModelAccessor.getVariableType(variable);
      String var_name = ModelAccessor.getVariableName(variable);
      String functionCall = "addParameter";
      String baseDomain = "";

      // If this is a class declaration and this is is a class variable, it must be initialized
      if(ModelAccessor.isObjectType(element) && ModelAccessor.isClass(var_type)){
        writer.write("check_error("+var_name+".isValid(), \"object variables must be initialized explicitly\");\n\n");
        continue;
      }

      writer.write("if("+var_name+".isNoId()){\n");
      writer.indent();

      if(ModelAccessor.isObjectType(element)){
        functionCall = "addVariable";
      }

      baseDomain = ModelAccessor.getDefaultDomain(variable);

      writer.write(var_name+ " = "+functionCall+"(" + baseDomain + ", \"" + var_name + "\");\n");

      // If it is an objectVariable for a predicate, we need to set up the variable
      if(ModelAccessor.isPredicate(element) && ModelAccessor.isClass(var_type))
        writer.write("completeObjectParam(" + var_type + ", " + var_name + ");\n");

      writer.unindent();
      writer.write("}\n");
    }

    writer.write("if (autoClose)\n");
    writer.indent();
    writer.write("close();\n");
    writer.unindent();

    // If a predicate and it has any constraints declared in-line, generate them
    if(ModelAccessor.isPredicate(element) && !element.getChildrenNamed("invoke").isEmpty()){
      writer.write("\n// Post parameter constraints\n");
      ConstraintWriter.generateConstraints(writer, element, "token", "");
    }

    writer.unindent();
    writer.write("}\n\n");

    // Implicit constructor, if necessary. Only applies to Objects. Also, is only necessary
    // for root user defined classes.
    if (ModelAccessor.isObjectType(element) && element.getFirstChildNamed("constructor") == null) {
      if(ModelAccessor.isPredefinedClass(superClass)) {
        writer.write("// implicit constructor\n");
        writer.write("void "+longname+"::constructor() {\n");
        writer.indent();
        writer.unindent();
        writer.write("}\n\n");
      }
      // Generate Default Factory
      ClassWriter.generateDefaultFactory(writer, element);
    }
  }

  public static void generateFileLocation(IndentWriter writer, IXMLElement element) throws IOException {
    String name = XMLUtil.getAttribute(element,"name");
    String location = XMLUtil.locationString(element);
    writer.write("\n");
    writer.write("// "+location+" "+name+"\n");
  }

  /**
   * Takes an Id XML element and returns the C++code for its name argument. The contexts are:
   * 1. An argument of a constraint. Container == <invoke>
   * 2. An argument for the condition in a guard. Container == <if>
   * 
   * @param writer The output destination
   * @param argument The Id xml element.
   * @param klass The predicate klass on which the rule is written. Used to resolve names for predicate fields.
   */
  public static String processId(IXMLElement argument, String klass) {
    XMLUtil.checkExpectedNode("id", argument);

    // Get the name and start processing it
    String idName = XMLUtil.getAttribute(argument, "name");

    // Do some preprocessing
    if(idName.indexOf("this.") == 0) // Prune reference to 'this' if present
      idName = idName.substring(5, idName.length());
    else if(impliedObjectReference(idName, klass)) // Prepend object reference if implied. For now, error out.
      idName = "ERROR - Must explciitly specify 'object' for class variable access for " + idName;

    // CASE 0: Trivial if we are referencing a field of this predicate/token. All fields will be in the variable
    // scope of the rule instance, and can be retrieved by name.
    if(idName.indexOf(".") < 0)
      return accessToVar(idName);

    // CASE 1: Now we know we are accessing a field variable of an object or a slave token. So, get the parts.
    // Note that it could be of the form a.b or a.b.c.d....
    int index = idName.indexOf(".");
    String prefix = idName.substring(0, index);
    String suffix = idName.substring(index+1, idName.length());

    // Is it an object or a token? Get the right accessor for the prefix
    String type = ModelAccessor.getClass(prefix);

    assert(DebugMsg.debugMsg("SharedWriter","SharedWriter:processId Token type for " + klass + "." + prefix + " is " + type));
    if (type != null){
      String isGuard = "false";
      if(ModelAccessor.isGuarded(idName, argument))
        isGuard = "true";

      // If the parent xml element is an <equals> tag then this is a guard which means if we end up
      // allocating a proxy variable, we will want it to be specifiable
      return "varFromObject(std::string(\"" + prefix + "\"), std::string(\"" + suffix + "\"), " + isGuard + ")";
    }
    else
      return "varfromtok(tok(getId(), std::string(\"" + prefix + "\")), std::string(\"" + suffix+ "\"))";
  }

  /**
   * Generates the code to obtain a variable from the scope of a RuleInstance
   */
  public static String accessToVar(String varName){
    return "var(getId(),std::string(\"" + varName + "\"))";
  }

  /**
   * Check the identifier for a reference that implies the object
   * variable should be a prefix
   */
  private static boolean impliedObjectReference(String idName, String klass) {
    assert(DebugMsg.debugMsg("SharedWriter","ModelAccessor::impliedObjectReference:start:" + idName + ", " + klass));
    if(klass.equals(""))
      return false;

    if(klass == null)
      return false;

    String var = idName;

    // Strip the prefix, if there is one
    int index = idName.indexOf(".");
    if(index > 0)
      var = idName.substring(0, index);

    // Now get the class, and test if the variable is in the scope of the class
    // or one of its ancestors.
    String classScope = XMLUtil.stripSuffix(klass, RuleWriter.delimiter);

    boolean result =  ModelAccessor.isMemberVariable(classScope, var);
    assert(DebugMsg.debugMsg("SharedWriter","ModelAccessor::impliedObjectReference:end:" + idName + ", " + klass + " = " + result));
    return result;
  }
}
