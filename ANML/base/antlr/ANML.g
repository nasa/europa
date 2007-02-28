header "post_include_hpp" {
#include <string>
#include <iostream>
#include <fstream>
#include "Debug.hh"
#include "antlr/TokenStreamSelector.hpp"
#include "antlr/ASTFactory.hpp"

class ANMLParser;
class ANMLLexer;

// Define a selector that can handle nested include files.
// These variables are public so the parser/lexer can see them.
extern antlr::TokenStreamSelector selector;
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
  antlr::TokenStreamSelector selector;
  ANMLParser* parser;
  ANMLLexer* mainLexer;
  const std::string* searchDir;

  antlr::RefAST ANMLParser::parse(const std::string& path, const std::string& filename) {
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

      antlr::ASTFactory astfactory;
      // Create parser attached to selector
      parser = new ANMLParser(selector);

      // Parse the input language: ANML
      parser->setFilename(filename);
      parser->initializeASTFactory(astfactory);
      parser->setASTFactory(&astfactory);
      parser->anml_program();
    }
    catch( antlr::ANTLRException& e )
    {
      std::cerr << "exception: " << e.getMessage() << std::endl;
      return antlr::nullAST;
    }
    catch( std::exception& e )
    {
      std::cerr << "exception: " << e.what() << std::endl;
      return antlr::nullAST;
    }
    return parser->getAST();
  }

#define LONG_RULE_SIZE 26

  /**
   * Custom traceIn for Antlr which uses debugMsg.
   */
  void ANMLParser::traceIn(const char* rname) {
		int indentsize = LONG_RULE_SIZE-strlen(rname)+traceDepth+2;
    char indent[indentsize+1];
		for(int i=0; i < indentsize; ++i)
			indent[i] = ' ';
		indent[LONG_RULE_SIZE-strlen(rname)+1] = '|';
		indent[indentsize] = '\0';
    ++traceDepth;

		debugMsg((std::string("ANMLParser:traceIn:")+rname).c_str(),
		         indent << "> " << rname << ((inputState->guessing > 0)? "; [guessing]" : ";") <<
						 " LA(1)==" << ((LT(1) == antlr::nullToken)? "null" : LT(1)->getText()) <<
						 " LA(2)==" << ((LT(2) == antlr::nullToken)? "null" : LT(2)->getText()) <<
						 " LA(3)==" << ((LT(3) == antlr::nullToken)? "null" : LT(3)->getText()));
  }

  /**
   * Custom traceOut for Antlr which uses debugMsg.
   */
  void ANMLParser::traceOut(const char* rname) {
    --traceDepth;
		int indentsize = LONG_RULE_SIZE-strlen(rname)+traceDepth+1;
    char indent[indentsize+1];
		for(int i=0; i < indentsize; ++i)
			indent[i] = ' ';
		indent[LONG_RULE_SIZE-strlen(rname)] = '|';
		indent[indentsize] = '\0';

		debugMsg((std::string("ANMLParser:traceOut:")+rname).c_str(),
		         indent << "< " << rname << ((inputState->guessing > 0)? "; [guessing]" : ";") <<
						 " LA(1)==" << ((LT(1) == antlr::nullToken)? "null" : LT(1)->getText()) <<
						 " LA(2)==" << ((LT(2) == antlr::nullToken)? "null" : LT(2)->getText()) <<
						 " LA(3)==" << ((LT(3) == antlr::nullToken)? "null" : LT(3)->getText()));
  }
}

class ANMLParser extends Parser;
options {
    buildAST = true;	// uses CommonAST by default
    exportVocab = ANML;
    k=3;
    //defaultErrorHandler=false;
}

{
	public:
		static antlr::RefAST parse(const std::string& path, const std::string& filename);
	protected:
		void ANMLParser::traceIn(const char* rname);
		void ANMLParser::traceOut(const char* rname);
}

anml_program 
    : (anml_stmt)*	      
      {#anml_program = #(#[ANML,"ANML"],#anml_program); }
;

anml_stmt
    : declaration
    | problem_stmt
    | transition_constraint
    | action_def            // <- implicit global object
;

declaration 
    : objtype_decl
    | ( vartype_decl
      | var_obj_declaration
      | function_declaration
      | predicate_declaration
      ) 
      SEMI_COLON!
;

problem_stmt
    : fact
    | goal
;
   
vartype_decl
    : VARTYPE^ user_defined_type_name COLON! var_type
;

// TODO: semantic layer to check whether we're declaring an object or a variable
var_obj_declaration!
    : t:var_type i:var_init_list
      {#var_obj_declaration = #(#[VARIABLE,"variable"], #t, #i);}
;

var_init_list
    : var_init (COMMA! var_init_list)?
;

var_type 
    : BOOL
    | (INT^ | FLOAT^) (range|enumer)?
    | STRING^ (enumer)?
    | VECTOR^ vector_body
    | user_defined_type_name
;

enumer
    : LCURLY^ constant (COMMA! constant)* RCURLY!
;

vector_body
    : parameters
;

// TODO: eventually allow at least constant expressions
range 
    : LBRACK^ signed_literal signed_literal RBRACK!
;


// TODO: eventually allow for expressions instead of constants. make expression semantics declarative, not procedural
// TODO: semantic layer to ensure that vars are only init'd once. 
var_init 
    : var_name (EQUAL^ (constant | arguments))?
;

parameters
    : LPAREN^ (parameter_list)? RPAREN!
;

parameter_list
    : parameter_decl (COMMA! parameter_list)?
;

parameter_decl!
    : t:var_type n:var_name
      {#parameter_decl = #(#[PARAMETER, "param"], #t, #n);}
;

objtype_decl!
    : ot:OBJTYPE n:user_defined_type_name (COLON x:obj_type)?
      b:objtype_body
      { #objtype_decl= #(#ot, #(#n, #x), #b); }
;

obj_type 
    : OBJECT
    | user_defined_type_name
;

objtype_body 
    : LCURLY^ (objtype_body_stmt)* RCURLY!
;

// TODO: semantic layer to make sure that out of all declarations inside an objtype_body,  
// only variables and objects are visible from outside
objtype_body_stmt
    : declaration
    | action_def
    | transition_constraint SEMI_COLON!
;

// In ANML, functions are variables with arguments.
function_declaration 
    : FUNCTION^ var_type function_signature (COMMA! function_signature)*
;

function_signature 
    : function_symbol parameters
;

// Predicates are functions on {true,false}.
predicate_declaration 
    : PREDICATE^ function_signature (COMMA! function_signature)*
;

// Fluents are the same for {Facts,Effects} and for {Goals,Conditions}
// For Effects and Facts :
// - OR is not allowed
// - Temporal qualifiers IN, BEFORE, AFTER and CONTAINS are not allowed.
// - existential quantifier is not allowed
// For Goals and Conditions
// - WHEN clause is not allowed

fact 
    : FACT^ (effect_proposition | effect_proposition_list)
;

goal 
    : GOAL^ (condition_proposition | condition_proposition_list)
;

effect_proposition 
    : proposition[FACT]
;

effect_proposition_list 
    : LCURLY^ effect_proposition (SEMI_COLON! effect_proposition)* (SEMI_COLON!)? RCURLY!
;

condition_proposition 
    : proposition[GOAL]
;

condition_proposition_list
    : LCURLY^ condition_proposition (SEMI_COLON! condition_proposition)* (SEMI_COLON!)? RCURLY!
;

// WHEN is only allowed for {Effects,Facts}, not allowed for {Conditions,Goals}. semantic layer to enforce this
proposition! [int type]
    : q:qualif_fluent
      {#proposition = #q;}
    | {type == FACT}? w:WHEN cc:LCURLY cp:condition_proposition RCURLY ec:LCURLY ep:effect_proposition RCURLY
      {#proposition = #(#w, #(#cc, #cp), #(#ec, #ep)); }
    | fr:FROM t:time_pt qf:qualif_fluent_list
      {#proposition = #(#fr, #t, #qf); }
    | fo:FOR o:object_name fc:LCURLY p:proposition[type] RCURLY
      {#proposition = #(#fo, #o, #(#fc, #p)); }
;

qualif_fluent!
    : q:temporal_qualif f:fluent_list
      {#qualif_fluent = #(#[FLUENT, "fluent"], #q, #f);}
;

qualif_fluent_list 
    : LCURLY^ qualif_fluent (SEMI_COLON! qualif_fluent)* (SEMI_COLON!)? RCURLY!
;

fluent_list
    : LCURLY^ fluent (options {greedy=true;} : SEMI_COLON! fluent)* (SEMI_COLON!)? RCURLY!
;

fluent
    : or_fluent
;

// TODO: OR is not allowed for effects and facts. check and throw exception if necessary
or_fluent
    : and_fluent (options {greedy=true;} : OR^ and_fluent)*
;

and_fluent
    : primary_fluent (options {greedy=true;} : AND^ primary_fluent)*
;

// NOTE : NOT is not supported for now
primary_fluent
    : relational_fluent 
    | quantif_clause fluent
    | LPAREN^ fluent RPAREN!
    //| NOT fluent
;

quantif_clause
    : (FORALL^ | EXISTS^) var_list
;

var_list
    : LPAREN^ var_type var_name (COMMA! var_name)* RPAREN!
;

// NOTE: if the rhs is not present, that means we're stating a predicate to be true
relational_fluent!
    : l:lhs_expr (o:relop r:expr)?
      {#relational_fluent = #(#o, #l, #r);}
;

// NOTE: removed start(fluent), end(fluent) from the grammar, it has to be taken care of by either functions or dot notation
lhs_expr
    : (IDENTIFIER LPAREN)=> function_symbol arguments
      {#lhs_expr = #(#[FUNCTION, "function"], #lhs_expr);}
    | qualified_var_name
;

// TODO: we should allow for full-blown expressions (logical and numerical) at some point
expr 
    : constant
    | arguments // vector assignment.
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
    : AT^ time_pt
    | OVER^ interval
    | IN^ interval (DUR! numeric_expr)?
    | AFTER^ time_pt (DUR! numeric_expr)?
    | BEFORE^ time_pt (DUR! numeric_expr)?
    | CONTAINS^ interval
;

// all(action) is shorthand for the interval [action.start,action.end]
interval 
    : ALL^ (action_label_arg)?
    | LBRACK^ time_pt time_pt RBRACK!
;

action_label_arg
    : LPAREN^ action_instance_label RPAREN!
;

time_pt 
    : numeric_expr
;

numeric_expr 
    : add_expr
;

add_expr
    : mult_expr (options {greedy=true;} : (PLUS^ | MINUS^) mult_expr)*
;
 
mult_expr
    : primary_expr ((MULT^ | DIV^) mult_expr)?
;

primary_expr 
    : atomic_expr    
    | LPAREN^ add_expr RPAREN!
;

// TODO: Verify the addition of enumerations and ranges to atomic expressions.
atomic_expr 
    : (PLUS! | MINUS^)? (enumer | range | numeric_literal | lhs_expr)
;

arguments
    : LPAREN^ (arg_list)? RPAREN!
;

arg_list 
    : expr (COMMA! arg_list)?
;

action_def 
    : ACTION^ action_symbol parameters action_body
;

action_body 
    : LCURLY^ (action_body_stmt SEMI_COLON!)* RCURLY!
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
    : DURATION^ numeric_expr
;

condition_stmt 
    : CONDITION^ (condition_proposition | condition_proposition_list) 
;
    
effect_stmt : EFFECT^ (effect_proposition | effect_proposition_list)
;

change_stmt 
    : CHANGE^ (change_proposition | change_proposition_list)
;

change_proposition_list
    : LCURLY^ change_proposition (SEMI_COLON! change_proposition)* (SEMI_COLON!)? RCURLY!
; 

change_proposition
    :! tq:temporal_qualif LCURLY cf:change_fluent (SEMI_COLON)? RCURLY
       {#change_proposition = #(#[FLUENT, "fluent"], #tq, #cf); }
    | WHEN^ LCURLY! condition_proposition RCURLY! LCURLY! change_proposition RCURLY!
;

change_fluent 
    : and_change_fluent 
;

and_change_fluent
    : primary_change_fluent (options {greedy=true;} : AND^ primary_change_fluent)*
;

primary_change_fluent
    : atomic_change 
    | quantif_clause change_fluent
    | LPAREN^ change_fluent RPAREN! 
;

atomic_change  
    : resource_change
    | transition_change
;

resource_change 
    : (CONSUMES^ | PRODUCES^ | USES^)  LPAREN! var_name (COMMA! numeric_expr)? RPAREN!
;

transition_change
    : qualified_var_name EQUAL^ directed_expr_list
;

directed_expr_list
    : expr (ARROW^ expr)*
;

decomp_stmt 
    : DECOMPOSITION^ (decomp_step | decomp_steps)
;

decomp_steps
    : LCURLY^ (decomp_step SEMI_COLON!)+ RCURLY!
;

decomp_step
    :! tq:temporal_qualif as:action_set
       { #decomp_step = #(#[ACTIONS, "actions"], #tq, #as); }
    | constraint
;

action_set
    : (quantif_clause)?
      (ORDERED^ | UNORDERED^ | DISJUNCTION^) action_set_element_list
;

action_set_element_list
    : LPAREN^ action_set_element (COMMA! action_set_element)* RPAREN!
;

action_set_element
    :! q:qualified_action_symbol a:arguments (COLON l:action_instance_label)?
      {#action_set_element = #(#[ACTION, "action"], #q, #a, #l); }
    | action_set
;

qualified_action_symbol
    :! o:object_name d:DOT a:action_symbol
       { #qualified_action_symbol = #(#o, #(#d, #a)); }
    | action_symbol
;
    
constraint 
    : constraint_expr
;

// TODO: define grammar to support infix notation for constraints
constraint_expr!
    : cs:constraint_symbol a:arguments
      {#constraint_expr = #(#[CONSTRAINT, "constraint"], #cs, #a); }
;

// This constraint is only allowed in the main body of an objtype definition
transition_constraint
    : TRANSITION^ var_name trans_pair_list
;

trans_pair_list
    : LCURLY^ trans_pair (SEMI_COLON! trans_pair)* (SEMI_COLON!)? RCURLY!
;

trans_pair 
    : constant ARROW^ (constant  | LCURLY^ constant (COMMA! constant)* RCURLY!)
;

unsigned_constant 
    : numeric_literal | string_literal | bool_literal
;
constant 
    : signed_literal | string_literal | bool_literal
;

signed_literal
    : (PLUS! | MINUS^)? numeric_literal
;

numeric_literal 
    : (NUMERIC_LIT | INF)
;

bool_literal
    : TRUE | FALSE
;

string_literal
    : STRING_LIT
;

action_symbol           : IDENTIFIER;
action_instance_label   : IDENTIFIER;
constraint_symbol       : IDENTIFIER;
function_symbol         : IDENTIFIER;

object_name             : IDENTIFIER;
var_name                : IDENTIFIER | START | END;

qualified_var_name!
    : n:var_name (d:DOT q:qualified_var_name)?
      {#qualified_var_name = #(#n, #(#d, #q)); }
;


user_defined_type_name  : IDENTIFIER;

{
#include <iostream>
#include <fstream>
#include "ANMLParser.hpp"
}

class ANMLLexer extends Lexer;
options { 
    k=2;
		charVocabulary = '\3'..'\377';
    //caseSensitive=false; 
    testLiterals=false;   // don't automatically test for literals
}

tokens {
    ANML;         // The root of the AST.
    VARIABLE;
    PARAMETER;
    FLUENT;
    ACTIONS;
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
    DISJUNCTION   = "or";
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
    TRUE          = "true";
    FALSE         = "false";
}

{
	void uponEOF() /*throws TokenStreamException, CharStreamException*/ {
		if ( selector.getCurrentStream() != mainLexer ) {
			// don't allow EOF until main lexer.  Force the
			// selector to retry for another token.
			selector.pop(); // return to old lexer/stream
			selector.retry();
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
		$setType(antlr::Token::SKIP);
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
		{ _ttype = antlr::Token::SKIP; }
;

// a numeric literal
NUMERIC_LIT
	{
		bool isDecimal = false;
	}
    : '.' {_ttype = DOT;}
      ( ('0'..'9')+ (EXPONENT)? (FLOAT_SUFFIX)? )?
    | (	'0' {isDecimal = true;} // special case for just '0'
			(	('x'|'X') // hex
				(
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
      | ('1'..'9') ('0'..'9')*  {isDecimal=true;}		// non-zero decimal
		) ( ('l'|'L') 

		// only check to see if it's a float if looks like decimal so far
		|	{isDecimal}?
            (   '.' ('0'..'9')* (EXPONENT)? (FLOAT_SUFFIX)?
            |   EXPONENT (FLOAT_SUFFIX)?
            |   FLOAT_SUFFIX
            )
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
		{$setType(antlr::Token::SKIP); newline();}
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
		{$setType(antlr::Token::SKIP);}
	;

