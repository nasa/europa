package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

class EnumerationWriter {

  public static void generateHeader(IndentWriter writer, IXMLElement en) throws IOException {
    writer.write("\n");
    String name = XMLUtil.getAttribute(en,"name");

    // Determine if we are scoped to a class, and get its name if necesaary.
    IXMLElement parent = (IXMLElement) en.getParent();
    if (parent.getName().equals("class")) {
      name = XMLUtil.getAttribute(parent, "name") + "::" + name;
    }

    // Now introduce a typedef and allocation function for the base values
    SharedWriter.generateFileLocation(writer, en);
    String enumDomain = ModelAccessor.getEnumerationType(en);
    writer.write("typedef " + enumDomain + " " + removeClassQualifier(name) + ";\n");
    if (parent.getName().equals("class")) {
      writer.write("static " + name + " " + name + "BaseDomain();\n");
    } else {
      writer.write(name + " " + name + "BaseDomain();\n");
    }
   	ModelAccessor.registerEnumeration(name,name,ModelAccessor.isInterval(en));
		IXMLElement values = XMLUtil.getSingleChild("set:interval",en);
		//if(values.getName().equals("set"))
   	SchemaWriter.addCommand("addEnum(\""+name+"\")");
		//else
    	//SchemaWriter.addCommand("addInterval(\""+name+"\")");
  }

  public static void generateImplementation(IndentWriter writer, IXMLElement en) throws IOException {
    String name = XMLUtil.getAttribute(en,"name");

    if(en.getParent().getName().equals("class"))
	name = XMLUtil.getAttribute(en.getParent(), "name") + "::" + name;

		// TODO: lemme see if I can work out how to add in the new typedefs here!
    writer.write("\n");
    writer.write(name + " "+ name+"BaseDomain(){\n");
    writer.indent();
		IXMLElement values = XMLUtil.getSingleChild("set:interval",en);
		// if child is a set then it's an enum, otherwise range.
		if(values.getName().equals("set"))
		{
   		writer.write("static " + name + " sl_enum(\"" + name + "\");\n");
    	writer.write("if (sl_enum.isOpen()) {\n");
    	writer.indent();
    	writer.write("// Insert values to initialize\n");

    	// Loop over values and insert for each
    	Enumeration set = values.enumerateChildren();
    	while (set.hasMoreElements()){
      	IXMLElement value = (IXMLElement) set.nextElement();
      	String val_type = ModelAccessor.getValueType(value);
      	String val_val = XMLUtil.escapeQuotes(ModelAccessor.getValue(value));
      	if(val_type.equals("symbol") || val_type.equals("string")) {
        	writer.write("sl_enum.insert(LabelStr(\""+ val_val + "\"));\n");
					SchemaWriter.addCommand("addValue(\""+name+"\", LabelStr(\""+val_val+"\"))");
      	}
      	else 
        	writer.write("sl_enum.insert(" + val_val + ");\n");
    	}

    	writer.write("sl_enum.close();\n");
    	writer.unindent();
    	writer.write("}\n");
    	writer.write("return(sl_enum);\n");
    	SchemaWriter.addFactory("REGISTER_TYPE_FACTORY("+name+", " +name+"BaseDomain());\n");
		}
		else if(values.getName().equals("interval"))
		{
			writer.write("static " + name + " sl_interval("+ XMLUtil.getAttribute(values,"min")+", "+
			                                              XMLUtil.getAttribute(values,"max")+", "+ 
																										"\""+name+"\");\n");
			writer.write("return(sl_interval);\n");
    	SchemaWriter.addFactory("REGISTER_ITYPE_FACTORY("+name+", " +name+"BaseDomain());\n");
		}
		else
			throw new RuntimeException("Typedef/enumeration generation had child other than set or interval.");
    writer.unindent();
    writer.write("}\n");

  }

  static private String getType(IXMLElement en) {
    IXMLElement enSet = (IXMLElement) en.enumerateChildren().nextElement();
    IXMLElement enValue = (IXMLElement) enSet.enumerateChildren().nextElement();
    return enValue.getName();
  }


    // GCC 4.0 prohibits class name qualifiers in typedefs.
    // Method cleans of foo:: components.
    static private String removeClassQualifier(String name) {
	int colonPosition = name.lastIndexOf(':');
        if (colonPosition == -1 ) {
	    return name; // no colon.
        } else {
            // return eveything to right of colon.
            return name.substring(colonPosition + 1);
	}
    }

}
