package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

/**
 * @class SemanticChecker
 * @brief performs various semantic checks and analysis
 * @author Andrew Bachmann
 * @date December, 2004
 */

public class SemanticChecker {

	private boolean isPrimitiveType(String type) {
		return (type.equals(NddlXmlStrings.x_int) ||
		        type.equals(NddlXmlStrings.x_boolean) ||
		        type.equals(NddlXmlStrings.x_float) ||
		        type.equals(NddlXmlStrings.x_string) ||
		        type.equals("STATE") ||
		        type.equals("symbol"));
	}

	private boolean keepGoing;

	private TypeResolver typeResolver;

	public SemanticChecker() {
		keepGoing = true;
		typeResolver = new TypeResolver();
	}

	/**
	 * @brief if keepGoing, prints the error message, else, throws it as an exception
	 */
	private void error(String message) {
		if (keepGoing) {
			System.err.println("error: "+message);
		} else {
			throw new RuntimeException(message);
		}
	}

	/**
	 * @brief performs the semantic check on this element and all of its subelements
	 * @returns true if the check succeeded
	 */
	public boolean check(IXMLElement element) {
		boolean result = true;
		// check this element
		String tag = element.getName();
		try {
  		if (tag.equals("symbol")) {
  			result = checkSymbol(element);
  		} else if (tag.equals("id")) {
  			result = checkIdentifier(element);
  		} else if (tag.equals("predicateinstance")) {
  			result = checkPredicateInstance(element);
  		} else if (tag.equals("loop")) {
  			result = checkLoop(element);
  		} else if (tag.equals("new")) {
  			result = checkNew(element);
  		} else if (tag.equals("var")) {
  			result = checkVariable(element);
  		} else if (tag.equals("class")) {
  			result = checkClass(element);
  		} else if (tag.equals("enum")) {
  		  result = checkEnumeration(element);
  		} else if (tag.equals("set")) {
  		  result = checkSet(element);
  		}
      else if(tag.equals("predicate")) {
        result = checkPredicate(element);
      }
  	} catch (RuntimeException re) {
  	  System.err.println();
  	  System.err.println(re);
  	  try {
  	    XMLUtil.dump(element);
  	  } catch (IOException ie) {
  	  }
  	  System.err.println();
  	  result = false;
    }  	  
		
		// check the children
  	Enumeration enumeration = element.enumerateChildren();
		while (enumeration.hasMoreElements()) {
			IXMLElement child = (IXMLElement)enumeration.nextElement();
			if (!check(child)) {
				result = false;
			}
		}
		return result;
	}

	/**
	 * @brief check the symbol for validity
	 * 1. must have a name
	 * 2. must have a type
	 * 3. type must be valid
	 * @returns true if the check succeeded
	 */
	public boolean checkSymbol(IXMLElement element) {
		String name = XMLUtil.getAttribute(element, NddlXmlStrings.x_value);
		String type = XMLUtil.getAttribute(element, NddlXmlStrings.x_type);
		IXMLElement context = typeResolver.findTypeContextNamed(element, type);
		if (context == null) {
			error(XMLUtil.locationString(element) + ":unknown type for symbol: '" + type + "'");
			return false;
		}
		return true;
	}

	/**
	 * @brief check the identifier for validity
	 * 1. must have a name
	 * 2. must have a type
	 * 3. type must be valid
	 * @returns true if the check succeeded
	 */
	public boolean checkIdentifier(IXMLElement element) {
		String name = XMLUtil.getAttribute(element, NddlXmlStrings.x_name);
		String type = XMLUtil.getAttribute(element, NddlXmlStrings.x_type);
		if (isPrimitiveType(type)) {
			return true;
		}
		IXMLElement context = typeResolver.findTypeContextNamed(element, type);
		if (context == null) {
			error(XMLUtil.locationString(element) + ":unknown type for identifier: '" + type + "'");
			return false;
		}
		return true;
	}

  /**
   * @brief check the predicate for validity
   *
   * @returns true if the check succeeded
   */

  public boolean checkPredicate(IXMLElement element) {
    for(Iterator it = element.getChildren().iterator(); it.hasNext();) {
      IXMLElement child = (IXMLElement) it.next();
      XMLUtil.checkExpectedNode("var,invoke,assign", child);
      if(child.getName().equals("invoke")) {
        for(Iterator childIt = child.getChildren().iterator(); childIt.hasNext();) {
          IXMLElement subChild = (IXMLElement) childIt.next();
          XMLUtil.checkExpectedNode("value,id,interval,symbol,set", subChild);
          if(subChild.getName().equals("id")) {
            if(XMLUtil.getAttribute(subChild, "name").indexOf('.') != -1) {
              error(XMLUtil.locationString(subChild) + ": invalid variable reference in predicate declaration: '" +
                                           XMLUtil.getAttribute(subChild, "name") + "'");
              return false;
            }
            if(!checkIdentifier(subChild))
              return false;
          }
        }
      }
      else if(child.getName().equals("var")) {}
      else if(child.getName().equals("assign")) {}
      else {
        error(XMLUtil.locationString(child) + ": invalid statement in predicate declaration.");
        return false;
      }
    }
    return true;
  }

	/**
	 * @brief check the predicate instance for validity
	 * 1. must have a type
	 * 2. type must be valid
	 * @returns true if the check succeeded
	 */
	public boolean checkPredicateInstance(IXMLElement element) {
		String type = XMLUtil.getAttribute(element, NddlXmlStrings.x_type);
		IXMLElement context = typeResolver.findTypeContextNamed(element, type);
		if (context == null) {
			error(XMLUtil.locationString(element) + ":unknown type for predicate specification: '" + type + "'");
			return false;
		}
		return true;
	}

	/**
	 * @brief check the loop for validity
	 * 1. must have a name
	 * 2. must have a type
	 * 3. type must be valid
	 * @returns true if the check succeeded
	 */
	public boolean checkLoop(IXMLElement element) {
		String name = XMLUtil.getAttribute(element, NddlXmlStrings.x_value);
		String type = XMLUtil.getAttribute(element, NddlXmlStrings.x_type);
		IXMLElement context = typeResolver.findTypeContextNamed(element, type);
		if (context == null) {
			error(XMLUtil.locationString(element) + ":unknown type for loop domain: '" + type + "'");
			return false;
		}
		return true;
	}

	/**
	 * @brief check the instantiation for validity
	 * 1. must have a type
	 * 2. type must be valid
	 * 3. TODO: compatible constructor exists
	 * @returns true if the check succeeded
	 */
	public boolean checkNew(IXMLElement element) {
		String type = XMLUtil.getAttribute(element, NddlXmlStrings.x_type);
		IXMLElement context = typeResolver.findTypeContextNamed(element, type);
		if (context == null) {
			error(XMLUtil.locationString(element) + ":unresolved type for allocation: '" + type + "'");
			return false;
		}
		// TODO: check that an appropriate constructor exists for this invokation
		return true;
	}

	/**
	 * @brief check the instantiation for validity
	 * 1. must have a type
	 * 2. type must be valid
	 * 3. check the assigned value for compatibility 
	 * @returns true if the check succeeded
	 */
	public boolean checkVariable(IXMLElement element) {
	        // Must have a type. 
		String type = XMLUtil.getAttribute(element, NddlXmlStrings.x_type);
		if (isPrimitiveType(type)) {
			return true;
		}
                // Must be a valid type.
		IXMLElement context = typeResolver.findTypeContextNamed(element, type);
		if (context == null) {
			String name = XMLUtil.getAttribute(element, "name");
			error(XMLUtil.locationString(element) + ":unresolved type for '" + name + "': '" + type + "'");
			return false;
		}

		// Variable assignment must be valid.     
		if (element.getChildrenCount() > 0 && !typesAreCompatible(element, type)) {  
		   return false;
		} 

	        // if all tests have passed then variable is OK.    
	        return true;
	}

	/**
	 * @brief check the class for validity
	 * 1. if it has a superclass, the name of the superclass must be valid
	 * @returns true if the check succeeded
	 */
	public boolean checkClass(IXMLElement element) {
		String superClassName = element.getAttribute("extends", null);
		if (superClassName != null) {
			IXMLElement superClass = typeResolver.findClassNamed(element.getParent(), superClassName);
			if (superClass == null) {
				String className = XMLUtil.getAttribute(element, "name");
				error("could not find class '" + superClassName + "', superclass of '" + className + "'\n" +
				      "as defined here: " + XMLUtil.locationString(element) + "\n");
				return false;
			}
		}
		return true;
	}

  /**
   * @brief check the enumeration for validity
   * @returns true if the check succeeded
   */
  public boolean checkEnumeration(IXMLElement element) {
    boolean result = true;
    
    String name = XMLUtil.getAttribute(element, "name");
    if (element.getChildrenCount() != 1) {
      error(XMLUtil.locationString(element) + ":an enumeration should have a single value child");
      if (!element.hasChildren()) {
        return false;
      }
      result = false;
    }
    IXMLElement value = element.getChildAtIndex(0);
    String tag = value.getName();
    if (tag.equals("set")) {
      if (value.hasChildren()) {
        // if the set is not empty, check the declared type
        String type = XMLUtil.getAttribute(value, "type");
        if (!isPrimitiveType(type) && !type.equals(name)) {
          error(XMLUtil.locationString(element) + ":inconsistent typing for enum '" + name + "'");
          result = false;
        }
      }
    } else if (tag.equals("interval")) {
      // intervals are numeric, so as long as the interval itself is valid,
      // the declaration must also be valid
    } else {
      System.err.println("warning: non-composite enum not checked");
      try {
        XMLUtil.dump(element);
      } catch (IOException ie) {
      }
    }
    return result;
  }
  
  /**
   * @brief check the set for validity
   * Compare the declared set type with the types of the members
   * @returns true if the check succeeded
   */
  public boolean checkSet(IXMLElement element) {
    boolean result = true;
    if (!element.hasChildren()) {
      // trivially valid
      return true;
    }
    String setType = XMLUtil.getAttribute(element, "type");
    Enumeration members = element.enumerateChildren();
 	 	while (members.hasMoreElements()) {
	  	IXMLElement value = (IXMLElement)members.nextElement();
		  String valueType = XMLUtil.getAttribute(value, "type");
  		String valueName = null;
	 		String tag = value.getName();
	  	if (tag.equals("value")) {
		    valueName = XMLUtil.getAttribute(value, "name");
		  } else if (tag.equals("symbol")) {
  		  valueName = XMLUtil.getAttribute(value, "value");
	 		} else {
	  	  error(XMLUtil.locationString(value)+":unknown child found in enumeration");
		    result = false;
  		}
      // check for type consistency        
      if (valueType.equals("int") && setType.equals("float")) {
        valueType = "float";
      }
      if (!valueType.equals(setType)) {
        error(XMLUtil.locationString(value)+":inconsistent set type for set member");
        result = false;
      }    
		}
    return result;
  }

   /**
   * @brief Extract assinged var and assingments type and call compatibleTypes to 
   *        determine if they are compatible. This method's main task is to navigate
   *        the XML data structure for element to exstract the type info we need.
   * @Returns true if the check succeeded
   */
    public boolean typesAreCompatible(IXMLElement element, String type) {
       Enumeration enumeration = element.enumerateChildren();
           while (enumeration.hasMoreElements()) {
		  IXMLElement child = (IXMLElement) enumeration.nextElement();
                  // if this is the type attribute - then check it.
		  if (child.hasAttribute(NddlXmlStrings.x_type)) {
                     String variablesTypeName = XMLUtil.getAttribute(child, NddlXmlStrings.x_type);
		     if (!assignableTypes(type, variablesTypeName)) {
	                  String name = XMLUtil.getAttribute(element, "name");
	                   error(XMLUtil.locationString(element)+":incompatible types in variable assignment '" + name +
		           "': '" + type + " not comptable with type " + variablesTypeName + "'");
                           return false;
		     }
		  }
	   }
	   return true;
    }


  /**
   * @brief check if variable of assingedVariableType can be assigned a value of type assingmentType
   * @returns true if the check succeeded
   */
    public boolean assignableTypes(String assignmentType, String assignedVariableType) {
	if (assignmentType.equalsIgnoreCase(assignedVariableType)) {
	     
	   return true;

	} else {
	    // enumerate allowable cases where assignmentType not string equal assingedVariableType
            // Note this hard coding apprach works for a few cases but is not flexible.
            // If we increase the number of exceptions we should read this information in from file. 

		if ( assignmentType.equalsIgnoreCase( "intEnum" ) ){
		    if (assignedVariableType.equalsIgnoreCase("int")) {                     
			 return true;
		      } 
		}
    	      
                if ( assignmentType.equalsIgnoreCase( "intRange" )) {	
                     if (assignedVariableType.equalsIgnoreCase("int")) {                     
			 return true;
		      } 
          	}

         	if ( assignmentType.equalsIgnoreCase( "float" ) ) {
		    if (assignedVariableType.equalsIgnoreCase("int")) {                     
			 return true;
		      } 
		}
    	        
                if ( assignmentType.equalsIgnoreCase( "floatEnum" )) {	
                     if (assignedVariableType.equalsIgnoreCase("float") ||
                         assignedVariableType.equalsIgnoreCase("int" )) {                     
			 return true;
		      } 
          	}

		if ( assignmentType.equalsIgnoreCase( "floatRange1" )) {	
                     if (assignedVariableType.equalsIgnoreCase("float") ||
                         assignedVariableType.equalsIgnoreCase("int")) {                     
			 return true;
		      } 
          	}	
	}

         return false;
    }


}
