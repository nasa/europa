// run antlr.Tool on this file to generate a tree parser

header {
package nddl;

import net.n3.nanoxml.*;
import java.io.*;
import java.util.Arrays;

}

class NddlTreeParser extends TreeParser;

options {
  importVocab = Nddl; // call its vocabulary "nddl"
}

{
	NddlParserState state = null;
	public NddlTreeParser(NddlParserState state) {
		super();
		this.state = state;
	}
	protected void copyPosition(IXMLElement object, AST n) throws ClassCastException {
		if(!(n instanceof NddlASTNode))
			throw new ClassCastException("cannot copy position from AST!");
		NddlASTNode node = (NddlASTNode) n;
		if ((node!=null) && (object!=null)) {
			if (node.getLine()>=0) {
				object.setAttribute(NddlXmlStrings.x_line,Integer.toString(node.getLine()));
			}
			if (node.getColumn()>=0) {
			  object.setAttribute(NddlXmlStrings.x_column,Integer.toString(node.getColumn()));
			}
		}
	}

	protected void copyFilename(IXMLElement object, AST n) throws ClassCastException {
		if(!(n instanceof NddlASTNode))
			throw new ClassCastException("cannot copy filename from AST!");
		NddlASTNode node = (NddlASTNode) n;
		if ((node!=null) && (object!=null)) {
			if (node.getColumn()>=0) {
			  object.setAttribute(NddlXmlStrings.x_filename,node.getFilename());
			}
		}
	}

	protected static IXMLElement clone(IXMLElement o)
	{
		IXMLElement toRet = o.createElement(o.getName());
		java.util.Properties attrs = o.getAttributes();
		for(java.util.Iterator i = attrs.keySet().iterator(); i.hasNext();)
		{
			String attrName = (String)i.next();
			toRet.setAttribute(attrName,(String)attrs.get(attrName));
		}

		java.util.Vector children = o.getChildren();
		for(int i=0;i<children.size();i++)
			toRet.addChild(clone((IXMLElement)children.get(i)));
		return toRet;
	}
	// utility to convert else into a negative if test
	protected static IXMLElement negate(IXMLElement positive) throws ClassCastException {
		IXMLElement toRet = clone(positive);
		if(positive.getName().equals("equals"))
			toRet.setName("nequals");
		else if(positive.getName().equals("nequals"))
			toRet.setName("equals");
		else
			throw new ClassCastException("Must perform expression negations on comparison operators.");
		return toRet;
	}

	/**
	 * Custom traceIn for Antlr which uses debugMsg.
	 */
	public void traceIn(String rname, AST t) {
		char[]indent = new char[traceDepth];
		Arrays.fill(indent,' ');
		++traceDepth;

		StringBuffer marker = new StringBuffer(rname.length()+23);
		marker.append("NddlTreeParser:traceIn:").append(rname);

		StringBuffer data = new StringBuffer(traceDepth + rname.length()+14); // attempt to hit the correct length right off
		data.append(indent).append("> ").append(rname).append((inputState.guessing > 0)? "; [guessing]" : "; ");
		assert(DebugMsg.debugMsg(marker.toString(), data.toString(), false));
	}

	/**
	 * Custom traceOut for Antlr which uses debugMsg.
	 */
	public void traceOut(String rname, AST t) {
		--traceDepth;
		char[]indent = new char[traceDepth];
		Arrays.fill(indent,' ');

		StringBuffer marker = new StringBuffer(rname.length()+24);
		marker.append("NddlTreeParser:traceOut:").append(rname);

		StringBuffer data = new StringBuffer(traceDepth + rname.length()+14); // attempt to hit the correct length right off
		data.append(indent).append("< ").append(rname).append((inputState.guessing > 0)? "; [guessing]" : "; ");
		assert(DebugMsg.debugMsg(marker.toString(), data.toString(), false));
	}
}

// ==========================================================
// toplevel start rule for the parser
// ==========================================================

// this is the start rule for the parser
// handles one jmpl parse tree
nddl[IXMLElement environment]
	: #(NDDL
    ( inclusion[environment]
    | enumeration[environment]
    | typeDefinition[environment]
    | classDeclaration[environment]
    | rule[environment]
    | goal[environment]
    | {IXMLElement goal;} goal=relation[environment]
			{goal.setName("goal");}
    | constraintInstantiation[environment]
    | allocation[environment]
		| assignment[environment]
    | variableDeclaration[environment]
		| invocation[environment]
    )*)
  ;

// ==========================================================
// include
// ==========================================================

inclusion[IXMLElement parent]
{IXMLElement include = new XMLElement("include");}
  : #(INCLUDE_DECL
	    { if (parent != null) parent.addChild(include);
			  copyPosition(include,_t); }
      str:STRING
    	{ String filename = str.getText();
    	  include.setAttribute(NddlXmlStrings.x_name,filename.substring(1,filename.length()-1)); }
     )
  ;

// ==========================================================
// enumerations
// ==========================================================

// enumeration defined at top level

enumeration![IXMLElement parent]
{IXMLElement enm = new XMLElement("enum");}
  : #(ENUM_KEYWORD
      { if (parent != null) parent.addChild(enm);
        copyPosition(enm,_t); copyFilename(enm,_t); }
      name:IDENT 
      { enm.setAttribute(NddlXmlStrings.x_name,name.getText());}
      valueSet[enm]
     )
  ;

typeDefinition![IXMLElement parent]
{IXMLElement enm = new XMLElement("typedef");}
  : #(TYPEDEF_KEYWORD
      { if (parent != null) parent.addChild(enm);
        copyPosition(enm,_t); copyFilename(enm,_t); }
      name:IDENT 
		 #(type:TYPE
         {enm.setAttribute(NddlXmlStrings.x_name,name.getText());
          enm.setAttribute(NddlXmlStrings.x_basetype,type.getText());}
        anyValue[enm])
		 )
  ;

symbolList![IXMLElement parent]
{IXMLElement set = new XMLElement("set");}
		: #(LBRACE
		    { if (parent != null) parent.addChild(set);
				  set.setAttribute(NddlXmlStrings.x_type,parent.getAttribute(NddlXmlStrings.x_name,null));
          /*copyPosition(set,_t);*/ }
					(sy:SYMBOL
   		 { IXMLElement symbol = new XMLElement("symbol");
    	   symbol.setAttribute(NddlXmlStrings.x_value,sy.getText());
   		   symbol.setAttribute(NddlXmlStrings.x_type,parent.getAttribute(NddlXmlStrings.x_name,null));
				 set.addChild(symbol); })*
				)
    ;

// ==========================================================
// component types
// ==========================================================

// this is valid at the toplevel
classDeclaration![IXMLElement parent]
{IXMLElement classDeclaration = new XMLElement("class");}
  : #(CLASS_KEYWORD
      { if (parent != null) parent.addChild(classDeclaration);
        copyPosition(classDeclaration,_t); copyFilename(classDeclaration,_t); }
      name:IDENT // name of the component type
    (#(EXTENDS_KEYWORD i:IDENT))?
			classBlock[classDeclaration]
      {classDeclaration.setAttribute(NddlXmlStrings.x_name,name.getText());
	     if(i!=null) classDeclaration.setAttribute(NddlXmlStrings.x_extends,i.getText());}
     )
  ;

classBlock[IXMLElement parent] {IXMLElement cte;}
: #(LBRACE (componentTypeEntry[parent])*)
;

// lots of things can be in a component type
componentTypeEntry![IXMLElement parent]
  : variableDeclaration[parent]
  | constructor[parent]
  | predicate[parent]
  | constraintInstantiation[parent]
  ;

// ==========================================================
// constructors
// ==========================================================

constructor![IXMLElement parent]
{IXMLElement constructor = new XMLElement("constructor");}
  : #(CONSTRUCTOR
      { if (parent != null) parent.addChild(constructor);
        copyPosition(constructor,_t); }
      name:IDENT // should be same as the class name
      #(LPAREN (constructorArgument[constructor])*)
      #(LBRACE (constructorSuper[constructor] 
               |assignment[constructor])*)
    )
  ;

constructorArgument![IXMLElement parent]
{IXMLElement argument = new XMLElement("arg");}
  : #(VARIABLE
      { if (parent != null) parent.addChild(argument);
        /*copyPosition(argument,_t);*/ }
      name:IDENT
      { argument.setAttribute(NddlXmlStrings.x_name,name.getText()); }
			type:TYPE
      { argument.setAttribute(NddlXmlStrings.x_type,type.getText()); }
    )
  ;

constructorSuper[IXMLElement parent]
{IXMLElement superClass = new XMLElement("super");}
  : #(SUPER_KEYWORD
      { if (parent != null) parent.addChild(superClass);
        /*copyPosition(superClass,_t);*/}
      variableArgumentList[superClass]
    )
  ;
  
assignment![IXMLElement parent]
{IXMLElement assign = new XMLElement("assign");}
  : { if (parent != null) parent.addChild(assign);}
    (#(EQUALS #(i:IDENT type:TYPE) initializer[assign] (x:EXTENDS_KEYWORD)?
      { copyPosition(assign,i);
        assign.setAttribute(NddlXmlStrings.x_name,i.getText());
        assign.setAttribute(NddlXmlStrings.x_type,type.getText());
				if(x!=null) assign.setAttribute(NddlXmlStrings.x_inherited,"true");})
    | #(IN_KEYWORD #(i2:IDENT type2:TYPE) initializer[assign] (x2:EXTENDS_KEYWORD)?
      { copyPosition(assign,i2);
        assign.setAttribute(NddlXmlStrings.x_name,i2.getText());
        assign.setAttribute(NddlXmlStrings.x_type,type2.getText());
				if(x2!=null) assign.setAttribute(NddlXmlStrings.x_inherited,"true");})
    )
  ;

// ==========================================================
// predicate
// ==========================================================

predicate![IXMLElement parent]
{IXMLElement predicate = new XMLElement("predicate");}
  : #(PREDICATE_KEYWORD
      { if (parent != null) parent.addChild(predicate); }
      { copyPosition(predicate,_t); }
      name:IDENT // should be same as the class name
      { predicate.setAttribute(NddlXmlStrings.x_name,name.getText()); }
      predicateStatements[predicate]
    )
  ;

predicateStatements![IXMLElement parent]
{IXMLElement group = (parent != null ? parent : new XMLElement("group"));}
  : #(LBRACE
      (ruleStatement[group])*
     )
  ;

// ==========================================================
// compatibility
// ==========================================================

rule![IXMLElement parent]
{IXMLElement rule = new XMLElement("compat");}
  : #(DCOLON
      { if (parent != null) parent.addChild(rule);
		    copyPosition(rule,_t); copyFilename(rule,_t); }
      i:IDENT
      { rule.setAttribute(NddlXmlStrings.x_class,i.getText()); }
      predicateName:IDENT
      { rule.setAttribute(NddlXmlStrings.x_name,predicateName.getText());}
      ruleBlock[rule]
     )
  ;

ruleBlock![IXMLElement parent]
{IXMLElement group = new XMLElement("group");}
  : { if (parent != null) parent.addChild(group); }
	  #(LBRACE (ruleStatement[group])*)
  ;

ruleStatement![IXMLElement parent]
  : {IXMLElement stub;} stub=relation[parent]
  | constraintInstantiation[parent]
  | assignment[parent]
  | variableDeclaration[parent]
  | ifStatement[parent]
  | loopStatement[parent]
	| ruleBlock[parent]
  ;


ifStatement![IXMLElement parent]
{IXMLElement statement = new XMLElement("if");}
  : #(IF_KEYWORD
      { if (parent != null) parent.addChild(statement); }
      expression[statement]
      ruleBlock[statement]
      ( 
				{IXMLElement negation = new XMLElement("if");
				 if (parent != null) parent.addChild(negation);
				 negation.addChild(negate(statement.getChildAtIndex(0)));}
				ruleBlock[negation]
			)?
     )
  ;

loopStatement![IXMLElement parent]
{IXMLElement loop = new XMLElement("loop");}
  : #(FOREACH_KEYWORD
      { if (parent != null) parent.addChild(loop); }
      { copyPosition(loop,_t); }
      name:IDENT
      { loop.setAttribute(NddlXmlStrings.x_name,name.getText()); }
      { String value; }
        value=identifier
			  type:TYPE
      { loop.setAttribute(NddlXmlStrings.x_value,value);
        loop.setAttribute(NddlXmlStrings.x_type,type.getText()); }
      ruleBlock[loop]
     )
  ;

// ==========================================================
// expressions
// ==========================================================

expression![IXMLElement parent]
  : #(DEQUALS
			{IXMLElement exp = new XMLElement("equals");
			 if (parent != null) parent.addChild(exp); }
			anyValue[exp]
			anyValue[exp]
			)
  | #(NEQUALS
			{IXMLElement exp = new XMLElement("nequals");
			 if (parent != null) parent.addChild(exp); }
			anyValue[exp]
			anyValue[exp]
			)
	| anyValue[parent]
  ;

// ==========================================================
// relations
// ==========================================================

goal![IXMLElement parent]
{IXMLElement goal = new XMLElement("goal");}
  : { if (parent != null) parent.addChild(goal);
	    copyPosition(goal,_t);}
    #(GOAL_KEYWORD { goal.setAttribute(NddlXmlStrings.x_mandatory, NddlXmlStrings.x_true); }
      predicateArgumentList[goal])
  | { if (parent != null) parent.addChild(goal); }
	  #(REJECTABLE_KEYWORD
      predicateArgumentList[goal]
     )
  ;

relation![IXMLElement parent]
returns [IXMLElement relation = new XMLElement("subgoal");]
  : #(SUBGOAL
      { if (parent != null) parent.addChild(relation); }
      { copyPosition(relation,_t); }
      { String originName; }
      (originName=identifier
       { relation.setAttribute(NddlXmlStrings.x_origin,originName); }
      )?
      ( t0:temporalRelationNoInterval
        { relation.setAttribute(NddlXmlStrings.x_relation,t0.getText()); }
      | t1:temporalRelationOneInterval
        { relation.setAttribute(NddlXmlStrings.x_relation,t1.getText()); }
        (numericInterval[relation])?
      | t2:temporalRelationTwoIntervals
        { relation.setAttribute(NddlXmlStrings.x_relation,t2.getText()); }
        (numericInterval[relation]
         (numericInterval[relation])?
        )?
      )
      predicateArgumentList[relation]
     )
  ;

predicateArgumentList![IXMLElement relation]
  : #(LPAREN
		(#(type:TYPE
		{IXMLElement pred = new XMLElement("predicateinstance");
     pred.setAttribute(NddlXmlStrings.x_type,type.getText());}
		(name:IDENT
     {pred.setAttribute(NddlXmlStrings.x_name,name.getText());}
		)? {relation.addChild(pred);}))*)
	| target:IDENT { if (relation != null) relation.setAttribute(NddlXmlStrings.x_target,target.getText()); }
  ;

temporalRelationNoInterval
  : TR_ANY_KEYWORD
  | TR_EQUALS_KEYWORD
  | TR_MEETS_KEYWORD
  | TR_MET_BY_KEYWORD
  ;
  
temporalRelationOneInterval
  : TR_ENDS_KEYWORD
  | TR_STARTS_KEYWORD
  | TR_AFTER_KEYWORD
  | TR_BEFORE_KEYWORD
  | TR_ENDS_AFTER_START_KEYWORD
  | TR_STARTS_BEFORE_END_KEYWORD
  | TR_ENDS_AFTER_KEYWORD
  | TR_ENDS_BEFORE_KEYWORD
  | TR_STARTS_AFTER_KEYWORD
  | TR_STARTS_BEFORE_KEYWORD
  ;

temporalRelationTwoIntervals
  : TR_CONTAINED_BY_KEYWORD
  | TR_CONTAINS_KEYWORD
  | TR_PARALLELED_BY_KEYWORD
  | TR_PARALLELS_KEYWORD
  | TR_STARTS_DURING_KEYWORD
  | TR_CONTAINS_START_KEYWORD
  | TR_ENDS_DURING_KEYWORD
  | TR_CONTAINS_END_KEYWORD
  ;
  
// ==========================================================
// constraints
// ==========================================================

constraintInstantiation![IXMLElement parent]
{IXMLElement constraint = new XMLElement("invoke");}
  : #(CONSTRAINT_INSTANTIATION
      { if (parent != null) parent.addChild(constraint); }
      { copyPosition(constraint,_t); }
      name:IDENT
      { constraint.setAttribute(NddlXmlStrings.x_name,name.getText()); }
      { IXMLElement arg; }
      variableArgumentList[constraint]
     )
  ;

invocation![IXMLElement parent]
{IXMLElement invocation = new XMLElement("invoke");}
  : { if (parent != null) parent.addChild(invocation);
	    copyPosition(invocation,_t); }
	( #(SPECIFY_KEYWORD i1:IDENT variableArgumentList[invocation]
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i1.getText());
		    invocation.setAttribute(NddlXmlStrings.x_name, "specify"); })
  |	#(FREE_KEYWORD i2:IDENT variableArgumentList[invocation]
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i2.getText());
		    invocation.setAttribute(NddlXmlStrings.x_name, "free"); })
  |	#(CONSTRAIN_KEYWORD i3:IDENT variableArgumentList[invocation]
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i3.getText());
		    invocation.setAttribute(NddlXmlStrings.x_name, "constrain"); })
  |	#(MERGE_KEYWORD i4:IDENT variableArgumentList[invocation]
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i4.getText());
		    invocation.setAttribute(NddlXmlStrings.x_name, "merge"); })
	| #(CLOSE_KEYWORD (i5:IDENT
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i5.getText());})?
		  { invocation.setAttribute(NddlXmlStrings.x_name, "close"); })
	| #(ACTIVATE_KEYWORD i6:IDENT
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i6.getText());
		    invocation.setAttribute(NddlXmlStrings.x_name, "activate"); })
	| #(REJECT_KEYWORD i7:IDENT
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i7.getText());
		    invocation.setAttribute(NddlXmlStrings.x_name, "reject"); })
	| #(CANCEL_KEYWORD i8:IDENT
		  { invocation.setAttribute(NddlXmlStrings.x_identifier,i8.getText());
		    invocation.setAttribute(NddlXmlStrings.x_name, "cancel"); }))
	;
// ==========================================================
// variables/identifiers
// ==========================================================

variableDeclaration![IXMLElement parent]
{IXMLElement var = new XMLElement("var");}
  : #(VARIABLE
	    { if (parent != null) parent.addChild(var);}
      name:IDENT
			{copyPosition(var,name);
       var.setAttribute(NddlXmlStrings.x_name,name.getText()); }
			type:TYPE
      {var.setAttribute(NddlXmlStrings.x_type,type.getText()); }
			(initializer[var])?
     )
  ;

/*! initializer := anyValue | allocation
 *
 initializer is used where you can create a new value
 or specify an existing variable, or a specific value */
initializer![IXMLElement parent]
  : anyValue[parent]
  | allocation[parent]
  ;

allocation![IXMLElement parent]
{IXMLElement alloc = new XMLElement("new");}
  : #(CONSTRUCTOR_INVOCATION
      { if (parent != null) parent.addChild(alloc); }
      name:IDENT 
      { alloc.setAttribute(NddlXmlStrings.x_type,name.getText()); }
      ( variableArgumentList[alloc] )?
     )
  ;

variableArgumentList![IXMLElement parent]
  : #(LPAREN (initializer[parent])*)
  ;

identifier!
returns [String name = null;]
  : ( i:IDENT {name = i.getText();}
		| THIS_KEYWORD {name = "this";})
  ;

// modifiers for nddl attributes
accessModifier[IXMLElement parent]
{IXMLElement keyword = new XMLElement("modifier");}
  : #(ACCESS_MODIFIER 
      ( PRIVATE_KEYWORD   
        { keyword.setAttribute(NddlXmlStrings.x_name,"private"); }
      | PROTECTED_KEYWORD
        { keyword.setAttribute(NddlXmlStrings.x_name,"protected"); }
      | PUBLIC_KEYWORD
        { keyword.setAttribute(NddlXmlStrings.x_name,"public"); }
      )
     )
  ;

/*! ==========================================================
  values
  ========================================================== */

type[IXMLElement parent]
 : INT_KEYWORD { parent.setAttribute(NddlXmlStrings.x_type,"int"); }
 | FLOAT_KEYWORD { parent.setAttribute(NddlXmlStrings.x_type,"float"); }
 | BOOL_KEYWORD { parent.setAttribute(NddlXmlStrings.x_type,"bool"); }
 | STRING_KEYWORD { parent.setAttribute(NddlXmlStrings.x_type,"string"); }
 | i:IDENT { parent.setAttribute(NddlXmlStrings.x_type,#i.getText()); }
 ;

anyValue![IXMLElement parent]
  : value[parent]
  | valueSet[parent]
  | numericInterval[parent]
  ;

valueSet![IXMLElement parent]
{IXMLElement set = new XMLElement("set");}
  : #(LBRACE
      { if (parent != null)
			  {
					parent.addChild(set); 
					//possibly other ways to label sets...
					if(parent.hasAttribute(NddlXmlStrings.x_basetype))
			  		set.setAttribute(NddlXmlStrings.x_type, parent.getAttribute(NddlXmlStrings.x_basetype,null));
				}
			}
      ( value[set])*
     )
  ;

numericInterval![IXMLElement parent]
{IXMLElement interval = new XMLElement("interval"), lower, upper;}
  : #(LBRACKET
      { if (parent != null) parent.addChild(interval); }
      lower=number[null] upper=number[null]

      { interval.setAttribute(NddlXmlStrings.x_min,lower.getAttribute(NddlXmlStrings.x_name,null)); }
      { interval.setAttribute(NddlXmlStrings.x_max,upper.getAttribute(NddlXmlStrings.x_name,null)); }
      { if (NddlXmlStrings.x_float.equals(lower.getAttribute(NddlXmlStrings.x_type,null)) ||
            NddlXmlStrings.x_float.equals(upper.getAttribute(NddlXmlStrings.x_type,null))) {
          interval.setAttribute(NddlXmlStrings.x_type,NddlXmlStrings.x_float);
        } else {
          interval.setAttribute(NddlXmlStrings.x_type,NddlXmlStrings.x_int);
        }
      }
     )
  ;

value![IXMLElement parent]
{IXMLElement element = null;}
  : booleanValue[parent]
  | { element = new XMLElement("id");             
      if (parent != null) parent.addChild(element);
      copyPosition(element,_t);}
    #(i:IDENT { element.setAttribute(NddlXmlStrings.x_name,i.getText());}
			 (type:TYPE {element.setAttribute(NddlXmlStrings.x_type,type.getText()); })?)
  | { element = new XMLElement("symbol");
      if (parent != null) parent.addChild(element);
      /*copyPosition(element,_t);*/}
    #(sym:SYMBOL (type2:TYPE)?)
    { element.setAttribute(NddlXmlStrings.x_value,sym.getText());
			if(type2!=null)
      	element.setAttribute(NddlXmlStrings.x_type,type2.getText());
			//here comes YADG (yet another dirty hack)
			else
   		  element.setAttribute(NddlXmlStrings.x_type,parent.getParent().getAttribute(NddlXmlStrings.x_name,null));}

  | { element = new XMLElement("value");
      if (parent != null) parent.addChild(element);
      /* copyPosition(element,_t); */}
    str:STRING
    { String string = str.getText();
      element.setAttribute(NddlXmlStrings.x_name, string.substring(1,string.length()-1));
      element.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_string);}
  | element=number[parent]
  ;


booleanValue![IXMLElement parent]
{IXMLElement value = new XMLElement("value");}
  : { if (parent != null) parent.addChild(value); 
      /*copyPosition(value,_t);*/ }
    ( t:TRUE_KEYWORD // true
      { value.setAttribute(NddlXmlStrings.x_name, NddlXmlStrings.x_true);    }
    | f:FALSE_KEYWORD // false
      { value.setAttribute(NddlXmlStrings.x_name, NddlXmlStrings.x_false);   }
	  )
    {value.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_boolean);}
  ;


/*! number := NUM_FLOAT | NUM_INT
 a number is either a floating point number (real)
   29.79
 or a fixed point number (integer)
   30
 */
number![IXMLElement parent]
returns [IXMLElement value = new XMLElement("value")]
  : { if (parent != null) parent.addChild(value);
      /*copyPosition(value,_t);*/}
    ( f:FLOAT // real number
      { value.setAttribute(NddlXmlStrings.x_name, f.getText());
        value.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_float); }
		| PINFF
      { value.setAttribute(NddlXmlStrings.x_name, "+inf");
        value.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_float); }
		| NINFF
      { value.setAttribute(NddlXmlStrings.x_name, "-inf");
        value.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_float); }
    | i:INT   // integer
      { value.setAttribute(NddlXmlStrings.x_name, i.getText());
        value.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_int); }
		| PINF
      { value.setAttribute(NddlXmlStrings.x_name, "+inf");
        value.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_int); }
		| NINF
      { value.setAttribute(NddlXmlStrings.x_name, "-inf");
        value.setAttribute(NddlXmlStrings.x_type, NddlXmlStrings.x_int); }
	  )
  ;
