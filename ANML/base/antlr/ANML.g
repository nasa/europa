header "post_include_hpp" {
#include <string>
#include <iostream>
#include <fstream>
#include "antlr/TokenStreamSelector.hpp"

class ANMLParser;
class ANMLLexer;

// Define a selector that can handle nested include files.
// These variables are public so the parser/lexer can see them.
extern ANTLR_USE_NAMESPACE(antlr)TokenStreamSelector selector;
extern const std::string* searchDir;
extern ANMLParser* parser;
extern ANMLLexer* mainLexer;
#include "ANMLLexer.hpp"
}

/* for ANTLR 3
grammar ANML;
options {k=3; backtrack=true; memoize=true;}
header {
    package anml;
}
*/
options {
  language="Cpp";
}

{

  // Define a selector that can handle nested include files.
  // These variables are public so the parser/lexer can see them.
  ANTLR_USE_NAMESPACE(antlr)TokenStreamSelector selector;
  ANMLParser* parser;
  ANMLLexer* mainLexer;
	const std::string* searchDir;

  bool ANMLParser::parse(const std::string& path, const std::string& filename) {
		searchDir = &path;
    try {
      // attach java lexer to the input stream,
      std::ifstream* input = new std::ifstream((path + "/" + filename).c_str());
      if (!*input) {
        std::cerr << "cannot find file " << filename << std::endl;
      }   
      mainLexer = new ANMLLexer(*input);

      // notify selector about starting lexer; name for convenience
      selector.addInputStream(mainLexer, "main");
      selector.select("main"); // start with main P lexer

      // Create parser attached to selector
      parser = new ANMLParser(selector);

      // Parse the input language: ANML
      parser->setFilename(filename);
      parser->anml_program();
    }
    catch( antlr::ANTLRException& e )
    {
      std::cerr << "exception: " << e.getMessage() << std::endl;
      return false;
    }
    catch( std::exception& e )
    {
      std::cerr << "exception: " << e.what() << std::endl;
      return false;
    }
    return true;
  }
}

class ANMLParser extends Parser;
options {
    //buildAST = true;	// uses CommonAST by default
    k=3;
    //defaultErrorHandler=false;
}

{
	public:
		static bool parse(const std::string& path, const std::string& filename);
}

anml_program 
	: (anml_stmt)*	      
;

anml_stmt
    : declaration
    | problem_stmt
;

declaration 
    : objtype_decl
    | (
       vartype_decl
       | var_obj_declaration
       | function_declaration
       | predicate_declaration
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
    | VECTOR LPAREN (arg_declaration_list)? RPAREN 
    | user_defined_type_name
;

// TODO: eventually allow at least constant expressions
range 
    : LBRACK numeric_literal numeric_literal RBRACK
;


// TODO: eventually allow for expressions instead of constants. make expression semantics declarative, not procedural
// TODO: semantic layer to ensure that vars are only init'd once. 
var_init 
    : var_name (EQUAL constant)?
;

arg_declaration_list
    : arg_declaration (COMMA arg_declaration)*
;

arg_declaration 
    : var_type var_name
;

objtype_decl 
    : OBJTYPE user_defined_type_name (COLON obj_type)?
      (LCURLY objtype_body RCURLY)?
;

obj_type 
    : OBJECT 
    | user_defined_type_name
;

objtype_body 
    : (objtype_body_stmt)*
;

// TODO: semantic layer to make sure that out of all declarations inside an objtype_body,  
// only variables and objects are visible from outside
objtype_body_stmt
    : declaration
    | action_def
    | transition_constraint SEMI_COLON
;

// In ANML, functions are variables with arguments.
function_declaration 
    : FUNCTION var_type function_signature (COMMA function_signature)*
;

function_signature 
    : function_symbol LPAREN (arg_declaration_list)? RPAREN
;

// Predicates are functions on {true,false}.
predicate_declaration 
    : PREDICATE function_signature (COMMA function_signature)*
;

// Fluents are the same for {Facts,Effects} and for {Goals,Conditions}
// For Effects and Facts :
// - OR is not allowed
// - Temporal qualifiers IN, BEFORE, AFTER and CONTAINS are not allowed.
// - existential quantifier is not allowed
// For Goals and Conditions
// - WHEN clause is not allowed

fact 
    : FACT (effect_proposition | LCURLY effect_proposition_list RCURLY)
;

goal 
    : GOAL (condition_proposition | LCURLY condition_proposition_list RCURLY)
;

effect_proposition 
    : proposition
;

effect_proposition_list 
    : effect_proposition (SEMI_COLON effect_proposition)*
;

condition_proposition 
    : proposition
;

condition_proposition_list 
    : condition_proposition (SEMI_COLON condition_proposition)*
;

// TODO : WHEN is only allowed for {Effects,Facts}, not allowed for {Conditions,Goals}. semantic layer to enforce this
// TODO : FROM and FOR branches to be implemented later
proposition
    : qualif_fluent
    | WHEN LCURLY condition_proposition RCURLY LCURLY effect_proposition RCURLY
    | FROM time_pt LCURLY qualif_fluent_list RCURLY
    | FOR object_name LCURLY fluent RCURLY
;

qualif_fluent 
    : temporal_qualif LCURLY fluent_list RCURLY
;

qualif_fluent_list 
    : qualif_fluent (SEMI_COLON qualif_fluent)*
;

fluent_list
    : fluent (SEMI_COLON fluent)*
;

fluent
    : or_fluent
;

// TODO: OR is not allowed for effects and facts. check and throw exception if necessary
or_fluent
    : and_fluent (options {greedy=true;} : OR and_fluent)*
;

and_fluent
    : primary_fluent (options {greedy=true;} : AND primary_fluent)*
;

// NOTE : NOT is not supported for now
primary_fluent
    : relational_fluent 
    | quantif_clause fluent
    | LPAREN fluent RPAREN
    //| NOT fluent
;

quantif_clause
    : (FORALL | EXISTS) LPAREN var_type var_name_list RPAREN
;

var_name_list
    : var_name (COMMA var_name)*
;

// NOTE: if the rhs is not present, that means we're stating a predicate to be true
relational_fluent 
    : lhs_expr (relop expr)?
;

// NOTE: removed start(fluent), end(fluent) from the grammar, it has to be taken care of by either functions or dot notation
// TODO: antlr is complaining about non-determinism here, but I don't see it, LPAREN should never be in follow(lhs_expr). anyway, order of subrules means parser does the right thing
lhs_expr
    : function_symbol LPAREN (arg_list)? RPAREN 
    | var_name (DOT var_name)*  
;
    
// TODO: we should allow for full-blown expressions (logical and numerical) at some point
expr 
    : constant
    | lhs_expr
;

    
// NOTE : Only EQUAL operator allowed for now in fluent expressions
relop
    : EQUAL
    /*
    | LTH
    | LEQ
    | GTH
    | GEQ
    */
;

// TODO: For effects and facts:
// - Temporal qualifiers IN, BEFORE, AFTER (and CONTAINS??) are not allowed. What about FROM?
// check and throw semantic exception if necessary
temporal_qualif 
    : AT time_pt
    | OVER interval
    | IN interval (DUR numeric_expr)?
    | AFTER time_pt (DUR numeric_expr)?
    | BEFORE time_pt (DUR numeric_expr)?
    | CONTAINS interval
;

// all(fluent) is shorthand for the interval [start(fluent),end(fluent)]
interval 
    : ALL (LPAREN fluent RPAREN)?
    | LBRACK time_pt time_pt RBRACK
;

time_pt 
    : numeric_expr
;

numeric_expr 
    : add_expr
;

add_expr
    : mult_expr (options {greedy=true;} : (PLUS | MINUS) mult_expr)*
;
 
mult_expr
    : primary_expr ((MULT | DIV) primary_expr)*
;

primary_expr 
    : atomic_expr    
    | LPAREN add_expr RPAREN
;

atomic_expr 
    : numeric_literal 
    | lhs_expr
;

arg_list 
    : expr (COMMA expr)*
;

action_def 
    : ACTION action_symbol LPAREN (arg_declaration_list)? RPAREN 
      LCURLY action_body RCURLY 
;

action_body 
    : (action_body_stmt SEMI_COLON)*
;

action_body_stmt
    : duration_stmt 
    | condition_stmt
    | effect_stmt 
    | change_stmt 
    | decomp_stmt 
    | constraint
;

// TODO: semantic layer to enforce that only one duration statement is allowed    
duration_stmt 
    : DURATION numeric_expr
;

condition_stmt 
    : CONDITION (condition_proposition | (LCURLY condition_proposition_list RCURLY)) 
;
    
effect_stmt : EFFECT (effect_proposition | LCURLY effect_proposition_list RCURLY)
;

change_stmt 
    : CHANGE (change_proposition | (LCURLY change_proposition_list RCURLY))
;

change_proposition
    : temporal_qualif LCURLY change_fluent RCURLY
    | WHEN LCURLY condition_proposition RCURLY LCURLY change_proposition RCURLY
;

change_proposition_list
    : change_proposition (COMMA change_proposition)*
; 

change_fluent 
    : and_change_fluent
;

and_change_fluent
    : primary_change_fluent (options {greedy=true;} : AND primary_change_fluent)*
;

primary_change_fluent
    : atomic_change 
    | quantif_clause change_fluent
    | LPAREN change_fluent RPAREN 
;

atomic_change  
    : resource_change
    | transition_change
;

resource_change 
    : (CONSUMES | PRODUCES | USES)  LPAREN var_name (COMMA numeric_expr)? RPAREN
;

transition_change
    : var_name (DOT var_name)* EQUAL expr (ARROW expr)*
;

decomp_stmt 
    : DECOMPOSITION (decomp_step | LCURLY (decomp_step)* RCURLY)
;

decomp_step
    : (temporal_qualif action_set | constraint ) SEMI_COLON
;

action_set
    : (quantif_clause)? (ORDERED | UNORDERED) LPAREN action_set_element_list RPAREN
;

action_set_element_list
    : action_set_element (COMMA action_set_element)*
;

action_set_element
    : (object_name DOT)? action_symbol LPAREN (arg_list)? RPAREN (COLON var_name)?
    | action_set
;
    
constraint 
    : constraint_expr
;

// TODO: define grammar to support infix notation for constraints
constraint_expr 
    : constraint_symbol LPAREN (arg_list)? RPAREN
;

// This constraint is only allowed in the main body of an objtype definition
transition_constraint 
    : TRANSITION var_name LCURLY trans_pair (SEMI_COLON trans_pair)* RCURLY
;

trans_pair 
    : constant ARROW (constant  | LCURLY constant (COMMA constant)* RCURLY)
;

constant 
    : numeric_literal | string_literal
;

numeric_literal 
    : NUMERIC_LIT
    | (PLUS|MINUS) INF
;

string_literal
    : STRING_LIT
;

action_symbol           : IDENTIFIER;
constraint_symbol       : IDENTIFIER;
function_symbol         : IDENTIFIER;

object_name             : IDENTIFIER;
var_name                : IDENTIFIER | START | END;

user_defined_type_name  : IDENTIFIER;

{
#include <iostream>
#include <fstream>
#include "ANMLParser.hpp"
}

class ANMLLexer extends Lexer;
options { 
    k=5;
		charVocabulary = '\3'..'\377';
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
    EXISTS        = "exists";
    FACT          = "fact";
    FORALL        = "forall";
    GOAL          = "goal";
    IN            = "in";
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
    INF           = "inf";
}

{
	void uponEOF() /*throws TokenStreamException, CharStreamException*/ {
		if ( selector.getCurrentStream() != mainLexer ) {
			// don't allow EOF until main lexer.  Force the
			// selector to retry for another token.
			selector.pop(); // return to old lexer/stream
			selector.retry();
		}
		else {
			ANTLR_USE_NAMESPACE(std)cout << "Hit EOF of main file" << ANTLR_USE_NAMESPACE(std)endl;
		}
	}
}

INCLUDE
	:	"#include" (WS)? f:STRING_LIT
		{
		ANTLR_USING_NAMESPACE(std)
		// create lexer to handle include
		string name = f->getText();
		ifstream* input = new ifstream((*searchDir + "/" + name).c_str());
		if (!*input) {
			cerr << "cannot find file " << name << endl;
			cerr << "tried " << *searchDir << "/" << name << endl;
		}
		ANMLLexer* sublexer = new ANMLLexer(*input);
		// make sure errors are reported in right file
		sublexer->setFilename(name);
		parser->setFilename(name);

		// you can't just call nextToken of sublexer
		// because you need a stream of tokens to
		// head to the parser.  The only way is
		// to blast out of this lexer and reenter
		// the nextToken of the sublexer instance
		// of this class.

		selector.push(sublexer);
		// ignore this as whitespace; ask selector to try
		// to get another token.  It will call nextToken()
		// of the new instance of this lexer.
		selector.retry(); // throws TokenStreamRetryException
		$setType(ANTLR_USE_NAMESPACE(antlr)Token::SKIP);
		}
	;

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
GEQ			    :	">="	;
GTH			    :	">"		;
LEQ			    :	"<="	;
LTH			    :	'<'		;

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
		{ _ttype = ANTLR_USE_NAMESPACE(antlr)Token::SKIP; }
;


// a numeric literal
NUMERIC_LIT
	{
		bool isDecimal = false;
		ANTLR_USE_NAMESPACE(antlr)RefToken t = ANTLR_USE_NAMESPACE(antlr)nullToken;
	}
    :   '.' {_ttype = DOT;}
            (	('0'..'9')+ (EXPONENT)? (f1:FLOAT_SUFFIX {t=f1;})?
            {
					if ( t &&
						  (t->getText().find('f') != ANTLR_USE_NAMESPACE(std)string::npos ||
							t->getText().find('F') != ANTLR_USE_NAMESPACE(std)string::npos ) ) {
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
			|	//float or double with leading zero
				(('0'..'9')+ ('.'|EXPONENT|FLOAT_SUFFIX)) => ('0'..'9')+
			|	('0'..'7')+									// octal
			)?
		|	('1'..'9') ('0'..'9')*  {isDecimal=true;}		// non-zero decimal
		)
		(	('l'|'L') { /* _ttype = NUM_LONG; */ }

		// only check to see if it's a float if looks like decimal so far
		|	{isDecimal}?
            (   '.' ('0'..'9')* (EXPONENT)? (f2:FLOAT_SUFFIX {t=f2;})?
            |   EXPONENT (f3:FLOAT_SUFFIX {t=f3;})?
            |   f4:FLOAT_SUFFIX {t=f4;}
            )
            {
					if ( t &&
						  (t->getText().find('f') != ANTLR_USE_NAMESPACE(std)string::npos ||
							t->getText().find('F') != ANTLR_USE_NAMESPACE(std)string::npos ) ) {
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
	:	'"'! (ESC|~('"'|'\\'|'\n'|'\r'))* '"'!
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
		{$setType(ANTLR_USE_NAMESPACE(antlr)Token::SKIP); newline();}
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
		{$setType(ANTLR_USE_NAMESPACE(antlr)Token::SKIP);}
	;

