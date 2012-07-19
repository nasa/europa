/* vim: set ts=8 ft=antlr3: */

grammar NDDL3;

options {
language=C;
output=AST;
ASTLabelType=pANTLR3_BASE_TREE;
}

tokens {
	CONSTRAINT_INSTANTIATION;
	CONSTRUCTOR;
	CONSTRUCTOR_INVOCATION;
	METHOD_CALL;
	NDDL;
	PREDICATE_INSTANCE;     
	TOKEN_RELATION;
	VARIABLE;
    EXPRESSION_ENFORCE;
    EXPRESSION_RETURN;
    FUNCTION_CALL;
    CLOSE;
}

@parser::includes
{
#include "NddlInterpreter.hh"


using namespace EUROPA;

// Forward declaration so that we can use this function in apifuncs
typedef struct NDDL3Parser_Ctx_struct NDDL3Parser, * pNDDL3Parser;
static void newNDDL3ParserFree(pNDDL3Parser ctx);
}

@parser::context 
{
    std::vector<PSLanguageException>* parserErrors;
}

@parser::apifuncs
{
    RECOGNIZER->displayRecognitionError = reportParserError;
    ctx->parserErrors = new std::vector<PSLanguageException>;
    ctx->free = newNDDL3ParserFree;
    // This is needed so that we can get to the CTX from reportParseError
    // Thanks to http://www.antlr.org/pipermail/antlr-interest/2009-May/034567.html
    // for the tip
    PARSER->super = (void *)ctx;
}

@parser::members {
// Declare it in members, so that we can refer to the original Free
static void newNDDL3ParserFree(pNDDL3Parser ctx) {
   delete ctx->parserErrors;
   NDDL3ParserFree(ctx);
}
}


@lexer::includes
{
#include "NddlInterpreter.hh" 

using namespace EUROPA;

// Forward declaration so that we can use this function in apifuncs
typedef struct NDDL3Lexer_Ctx_struct NDDL3Lexer, * pNDDL3Lexer;
static void newNDDL3LexerFree(pNDDL3Lexer ctx);
}

@lexer::context {
    NddlInterpreter* parserObj;
    std::vector<PSLanguageException>* lexerErrors;
}

@lexer::apifuncs
{
    RECOGNIZER->displayRecognitionError = reportLexerError;
    ctx->lexerErrors = new std::vector<PSLanguageException>;
    ctx->free = newNDDL3LexerFree;
}

@lexer::members {
static void newNDDL3LexerFree(pNDDL3Lexer ctx) {
    delete ctx->lexerErrors;
    NDDL3LexerFree(ctx);
}
}

nddl	
    :   nddlStatement*
	       -> ^(NDDL nddlStatement*)
    ;

nddlStatement
    :   typeDefinition
    |   enumDefinition
    |   variableDeclarations
    |   assignment
    |   constraintInstantiation
    |   classDeclaration
    |   allocationStmt
    |   rule
    |   problemStmt
    |   relation
    |   methodInvocation
    |   noopstatement
    ;

enumDefinition
    : 'enum'^ IDENT enumValues
    ;
   
enumValues
    : '{'^ IDENT (','! IDENT)* '}'!
    ;
                
typeDefinition
	:	'typedef' type baseDomain IDENT ';'
	       -> ^('typedef' IDENT type baseDomain)
	;

baseDomain  
    :   intervalBaseDomain
    |   enumeratedBaseDomain
    ;

intervalBaseDomain
    :   '['^ numericLiteral (','!)? numericLiteral ']'!
    ;

enumeratedBaseDomain
    :   '{'^ baseDomainValue (','! baseDomainValue)* '}'!
    ;
       
baseDomainValue
    : literalValue
    | qualified  
    ;
                
variableDeclarations
    :   ('filter')? type nameWithBaseDomain (',' nameWithBaseDomain)* ';'
            -> ^(VARIABLE type nameWithBaseDomain (nameWithBaseDomain)*)
    ;

nameWithBaseDomain
    :   (   variable=IDENT ('('^ value=initializer ')'! )?
        |   variable=IDENT '='^ value=initializer
        )
    ;

anyValue
    :   literalValue
    |   baseDomain
    |   qualified
    ;

allocation
    :   'new'! constructorInvocation
    ;

constructorInvocation
    :   IDENT variableArgumentList
            -> ^(CONSTRUCTOR_INVOCATION IDENT variableArgumentList)
    ;

qualified
    :   ('this' | IDENT) ('.'^ IDENT)*
    ;

assignment
    :   qualified ('in' | '=') initializer ';' -> ^('=' qualified initializer)
    ;
    
initializer
    :   anyValue
    |   allocation
    ;
        
classDeclaration
	:	'class' c=IDENT
		(	(('extends' x=IDENT)? classBlock)
		        -> ^('class' $c ^('extends' $x)? classBlock)
		|	';'
                -> ^('class' $c ';')
		) 
	;

classBlock
	:	'{'^ classStatement* '}'!
	;

classStatement
	:	variableDeclarations
	|	constructor
	|	predicate
	|	noopstatement
	;

constructor
	:	IDENT constructorParameterList constructorBlock
			-> ^(CONSTRUCTOR IDENT constructorParameterList constructorBlock)
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
	:	type IDENT
			-> ^(VARIABLE IDENT type)
	;

predicate
	:	'predicate'^ IDENT predicateBlock 
	;

predicateBlock
	:	'{'^ predicateStatement* '}'!
	;

// Note: Allocations are not legal here.
predicateStatement
	:	variableDeclarations
	|	constraintInstantiation
	|	assignment
	;

rule
    :	IDENT '::'^ IDENT ruleBlock
	;

ruleBlock
	:	'{'^ ruleStatement* '}'!
	|	ruleStatement -> ^('{' ruleStatement)
	;

ruleStatement
	:	relation
	|	variableDeclarations
	|	constraintInstantiation
	|	flowControl
	|	noopstatement
	;

type	
    :	'int'
	|	'float'
	|	'bool'
	|	'string'
	|	IDENT
	;

relation
    :	(token=IDENT | token='this')? temporalRelation predicateArgumentList ';'
			-> ^(TOKEN_RELATION $token temporalRelation predicateArgumentList)
    ;

problemStmt
    :	('rejectable'^ | 'goal'^ | 'fact'^) predicateArgumentList ';'!
	;
        
predicateArgumentList
	:	IDENT
	|	'('^ predicateArguments? ')'!
	;

predicateArguments
	:	predicateArgument (','! predicateArgument)*
	;

predicateArgument
	:	qualified IDENT?
	        -> ^(PREDICATE_INSTANCE qualified IDENT?)
	;

constraintInstantiation
	:	((IDENT variableArgumentList ';')=>IDENT variableArgumentList ';')
			-> ^(CONSTRAINT_INSTANTIATION IDENT variableArgumentList)
		| enforceStatement	
	;

enforceStatement
    :	'enforce'? result=cexpression ';' 
    	-> ^(EXPRESSION_ENFORCE $result)
	;

cexpression 
	:	cbooleanOrExpression
	;
	
cbooleanOrExpression
	:	cbooleanAndExpression ('||'^ cbooleanAndExpression)*
	;

cbooleanAndExpression 
	:	a=crelationalExpression ('&&'^ b=crelationalExpression)*
	;
        
crelationalExpression 
	:	a=cadditiveExpression (('==' | '!=' | '<' | '>' | '>=' | '<=')^ cadditiveExpression)*
	;
	        
cadditiveExpression 
	:	cmultiplicativeExpression (('+' | '-')^ cmultiplicativeExpression)*
    ;

cmultiplicativeExpression 
	:	cprimary ('*'^ cprimary)*
	;

cprimary 
	: anyValue 
	| name=IDENT '(' ex=cexpressionList ')' -> ^(FUNCTION_CALL $name ^('(' cexpressionList))
	| '('! cbooleanOrExpression ')'!
	;

cexpressionList 
	: cexpression (','! cexpression )* 
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

flowControl
@init {
   bool hasElse = false;
} 
    :	'if' '(' result=cexpression ')' a=ruleBlock (('else')=> 'else' b=ruleBlock {hasElse = true;}|) 
         -> {hasElse == false}? ^('if' ^(EXPRESSION_RETURN $result) $a)
         -> ^('if' ^(EXPRESSION_RETURN $result) $a $b)
	|	'foreach'^ '('! IDENT 'in'! qualified ')'! ruleBlock
	;
	
guardExpression
	:	'('! anyValue (('=='^ | '!='^) anyValue)? ')'!
	;
          
allocationStmt
	:	allocation ';'!
	;

temporalRelation
    :   'after'
    |   'any'
    |   'before'
    |   'contained_by'
    |   'contains'
    |   'contains_end'
    |   'contains_start'
    |   'ends'
    |   'ends_after'
    |   'ends_after_start'
    |   'ends_before'
    |   'ends_during'
    |   'equal'
    |   'equals'
    |   'meets'
    |   'met_by'
    |   'parallels'
    |   'paralleled_by'
    |   'starts'
    |   'starts_after'
    |   'starts_before'
    |   'starts_before_end'
    |   'starts_during'
	;

literalValue
    :   booleanLiteral
    |   numericLiteral
    |   stringLiteral
    ;
    
booleanLiteral
    :   'true'
    |   'false' 
    ;

numericLiteral
	:	INT
	|	FLOAT
	|	('+'!)? ('inf' | 'inff')
	|	'-inf' 
    |   '-inff' 
	;

stringLiteral
    :   STRING
    ;
    
methodInvocation
    :   qualified '.' methodName variableArgumentList ';'
            -> ^(METHOD_CALL qualified methodName variableArgumentList)
	|	'close' variableArgumentList ';' // TODO: hack!
			-> ^(CLOSE CLOSE)
	;

methodName
	:	IDENT
	;
	
noopstatement
	:	';'!
	;

INCLUDE :	'#include' WS+ file=STRING 
                {
                        std::string fullName = std::string((const char*)($file.text->chars));
                        
                        // Look for the included file in include path
                        fullName = CTX->parserObj->getFilename(fullName);

                        // TODO: if we couldn't find it, look in the same dir as the current file
                         
                        if (fullName.length() == 0) {
                            std::string path = "";
                            std::vector<std::string> parserPath = CTX->parserObj->getIncludePath();
                            for (unsigned int i=0; i<parserPath.size();i++) {
                                path += parserPath[i] + ":";
                            }
                            //Error message here.
                            CONSTRUCTEX();
                            FAILEDFLAG = ANTLR3_TRUE;
                            RECOGNIZER->state->errorCount++;
                            std::string message = ("ERROR!: couldn't find file: " + std::string((const char*)$file.text->chars)
                                                   + ", search path \"" + path + "\"");
                            RECOGNIZER->state->exception->message = strdup(message.c_str());
                            reportLexerError(RECOGNIZER, NULL); //Note: the second argument does not appear to be used for anything.
                            check_runtime_error(false, std::string("ERROR!: couldn't find file: " + std::string((const char*)$file.text->chars)
                                                                   + ", search path \"" + path + "\"").c_str());
                        }

                        if (!CTX->parserObj->queryIncludeGuard(fullName)) {
                            CTX->parserObj->addInclude(fullName);

                            // Create a new input stream and take advantage of built in stream stacking
                            // in C target runtime.

                            pANTLR3_STRING_FACTORY factory = antlr3StringFactoryNew();
                            pANTLR3_STRING fName = factory->newStr(factory,(ANTLR3_UINT8 *)fullName.c_str());
                        
                            pANTLR3_INPUT_STREAM in = antlr3AsciiFileStreamNew(fName->chars);
                            PUSHSTREAM(in);
                            CTX->parserObj->addInputStream(in);
                            factory->destroy(factory,fName);
                            factory->close(factory);
                        } else {
                            //std::cout << "Ignoring already included file " << fullName << std::endl;
                        }
                        
                        $channel=HIDDEN;
		}
	;

IDENT	:	 ('a'..'z'|'A'..'Z'|'_'|'$') ('a'..'z'|'A'..'Z'|'_'|'0'..'9'|'$')*
	;

STRING	:	'"' (~('\\'|'"') | ESCAPE_SEQUENCE)* '"'
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
	
INT	:	('+' | '-')? ('0' | '1'..'9' '0'..'9'*) INT_SUFFIX?
	;
	
fragment INT_SUFFIX
	:	('l'|'L')
	;

FLOAT	:	('+' | '-')? ('0'..'9')+ '.' ('0'..'9')* EXPONENT? FLOAT_SUFFIX?
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
