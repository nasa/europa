package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

public class ImplementationGenerator {
  private static final String prefix = "";
  public ImplementationGenerator() {
    this(null);
  }
  public ImplementationGenerator(IndentWriter writer) {
    setWriter(writer);
  }
  IndentWriter writer;
  public void setWriter(IndentWriter w) {
    writer = w;
  }
  public void generate(IXMLElement element) throws IOException {
    writer.write("#include \"NddlUtils.hh\"\n");
    writer.write("\n");
    writer.write("namespace NDDL {\n");
    writer.indent();

		// all models require a type factory for Objects (this is implicit)
		SchemaWriter.addFactory("REGISTER_TYPE_FACTORY(Object, ObjectDomain(\"Object\"));\n");

    for (Enumeration e = element.enumerateChildren() ; e.hasMoreElements() ; )
	generateImplementation((IXMLElement)e.nextElement());

    writer.write("\n");
    writer.unindent();
    writer.write("} // namespace NDDL\n");
  }

  // Implementation
  private void generateImplementation(IXMLElement element) throws IOException {
    if (element.getName().equals("class")) {
      ClassWriter.generateImplementation(writer, element);
    } else if (element.getName().equals("enum")) {
      EnumerationWriter.generateImplementation(writer, element);
    } else if (element.getName().equals("typedef")) {
      EnumerationWriter.generateImplementation(writer, element);
    } else if (element.getName().equals("compat")) {
      RuleWriter.generateRule(writer, element);
    }
  }
}
