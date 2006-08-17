
package nddl;

import net.n3.nanoxml.*;
import java.io.*;

public class XMLUtil {

  /**
   * @brief Handy utility for shorthand to check that the xml node we get is the one we expect
   * @param expectedNodeTypes A ':' delimited string of acceptable node types
   * @param node The node in question 
   */
  public static void checkExpectedNode(String expectedNodeTypes, IXMLElement node) {
    if(expectedNodeTypes.indexOf(node.getName()) < 0)
      throw new RuntimeException("Expecting <"+expectedNodeTypes+"> but found " + node.getName());
  }

  /**
   * @brief Get singleton child, and perform test to ensure no siblings, and expected node type
   * @param expectedNodeTypes A ':' delimited string of acceptable node types
   * @param node The node in question 
   */
  public static IXMLElement getSingleChild(String expectedNodeTypes, IXMLElement node) {
    if(node.getChildrenCount() != 1)
      throw new RuntimeException("Expected node "+node.getName()+" to have exactly one child.");
    IXMLElement toRet = node.getChildAtIndex(0);
    checkExpectedNode(expectedNodeTypes,toRet);
    return toRet;
  }

  public static String getAttribute(IXMLElement element, String expectedNodeType, String name) {
    checkExpectedNode(expectedNodeType, element);

    if (!element.hasAttribute(name))
      throw new RuntimeException("Attempt to get attribute " + name + " for element " + element.getName());
    return element.getAttribute(name,"");
  }

  public static String getAttribute(IXMLElement element, String name) {
    if (!element.hasAttribute(name))
      throw new RuntimeException("Attempt to get attribute " + name + " for element " + element.getName());
    return element.getAttribute(name,"");
  }

  public static String inheritAttribute(IXMLElement element, String name) {
    if (element == null)
      return null;
    if (element.hasAttribute(name))
      return element.getAttribute(name,"");
    return inheritAttribute(element.getParent(),name);
  }

  public static void dump(IXMLElement element) throws IOException {
    new XMLWriter(System.err).write(element,true);
  }

  public static String locationString(IXMLElement element) {
		return locationString(element, ':');
	}

  public static String locationString(IXMLElement element, char seperator) {
    String filename = inheritAttribute(element,"filename");
    String line = inheritAttribute(element,"line");
		// if we don't have a filename, we need to ditch any line information we
		// have and claim what little we can guess.
    if (filename == null)
      return ModelAccessor.getModelName()+".xml";
    if (line == null)
      return filename;
    return filename+seperator+line;
  }

  public static String nameOf(IXMLElement element) {
    return XMLUtil.getAttribute(element,"name").replaceAll("\\.","::");
  }

  public static String typeOf(IXMLElement element) {
    return XMLUtil.getAttribute(element,"type").replaceAll("\\.","::");
  }

  public static String qualifiedName(IXMLElement element) {
    if (element == null)
      return null;
    String parentName = qualifiedName(element.getParent());
    String name = null;
    if (element.hasAttribute("name"))
      name = element.getAttribute("name","");
    if (parentName == null)
      return name;
    if (name == null)
      return parentName;
    return parentName+"::"+name;
  }

  public static void reportError(IndentWriter writer, String errMsg) throws IOException {
    writer.write("!ERROR" + errMsg + "\n");
  }

  /**
   * Helper utility to strip a given prefix from a string, if present.
   */
  public static String stripPrefix(String str, String prefix) {
    if(str == null)
      return null;

    int index = str.indexOf(prefix);
    if (index > 0)
      return str.substring(index+1, str.length());
    else
      return str;
  }

  /**
   * Helper utility to strip a given suffix from a string, if present.
   */
  public static String stripSuffix(String str, String suffix) {
    if(str == null)
      return null;

    int index = str.indexOf(suffix);
    if (index > 0)
      return str.substring(0, index);
    else
      return str;
  }

  public static String escapeQuotes(String str) {
    int index = 0;
    while(str.indexOf('"', index) != -1) {
      index = str.indexOf('"', index);
      str = str.substring(0, index) +   '\\' + str.substring(index);
      index = index + 2;
    }
    return str;
  }
}
