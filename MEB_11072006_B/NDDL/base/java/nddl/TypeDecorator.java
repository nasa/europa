package nddl;

import java.io.*;
import net.n3.nanoxml.*;
import java.util.*;

/**
 * @class TypeDecorator
 * @brief adds type information to provided xml
 * @author Andrew Bachmann
 * @date December, 2004
 */

public class TypeDecorator {

	private boolean keepGoing;

	private TypeResolver typeResolver;

	public TypeDecorator() {
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
	 * @brief recursively decorates the xml with the appropriate types
	 * @returns true if the decoration succeeded
	 */
	public boolean decorate(IXMLElement element) {
		boolean result = true;
		// decorate this element
		String tag = element.getName();
		if (tag.equals("ident")) {
			result = decorateIdent(element);
		} else if (tag.equals("symbol")) {
			result = decorateSymbol(element);
		} else if (tag.equals("assign")) {
			result = decorateAssign(element);
		} else if (tag.equals("id")) {
			result = decorateIdentifier(element);
		} else if (tag.equals("predicateinstance")) {
			result = decoratePredicateInstance(element);
		} else if (tag.equals("loop")) {
			result = decorateLoop(element);
		}
		// decorate the children
		if (element.hasChildren()) {
			Enumeration enumeration = element.enumerateChildren();
			while (enumeration.hasMoreElements()) {
				IXMLElement child = (IXMLElement)enumeration.nextElement();
				if (!decorate(child)) {
					result = false;
				}
			}
		}
		// sets need to be decorated after their children
		if (tag.equals("set")) {
			result &= decorateSet(element);
		}
		return result;
	}

	/**
	 * @brief change an ident into a decorated symbol or decorated identifier
	 * Attempts to look up the ident as both a symbol and a variable.
	 * If both lookups succeed, or if both lookups fail, it is an error.
	 * Otherwise, change the ident into symbol or id as appropriate, and add the type.
	 * @returns true if the decoration succeeded
	 */
	public boolean decorateIdent(IXMLElement element) {
		String name = XMLUtil.getAttribute(element, NddlXmlStrings.x_value);
		if (name.equals("MINUS_INFINITY") || name.equals("PLUS_INFINITY")) {
			// ignore, should be changed to +inf, and handled as <int value="+inf">
			return true;
		}
		String symbolType = typeResolver.findSymbolType(element, name);
		String variableType = typeResolver.findVariableType(element, name);
		if ((symbolType != null) && (variableType != null)) {
			error("ambiguous ident: '" + name + "'");
			return false;
		}
		if (symbolType != null) {
			element.setName("symbol");
			element.setAttribute(NddlXmlStrings.x_type, symbolType);
			return true;
		}
		if (variableType != null) {
			element.removeAttribute(NddlXmlStrings.x_value);
			element.setName("id");
			element.setAttribute(NddlXmlStrings.x_name, name);
			element.setAttribute(NddlXmlStrings.x_type, variableType);
			return true;
		}
		error(XMLUtil.locationString(element) + ":unresolved ident: '" + name + "'");
		return false;
	}

	/**
	 * @brief look up the symbol's type, and add it to the symbol
	 * @returns true if the decoration succeeded
	 */
	public boolean decorateSymbol(IXMLElement element) {
		String name = XMLUtil.getAttribute(element, NddlXmlStrings.x_value);
		String symbolType = typeResolver.findSymbolType(element, name);
		if (symbolType != null) {
			element.setName("symbol");
			element.setAttribute(NddlXmlStrings.x_type, symbolType);
			return true;
		}
		error(XMLUtil.locationString(element) + ":unresolved symbol: '" + name + "'");
		return false;
	}

	/**
	 * @brief change an identifier into a decorated symbol or decorated identifier
	 * First attempts to find quotes or double quotes. If successful the indentifier 
	 * is an enumeration symbol, change the xml to a decorated symbol.
	 * Then attempts to break up the symbol into two parts and look up the identifier 
	 * as a variable name, and decorate it with the type.
	 * @returns true if the decoration succeeded
	 */
	public boolean decorateIdentifier(IXMLElement element) {
		String name = XMLUtil.getAttribute(element, NddlXmlStrings.x_name);
		int location = name.indexOf("'");
		if (location == -1)
		    location = name.indexOf('"');
		if (location != -1) {
		    int dotlocation = location -1;
		    String enumType = name.substring(0, dotlocation);
			IXMLElement context = typeResolver.findTypeContextNamed(element, enumType);
			if ((context != null) && (context.getName().equals("enum"))) {
				String symbol = name.substring(dotlocation + 1);
				if (symbol.startsWith("'") && symbol.endsWith("'")) {
					symbol = symbol.substring(1, symbol.length() - 1);
				}
				if (typeResolver.isEnumerationMember(context, symbol)) {
					element.setName("symbol");
					element.setAttribute(NddlXmlStrings.x_value, symbol);
					element.setAttribute(NddlXmlStrings.x_type, enumType);
					element.removeAttribute(NddlXmlStrings.x_name);
					return true;
				}
			}
		}
		String variableType = typeResolver.findVariableType(element, name);
		if (variableType != null) {
			element.setAttribute(NddlXmlStrings.x_type, variableType);
			return true;
		}
		error(XMLUtil.locationString(element) + ":unresolved id: '" + name + "'");
		return false;
	}

	/**
	 * @brief decorate assignments using the decorateIdentifier(element) function
	 * @returns true if the decoration succeeded
	 */
	public boolean decorateAssign(IXMLElement element) {
		return decorateIdentifier(element);
	}

	/**
	 * @brief add type information to a predicate instance
	 * @returns true if the decoration succeeded
	 */
	public boolean decoratePredicateInstance(IXMLElement element) {
		String name = element.getAttribute(NddlXmlStrings.x_name, null);
		String type = XMLUtil.getAttribute(element, NddlXmlStrings.x_type);
		if (name == null) {
			// try interpreting the type as a name
			String variableType = typeResolver.findVariableType(element, type);
			if (variableType != null) {
				element.setName("id");
				element.setAttribute(NddlXmlStrings.x_name, type);
				element.setAttribute(NddlXmlStrings.x_type, variableType);
				return true;
			}
		}
		IXMLElement context = typeResolver.findTypeContextNamed(element, type);
		if (context != null) {
			return true;
		}
		error(XMLUtil.locationString(element) + ":unresolved predicate specification: '" + type + "'");
		return false;
	}

	/**
	 * @brief add type information to a loop
	 * @returns true if the decoration succeeded
	 */
	public boolean decorateLoop(IXMLElement element) {
		String name = XMLUtil.getAttribute(element, NddlXmlStrings.x_value);
		String variableType = typeResolver.findVariableType(element, name);
		if (variableType != null) {
			element.setAttribute(NddlXmlStrings.x_type, variableType);
			return true;
		}
		error(XMLUtil.locationString(element) + ":unresolved loop domain: '" + name + "'");
		return false;
	}

	/**
	 * @brief add type information to a loop
	 * @returns true if the decoration succeeded
	 */
	public boolean decorateSet(IXMLElement element) {
		String setType = null;
		Enumeration enumeration = element.enumerateChildren();
		while (enumeration.hasMoreElements()) {
			IXMLElement child = (IXMLElement)enumeration.nextElement();
			if (!child.getName().equals("symbol") &&
			    !child.getName().equals("value")) {
				error(XMLUtil.locationString(child) + ":missing type for set element");
				return false;
			}
			String childType = XMLUtil.getAttribute(child, NddlXmlStrings.x_type);
			if (setType == null) {
				setType = childType;
				continue;
			}
			if (childType.equals("float") && setType.equals("int")) {
				setType = "float";
				continue;
			}
			if (childType.equals("int") && setType.equals("float")) {
				continue;
			}
			if (!childType.equals(setType)) {
				error(XMLUtil.locationString(element) + ":inconsistent types for set elements");
				return false;
			}			
		}
		if (setType == null) {
			// TODO: trivial empty set, what can the type be?
		} else {
			element.setAttribute(NddlXmlStrings.x_type, setType);
		}
		return true;
	}

}
