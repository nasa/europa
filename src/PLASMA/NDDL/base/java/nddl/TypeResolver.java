package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

/**
 * @class TypeResolver
 * @brief performs type resolution and transforms the xml based on the types
 * @author Andrew Bachmann
 * @date December, 2004
 */

public class TypeResolver {

	private boolean keepGoing;

	public TypeResolver() {
		keepGoing = true;
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
	 * @brief find where this symbol is defined, and return the type of this symbol as a string
	 * @return the type of the symbol as a string, or null if it is not defined as a symbol
	 */
	public String findSymbolType(IXMLElement context, String name) {
		// check for enum context
		if (context.getName().equals("enum")) {
			return XMLUtil.getAttribute(context, "name");
		}

		// check for enumerations
		Enumeration enumerations = context.getChildrenNamed("enum").elements();
		while (enumerations.hasMoreElements()) {
			IXMLElement enumeration = (IXMLElement)enumerations.nextElement();
			if (!enumeration.hasChildren()) {
				throw new RuntimeException("enumeration with no children");
			}
			if (isEnumerationMember(enumeration, name)) {
				return XMLUtil.getAttribute(enumeration, "name");
			}
		}

		// check the inherited context
		IXMLElement inheritedContext = getInheritedContext(context);
		if (inheritedContext != null) {
			String type = findSymbolType(inheritedContext, name);
			if (type != null) {
				return type;
			}
		}

		// check the enclosing context
		IXMLElement enclosingContext = getEnclosingContext(context);
		if (enclosingContext != null) {
			return findSymbolType(enclosingContext, name);
		}
		return null;
	}

	/**
	 * @brief check an enumeration to see if name is a symbol defined in it
	 * @return true if the name is defined as a symbol in the enumeration
	 */
	public boolean isEnumerationMember(IXMLElement enumeration, String name) {
		IXMLElement value = (IXMLElement)enumeration.getChildAtIndex(0);
		if (value.getName().equals("symbol") &&
		    XMLUtil.getAttribute(value, "value").equals(name)) {
			return true;
		}
		if (value.getName().equals("set")) {
			Enumeration members = value.enumerateChildren();
			while (members.hasMoreElements()) {
				IXMLElement member = (IXMLElement)members.nextElement();
				if (member.getName().equals("symbol") &&
				    XMLUtil.getAttribute(member, "value").equals(name)) {
					return true;
				}
			}
		}
		return false;
	}

	/**
	 * @brief convenience wrapper for findVariableType
	 * @return the type of the variable as a string, or null if it is not declared as a variable
	 */
	public String findVariableType(IXMLElement context, String name) {
		int pos = name.indexOf('.');
		if (pos == -1) {
			return findVariableType(context, name, null);
		} else {
			String first = name.substring(0, pos);
			String rest = name.substring(pos + 1);
			return findVariableType(context, first, rest);
		}
	}

	/**
	 * @brief find where this variable is declared, and return the type of this variable as a string
	 * @return the type of the variable as a string, or null if it is not declared as a variable
	 */
	public String findVariableType(IXMLElement context, String name, String rest) {
		// check for loop context
		if (context.getName().equals("loop")) {
			if (XMLUtil.getAttribute(context, "name").equals(name)) {
				String type = XMLUtil.getAttribute(context, "type");
				return findFieldType(context, type, rest);
			}
		}

		// check for predicate context
		if (context.getName().equals("predicate")) {
			if (name.equals("start") || name.equals("time") || name.equals("duration") || name.equals("end")) {
				String type = "int";
				return findFieldType(context, type, rest);
			}
			if (name.equals("state")) {
				String type = "STATE";
				return findFieldType(context, type, rest);
			}
			if (name.equals("this")) {
				String type = XMLUtil.getAttribute(context, "name");
				return findFieldType(context, type, rest);
			}
		}

		// check for class context
		if (context.getName().equals("class")) {
			if (name.equals("object")) {
				String type = XMLUtil.getAttribute(context, "name");
				return findFieldType(context, type, rest);
			}
			if (name.equals("this")) {
				String type = XMLUtil.getAttribute(context, "name");
				return findFieldType(context, type, rest);
			}
		}

		// check for variables
		Enumeration variables = context.getChildrenNamed("var").elements();
		while (variables.hasMoreElements()) {
			IXMLElement variable = (IXMLElement)variables.nextElement();
			if (XMLUtil.getAttribute(variable, "name").equals(name)) {
				String type = XMLUtil.getAttribute(variable, "type");
				return findFieldType(context, type, rest);
			}
		}

		// check for arguments
		Enumeration arguments = context.getChildrenNamed("arg").elements();
		while (arguments.hasMoreElements()) {
			IXMLElement argument = (IXMLElement)arguments.nextElement();
			if (XMLUtil.getAttribute(argument, "name").equals(name)) {
				String type = XMLUtil.getAttribute(argument, "type");
				return findFieldType(context, type, rest);
			}
		}

		// check for goals
		Enumeration goals = context.getChildrenNamed("goal").elements();
		while (goals.hasMoreElements()) {
			IXMLElement goal = (IXMLElement)goals.nextElement();
			Enumeration instances = goal.getChildrenNamed("predicateinstance").elements();
			while (instances.hasMoreElements()) {
				IXMLElement instance = (IXMLElement)instances.nextElement();
				String instanceName = instance.getAttribute("name", null);
				if ((instanceName != null) && (instanceName.equals(name))) {
					String type = XMLUtil.getAttribute(instance, "type");
					return findFieldType(context, type, rest);
				}
			}
		}

		// check for subgoals
		Enumeration subgoals = context.getChildrenNamed("subgoal").elements();
		while (subgoals.hasMoreElements()) {
			IXMLElement subgoal = (IXMLElement)subgoals.nextElement();
			Enumeration instances = subgoal.getChildrenNamed("predicateinstance").elements();
			if (!instances.hasMoreElements()) {
				IXMLElement multiple = subgoal.getFirstChildNamed("multiple");
				if (multiple != null) {
					String multipleName = multiple.getAttribute("name", null);
					if ((multipleName != null) && (multipleName.equals(name))) {
						String type = XMLUtil.getAttribute(multiple, "type");
						return findFieldType(context, type, rest);
					}
					instances = multiple.getChildrenNamed("predicateinstance").elements();
				}
			}
			while (instances.hasMoreElements()) {
				IXMLElement instance = (IXMLElement)instances.nextElement();
				String instanceName = instance.getAttribute("name", null);
				if ((instanceName != null) && (instanceName.equals(name))) {
					String type = XMLUtil.getAttribute(instance, "type");
					return findFieldType(context, type, rest);
				}
			}
		}

		// check the inherited context
		IXMLElement inheritedContext = getInheritedContext(context);
		if (inheritedContext != null) {
			String type = findVariableType(inheritedContext, name, rest);
			if (type != null) {
				return type;
			}
		}

		// check the enclosing context
		IXMLElement enclosingContext = getEnclosingContext(context);
		if (enclosingContext != null) {
			return findVariableType(enclosingContext, name, rest);
		}
		return null;
	}

	public boolean isPrimitiveType(String type) {
		return (type.equals(NddlXmlStrings.x_int) ||
		        type.equals(NddlXmlStrings.x_boolean) ||
		        type.equals(NddlXmlStrings.x_float) ||
		        type.equals(NddlXmlStrings.x_string) ||
		        type.equals("symbol"));
	}

	/**
	 * @brief find the type of the field of the supplied type
	 * @return the type of the field as a string, or null if it is not declared as a field
	 */
	public String findFieldType(IXMLElement context, String type, String field) {
//		System.err.println("findFieldType("+context.getName()+","+type+","+field+")");
		if (field == null) {
			return type;
		}
		if (isPrimitiveType(type)) {
			error("the primitive type '" + type + "' is not a composite type");
			return null;
		}
		IXMLElement typeContext = findTypeContextNamed(context, type);
		if (typeContext == null) {
			error("could not find a composite type named '"+type+"'");
			return null;
		}

		return findVariableType(typeContext, field);
	}

	/**
	 * @brief find the XML element corresponding to the enclosing context
	 * For compatibilities, this is the enclosing class.
	 * For all others, this is the enclosing XML element.
	 * @return the enclosing context, or null if none exists
	 */
	public IXMLElement getEnclosingContext(IXMLElement context) {
		IXMLElement enclosingContext = null;
		if (context.getName().equals("compat")) {
			enclosingContext = getEnclosingContextForCompat(context);
		}
		if (enclosingContext != null) {
			return enclosingContext;
		}
		return context.getParent();
	}

	/**
	 * @brief find the XML element corresponding to this compat's class
	 * @return the XML element corresponding to this compat's class, or null if it's not defined
	 */
	public IXMLElement getEnclosingContextForCompat(IXMLElement compat) {
		String className = XMLUtil.getAttribute(compat, "class");
		IXMLElement klass = findClassNamed(compat.getParent(), className);
		if (klass == null) {
			error("could not find class '" + className + "' for compatibility\n" +
			      "defined here: " + XMLUtil.locationString(compat) + "\n");
		}
		return klass;
	}

	/**
	 * @brief find the XML element corresponding to the inherited context
	 * For compatibilities, this is the most specific predicate
	 * For predicates, this is the next specific predicate
	 * For classes, this is the super class
	 * @return the inherited context, or null if none exists
	 */
	public IXMLElement getInheritedContext(IXMLElement context) {
		if (context.getName().equals("class")) {
			return getInheritedContextForClass(context);
		}
		if (context.getName().equals("compat")) {
			return getInheritedContextForCompat(context);
		}
		if (context.getName().equals("predicate")) {
			return getInheritedContextForPredicate(context);
		}
		return null;
	}

	/**
	 * @brief find the XML element corresponding to this class' superclass, if it has one
	 * if the class has no superclass, returns null
	 * @return the XML element corresponding to this class' superclass, or null if it has none
	 */
	public IXMLElement getInheritedContextForClass(IXMLElement klass) {
		String className = XMLUtil.getAttribute(klass, "name");
		IXMLElement scope = klass.getParent();
		String superClassName = klass.getAttribute("extends", null);
		if (superClassName != null) {
			IXMLElement superClass = findClassNamed(scope, superClassName);
			if (superClass == null) {
				error("could not find class '" + superClassName + "', superclass of '" + className + "'\n" +
				      "as defined here: " + XMLUtil.locationString(klass) + "\n");
			}
			return superClass;
		}
		return null;
	}

	/**
	 * @brief find the XML element corresponding to the most specific predicate for this compatibility
	 * if there is no predicate defined for this compatibility, returns null
	 * @return the XML element corresponding to the most specific predicate for this compatibility, or null if it has none
	 */
	public IXMLElement getInheritedContextForCompat(IXMLElement compat) {
		String className = XMLUtil.getAttribute(compat, "class");
		IXMLElement klass = findClassNamed(compat, className);
		if (klass == null) {
			error("could not find class '" + className + "' for compatibility\n" +
			      "defined here: " + XMLUtil.locationString(compat) + "\n");
			return null;
		}
		
		String predicateName = XMLUtil.getAttribute(compat, "name");
		while (true) {
			Enumeration predicates = klass.getChildrenNamed("predicate").elements();
			while (predicates.hasMoreElements()) {
				IXMLElement predicate = (IXMLElement)predicates.nextElement();
				if (XMLUtil.getAttribute(predicate, "name").equals(predicateName)) {
					return predicate;
				}
			}
			String superClassName = klass.getAttribute("extends", null);
			if (superClassName == null) {
				String message = "no predicate '" + predicateName + "' \n";
				message += "in class '" + className + "' defined here: " + XMLUtil.locationString(findClassNamed(compat, className)) + "\n";
				message += "for compatibility defined here: " + XMLUtil.locationString(compat) + "\n";
				error(message);
				return null;
			}
			IXMLElement superClass = findClassNamed(klass.getParent(), superClassName);
			if (superClass == null) {
				return null; // error message for this is already printed in process(element)
			}
			klass = superClass;
		}
	}

	public IXMLElement getInheritedContextForPredicate(IXMLElement predicate) {
		IXMLElement klass = predicate.getParent();
		if ((klass == null) || (!klass.getName().equals("class"))) {
			error("predicate not contained in a class" + XMLUtil.locationString(predicate) + "\n");
			return null;
		}

		String predicateName = XMLUtil.getAttribute(predicate, "name");
		while (true) {
			String superClassName = klass.getAttribute("extends", null);
			if (superClassName == null) {
				return null; // exhausted the inherited contexts (not an error)
			}
			IXMLElement superClass = findClassNamed(klass.getParent(), superClassName);
			if (superClass == null) {
				return null; // error message for this is already printed in process(element)
			}
			klass = superClass;
			Enumeration predicates = klass.getChildrenNamed("predicate").elements();
			while (predicates.hasMoreElements()) {
				IXMLElement nextPredicate = (IXMLElement)predicates.nextElement();
				if (XMLUtil.getAttribute(nextPredicate, "name").equals(predicateName)) {
					return nextPredicate;
				}
			}
		}
	}

	public IXMLElement findClassNamed(IXMLElement scope, String name) {
		StringTokenizer classTokens = new StringTokenizer(name, ".");
		while (classTokens.hasMoreTokens()) {
			String token = classTokens.nextToken();
			boolean found = false;
			while (scope != null) {
				Enumeration classes = scope.getChildrenNamed("class").elements();
				while (classes.hasMoreElements()) {
					IXMLElement klass = (IXMLElement)classes.nextElement();
					if (XMLUtil.getAttribute(klass, "name").equals(token)) {
						scope = klass;
						found = true;
						break;
					}
				}
				if (found) {
					break;
				}
				scope = scope.getParent();
			}
			if (!found) {
				return null;
			}
		}
		return scope;
	}

	public IXMLElement findTypeContextNamed(IXMLElement scope, String name) {
//		System.err.println("findTypeContextNamed("+scope.getName()+","+name+")");
		if (name == null) {
			return scope;
		}
		String token = name;
		String rest = null;
		int location = name.indexOf('.');
		if (location != -1) {
			token = name.substring(0, location);
			rest = name.substring(location + 1);
		}

		if (token.equals("object")) {
			// find the enclosing class scope
			while (!scope.getName().equals("class")) {
				scope = getEnclosingContext(scope);
				if (scope == null) {
					error("'object' reference outside of class scope");
					return null;
				}
			}
			return findTypeContextNamed(scope, rest);
		}

		if (token.equals("this")) {
			// find the enclosing predicate/class scope
			while (!scope.getName().equals("predicate") && !scope.getName().equals("class")) {
				scope = getEnclosingContext(scope);
				if (scope == null) {
					error("'this' reference outside of class/predicate scope");
					return null;
				}
			}
			return findTypeContextNamed(scope, rest);
		}

		Enumeration classes = scope.getChildrenNamed("class").elements();
		while (classes.hasMoreElements()) {
			IXMLElement klass = (IXMLElement)classes.nextElement();
			if (XMLUtil.getAttribute(klass, "name").equals(token)) {
				return findTypeContextNamed(klass, rest);
			}
		}

		Enumeration predicates = scope.getChildrenNamed("predicate").elements();
		while (predicates.hasMoreElements()) {
			IXMLElement predicate = (IXMLElement)predicates.nextElement();
			if (XMLUtil.getAttribute(predicate, "name").equals(token)) {
				return findTypeContextNamed(predicate, rest);
			}
		}

		Enumeration enumerations = scope.getChildrenNamed("enum").elements();
		while (enumerations.hasMoreElements()) {
			IXMLElement enumeration = (IXMLElement)enumerations.nextElement();
			if (XMLUtil.getAttribute(enumeration, "name").equals(token)) {
				return findTypeContextNamed(enumeration, rest);
			}
		}

		Enumeration variables = scope.getChildrenNamed("var").elements();
		while (variables.hasMoreElements()) {
			IXMLElement variable = (IXMLElement)variables.nextElement();
			if (XMLUtil.getAttribute(variable, "name").equals(token)) {
				String type = XMLUtil.getAttribute(variable, "type");
				return findTypeContextNamed(findTypeContextNamed(scope, type), rest);
			}
		}

		IXMLElement inheritedContext = getInheritedContext(scope);
		if (inheritedContext != null) {
			IXMLElement context = findTypeContextNamed(inheritedContext, name);
			if (context != null) {
				return context;
			}
		}

		IXMLElement enclosingContext = getEnclosingContext(scope);
		if (enclosingContext != null) {
			IXMLElement context = findTypeContextNamed(enclosingContext, name);
			if (context != null) {
				return context;
			}
		}

		return null;
	}

}
