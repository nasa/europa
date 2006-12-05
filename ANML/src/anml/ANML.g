header {
    package anml;

    import java.io.*;
}

/* for ANTLR 3
grammar ANML;
options {k=3; backtrack=true; memoize=true;}
header {
    package anml;
}
*/

class ANMLParser extends Parser;
options {
    //buildAST = true;	// uses CommonAST by default
    k=3;
    //defaultErrorHandler=false;
}

anml_program 
	: (anml_stmt)*	      
;

anml_stmt
    : declaration
    | problem_stmt
    | include_file
;

// quick implementation in java
include_file : 
    INCLUDE s1:STRING_LIT
{
    try {
        String filename=s1.getText().replace('\"',' ').trim();
        ANMLLexer sublexer = new ANMLLexer(new FileInputStream(filename));
        ANMLParser parser = new ANMLParser(sublexer);
        parser.anml_program();
    }
    catch (Exception e) {
    	throw new RuntimeException(e);
    }
}
;
  
declaration 
    : objtype_decl
    | (
       vartype_decl
       | pred_declaration
       | var_obj_declaration
       | function_declaration
      ) 
      SEMI_COLON
;

problem_stmt
    : fact
    | goal
;
   
vartype_decl 
    : VARTYPE user_defined_type_name COLON var_type
;

// TODO: semantic layer to check whether we're declaring an object or a variable
var_obj_declaration 
    : var_type var_init (COMMA var_init)*
;

var_type 
    : BOOL
    | (INT | FLOAT) (range)?
    | STRING
    | ENUM LCURLY constant (COMMA constant)* RCURLY
    | VECTOR typed_arg_list 
    | user_defined_type_name
;

// TODO: should initialization allow for expressions instead of constants?
// yes, but vars can only be init'd once
var_init 
    : var_name (EQUAL constant)?
;

typed_arg_list 
    : LPAREN (arg_declaration (COMMA arg_declaration)*)? RPAREN
;

arg_declaration 
    : var_type var_name
;

objtype_decl 
    : OBJTYPE user_defined_type_name (COLON obj_type)?
      (LCURLY objtype_body RCURLY)?
;

// TODO: out of all declarations inside an objtype_body,  only variables and objects are visible from outside
objtype_body 
    : (declaration | constraint | action_def)*
;

obj_type 
    : OBJECT 
    | user_defined_type_name
;

function_declaration 
    : FUNCTION var_type function_signature (COMMA function_signature)*
;

// In ANML, functions are not mathematical functions.
// Instead, they are variables with arguments.
// Propositions are functions on {true,false}.
// TODO: Shouldn't syntax for propositions and functions be the same then?
function_signature 
    : function_symbol typed_arg_list
;

pred_declaration 
    : PREDICATE predicate (COMMA predicate)*
;

predicate 
    : predicate_symbol typed_arg_list
;

proposition 
    : cntxt_proposition  
    | FOR object_name LCURLY cntxt_proposition RCURLY
;

cntxt_proposition 
    : qualif_fluent
    | FROM time_pt LCURLY qualif_fluent_list RCURLY
;

qualif_fluent_list 
    : qualif_fluent (SEMI_COLON qualif_fluent)*
;

qualif_fluent 
    : temporal_qualif LCURLY fluent_list RCURLY
;

fluent_list 
    : fluent (SEMI_COLON fluent)*
;

temporal_qualif 
    : AT time_pt
    | OVER interval
    | IN interval (DUR numeric_term)?
    | AFTER time_pt (DUR numeric_term)?
    | BEFORE time_pt (DUR numeric_term)?
    | CONTAINS interval
;

// all(fluent) is shorthand for the interval [start(fluent),end(fluent)]
interval 
    : ALL (LPAREN fluent RPAREN)?
    | LBRACK time_pt time_pt RBRACK
;

time_pt 
    : numeric_term
;

numeric_term 
    : numeric_add_term
;

numeric_add_term
    : numeric_mult_term (options {greedy=true;} : (PLUS | MINUS) numeric_mult_term)*
;
 
numeric_mult_term
    : numeric_simple_term ((MULT | DIV) numeric_simple_term)*
;

numeric_simple_term 
    : numeric_atomic_term    
;

// TODO: added object dot notation here as it seemed to be missing
// TODO: removed start(fluent), end(fluent) from the grammar, it has to be taken care of by either functions or dot notation
numeric_atomic_term 
    : numeric_literal 
    | var_name (DOT var_name)* 
    | function_symbol LPAREN term_list RPAREN
;

fluent 
    : or_fluent
;

or_fluent
    : and_fluent (options {greedy=true;} : OR and_fluent)*
;

and_fluent
    : simple_fluent (options {greedy=true;} : AND simple_fluent)*
;

simple_fluent
    : atomic_fluent 
    | LPAREN fluent RPAREN
    | NOT fluent
;

atomic_fluent 
    : term (relop term)?
;

relop
    : EQUAL
    | LT
    | LE
    | GT
    | GE
;

term_list 
    : LPAREN (term (COMMA term)*)? RPAREN
;

// TODO: shouldn't terms be able to take advantage of the full power of numeric_terms and fluents?
// TODO: typically, numeric terms and fluents are unified and expressions and type checking makes sure semantics are correct we should probably do that for ANML
term 
    : constant
    | var_name (DOT var_name)*
    | function_symbol term_list
;

fact 
    : FACT proposition
    | FACT LCURLY proposition (SEMI_COLON proposition)* RCURLY
;

goal 
    : GOAL (proposition  | (LCURLY proposition (SEMI_COLON proposition)* RCURLY))
;

transition 
    : TRANSITION var_name LCURLY trans_pair (SEMI_COLON trans_pair)* RCURLY
;

trans_pair 
    : constant ARROW (constant  | LCURLY constant (COMMA constant)* RCURLY)
;

action_def 
    : ACTION action_symbol typed_arg_list LCURLY action_body RCURLY 
;

// TODO: This easily allows the different kinds of action_body_stmts to be mixed in any order,
// but semantic layer needs to enforce that only one duration_stmt is allowed
action_body 
    : (action_body_stmt SEMI_COLON)*
;

action_body_stmt
    : duration_stmt 
    | condition_stmt
    | change_stmt 
    | effect_stmt 
    | decomp_stmt 
    | constraint
;
    
duration_stmt 
    : DURATION numeric_term
;

condition_stmt 
    : CONDITION (condition | (LCURLY condition (COMMA condition)* RCURLY)) 
;

condition 
    : or_condition
;

or_condition
    : and_condition (OR and_condition)*
;

and_condition
    : simple_condition (AND simple_condition)*
;

// TODO: added cond_fluent, otherwise conditions that don't start with a temporal_qualif must start with a "{"
// which is weird, specially if the condition is already inside brackets (like in a "when")
simple_condition
    : temporal_qualif LCURLY cond_fluent RCURLY
    | cond_fluent
;
     
cond_fluent 
    : or_cond_fluent 
;

or_cond_fluent
    : and_cond_fluent (options {greedy=true;} : OR and_cond_fluent)*
;

and_cond_fluent
    : simple_cond_fluent (options {greedy=true;} : AND simple_cond_fluent)*
;

simple_cond_fluent
    : atomic_fluent 
    | LPAREN condition RPAREN
;
    
effect_stmt : EFFECT (effect | (LCURLY effect (COMMA effect)* RCURLY))
;

effect 
    : and_effect
;

and_effect
    : simple_effect (AND simple_effect)*
;

simple_effect
    : temporal_qualif LCURLY effect_fluent RCURLY
    | WHEN LCURLY condition RCURLY LCURLY effect RCURLY
    | LPAREN effect RPAREN
;

effect_fluent 
    : atomic_fluent 
;

change_stmt 
    : CHANGE (change | (LCURLY change (COMMA change)* RCURLY))
;

change 
    : temporal_qualif LCURLY change_expression RCURLY
    | WHEN LCURLY condition RCURLY LCURLY change RCURLY
    // TODO: what's the difference between a bunch of changes inside {} and this:
    // | LPAREN change RPAREN AND change
;

change_expression 
    : and_change_expression
;

and_change_expression
    : simple_change_expression (AND simple_change_expression)*
;

// TODO: this allows nested WHEN expressions. is this what we want?
simple_change_expression
    : atomic_change (options {greedy=true;} : AND atomic_change)?
    | WHEN LCURLY cond_fluent RCURLY LCURLY change_expression RCURLY
;

atomic_change  
    : resource_change
    | transition
;

// TODO: this should probably be constraints like any others
resource_change 
    : (CONSUMES | PRODUCES | USES)  LPAREN var_name (COMMA numeric_term)? RPAREN
;

decomp_stmt 
    : DECOMPOSITION (decomp_step | LCURLY (decomp_step)* RCURLY)
;


// TODO: changed to allow only subactions and constraints, verify with David
// TODO: semantic layer must make sure there is at least one action
decomp_step
    : (temporal_qualif action_set | constraint ) SEMI_COLON
;

action_set
    : (ORDERED | UNORDERED) LPAREN (action_set_element (COMMA action_set_element)*)? RPAREN
;

action_set_element
    : (object_name DOT)? action_symbol term_list (COLON var_name)?
    | action_set
;
    
// TODO: Added "constraint" keyword to disambiguate for now
// TODO: define grammar to support infix notation for constraints
constraint 
    : CONSTRAINT constraint_symbol term_list
;

// TODO: allow expressions?
range 
    : LBRACK numeric_literal numeric_literal RBRACK
;

constant 
    : numeric_literal | string_literal
;

numeric_literal 
    : NUMERIC_LIT
    | (PLUS|MINUS) INFINITY
;

string_literal
    : STRING_LIT
;

action_symbol           : IDENTIFIER;
constraint_symbol       : IDENTIFIER;
function_symbol         : IDENTIFIER;
predicate_symbol        : IDENTIFIER;
proposition_symbol      : IDENTIFIER;

object_name             : IDENTIFIER;
var_name                : IDENTIFIER | START | END;
user_defined_type_name  : IDENTIFIER;


class ANMLLexer extends Lexer;
options { 
    k=5;
    //caseSensitive=false; 
    testLiterals=false;   // don't automatically test for literals
}

tokens {
    ACTION        = "action";
	AFTER         = "after";
	ALL           = "all";
	AT            = "at";
	BEFORE        = "before";
    BOOL          = "bool";
    CHANGE        = "change";
    CONTAINS      = "contains";
    CONDITION     = "condition";
    CONSTRAINT    = "constraint";
    CONSUMES      = "consumes";
    DECOMPOSITION = "decomposition";
    DUR           = "dur";
    DURATION      = "duration";
    EFFECT        = "effect";
    END           = "end";
    ENUM          = "enum";
    FACT          = "fact";
    GOAL          = "goal";
    IN            = "in";
    INCLUDE       = "include";
    INT           = "int";
    FLOAT         = "float";
    FOR           = "for";
    FROM          = "from";
    FUNCTION      = "function";
    NOT           = "not";
    OVER          = "over";
    OBJECT        = "object";
    OBJTYPE       = "objtype";
    ORDERED       = "ordered";
    UNORDERED     = "unordered";
    PREDICATE     = "predicate";
    PRODUCES      = "produces";
    START         = "start";
    STRING        = "string";
    TRANSITION    = "transition";
    USES          = "uses";
    VARTYPE       = "vartype";
    VECTOR        = "vector";
    WHEN          = "when";
    INFINITY      = "inf";
}

IDENTIFIER 
	options {testLiterals=true;}
	:	('a'..'z'|'A'..'Z'|'_'|'$') ('a'..'z'|'A'..'Z'|'_'|'0'..'9'|'$')*
;

COMMA           :   ','     ;
COLON			:	':'		;
SEMI_COLON		:	';'		;

LPAREN			:	'('		;
RPAREN			:	')'		;
LBRACK			:	'['		;
RBRACK			:	']'		;
LCURLY			:	'{'		;
RCURLY			:	'}'		;

// Operators
AND             :   '&'     ;
OR              :   '|'     ;

EQUAL	        :   '='     ;
GE			    :	">="	;
GT			    :	">"		;
LE			    :	"<="	;
LT			    :	'<'		;

PLUS            :   '+'     ;
MINUS           :   '-'     ;
MULT            :   '*'     ;
DIV             :   '/'     ;

ARROW           :   "->" ;

//DOT             :   '.'     ;

// Whitespace -- ignored
WS	:	(	' '
		|	'\t'
		|	'\f'
			// handle newlines
		|	(	options {generateAmbigWarnings=false;}
			:	"\r\n"  // Evil DOS
			|	'\r'    // Macintosh
			|	'\n'    // Unix (the right way)
			)
			{ newline(); }
		)+
		{ _ttype = Token.SKIP; }
;

// a numeric literal
NUMERIC_LIT
	{boolean isDecimal=false; Token t=null;}
    :   '.' { _ttype = DOT; }
            (	('0'..'9')+ (EXPONENT)? (f1:FLOAT_SUFFIX {t=f1;})?
                {
				if (t != null && t.getText().toUpperCase().indexOf('F')>=0) {
                	//_ttype = NUM_FLOAT;
				}
				else {
                	//_ttype = NUM_DOUBLE; // assume double
				}
				}
            )?

	|	(	'0' {isDecimal = true;} // special case for just '0'
			(	('x'|'X')
				(											// hex
					// the 'e'|'E' and float suffix stuff look
					// like hex digits, hence the (...)+ doesn't
					// know when to stop: ambig.  ANTLR resolves
					// it correctly by matching immediately.  It
					// is therefor ok to hush warning.
					options {
						warnWhenFollowAmbig=false;
					}
				:	HEX_DIGIT
				)+
			|	('0'..'7')+									// octal
			)?
		|	('1'..'9') ('0'..'9')*  {isDecimal=true;}		// non-zero decimal
		)
		(	('l'|'L') { /*_ttype = NUM_LONG;*/ }

		// only check to see if it's a float if looks like decimal so far
		|	{isDecimal}?
            (   '.' ('0'..'9')* (EXPONENT)? (f2:FLOAT_SUFFIX {t=f2;})?
            |   EXPONENT (f3:FLOAT_SUFFIX {t=f3;})?
            |   f4:FLOAT_SUFFIX {t=f4;}
            )
            {
			if (t != null && t.getText().toUpperCase() .indexOf('F') >= 0) {
                //_ttype = NUM_FLOAT;
			}
            else {
	           	//_ttype = NUM_DOUBLE; // assume double
			}
			}
        )?
	;


// a couple protected methods to assist in matching floating point numbers
protected
EXPONENT
	:	('e'|'E') ('+'|'-')? ('0'..'9')+
	;


protected
FLOAT_SUFFIX
	:	'f'|'F'|'d'|'D'
	;


// hexadecimal digit (again, note it's protected!)
protected
HEX_DIGIT
	:	('0'..'9'|'A'..'F'|'a'..'f')
	;

// string literals
STRING_LIT
	:	'"' (ESC|~('"'|'\\'|'\n'|'\r'))* '"'
	;

// escape sequence -- note that this is protected; it can only be called
//   from another lexer rule -- it will not ever directly return a token to
//   the parser
// There are various ambiguities hushed in this rule.  The optional
// '0'...'9' digit matches should be matched here rather than letting
// them go back to STRING_LITERAL to be matched.  ANTLR does the
// right thing by matching immediately; hence, it's ok to shut off
// the FOLLOW ambig warnings.
protected
ESC
	:	'\\'
		(	'n'
		|	'r'
		|	't'
		|	'b'
		|	'f'
		|	'"'
		|	'\''
		|	'\\'
		|	('u')+ HEX_DIGIT HEX_DIGIT HEX_DIGIT HEX_DIGIT
		|	'0'..'3'
			(
				options {
					warnWhenFollowAmbig = false;
				}
			:	'0'..'7'
				(
					options {
						warnWhenFollowAmbig = false;
					}
				:	'0'..'7'
				)?
			)?
		|	'4'..'7'
			(
				options {
					warnWhenFollowAmbig = false;
				}
			:	'0'..'7'
			)?
		)
	;

// Single-line comments
SL_COMMENT
	:	"//"
		(~('\n'|'\r'))* ('\n'|'\r'('\n')?)
		{$setType(Token.SKIP); newline();}
	;

// multiple-line comments
ML_COMMENT
	:	"/*"
		(	/*	'\r' '\n' can be matched in one alternative or by matching
				'\r' in one iteration and '\n' in another.  I am trying to
				handle any flavor of newline that comes in, but the language
				that allows both "\r\n" and "\r" and "\n" to all be valid
				newline is ambiguous.  Consequently, the resulting grammar
				must be ambiguous.  I'm shutting this warning off.
			 */
			options {
				generateAmbigWarnings=false;
			}
		:
			{ LA(2)!='/' }? '*'
		|	'\r' '\n'		{newline();}
		|	'\r'			{newline();}
		|	'\n'			{newline();}
		|	~('*'|'\n'|'\r')
		)*
		"*/"
		{$setType(Token.SKIP);}
	;

