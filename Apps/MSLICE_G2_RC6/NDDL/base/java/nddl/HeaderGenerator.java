package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

public class HeaderGenerator {
    private static final String prefix = "";
    public HeaderGenerator() {
	this(null);
    }
    public HeaderGenerator(IndentWriter writer) {
	setWriter(writer);
    }
    IndentWriter writer;
    public void setWriter(IndentWriter w) {
	writer = w;
    }

  /**
   * Primary procedure to output public declarations. Basically a top down
   * output, assuming we start at the nddl.
   */
    public void generate(IXMLElement nddl) throws IOException {
      XMLUtil.checkExpectedNode("nddl", nddl);

      // Output include and namespace information
//       writer.write("#include \"Db.hh\"\n");
//       writer.write("#include \"NddlUtils.hh\"\n");

//       writer.write("/** Custom Include Files **/\n");
//       Set customIncludes = ModelAccessor.getCustomIncludes();
//       for(Iterator i=customIncludes.iterator(); i.hasNext();) {
// 	  String includeFile = (String) i.next();
// 	  writer.write("#include \"" + includeFile + "\"\n");
//       }

//       writer.write("\n");
//       writer.write("namespace NDDL {\n");
//       writer.write("\n");
      writer.indent();

      // Iterate over classes and write necessary type declarations. Note that we are
      // not going to generate code for global variables
      for (Enumeration e = nddl.enumerateChildren() ; e.hasMoreElements() ; ) {
	IXMLElement element = (IXMLElement)e.nextElement();
	if (element.getName().equals("class"))
	    ClassWriter.writeTypedefs(writer, element);
      }

      // Now iterate over classes and generate the necessary class declarations
      for (Enumeration e = nddl.enumerateChildren() ; e.hasMoreElements() ; ) {
	IXMLElement element = (IXMLElement)e.nextElement();
	if (ModelAccessor.isEnumeration(element))
	  EnumerationWriter.generateHeader(writer, element);
	else if (ModelAccessor.isObjectType(element))
	  SharedWriter.writeClassDeclaration(writer, element);
      }

      // Complete namespace
      writer.unindent();
      writer.write("} // End NDDL namespace\n");
    }
}
