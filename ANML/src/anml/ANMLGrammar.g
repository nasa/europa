header {
    package anml;
}

class ANMLParser extends Parser;
options {
    //buildAST = true;	// uses CommonAST by default
    k=3;
    //defaultErrorHandler=false;
}


declaration : vartype_decl
              | var_declaration
              | fun_declaration
              | pred_declaration
              | objtype_decl
              | obj_declaration
;

vartype_decl : VARTYPE user_defined_type_name COLON var_type
;

var_declaration : var_type var_init (COMMA var_init)*
;

var_type : BOOL
           | (INT | FLOAT) (range)?
           | STRING
           | ENUM LCURLY constant (COMMA constant)* RCURLY
           | VECTOR ( typed_arg_list )
           | user_defined_type_name
;

var_init : variable (EQUAL constant)?
;

typed_arg_list : arg_declaration (COMMA arg_declaration)*
;

arg_declaration : var_type variable (COMMA variable)*
;

objtype_decl : 
   OBJTYPE user_defined_type_name (COLON obj_type)?
   (LCURLY obj_body RCURLY)?
;
   
obj_body : (declaration | constraint | action_def)*
;

obj_declaration : obj_type IDENTIFIER (COMMA IDENTIFIER)*
;

obj_type : OBJECT | user_defined_type_name
;

fun_declaration : FUNCTION var_type function (COMMA function)*
;

function : function_symbol ( typed_arg_list )
;

pred_declaration : PREDICATE predicate (COMMA predicate)*
;

predicate : predicate_symbol ( typed_arg_list )
;

proposition : cntxt_proposition  
            | FOR object LCURLY cntxt_proposition RCURLY
;

cntxt_proposition : qualif_fluent
                  | FROM time_pt LCURLY qualif_fluent_list RCURLY
;

qualif_fluent_list : qualif_fluent (SEMI_COLON qualif_fluent)*
;

qualif_fluent : temporal_qualif LCURLY fluent_list RCURLY
;

fluent_list : fluent (SEMI_COLON fluent)*
;

temporal_qualif : AT time_pt
                | OVER interval
                | IN interval (DUR numeric_term)?
                | AFTER time_pt (DUR numeric_term)?
                | BEFORE time_pt (DUR numeric_term)?
                | CONTAINS interval
;

interval : ALL (LPAREN fluent RPAREN)?
         | LBRACK time_pt COMMA time_pt RBRACK
;

time_pt : numeric_term
;

numeric_term : atomic_term ((PLUS | MINUS | MULT | DIV) atomic_term)?
             | START (LPAREN fluent RPAREN)?
             | END (LPAREN fluent RPAREN)?
;

atomic_term : NUMERIC_LITERAL | variable
;

fluent : atomic_fluent ((AND | OR) atomic_fluent)?
       | LPAREN fluent RPAREN
       | NOT fluent
;

atomic_fluent : proposition_symbol
              | predicate_symbol term_list
              | term EQUAL term
;

term_list : LPAREN (term (COMMA term)*)? RPAREN
;

term : constant
     | variable
     | function_symbol term_list
;

fact : FACT proposition
     | FACT LCURLY proposition (COMMA proposition)* RCURLY
;

goal : GOAL (proposition  | (LCURLY proposition (SEMI_COLON proposition)* RCURLY))
;

transition : TRANSITION variable LCURLY trans_pair (SEMI_COLON trans_pair)* RCURLY
;

trans_pair : constant ARROW (constant  | LCURLY constant (COMMA constant)* RCURLY)
;

action_def : ACTION action_name LPAREN typed_list RPAREN LCURLY action_body RCURLY 
;

typed_list : (var_type variable (COMMA var_type variable)*)?
;

action_body : (duration_stmt)? 
              (condition_stmt)*
              (effect_stmt)*
              (change_stmt)*
              (decomp_stmt)* 
;

duration_stmt : DURATION numeric_term
;

condition_stmt : CONDITION (condition | (LCURLY condition (COMMA condition)* RCURLY)) 
;

// TODO: fix the grammar for precedence and associativity
condition : temporal_qualif LCURLY cond_fluent RCURLY
          | LPAREN condition RPAREN (AND | OR) condition
;
 
cond_fluent : atomic_fluent ((AND | OR) atomic_fluent)?
            | LPAREN cond_fluent RPAREN
            | ORDERED LPAREN (cond_fluent)+ RPAREN
            | UNORDERED LPAREN (cond_fluent)+ RPAREN
;

effect_stmt : EFFECT (effect | (LCURLY effect (COMMA effect)* RCURLY))
;

effect : temporal_qualif LCURLY effect_fluent RCURLY
       | LPAREN effect RPAREN AND effect
       | WHEN LCURLY condition RCURLY LCURLY effect RCURLY
;

effect_fluent : atomic_fluent (AND effect_fluent)?
              | LPAREN effect_fluent RPAREN
              | WHEN LCURLY cond_fluent CURLY LCURLY effect_fluent RCURLY
;

change_stmt : CHANGE (change | (LCURLY change (COMMA change)* RCURLY))
;

change : temporal_qualif LCURLY change_expression RCURLY
       | LPAREN change RPAREN AND change
       | WHEN LCURLY condition RCURLY LCURLY change RCURLY
;

change_expression : atomic_change (AND atomic_change)?
                  | WHEN LCURLY cond_fluent RCURLY LCURLY change_expression RCURLY
;

atomic_change  : resource_change
               | transition
;

// TODO: this should probably be constraints like any others
resource_change : (CONSUMES | PRODUCES | USES)  LPAREN variable (COMMA numeric_term)?
;

decomp_stmt : DECOMPOSITION (condition | LCURLY condition (COMMA condition)* RCURLY)
;

// TODO: define constraint
constraint : "CONSTRAINT"
;

// TODO: allow expressions?
range : LBRACK NUMERIC_LITERAL COMMA NUMERIC_LITERAL RBRACK
;

constant : NUMERIC_LITERAL | STRING_LITERAL
;

action_name             : IDENTIFIER ;
user_defined_type_name  : IDENTIFIER ;
function_symbol         : IDENTIFIER ;
predicate_symbol        : IDENTIFIER;
proposition_symbol      : IDENTIFIER;
object                  : IDENTIFIER;
variable                : IDENTIFIER;


class ANMLLexer extends Lexer;
options { 
    k=5;
    //caseSensitive=false; 
    testLiterals=false;   // don't automatically test for literals
}
/*
{
	static final int DOT = 0;
	static final int NUM_FLOAT = 1;
	static final int NUM_DOUBLE = 2;
	static final int NUM_LONG = 3;	
}
*/
tokens {
	ACTION   = "action";
	AFTER    = "after";
	ALL      = "all";
	AT       = "at";
	BEFORE   = "before";
    BOOL     = "bool";
    CHANGE   = "change";
    CONTAINS = "contains";
    CONDITION = "condition";
    CONSUMES = "consumes";
    DECOMPOSITION = "decomposition";
    DUR      = "dur";
    DURATION = "duration";
    EFFECT   = "effect";
    END      = "end";
    ENUM     = "enum";
    FACT     = "fact";
    IN       = "in";
    INT      = "int";
    FLOAT    = "float";
    FOR      = "for";
    FROM     = "from";
    FUNCTION = "function";
    NOT      = "not";
    OBJECT   = "object";
    OBJTYPE  = "objtype";
    PRODUCES = "produces";
    START    = "start";
    STRING   = "string";
    USES     = "uses";
    VARTYPE  = "vartype";
    VECTOR   = "vector";
    WHEN     = "when";
}

IDENTIFIER 
	options {testLiterals=true;}
	:	('a'..'z'|'A'..'Z'|'_'|'$') ('a'..'z'|'A'..'Z'|'_'|'0'..'9'|'$')*
;

COMMA           :   ','     ;
COLON			:	':'		;
SEMI_COLON		:	';'		;
EQUAL	        :   '='     ;
LPAREN			:	'('		;
RPAREN			:	')'		;
LBRACK			:	'['		;
RBRACK			:	']'		;
LCURLY			:	'{'		;
RCURLY			:	'}'		;
PLUS            :   '+'     ;
MINUS           :   '-'     ;
MULT            :   '*'     ;
DIV             :   '/'     ;
AND             :   '&'     ;
OR              :   '|'     ;

ARROW : '-' '>' ;

// a numeric literal
NUMERIC_LITERAL
	{boolean isDecimal=false; Token t=null;}
    :   '.' { /*_ttype = DOT;*/}
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
		|	('1'..'9') ('0'..'9')*  {isDecimal=true;}		// non_zero decimal
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
STRING_LITERAL
	:	'"' (ESC|~('"'|'\\')|'#')* '"'
	;

// escape sequence 
//   note that this is protected; it can only be called from another lexer rule 
//   it will not ever directly return a token to the parser
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

