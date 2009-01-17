/* vim: set ts=8 ft=antlr3: */
tree grammar NDDL3Tree;

options {
	language=C;
	tokenVocab=NDDL3;
	ASTLabelType=pANTLR3_BASE_TREE;
}

@includes
{
#include "Interpreter.hh"

using namespace EUROPA;
}


nddl[Expr*& expr]
@init {
    expr = new ExprNoop("NDDL3");	
}
        :               
	^(NDDL
		(	typeDefinition[expr]
		|	classDeclaration[expr]
		|	rule[expr]
		|	goal[expr]
		|	fact[expr]
		|	relation[expr]
		|	constraintInstantiation[expr]
		|	allocation[expr]
		|	assignment[expr]
		|	variableDeclaration[expr]
		|	invocation[expr]
		)*)
	;

// ==========================================================
// enumerations
// ==========================================================

// enumeration defined at top level

enumeration[Expr* expr]
	:	^('enum'
			IDENT
			valueSet[expr]
		)
	;

typeDefinition[Expr* expr]
	:	^('typedef'
			name=IDENT
			type[expr]
		)
	;

classDeclaration[Expr* expr]
	:	^('class'
			name=IDENT
			((^('extends' extends=IDENT)
				classBlock[expr])
			|	classBlock[expr]
			|	';'
			)
		)
  ;

classBlock[Expr* expr]
	:	^('{'
			componentTypeEntry[expr]*
		)
	;

componentTypeEntry[Expr* expr]
	:	variableDeclaration[expr]
	|	constructor[expr]
	|	predicate[expr]
	|	constraintInstantiation[expr]
	;

constructor[Expr* expr]
	:	^(CONSTRUCTOR
			name=IDENT
			^('(' constructorArgument[expr]*)
			^('{'	(	constructorSuper[expr]
				|	assignment[expr])*)
		)
	;

constructorArgument[Expr* expr]
	:	^(VARIABLE
			IDENT
			type[expr]
		)
	;

constructorSuper[Expr* expr]
	:	^('super'
			variableArgumentList[expr]
		)
	;
  
assignment[Expr* expr]
	:	^('='
			qualified[expr]
			initializer[expr]
		)
	;

predicate[Expr* expr]
	:	^('predicate'
			pred=IDENT
			predicateStatements[expr]
		)
	;

predicateStatements[Expr* expr]
	:	^('{'
			ruleStatement[expr]*
		)
	;

rule[Expr* expr]
 	:	^('::'
			className=IDENT
			predicateName=IDENT
			ruleBlock[expr]
		)
	;

ruleBlock[Expr* expr]
	:	^('{'
			ruleStatement[expr]*
		)
	;

ruleStatement[Expr* expr]
	:	relation[expr]
	|	constraintInstantiation[expr]
	|	assignment[expr]
	|	variableDeclaration[expr]
	|	ifStatement[expr]
	|	loopStatement[expr]
	|	ruleBlock[expr]
	;


ifStatement[Expr* expr]
	:	^('if'
			expression[expr]
			ruleBlock[expr]
			ruleBlock[expr]?
		)
  ;

loopStatement[Expr* expr]
	:	^('foreach'
			name=IDENT
			val=identifier[expr]
			ruleBlock[expr]
		)
	;

// ==========================================================
// expressions
// ==========================================================

expression[Expr* expr]
	:	^('=='
			anyValue[expr]
			anyValue[expr]
		)
	|	^('!='
			anyValue[expr]
			anyValue[expr]
		)
	|	anyValue[expr]
	;

goal[Expr* expr]
	:	^('goal'
			predicateArgumentList[expr]
		)
	|	^('rejectable'
			predicateArgumentList[expr]
		)
	;

fact[Expr* expr]
	:	^('fact'
			predicateArgumentList[expr]
		)
  ;

relation[Expr* expr]
	:	^(SUBGOAL
			originName=identifier[expr]?
			(	tr=temporalRelationNoInterval[expr]
			|	tr=temporalRelationOneInterval[expr]
				numericInterval[expr]?
			|	tr=temporalRelationTwoIntervals[expr]
				(numericInterval[expr]
				numericInterval[expr]?)?
			)
			predicateArgumentList[expr]
		)
	;

predicateArgumentList[Expr* expr]
	:	^('('
			predicateArgument[expr]*
		)
		|	target=IDENT
	;

predicateArgument[Expr* expr]
	:	^(qualified[expr] name=IDENT?)
	;
	
qualified[Expr* expr]
        :       identifier[expr]                 
        |       ^('.' identifier[expr] qualified[expr]*)
        ;
	

temporalRelationNoInterval[Expr* expr]
	:	'any'
	|	'equals'
	|	'meets'
	|	'met_by'
	;
  
temporalRelationOneInterval[Expr* expr]
	:	'ends'
	|	'starts'
	|	'after'
	|	'before'
 	|	'ends_after_start'
	|	'starts_before_end'
	|	'ends_after'
	|	'ends_before'
	|	'starts_after'
	|	'starts_before'
	;

temporalRelationTwoIntervals[Expr* expr]
	:	'contained_by'
	|	'contains'
	|	'paralleled_by'
	|	'parallels'
	|	'starts_during'
	|	'contains_start'
	|	'ends_during'
	|	'contains_end'
	;
  
constraintInstantiation[Expr* expr]
	:	^(CONSTRAINT_INSTANTIATION
			name=IDENT
			variableArgumentList[expr]
		)
	;

invocation[Expr* expr]
	:	^('specify' i=IDENT variableArgumentList[expr])
	|	^('free' i=IDENT variableArgumentList[expr])
	|	^('constrain' i=IDENT variableArgumentList[expr])
	|	^('merge' i=IDENT variableArgumentList[expr])
	|	^('close' i=IDENT)
	|	^('activate' i=IDENT)
	|	^('reject' i=IDENT)
	|	^('cancel' i=IDENT)
	|	^('reset' i=IDENT)
	;

variableDeclaration[Expr* expr]
	:	^(VARIABLE type[expr] variableInitialization[expr])
	;
	
variableInitialization[Expr* expr]
        :       name=IDENT
        |       ^('=' name=IDENT initializer[expr])
        ;	

initializer[Expr* expr]
	:	anyValue[expr] 
	|       allocation[expr]
	;

allocation[Expr* expr]
	:	^(CONSTRUCTOR_INVOCATION
			name=IDENT 
			variableArgumentList[expr]?
		)
	;

variableArgumentList[Expr* expr]
	:	^('('
			initializer[expr]*
		)
	;

identifier[Expr* expr]
	:	IDENT
	|	'this'
	;

type[Expr* expr]
        :       simpleType[expr] 
        |       ^(simpleType[expr] inlineType[expr])
        ;

simpleType[Expr* expr]
	:	'int'
	|	'float'
	|	'boolean'
	|	'string'
	|	object=IDENT
	;
	
inlineType[Expr* expr]
        :       valueSet[expr]
        |       numericInterval[expr]
        ;	

anyValue[Expr* expr]
	:	value[expr]
	|	valueSet[expr]
	|	numericInterval[expr]
	;

valueSet[Expr* expr]
	:	^('{'
			value[expr]*
		)
	;

numericInterval[Expr* expr]
	:	^('['   
			lower=number[expr]
			upper=number[expr]
		)
	;

value[Expr* expr]
	:	booleanValue[expr]
	|	^(i=IDENT type[expr]?)
	|	str=STRING
	|	element=number[expr]
	;

booleanValue[Expr* expr]
	:	'true'
	|	'false'
	;


number[Expr* expr]
	:	floating=FLOAT // real number
	|	integer=INT
	|	pinf='inf'
	|	ninf='-inf'
	;
