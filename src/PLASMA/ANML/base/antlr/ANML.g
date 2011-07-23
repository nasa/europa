/* vim: set ts=8 ft=antlr3: */

grammar ANML;

options {
language=C;
output=AST;
ASTLabelType=pANTLR3_BASE_TREE;
}

tokens {
	ANML;
	Types;
	Constants; // includes constant functions (i.e. constant mappings to constant values)
	Fluents; // includes functions mapping to fluents
	Actions;
	Parameters;
	Arguments;
	Stmts;
	Block;
	Decompositions;
	//List;
	
	TypeRef;
	LabelRef;
	Ref;
	Bind;
	Access;
	
	TypeRefine;

	Type;
	Fluent;
	FluentFunction;
	Constant;
	ConstantFunction;
	Parameter;
	Action;	
	//Stmt;
	
	Label; // primarily for fluent and action references -- an interval with a label
	DefiniteInterval;
	DefinitePoint;
	IndefiniteInterval;
	IndefinitePoint;
	Bra;
	Ket;
	Before;
	At;
	After;
	//Dots; //for extending to processes
	// this
	TBra;
	TKet;
	TStart;
	TEnd;
	TDuration;
	
	// but only the following are valid combinations
	/*	
	IntervalOO;  // default?  enables point/interval chaining on defaults
				// i.e., in the way ANML decomposes compound intervals
				// or PDDL's 'all' (interim).
	IntervalOC;
	IntervalCO;
	IntervalCC; // can degenerate into a point
			// should the others be trivially true if they degenerate...
			// or should it be an error?
	
	// limits are evaluated over intervals, but they are about
	// "the value that would have been" at a given point
	// nonetheless, when the other side is closed they break down into pieces
	IntervalLO;
	IntervalLC;
	IntervalOL;
	IntervalCL;
	
	Point; // CC
	*/
	
	Chain;
	
	TimedStmt;
	TimedExpr;
	ContainsSomeStmt; //special case of indefinite
	ContainsAllStmt; //special case of indefinite
	ContainsSomeExpr;
	ContainsAllExpr;
	
	ForAllExpr;
	ForAllStmt;
	ExistsExpr;
	ExistsStmt;
	//Expr;
	//Constraints; // several conditions
	//Condition;

	When;
	WhenElse;
	
	Enum;
	Range;
	
	ID;
	
	This;
	
	//Zero;	
}


@parser::includes
{
#include "AnmlInterpreter.hh"

using namespace EUROPA;
}

@lexer::includes
{
#include "AnmlInterpreter.hh" 

using namespace EUROPA;
}

anml	
    :   literal*
	       -> ^(ANML literal*)
    ;

    
/* primitives */	  
literal 
	: INT
	| FLOAT
	| STRING
	| True
	| False
	| Infinity
//	| boolean_literal 
//	| vector_literal
;
//vector_literal : '('! literal (',' literal)* ')'!;
//vector_init : arg_list;

	  
num_relop : Equal | NotEqual | LessThan | GreaterThan | LessThanE | GreaterThanE;
  
	  
NotLog: 'not' 
	| 'Not' | 'NOT' 
	;
AndLog : 'and' 
	| 'And' | 'AND'
	;
OrLog : 'or' 
	| 'Or' | 'OR'
	;
XorLog : 'xor' 
	| 'Xor' | 'XOR'
	;
EqualLog: 'equals' 
	| 'iff' | 'Equals' | 'EQUALS'
	;
Implies: 'implies' 
	| 'Implies' | 'IMPLIES'
	;

NotBit: '!' | '~';
AndBit: '&';
OrBit: '|';				
XorBit : '~&|';  
  // 'a ~&| b' is supposed to suggest both '(a~&b)' and '(a|b)', i.e., '(~(a&b) and a|b)'
  // '^' is taken by Delta, so a^b could be confusing, albeit, ^ only means Delta if a unary operator applied
  // on the left, so perhaps it would be okay to use '^' for XorBit instead, in the same way that '-'
  // means inversion and subtraction depending on unary/binary 

Times: '*';
Divide: '/';
Plus: '+';
Minus: '-';

Equal : '==';
NotEqual : '!=' | '~=';
LessThan : '<' ;
LessThanE : '<=';
GreaterThan: '>' ; 
GreaterThanE: '>=';

SetAssign: ':in';

Assign : ':=';

Consume: ':consume';
Produce: ':produce';
Use: ':use';
Lend: ':lend';

Change: ':->';
// _ and :_ are special

When: 'when';
Else: 'else';
Exists: 'exists';
ForAll: 'forall';

With: 'with';
Within: 'in';
Contains: 'contains';
	  
Constant: 'constant' | 'const';
Fluent: 'fluent';
Function: 'function';
Predicate: 'predicate';
Variable: 'variable' | 'var';
Type: 'type';
Action: 'action';
Fact: 'fact';
Goal: 'goal';
Decomposition: ':decomposition';
Ordered: 'ordered';
Unordered: 'unordered';	  
	  
Boolean 
	: 'bool'
	| 'boolean'
;
Integer : 'int' | 'integer';
Float : 'float' ;
Symbol : 'symbol' ;
String : 'string' ;
Object : 'object' ;

Vector : 'vector';

Delta : '^';

Undefined: 'undefined' 
	| 'U' | 'UNDEFINED' | 'Undefined'
	;
All: 'all';
Start: 'start';
End: 'end';
Duration: 'duration';
Infinity: 'infinity' | 'Infinity' | 'INFINITY' | 'infty' | 'INFTY' | 'inff' | 'INFF' ;

// cannot lex SymbolLiteral or VectorLiteral or ObjectLiteral
// cannot parse SymbolLiteral or ObjectLiteral
True:	('T' | 'TRUE' | 'true' | 'True');
False:	('F' | 'FALSE' | 'false' | 'False');
INT: DIGIT+;
FLOAT
	: DIGIT+ Dot DIGIT+
	| DIGIT+ 'f'
;
STRING:	 '"' ESC* '"';	


ID : LETTER (LETTER | DIGIT | '_')*;

fragment ESC : '\\'. | ~'\"';
fragment LETTER  : 	'a'..'z'|'A'..'Z' ;
fragment DIGIT : 	'0'..'9' ;

WS  :  (' '|'\r'|'\t'|'\u000C'|'\n')+ {$channel=HIDDEN;} ;
SLC : '//' (~'\n')* '\n' {$channel=HIDDEN;} ;
MLC : '/*' (~'*' | '*'+(~('*'|'/')))* '*'+'/' {$channel=HIDDEN;} ;

Dot: '.';
Dots: '...';
Comma: ',';
Semi: ';';
Colon: ':';
LeftP: '(';
RightP: ')';
LeftC: '{';
RightC: '}';
LeftB: '[';
RightB: ']';
Undefine: ':_';
Skip: '_';

//WS  :  (' '|'\r'|'\t'|'\u000C'|'\n')+ {skip();} ;
//COMMA : ',' {skip();} ;
// WS  :   (' '|'\t')+ {skip();} ;
//NEWLINE :'\r'? '\n' ;



    