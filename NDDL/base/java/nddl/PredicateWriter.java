package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

class PredicateWriter {


  public static void declareTokenFactory(IndentWriter writer, IXMLElement predicate) throws IOException {
    XMLUtil.checkExpectedNode("predicate", predicate);
    IXMLElement objectClass = predicate.getParent();
    String klass = XMLUtil.getAttribute(objectClass, "name") + "::" + XMLUtil.getAttribute(predicate, "name");
    String predicateName = XMLUtil.getAttribute(objectClass, "name") + "." + XMLUtil.getAttribute(predicate, "name");
    writer.write("DECLARE_TOKEN_FACTORY("+klass+", "+predicateName+");\n");

    SchemaWriter.addFactory("REGISTER_TOKEN_FACTORY("+klass + "::Factory);\n");
  }

  public static void generateImplementation(IndentWriter writer, IXMLElement predicate) throws IOException {
    String name = XMLUtil.getAttribute(predicate,"name");
    String longname = XMLUtil.qualifiedName(predicate);

    IXMLElement parentClass = predicate.getParent();
    String parentName = XMLUtil.getAttribute(parentClass, "name");
    String scopedName = parentName + "." + name;

    ModelAccessor.setCurrentPredicate(name);

    for(int i = 0, n = predicate.getChildren().size(); i < n; i++) {
      IXMLElement param = predicate.getChildAtIndex(i);
      if(param.getName().equals("invoke"))
        break;
    }

    String superClass = ModelAccessor.getPredicateSuperClass(predicate);
    String superCppClass = ModelAccessor.getCppClass(superClass);

    writer.write("\n");
    SharedWriter.generateFileLocation(writer, predicate);
    writer.write(longname+"::"+name+"(const PlanDatabaseId& planDatabase, const LabelStr& name, bool rejectable, bool isFact, bool close)\n");
    writer.write(" : "+ superCppClass +"(planDatabase, name, rejectable, isFact, false) {\n");
    writer.indent();
    writer.write("handleDefaults(close);\n");
    writeAssignments(writer,predicate);

    writer.unindent();
    writer.write("}\n\n");

    writer.write(longname+"::"+name+"(const TokenId& parent, const LabelStr& name, const LabelStr& relation, bool close)\n");
    writer.write(" : "+superCppClass+"(parent, name, relation, false)");
    writer.write(" {\n");
    writer.indent();
    writer.write("handleDefaults(close);\n");
    writeAssignments(writer,predicate);
    writer.unindent();
    writer.write("}\n\n");

    Vector predicateVariables = predicate.getChildrenNamed("var");
    SharedWriter.defineHandleDefaults(writer, predicate, predicateVariables);
    ModelAccessor.resetCurrentPredicate();
  }

	static private void writeAssignments(IndentWriter writer, IXMLElement predicate) throws IOException
	{
    //Ganked from ClassWriter
    Vector predicateAssignments = predicate.getChildrenNamed("assign");

    //Use the set below  to track when an assignment is being made more than once for same variable
    Set assignmentsMade = new HashSet(); 

    for(int i=0;i<predicateAssignments.size(); i++){
      IXMLElement element = (IXMLElement)predicateAssignments.elementAt(i);
      String target = XMLUtil.getAttribute(element,"name");

      if(assignmentsMade.contains(target)){
	  writer.write("!ERROR: Duplicate assignment for " + target + "\n");
      }
      else {
	  assignmentsMade.add(target);
	  String type = XMLUtil.getAttribute(element, "type");
	  IXMLElement sourceElement = element.getChildAtIndex(0);

	  String value = ModelAccessor.getValue(sourceElement);

	  if(sourceElement.getName().equals("id") && type.equals(NddlXmlStrings.x_string))
	    value = XMLUtil.escapeQuotes(value);
	  else if(sourceElement.getName().equals(NddlXmlStrings.x_symbol) || type.equals(NddlXmlStrings.x_string))
	    value = "LabelStr(\""+XMLUtil.escapeQuotes(value)+"\")";

	    // If it's a singleton, set both upper and lower bounds to same value
		if((ModelAccessor.isInterval(type) || ModelAccessor.isNumericPrimitive(type)) && value.indexOf(",") < 0)
			value = value + ", " + value;

		if(element.hasAttribute("inherited"))
		{
			writer.write("getVariable(\""+target+"\")->");
			writer.write("restrictBaseDomain("+ModelAccessor.getDomain(type) + "("+value +", \""+type+"\"));\n");
		}
		else
		{
	  	writer.write(target+" = addParameter(");
	  	writer.write(ModelAccessor.getDomain(type) + "("+value +", \""+type+"\")");
    	writer.write(", "+ makeObjectVariableNameString(target)+");\n");
		}
      }
    }
	}

  static private String makeObjectVariableNameString(String name){
    String result = "\"" + name + "\"";
    return result;
  }

}
