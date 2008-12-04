/* vim: set ts=8 ft=antlr3: */

grammar NDDL3;

options {
	output=AST;
	language=C;
	ASTLabelType=pANTLR3_BASE_TREE;
	//k=4;
}

tokens {
	CONSTRUCTOR;
	CONSTRUCTOR_INVOCATION;
	CONSTRAINT_INSTANTIATION;
	SUBGOAL;
	VARIABLE;
}

nddl
	:	nddlStatement*
        ;

nddlStatement
        :	constraintSignature
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

typeDefinition
	:	'typedef' b=typeWithBaseDomain n=IDENT ';'
			-> ^('typedef' $n $b)
	;

// MEB Language Change: domain no longer optional
typeWithBaseDomain
	:	(	'int'^
		|	'float'^
		|	'boolean'^
		|	'string'^
		|	IDENT^) domain
	;

// MEB Interpreter Support Missing
constraintSignature
	:	'constraint' IDENT typeArgumentList
		('extends' IDENT typeArgumentList)? 
		(signatureBlock | ';') 
	;

signatureBlock
	:	'{'^ (signatureExpression)? '}'!
	;

signatureExpression
	:	signatureAtom (('&&'^ | '||'^) signatureAtom)*
	;

signatureAtom
	:	'('^ signatureExpression ')'!
	|	IDENT '<:'^ (type | 'numeric' )
	;

classDeclaration
	:	'class' c=IDENT
		(	(('extends' x=IDENT)? cb=classBlock)
				-> ^('class' $c ^('extends' $x)? $cb)
		|	';'
					-> ^('class' $c ';')
		)
	;

classBlock
	:	'{'^ classStatement* '}'!
	;

classStatement
	:	variableDeclaration
	|	constructor
	|	predicate
	|	noopstatement
	;

constructor
	:	c=IDENT pl=constructorParameterList cb=constructorBlock
			-> ^(CONSTRUCTOR $c $pl $cb)
	;

constructorBlock
	:	'{'^ constructorStatement* '}'!
	;

constructorStatement
	:	assignment
	|	superInvocation
	|	noopstatement
	;

constructorParameterList
	:	'('^ constructorParameters? ')'!
	;

constructorParameters
	:	constructorParameter  (','! constructorParameters)?
	;

constructorParameter
	:	t=type n=IDENT
			-> ^(VARIABLE $n $t)
	;

predicate
	:	'predicate'^ IDENT predicateBlock 
	;

predicateBlock
	:	'{'^ predicateStatement* '}'!
	;

// Note: Allocations are not legal here.
predicateStatement
	:	variableDeclaration
	|	constraintInstantiation
	|	assignment
	;


rule	:	IDENT '::'^ IDENT ruleBlock
	;

ruleBlock
	:	'{'^ ruleStatement* '}'!
	|	rs=ruleStatement -> ^('{' $rs)
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

relation:	(token=IDENT | 'this')? rel=temporalRelation args=predicateArgumentList ';'
			-> ^(SUBGOAL $token $rel $args)
        ;

goal	:	('rejectable'^ | 'goal'^) predicateArgumentList ';'!
	;

predicateArgumentList
	:	IDENT
	|	'('^ predicateArguments? ')'!
	;

predicateArguments
	:	predicateArgument (','! predicateArgument)*
	;

predicateArgument
	:	p=qualified (n=IDENT)?
			-> ^($p $n)
	;

constraintInstantiation
	:	c=IDENT args=variableArgumentList ';'
			-> ^(CONSTRAINT_INSTANTIATION $c $args)
	;

constructorInvocation
	:	c=IDENT args=variableArgumentList
			-> ^(CONSTRUCTOR_INVOCATION $c $args)
	;

superInvocation
	:	'super'^ variableArgumentList ';'!
	;

variableArgumentList
	:	'('^ variableArguments? ')'!
	;

variableArguments
	:	variableArgument (','! variableArgument)*
	;

// Note: Allocation not legal here
variableArgument
	:	anyValue
	;

typeArgumentList
	:	'('^ typeArguments? ')'!
	;

typeArguments
	:	typeArgument (','! typeArgument)*
	;

typeArgument
	:	IDENT
	;

domain	:	numericLiteral
	|	intervalNumericDomain
	|	enumeratedNumericDomain
	|	enumeratedStringDomain
	|	enumeratedBoolDomain
	|	enumeratedObjectDomain
	;

intervalNumericDomain
	:	'['^ numericLiteral (','!)? numericLiteral ']'!
	;

enumeratedNumericDomain
	:	'{'^ numericSet '}'!
	;

numericSet
	:	numericLiteral (','! numericLiteral)*
	;

enumeratedObjectDomain
	:	'{'^ objectSet '}'!
	;

objectSet
	:	(constructorInvocation|qualified) (','! (constructorInvocation|qualified))*
	;

enumeratedStringDomain
	:	'{'^ stringSet '}'!
	;

stringSet
	:	STRING (','! STRING)*
	;
         
enumeratedBoolDomain
	:	'{'^ boolSet '}'!
	;

boolSet	:	boolLiteral (','! boolLiteral)*
	;

flowControl
	:	'if'^ expression ruleBlock (options {k=1;}:'else'! ruleBlock)?
	|	'foreach'^ '('! IDENT 'in'! qualified ')'! ruleBlock
	;

// Note: Allocation not legal here
expression
	:	'('! anyValue (('=='^ | '!='^) anyValue)? ')'!
	;
          
allocation
	:	'new'! constructorInvocation
	;

allocationStatement
	:	allocation ';'!
	;

variableDeclaration
	:	('filter')? type^ nameWithBase (','! nameWithBase)* ';'!
	;

nameWithBase
	:	(	variable=IDENT ('('^ value=anyValue ')'! )?
		|	variable=IDENT '='^ value=anyValue)
	;

assignment
	:	variable=qualified ('in' | '=') value=anyValue ';'
		-> ^('=' $variable $value)
	;

anyValue:	STRING
	|	boolLiteral
	|	qualified
	|	domain
	|	allocation
	;

qualified
	:	'this'
	|	'this.'? IDENT ('.'^ IDENT)*
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

numericLiteral
	:	INT
	|	FLOAT
	|	('+'!)? 'inf'
	|	'-inf' 
	;

boolLiteral
	:	'true'
	|	'false' 
	;

function:	qualified '.'!
		(	'specify'^ variableArgumentList
		|	'free'^ variableArgumentList
		|	'constrain'^ variableArgumentList
		|	'merge'^ variableArgumentList
		|	'activate'^ '('! ')'!
		|	'reset'^ '('! ')'!
		|	'reject'^ '('! ')'!
		|	'cancel'^ '('! ')'!) ';'!
	|	(IDENT '.'!)? 'close'^ '('! ')'! ';'!
	;

tokenNameList
	:	'('^ (tokenNames)? ')'!
	;

tokenNames
	:	IDENT (','! IDENT)*
        ;

noopstatement
	:	';'!
	;
	
INCLUDE :	'#include' WS+ file=STRING '\r'? '\n'	{
			pANTLR3_STRING      fName;
			pANTLR3_INPUT_STREAM    in; 

			// Create an initial string, then take a substring
			// We can do this by messing with the start and end 
			// pointers of tokens and so on. This shows a reasonable way to
			// manipulate strings.

			fName = $file.text;

			// Create a new input stream and take advantage of built in stream stacking
			// in C target runtime.

			in = antlr3AsciiFileStreamNew(fName->chars);
			PUSHSTREAM(in);

			// Note that the input stream is not closed when it EOFs, I don't bother
			// to do it here (hence this is leaked at the program end), 
			// but it is up to you to track streams created like this
			// and destroy them when the whole parse session is complete. Remember that you 
			// don't want to do this until all tokens have been manipulated all the way through 
			// your tree parsers etc as the token does not store the text it just refers
			// back to the input stream and trying to get the text for it will abort if you 
			// close the input stream too early.

			$channel=HIDDEN;
		}
	;

IDENT	:	 ('a'..'z'|'A'..'Z'|'_'|'$') ('a'..'z'|'A'..'Z'|'_'|'0'..'9'|'$')*
	;

STRING	:	'"'! (~('\\'|'"') | ESCAPE_SEQUENCE)* '"'!
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
