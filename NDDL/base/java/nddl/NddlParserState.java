package nddl;

import java.io.File;
import java.io.PrintStream;
import java.util.Stack;
import java.util.Iterator;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import java.util.List;
import java.util.LinkedList;
import java.util.ArrayList;
import java.util.Collections;
import antlr.RecognitionException;
import antlr.CommonAST;
import antlr.Token;
import antlr.collections.AST;
import antlr.SemanticException;

/*
	A class to maintain the state of a NddlParser.  This is mainly for the
	type checking system, but is also a mechanism to pass information out
	to the tree parser.
*/


public class NddlParserState implements NddlTokenTypes
{
	protected Stack parsers = new Stack();
	protected List parsedFiles = new ArrayList();
	protected PrintStream err = System.err;

	private LinkedList context = new LinkedList();
	private int anonymousContext = 0;
	protected int errorCount = 0;
	protected int warningCount = 0;
	protected int suppressedCount = 0;
	protected Set predeclaredClasses = new HashSet();
	protected Map names = new HashMap();
	protected Map primatives = new HashMap();
	protected Map constraints = new HashMap();

	public NddlParserState(NddlParserState init)
	{
		if(init == null)
		{
			addType("Object",new NddlType("Object",null));
			NddlType numeric = new NddlType(NUMERIC,null);
			addPrimative("numeric",numeric);
			addPrimative("int",new NddlType(INT,numeric));
			addPrimative("float",new NddlType(FLOAT,numeric));
			addPrimative("bool",new NddlType(BOOL,numeric));
			addPrimative("string",new NddlType(STRING));
		}
		else
		{
			primatives.putAll(init.primatives);
			names.putAll(init.names);
			constraints.putAll(init.constraints);
			predeclaredClasses.addAll(init.predeclaredClasses);
			parsers.addAll(init.parsers);
			err = init.err;
			errorCount = init.errorCount;
			warningCount = init.warningCount;
		}
	}
	
	public void setErrStream(PrintStream err) {this.err = err;}
	public void addFile(File file) {parsedFiles.add(file);}
	public boolean containsFile(File file) { return parsedFiles.contains(file);}
	public void pushParser(NddlParser parser) {this.parsers.push(parser);}
	public NddlParser popParser() {return (NddlParser)parsers.pop();}
	public NddlParser getParser() {return (NddlParser)parsers.peek();}

	public void resetCounts() {
		warningCount = 0;
		errorCount = 0;
	}

	public void warn(String type, String message)
	{
		++warningCount;
		if(NddlParser.warnings.contains(type) || NddlParser.warnings.contains("all"))
			err.println(getErrorPrefix("warning", null)+message);
		else
			++suppressedCount;
	}
	public void warn(String type, Exception ex)
	{
		++warningCount;
		if(NddlParser.warnings.contains(type) || NddlParser.warnings.contains("all"))
			err.println(getErrorPrefix("warning",ex)+ex.getMessage());
		else
			++suppressedCount;
	}
	public void error(String message)
	{
		++errorCount;
		err.println(getErrorPrefix("error",null)+message);
	}
	public void error(Exception ex)
	{
		++errorCount;
		err.println(getErrorPrefix("error",ex)+ex.getMessage());
	}
	private String getErrorPrefix(String type, Exception ex)
	{
		StringBuffer prefix = new StringBuffer(60);

		if(ex instanceof RecognitionException)
		{
			RecognitionException re = (RecognitionException)ex;
			if(re.getFilename() != null && !re.getFilename().equals("null"))
				prefix.append(re.getFilename());
			else if(getParser().getFilename() != null && !getParser().getFilename().equals("null"))
				prefix.append(getParser().getFilename());

			if(re.getLine()>0)
				prefix.append(":").append(re.getLine());
			else if(getParser().getLexer().getLine()>0)
				prefix.append(":").append(getParser().getLexer().getLine());
		}
		else
		{
			prefix.append(getParser().getFilename());
			if(getParser().getLexer().getLine()>0)
				prefix.append(":").append(getParser().getLexer().getLine());
		}
		return prefix.append(": ").append(type).append(": ").toString();
	}

	public int getWarnCount() {return warningCount;}
	public int getSuppressedCount() {return suppressedCount;}

	public int getErrorCount() {return errorCount;}

	public boolean printWarnCount()
	{
		if(warningCount == 0) return false;
		if(warningCount == 1) err.print("1 warning");
		else err.print(warningCount+" warnings");
		if(suppressedCount == 1) err.println(" ("+suppressedCount+" message suppressed)");
		else if(suppressedCount > 1) err.println(" ("+suppressedCount+" messages suppressed)");
		else err.println();
		return true;
	}

	public boolean printErrorCount()
	{
		if(errorCount == 0) return false;
		else if(errorCount == 1) err.println("1 error");
		else err.println(errorCount+" errors");
		return true;
	}

	private NddlName getNddlName(String name)
	{
		NddlName toRet = null;
		if(!names.containsKey(name))
		{
			toRet = new NddlName(name);
			names.put(name, toRet);
		}
		else
			toRet = (NddlName)names.get(name);
		return toRet;
	}

	public void addVariable(String name,NddlType type) throws SemanticException
	{
		NddlVariable var = new NddlVariable(withContext(name),type);
		addVariable(withContext(name),var);
	}
	private void addVariable(String qualified, NddlVariable variable)
	{
		assert(DebugMsg.debugMsg("NddlParserState:addVariable",qualified + "-> "+variable));
		NddlName name = getNddlName(NddlUtil.last(qualified));
		if(!name.addVariable(variable))
			error("Name \""+qualified+"\" previously defined");
	}
	private void addPrimative(String keyword, NddlType type)
	{
		primatives.put(keyword,type);
	}
	private void addType(String qualified, NddlType type)
	{
		assert(DebugMsg.debugMsg("NddlParserState:addType",NddlUtil.last(qualified) + "-> "+type));
		NddlName name = getNddlName(NddlUtil.last(qualified));
		if(predeclaredClasses.contains(qualified))
		{
			assert(DebugMsg.debugMsg("NddlParserState:addType","Redefining predeclared class "+qualified + "-> "+type));
			name.redefineType(type);
			predeclaredClasses.remove(qualified);
		}
		else if(!name.addType(type) && !predeclaredClasses.contains(qualified))
			error("Type \""+qualified+"\" previously defined");
	}
	private void addSymbol(String qualified, NddlType type)
	{
		NddlName name = getNddlName(NddlUtil.last(qualified));
		if(!name.containedByEnum(type))
			error("Symbol \""+qualified+"\" previously defined");
	}
	public void addPredeclaredClass(String type) throws SemanticException
	{
		assert(DebugMsg.debugMsg("NddlParserState:addPredeclaredClass"," Predeclaring \""+type+"\""));
		NddlName name = getNddlName(type);
		if(name.containsType(type))
			error("Cannot forward declare existing type "+name);
		if(predeclaredClasses.contains(name))
			warn("redeclarations","Multiple forward declarations of "+name);
		NddlType superType = getType("Object");
		addType(type,new NddlType(type,superType));
		predeclaredClasses.add(type);
	}

	public String getContext()
	{
		switch(context.size())
		{
			case 0: return "";
			case 1: return (String)context.getFirst();
			default: 
				StringBuffer toRet = new StringBuffer(100);
				toRet.append((String)context.getFirst());
				for(int i=1;i<context.size();i++)
					toRet.append(".").append((String)context.get(i));
				return toRet.toString();
		}
	}
	public boolean isContext(String comp)
	{
		return getContext().equals(comp);
	}
	public boolean isInherited(NddlVariable var)
	{
		return !var.getName().startsWith(NddlUtil.first(getContext())) ||
		       var.getName().matches(".*(object|state|time|start|end|duration)");
	}
	public void closeContext()
	{
		context.removeLast();
	}
	public void openAnonymousContext()
	{
		openContext((++anonymousContext)+"");
	}
	public void openContext(String name)
	{
		context.add(name);
	}

	/*
	 * There have been major flaws in qualification for quite some time now.
	 * The primary problem is that the previous methods of name storage
	 * and retrieval haven't been as expressive as they need to be, this new
	 * method should solve all these issues and more.
	 *
	 * This code was also fairly undocumented, now documentation should
	 * clarify the code enough that it remains maintainable.
	 */

	/**
	 * The primary means for allowing the user to find the NddlVariable or
	 * NddlType which best describes the provided name.
	 *
	 * Reiterating the method by which this goal must be achieved (as it's not
	 * generally as well understood as you might believe)
	 * 
	 * - check primative types for name (int, float, string, bool)
	 * - if (name isn't qualified)
	 *   - search for name
	 *     - in current context
	 *     - in parent(s) of current context
	 *     - start search again in enclosing context
	 * - else
	 *   - split name into required context and goal name
	 *   - 
	 *   - find the first element in the required context given current context
	 *   - recursively search using required context
	 */
	public Object getName(String name, NddlType hint, boolean searchVars, boolean searchTypes, boolean searchPredicates, boolean searchSymbols) throws SemanticException
	{
		assert(DebugMsg.debugMsg("NddlParserState:getName"," getName(\""+name+"\", hint: "+hint+", search: "+(searchVars? "[vars]" : "")+
		                                               (searchTypes? "[types]" : "")+
																									 (searchPredicates? "[predicates]" : "")+
																									 (searchSymbols? "[symbols]" : "")+ ")"));

		// check for primatives, they don't follow the usual naming conventions in NddlType so they
		// require this extra check.
		NddlType primative = getPrimative(name);
		if(primative != null && searchTypes)
			return primative;
		else if(primative != null)
			return null;


		NddlVariable var = null;
		NddlType type = null;
		NddlType pred = null;
		NddlType enumer = null;

		if(NddlUtil.first(name).equals("this"))
			name = NddlUtil.append(getThisTypeName(),NddlUtil.rest(name));
		if(!names.containsKey(NddlUtil.last(name))) return null;

		String reqContext = getRequiredContext(name);
		String unqualifiedName = NddlUtil.last(name);
		NddlName matches = (NddlName)names.get(unqualifiedName);

		// hints will let you avoid flags, use with caution
		if(hint!=null && !hint.isTypeless() && matches.isType(hint)) return hint;

		boolean searching = true; // as soon as we find something at the most specific level, return it
		if(matches != null)
		{
			if(reqContext != null)
			{
				assert(DebugMsg.debugMsg("NddlParserState:getName"," Requiring context: "+reqContext));
				String searchPath = reqContext;
				while(searchPath != null && searching)
				{
					String searchString = NddlUtil.append(searchPath,unqualifiedName);
					assert(DebugMsg.debugMsg("NddlParserState:getName","Searching for: "+searchString));
					if(searchVars && var == null) var = matches.getVariable(searchString);
					if(searchTypes && type == null) type = matches.getType(searchString);
					if(searchPredicates && pred == null) pred = matches.getPredicate(searchString);
					if(searchSymbols && enumer == null) enumer = matches.getEnum(searchString);
					if(type != null || var != null || pred != null || enumer != null) searching = false;
					searchPath = getParentContext(searchPath);
				}
			}
			else
			{
				String enclosingContext = getContext(), parentContext;
				while(enclosingContext != null && !enclosingContext.equals("") && searching)
				{
					parentContext = enclosingContext;
					while(parentContext != null && searching)
					{
						String searchString = NddlUtil.append(parentContext,unqualifiedName);
						assert(DebugMsg.debugMsg("NddlParserState:getName","Searching for: "+searchString));
						if(searchVars && var == null) var = matches.getVariable(searchString);
						if(searchTypes && type == null) type = matches.getType(searchString);
						if(searchPredicates && pred == null) pred = matches.getPredicate(searchString);
						if(searchSymbols && enumer == null) enumer = matches.getEnum(searchString);
						if(type != null || var != null || pred != null || enumer != null) searching = false;
						parentContext = getParentContext(parentContext);
					}
					enclosingContext = NddlUtil.butLast(enclosingContext);
				}
				if(searching)
				{
					if(searchVars && var == null) var = matches.getVariable(unqualifiedName);
					if(searchTypes && type == null) type = matches.getType(unqualifiedName);
					if(searchPredicates && pred == null) pred = matches.getPredicate(unqualifiedName);
					if(searchSymbols && enumer == null) enumer = matches.getEnum(unqualifiedName);
				}
			}
		}

		assert(DebugMsg.debugMsg("NddlParserState:getName","returning getName(\""+name+"\"): "+(var!=null? "[var: "+var+"] " : "")
		                                                                               +(type!=null? "[type: "+type+"] " : "")
		                                                                               +(pred!=null? "[predicate: "+pred+"] " : "")
		                                                                               +(enumer!=null? "[enum: "+enumer+"]" : "")));
		if(var != null && type == null && pred == null && enumer == null) return var;
		else if(type != null && var == null && pred == null && enumer == null) return type;
		else if(pred != null && var == null && type == null && enumer == null) return pred;
		else if(enumer != null && var == null && type == null && pred == null) return enumer;
		else if(var == null && type == null && pred == null && enumer == null) return null;
		
		// contstruct useful exception:
		throw new SemanticException("Could not resolve name \""+name+"\" unambiguously: "+(var!=null? "[var: "+var+"] " : "")+
		                                                                       (type!=null? "[type: "+type+"] " : "")+
		                                                                       (pred!=null? "[predicate: "+pred+"] " : "")+
		                                                                       (enumer!=null? "[enum: "+enumer+"]" : ""));
	}

	private String getRequiredContext(String name) throws SemanticException
	{
		if(name.indexOf('.') == -1) return null;
		NddlType contextType = getNameType(NddlUtil.butLast(name),null,true,true,true,false);
		if(contextType == null) throw new SemanticException("Could not determine type of "+NddlUtil.butLast(name));
		return contextType.getName();
	}

	public boolean isName(String name) {
		if(!names.containsKey(name)) return false;
		NddlName n = getNddlName(name);
		return !n.isEmpty();
	}
	public NddlType getNameType(String name, NddlType hint, boolean searchVars, boolean searchTypes, boolean searchPredicates, boolean searchSymbols) throws SemanticException
	{
		Object o = getName(name,hint,searchVars,searchTypes,searchPredicates,searchSymbols);
		assert(DebugMsg.debugMsg("NddlParserState:getNameType"," Looking for \""+name+"\" found "+o));
		if(o == null) return null;
		else if(o instanceof NddlType) return (NddlType)o;
		else if(o instanceof NddlVariable) return ((NddlVariable)o).getType();
		throw new ClassCastException();
	}
	public NddlVariable getVariable(String name) throws SemanticException { return (NddlVariable)getName(name,null,true,false,false,false); }
	public NddlType getPrimative(String name) throws SemanticException { return (NddlType)primatives.get(name);}
	public NddlType getType(String name) throws SemanticException { return (NddlType)getName(name,null,false,true,false,false); }
	public NddlType getPredicate(String name) throws SemanticException { return (NddlType)getName(name,null,false,false,true,false); }
	public NddlType getSymbol(String name) throws SemanticException { return (NddlType)getName(name,null,false,false,false,true); }

	private String getParentContext(String context) throws SemanticException
	{
		if(context != null && !context.equals(""))
		{
			NddlName name = (NddlName)names.get(NddlUtil.last(context));
			if(name == null) return null;
			NddlType type = name.getType(context);
			if(type != null && type.getSuperType() != null) return type.getSuperType().getName();
			NddlType pred = name.getPredicate(context);
			if(pred != null && pred.getSuperType() != null) return pred.getSuperType().getName();
		}
		return null;
	}

	public void validateConstraint(String name, List arguments) throws SemanticException
	{
		ConstraintSignature cs = null;
		try {
			cs = getConstraint(name, arguments);
		} catch(SemanticException ex) {warn("constraints",ex);}
		for(int i=0;i<arguments.size();i++)
		{
			if(((NddlType)arguments.get(i)).isPredicate())
				throw new SemanticException("Attempt to call constraint "+name+" with at least one token as an argument.");
		}
		if(cs!=null && !cs.matches(arguments))
			throw new SemanticException("Constraint with signature " + cs.getSignature() + " cannot be invoked with "+ arguments);
	}
	public void defineConstraint(ConstraintSignature toDef) throws SemanticException
	{
		String sig = toDef.getSignature();
		if(constraints.containsKey(sig))
			throw new SemanticException("Constraint signature "+sig+" previously defined.");
		constraints.put(sig,toDef);
	}
	public ConstraintSignature getConstraint(String name, List arguments) throws SemanticException
	{
		String sig = name+"["+arguments.size()+"]";
		if(!constraints.containsKey(sig))
			throw new SemanticException("Could not find constraint signature: "+sig);
		return (ConstraintSignature)constraints.get(sig);
	}

	public boolean isType(String name) throws SemanticException
	{
		return getType(name) != null;
	}
	public boolean isObjectType(String name) throws SemanticException
	{
		NddlType type = getVariable(name).getType();
		return type != null && type.isObject();
	}
	public boolean isConstructorType(String name,List argTypes) throws SemanticException
	{
		assert(DebugMsg.debugMsg("NddlParserState:isConstructorType",name +NddlUtil.listAsString(argTypes)));
		NddlType type = null; 
		if(name.equals("super"))
			type = getType(NddlUtil.first(getContext())).getSuperType();
		else
			type = getType(name);
		// the compiler probably needs constructorClass information if I want to ensure that the correct
		// class gets called.
		return type != null && type.hasConstructor(argTypes) != null;
	}
	public boolean isEnumerationType(String name) throws SemanticException
	{
		NddlType type = getType(name);
		return type != null && type.isSymbol();
	}
	public boolean isPredicateType(String name) throws SemanticException
	{
		return getPredicate(name) != null;
	}
	public boolean isSymbol(String symbol) throws SemanticException
	{
		return getSymbol(symbol) != null;
	}
	public boolean isPredicateVariable(String name) throws SemanticException
	{
		NddlVariable var = getVariable(name);
		return var != null && var.getType().isPredicate();
	}
	public boolean isVariable(String name) throws SemanticException
	{
		return getVariable(name) != null;
	}

	public void addClass(String name,String stName) throws SemanticException
	{
		NddlType superType = getType(stName);
		if(superType == null) throw new SemanticException("Could not find type \""+stName+"\" (undeclared)");
		NddlType type = new NddlType(name,superType);
		addType(withContext(name),type);
		Set predicates = superType.getPredicates();
		if(predicates != null)
			// foreach predicate inheirited from this classes parent
			for(Iterator i = predicates.iterator(); i.hasNext();)
			{
				String predicate = (String)i.next();
				// create token variables
				type.addPredicate(predicate);
				addTokenVars(NddlUtil.append(name,predicate));
			}
	}
	public void addPredicate(String name) throws SemanticException
	{
		//find predicate's superType
		NddlType superPred = null;
		NddlType classType = getType(getContext());
		classType.addPredicate(name);
		if(classType.getSuperType() != null)
			superPred = getPredicate(NddlUtil.append(classType.getSuperType().getName(),name));
		NddlName nn = getNddlName(name);
		name = withContext(name);
		assert(DebugMsg.debugMsg("NddlParserState:addPredicate","Attempting to predicate "+name));
		NddlType type = new NddlType(name,superPred,PREDICATE,null);
		if(!nn.addPredicate(type))
			error("Predicate type \""+name+"\" previously defined");
		addTokenVars(name);
	}

	// until a rule for a token is defined, it has no variables.
	public void addTokenVars(String name) throws SemanticException
	{
		NddlName hasVars = (NddlName)names.get("object");
		if(hasVars == null || !hasVars.containsVariable(NddlUtil.append(name,"object")))
		{
			// add the time variable for resources, note that this could cause some problems.
			// resource, and in truth all the default token, variables should be specified explicitly somehow.
			NddlVariable time = new NddlVariable(NddlUtil.append(name,"time"),(NddlType)getPrimative("int").clone());
			addVariable(time.getName(),time);

			// populate with token variables
			NddlVariable object = new NddlVariable(NddlUtil.append(name,"object"),getType(NddlUtil.butLast(name)));
			addVariable(object.getName(),object);

			NddlType tokenStateType = (NddlType)(getType("TokenStates"));
			if(tokenStateType!=null)
			{
				NddlVariable state = new NddlVariable(NddlUtil.append(name,"state"),(NddlType)tokenStateType.clone());
				addVariable(state.getName(),state);
			}

			NddlVariable start = new NddlVariable(NddlUtil.append(name,"start"),(NddlType)getPrimative("int").clone());
			addVariable(start.getName(),start);

			NddlVariable end = new NddlVariable(NddlUtil.append(name,"end"),(NddlType)getPrimative("int").clone());
			addVariable(end.getName(),end);

			NddlVariable duration = new NddlVariable(NddlUtil.append(name,"duration"),(NddlType)getPrimative("int").clone());
			try{duration.getType().intersect(0.0,Double.POSITIVE_INFINITY);} catch(EmptyDomainException ex) {/*not, in fact, possible*/}
			addVariable(duration.getName(),duration);
		}
	}
	public void addEnumeration(String name,Set syms) throws SemanticException
	{
		NddlType type = new NddlType(withContext(name),null,SYMBOL,syms);
		addType(withContext(name),type);
		for(Iterator i = syms.iterator(); i.hasNext();)
			addSymbol((String)i.next(),type);
	}

	// returns mangled name without context
	public String addConstructor(String name,List params) throws SemanticException
	{
		// the constructor name should match the context name
		// if inner classes are ever allowed this may be invalid.
		if(!isContext(name)) throw new SemanticException("Constructors name doesn't match class name: "+name);
		//construct name as name-argtype1-argtype2-
		String mangled = name+"-";
		if(params != null)
		{
			for(int i=0;i<params.size();i++)
				mangled += ((NddlVariable)params.get(i)).getType().mangled()+"-";

			//add parameter variables to names map
			for(int i=0;i<params.size();i++)
			{
				NddlVariable param = (NddlVariable)params.get(i);
				String fullname = NddlUtil.append(withContext(mangled),param.getName());
				addVariable(fullname,new NddlVariable(fullname,param.getType()));
			}
		}
		getType(name).addConstructor(params);
		return mangled;
	}
	public void defineType(String name,NddlType definition) throws SemanticException
	{
		addType(name,definition);
	}
	public String getThisTypeName() throws SemanticException
	{
		String context = getContext();
		NddlName name = (NddlName)names.get(NddlUtil.last(context));
		while(name == null)
		{
			context = NddlUtil.butLast(context);
			name = (NddlName)names.get(NddlUtil.last(context));
		}
		return context;
	}

	public String toString()
	{
		StringBuffer toRet = new StringBuffer(2000);
		toRet.append("Context: ").append(context).append("\n");
		toRet.append("Anonymous Context: ").append(anonymousContext).append("\n");
		// sort and print the names (values of the names map) by name alpha
		toRet.append("Names: \n");
		List sortedNames = new ArrayList(names.keySet()); Collections.sort(sortedNames);
		for(int i=0;i<sortedNames.size();i++)
			toRet.append("  ").append(names.get(sortedNames.get(i))).append("\n");
		toRet.append("Undefined Predeclared Classes: ").append(predeclaredClasses).append("\n");
		return toRet.toString();
	}

	protected String withContext(String name)
	{
		return NddlUtil.append(getContext(),name);
	}
}
