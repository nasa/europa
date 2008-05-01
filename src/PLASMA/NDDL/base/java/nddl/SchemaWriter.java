package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

class SchemaWriter {

    public static void addConstraintRegistration(String registration){
	s_constraintRegistrations.add(registration);
    }

    public static void addObjectType(String command){
	s_objectTypeCommands.add(command);
    }

  /**
   * Commands are logged for output when initialization of the schema is required. These commans are invocations on the
   * Schema interface in the PlanDatabase. As we process the model, we log these commands.
   * @see generate
   */
  public static void addCommand(String command) {
    s_commands.add(command);
  }

    /**
     * Factory allocations are logged for output when we initialize the schema
     */
    public static void addFactory(String factorySignature){
	s_factoryAllocations.add(factorySignature);
    }

  /**
   * The pattern we have is to generate the schema through an implementation of thie schema() function which will
   * be invoked by the client.
   */
  public static void generate(IndentWriter writer) throws IOException {

		writer.write("namespace NDDL {\n");
		writer.indent();

    // Implement expected initialization hooks
    writer.write("// Boot-strap code to initialize schema\n");
    writer.write("extern \"C\" SchemaId loadSchema(){\n");
    writer.indent();


    String modelName = ModelAccessor.getModelName();
    if(modelName == null)
      throw new RuntimeException("Failed to set model name. Bug in NddlCompiler");

    writer.write("Id<Schema> id = Schema::testInstance(\""+modelName+"\");\n");

		// Register Constraints
		writer.write("// Register Constraints\n");
    for(Iterator it = s_constraintRegistrations.iterator(); it.hasNext(); ){
	    String constraint = (String)it.next();
	    writer.write(constraint+";\n");
    }

    writer.write("// Invoke commands to populate schema with type definitions\n");

    // Iterate over all type definition commands
    for(int i=0;i<s_objectTypeCommands.size();i++){
	    writer.write("id->" + s_objectTypeCommands.elementAt(i) + ";\n");
    }

    // Iterate over all type definition commands
    for(int i=0;i<s_commands.size();i++){
	    writer.write("id->" + s_commands.elementAt(i) + ";\n");
    }

    // Force allocation of type factories. This should do a mapping for external names
    // to internal
    writer.write("// Force allocation of model specific type factories\n");

    // Allocate Factories
    writer.write("// Allocate factories\n");
    for(Iterator it = s_factoryAllocations.iterator(); it.hasNext(); ){
	    String command = (String)it.next();
	    writer.write(command);
    }

    // Iterate over all rules and allocate singletons - this registers rules
    writer.write("// Allocate rules\n");
    Set ruleNames = RuleWriter.getRules();
    for(Iterator it = ruleNames.iterator(); it.hasNext(); ){
	    String ruleName = (String)it.next();
	    writer.write("new " + ruleName + "();\n");
    }

    writer.write("return id;\n");
    writer.unindent();
    writer.write("}\n\n");
		writer.unindent();
		writer.write("}\n");

  }

    private static Vector s_objectTypeCommands = new Vector();
    private static Vector s_commands = new Vector();
    private static Vector s_factoryAllocations = new Vector();
    private static Vector s_constraintRegistrations = new Vector();

  static {
	  // Load built-in types
      s_objectTypeCommands.add("addObjectType(\"Timeline\",\"Object\")");
  }
}
