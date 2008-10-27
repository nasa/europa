/* vim: set ts=8 ft=antlr3: */

grammar NDDL3;

options {
	output=AST;
	language=C;
	ASTLabelType=pANTLR3_BASE_TREE;
	//k=4;
}

nddl	:	nddlStatement*
        ;

nddlStatement
	:	inclusion 
        |	constraintSignature 
        |	typeDefinition
        |	variableDeclaration
        |	classDeclaration 
        |	rule 
        |	allocationStatement
        |	assignment
        |	function
        |	constraintInstantiation
        |	relation
        |	goal
        |	noopstatement
        ;

inclusion
	:	'#include' STRING
        ;

typeDefinition
	:	'typedef' typeWithBaseDomain IDENT ';'
	;

typeWithBaseDomain
	:	'int'     (intervalIntDomain|enumeratedIntDomain)?
	|	'float'   (intervalFloatDomain|enumeratedFloatDomain)?
	|	'boolean' enumeratedBoolDomain?
	|	'string'  enumeratedStringDomain?
	|	IDENT     enumeratedObjectDomain?
	;

constraintSignature
	:	'constraint' IDENT typeArgumentList
		('extends' IDENT typeArgumentList)? 
		(signatureBlock | ';') 
	;

signatureBlock
	:	'{' (signatureExpression)? '}'
	;

signatureExpression
	:	signatureAtom (('&&' | '||') signatureAtom)*
	;

signatureAtom
	:	'(' signatureExpression ')'
	|	IDENT '<:' (type | 'numeric' )
	;

classDeclaration
	:	'class' IDENT
		(	(('extends' IDENT)? classBlock)
		|	';')
	;

classBlock
	:	'{' classStatement* '}'
	;

classStatement
	:	variableDeclaration
	|	constructor
	|	predicate
	|	noopstatement
	;

constructor
	:	IDENT constructorParameterList constructorBlock
	;

constructorBlock
	:	'{' constructorStatement* '}'
	;

constructorStatement
	:	assignment
	|	superInvocation
	|	noopstatement
	;

constructorParameterList
	:	'(' constructorParameters? ')'
	;

constructorParameters
	:	constructorParameter  (',' constructorParameters)?
	;

constructorParameter
	:	type IDENT
	;

predicate
	:	'predicate' IDENT predicateBlock 
	;

predicateBlock
	:	'{' predicateStatement* '}'
	;

// Note: Allocations are not legal here.
predicateStatement
	:	variableDeclaration
	|	constraintInstantiation
	|	assignment
	;


rule	:	IDENT '::' IDENT ruleBlock
	;

ruleBlock
	:	'{' ruleStatement* '}'
	|	ruleStatement
	;

ruleStatement
	:	relation
	|	variableDeclaration
	|	constraintInstantiation
	|	flowControl
	|	noopstatement
	;

type	:	'int'
	|	'float'
	|	'boolean'
	|	'string'
	|	IDENT
	;

relation:	(IDENT | 'this')? temporalRelation predicateArgumentList ';'
        ;

goal	:	('rejectable' | 'goal') predicateArgumentList ';'
	;

predicateArgumentList
	:	IDENT
	|	'(' predicateArguments? ')'
	;

predicateArguments
	:	predicateArgument (',' predicateArgument)*
	;

predicateArgument
	:	qualified (IDENT)?
	;

constraintInstantiation
	:	IDENT variableArgumentList ';'
	;

constructorInvocation
	:	IDENT variableArgumentList
	;

superInvocation
	:	'super' variableArgumentList ';'
	;

variableArgumentList
	:	'(' variableArguments? ')'
	;

variableArguments
	:	variableArgument (',' variableArgument)*
	;

// Note: Allocation not legal here
variableArgument
	:	anyValue
	;

typeArgumentList
	:	'(' typeArguments? ')'
	;

typeArguments
	:	typeArgument (',' typeArgument)*
	;

typeArgument
	:	IDENT
	;

domain	:	intLiteral
	|	intervalIntDomain
	|	enumeratedIntDomain
	|	floatLiteral
	|	intervalFloatDomain
	|	enumeratedFloatDomain
	|	enumeratedStringDomain
	|	enumeratedBoolDomain
	;

intervalIntDomain
	:	'[' intLiteral ','? intLiteral ']'
	;

intervalFloatDomain
	:	'[' floatLiteral ','? floatLiteral ']'
	;

enumeratedIntDomain
	:	'{' intSet '}'
	;

intSet	:	intLiteral (',' intLiteral)*
	;

enumeratedFloatDomain
	:	'{' floatSet '}'
	;

floatSet:	floatLiteral (',' floatLiteral)*
	;

enumeratedObjectDomain
	:	'{' objectSet '}'
	;

objectSet
	:	(constructorInvocation|qualified) (',' (constructorInvocation|qualified))*
	;

enumeratedStringDomain
	:	'{' stringSet '}'
	;

stringSet
	:	STRING (',' STRING)*
	;
         
enumeratedBoolDomain
	:	'{' boolSet '}'
	;

boolSet	:	boolLiteral (',' boolLiteral)*
	;

flowControl
	:	'if' expression ruleBlock (options {k=1;}:'else' ruleBlock)?
	|	'foreach' '(' IDENT 'in' qualified ')' ruleBlock
	;

// Note: Allocation not legal here
expression
	:	'(' anyValue (('==' | '!=') anyValue)? ')'
	;
          
allocation
	:	'new' constructorInvocation
	;

allocationStatement
	:	allocation ';'
	;

variableDeclaration
	:	('filter')? type nameWithBase (',' nameWithBase)* ';'
	;

nameWithBase
	:	IDENT ('(' anyValue ')' )?
	|	IDENT '=' anyValue
	;

assignment
	:	qualified ('in' | '=') anyValue ';'
	;

anyValue:	STRING
	|	boolLiteral
	|	qualified
	|	domain
	|	allocation
	;

qualified
	:	'this'
	|	'this.'? IDENT ('.' IDENT)*
	;

temporalRelation
	:	'any'
	|	'ends'
	|	'starts'
	|	'equals'
	|	'equal'
	|	'before'
	|	'after'
	|	'contains'
	|	'contained_by'
	|	'ends_before'
	|	'ends_after'
	|	'starts_before_end'
	|	'ends_after_start'
	|	'contains_start'
	|	'starts_during'
	|	'contains_end'
	|	'ends_during'
	|	'meets'
	|	'met_by'
	|	'parallels'
	| 	'paralleled_by'
	|	'starts_before'
	|	'starts_after'
	;

intLiteral
	:	INT
	|	'+'? 'inf'
	|	'-inf' 
	;

floatLiteral
	:	FLOAT
	|	'+'? 'inff'
	|	'-inff' 
	;

boolLiteral
	:	'true'
	|	'false' 
	;

function:	qualified '.' 
		(	'specify' variableArgumentList
		|	'free' variableArgumentList
		|	'constrain' variableArgumentList
		|	'merge' variableArgumentList
		|	'activate()'
		|	'reset()'
		|	'reject()'
		|	'cancel()') ';'
	|	(IDENT '.')? 'close()' ';'
	;

tokenNameList
	:	'(' (tokenNames)? ')'
	;

tokenNames
	:	IDENT (',' IDENT)*
        ;

noopstatement
	:	';'
	;

IDENT	:	 ('a'..'z'|'A'..'Z'|'_'|'$') ('a'..'z'|'A'..'Z'|'_'|'0'..'9'|'$')*
	;

STRING	:	'"' ( ESCAPE_SEQUENCE | ~('\\'|'"') )* '"'
	;

fragment ESCAPE_SEQUENCE
	:	'\\' ('b'|'t'|'n'|'f'|'r'|'\"'|'\''|'\\')
	|	UNICODE_ESC
	|	OCTAL_ESC
	;

fragment OCTAL_ESC
	:	'\\' ('0'..'3') ('0'..'7') ('0'..'7')
	|	'\\' ('0'..'7') ('0'..'7')
	|	'\\' ('0'..'7')
	;

fragment UNICODE_ESC
	:	'\\' 'u' HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT
	;

fragment HEX_DIGIT
	:	('0'..'9'|'a'..'f'|'A'..'F')
	;
	
fragment DIGIT
	:	('0'..'9')
	;
	
INT	:	('0' | '1'..'9' '0'..'9'*) INT_SUFFIX?
	;
	
fragment INT_SUFFIX
	:	('l'|'L')
	;

FLOAT	:	('0'..'9')+ '.' ('0'..'9')* EXPONENT? FLOAT_SUFFIX?
	|	'.' ('0'..'9')+ EXPONENT? FLOAT_SUFFIX?
	|	('0'..'9')+ EXPONENT FLOAT_SUFFIX?
	|	('0'..'9')+ FLOAT_SUFFIX
	;

fragment EXPONENT
	:	('e'|'E') ('+'|'-')? ('0'..'9')+
	;
	
fragment FLOAT_SUFFIX
	:	('f'|'F'|'d'|'D')
	;
	
COMMENT	:	'/*' ( options {greedy=false;} : . )* '*/' {$channel=HIDDEN;}
	;

LINE_COMMENT
	:	'//' ~('\n'|'\r')* '\r'? '\n' {$channel=HIDDEN;}
	;

WS	:	(' '|'\r'|'\t'|'\u000C'|'\n') {$channel=HIDDEN;}
	;
