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
    FUNCTION_CALL;
	NDDL;
	PREDICATE_INSTANCE;     
	TOKEN_RELATION;
	VARIABLE;
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
    std::vector<NddlParserException>* lexerErrors;
}

@lexer::apifuncs
{
    RECOGNIZER->displayRecognitionError = reportLexerError;
    ctx->lexerErrors = new std::vector<NddlParserException>;
    ctx->free = newNDDL3LexerFree;
}

@lexer::members {
static void newNDDL3LexerFree(pNDDL3Lexer ctx) {
    delete ctx->lexerErrors;
    NDDL3LexerFree(ctx);
}
}

@parser::includes
{
#include "NddlInterpreter.hh"
using namespace EUROPA;

// Forward declaration so that we can use this function in apifuncs
typedef struct NDDL3Parser_Ctx_struct NDDL3Parser, * pNDDL3Parser;
static void newNDDL3ParserFree(pNDDL3Parser ctx);

//This macro creates an implicit variable. It increments a counter and returns the current interation.
//The name is the name of a newly created c string that holds the name of the variable. Because of
//pointers, you need DELETE_VAR after use.
class _Deleter { 
public: 
    _Deleter(char** target) { 
        t = target; 
    } 
    ~_Deleter() { 
        if(*t) { 
            delete[] *t; } 
        } 
private: 
    char** t;
};
#define DELETE_STRING_AT_END(var) _Deleter var##_##deleter(&var);
#define CREATE_VAR(name) char* name = CTX->parserObj->createImplicitVariable(); DELETE_STRING_AT_END(name);
}

@parser::context {
    NddlInterpreter* parserObj;
    std::vector<NddlParserException>* parserErrors;
}

@parser::apifuncs
{
    RECOGNIZER->displayRecognitionError = reportParserError;
    ctx->parserErrors = new std::vector<NddlParserException>;
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
    |   enforceStatement
    |   constraintSignature!
    ;

enumDefinition
    : 'enum'^ IDENT enumValues
    ;
   
enumValues
    : '{'^ IDENT (','! IDENT)* '}'!
    ;
                
// MEB Language Change: domain no longer optional
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
    :   (('filter')? type nameWithBaseDomain (',' nameWithBaseDomain)* ';'
            -> ^(VARIABLE type nameWithBaseDomain (nameWithBaseDomain)*)
        | 'bool' var=qualified '=' set=testExpression[(const char*)($var.text->chars)] ';'
            -> ^(VARIABLE IDENT["bool"] $var) $set
        )
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

///
expressionSingleton[std::vector<std::string> &argumentlist]
@init {
    CREATE_VAR(variable);
    argumentlist.push_back(variable);
}
    : a=anyValue -> ^(VARIABLE IDENT["bool"] IDENT[variable]) 
        ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[variable] $a));


expressionArgList[std::vector<std::string> &argumentlist]
@init {
}
    : a=expressionSingleton[argumentlist] (',' b=expressionSingleton[argumentlist])* -> $a $b;

functionCall[const char* var]
@init {
    std::vector<std::string> argumentlist;
    std::string stringargs = "";
}
    : functionName=IDENT '(' args=expressionArgList[argumentlist] ')'
        { for (unsigned int i = 0; i < argumentlist.size(); i++) { stringargs += argumentlist[i] + ","; } }
        -> $args ^(FUNCTION_CALL $functionName ^('(' IDENT[var] IDENT[{(ANTLR3_UINT8*)stringargs.c_str()}]));


expressionLiteralNumber[const char* var]
    :   a=INT -> ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $a))
    |   b=FLOAT -> ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $b))
    |   d=booleanLiteral -> ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $d))
    |   f=baseDomain -> ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $f))
    |   g=stringLiteral -> ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $g))
    |   e=functionCall[var] -> $e
    |   c=qualified -> ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $c));

expressionNumberNeg[const char* var, bool negateRight]
@init {
   CREATE_VAR(implicit_var_neg);
}
    :   a=expressionLiteralNumber[(negateRight ? implicit_var_neg : var)]
        -> {negateRight}?
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_neg}])
           $a
           ^(CONSTRAINT_INSTANTIATION IDENT["neg"] ^('(' IDENT[{(ANTLR3_UINT8*)var}] IDENT[{(ANTLR3_UINT8*)implicit_var_neg}]))
        -> $a;
           
additionExpression[const char* var, bool negateRight]
@init {
   CREATE_VAR(implicit_var_left);
   CREATE_VAR(implicit_var_right);
   bool isPlus = true, isSingle = false;
}
    :   a=expressionNumberNeg[implicit_var_left, negateRight] ('+' b=additionExpression[implicit_var_right, false] | {isSingle = true;})
        -> {isSingle == false}? 
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}])
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_right}])
           $a $b
           ^(CONSTRAINT_INSTANTIATION IDENT["addEq"] ^('(' IDENT[{(ANTLR3_UINT8*)implicit_var_left}] IDENT[{(ANTLR3_UINT8*)implicit_var_right}] IDENT[var]))

        -> ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}]) $a
           ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] IDENT[{(ANTLR3_UINT8*)implicit_var_left}]))

    |   a=expressionNumberNeg[implicit_var_left, negateRight] '-' b=additionExpression[implicit_var_right, true]
        -> ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}])
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_right}])
           $a $b
           ^(CONSTRAINT_INSTANTIATION IDENT["addEq"] ^('(' IDENT[{(ANTLR3_UINT8*)implicit_var_left}] IDENT[{(ANTLR3_UINT8*)implicit_var_right}] IDENT[var]));



relationalExpression[const char* var]
@init {
   CREATE_VAR(implicit_var_left);
   CREATE_VAR(implicit_var_right);
#define SETCON(name) contraint = (char**)&(name); invert = false;
#define IVRCON(name) contraint = (char**)&(name); invert = true;
   char const* TEST_LT = "TestLessThan";
   char const* TEST_LEQ = "testLEQ";
   char const* TEST_EQ = "testEQ";
   char const* TEST_NEQ = "testNEQ";
   bool invert = false; //Invert is so that > and >= can work - there are no constraints for these, so feed the args to < and <= backwards.
   char** contraint = NULL;
}
    :   a=additionExpression[implicit_var_left, false] 
            ('==' {SETCON(TEST_EQ);} | '!=' {SETCON(TEST_NEQ);} 
            | '<' {SETCON(TEST_LT);} | '<=' {SETCON(TEST_LEQ);} 
            | '>' {IVRCON(TEST_LT);} | '>=' {IVRCON(TEST_LEQ);}) 
        b=additionExpression[implicit_var_right, false]
        -> {!invert}?
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}])
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_right}])
           $a $b ^(CONSTRAINT_INSTANTIATION IDENT[{(ANTLR3_UINT8*)(*contraint)}]
            ^('(' IDENT[var] IDENT[implicit_var_left] IDENT[implicit_var_right]))
        -> {invert}?
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}])
           ^(VARIABLE IDENT["float"] IDENT[{(ANTLR3_UINT8*)implicit_var_right}])
           $a $b ^(CONSTRAINT_INSTANTIATION IDENT[{(ANTLR3_UINT8*)(*contraint)}]
            ^('(' IDENT[var] IDENT[implicit_var_right] IDENT[implicit_var_left]))
        -> ;

///
booleanLiteralExpression[const char* var]
    :   val=booleanLiteral -> ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $val ))
    |   varname=qualified ->  ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[var] $varname))
    |   re=relationalExpression[var] -> $re
    |   fn=functionCall[var] -> $fn
    |   '(' paren=booleanOrExpression[var] ')' -> $paren
    ;


booleanAndExpression[const char* var]
@init {
   CREATE_VAR(implicit_var_left);
   CREATE_VAR(implicit_var_right);
   bool isSingle = false;
}
    :   a=booleanLiteralExpression[implicit_var_left] ('&&' b=booleanAndExpression[implicit_var_right] | {isSingle = true;})
        //This first one runs in the case of a single expression with no &&.
        -> {isSingle}? ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}]) 
           $a ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[implicit_var_left] IDENT[var])) //Need to think of a better way.
        //This second one runs in the case of a && b.
        -> ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}])
           ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_right}])
           $a $b ^(CONSTRAINT_INSTANTIATION IDENT["testAnd"] ^('(' IDENT[var] IDENT[implicit_var_left] IDENT[implicit_var_right])) ;


booleanOrExpression[const char* var]
@init {
   CREATE_VAR(implicit_var_left);
   CREATE_VAR(implicit_var_right);
   bool isSingle = false;
}
    :   a=booleanAndExpression[implicit_var_left] ('||' b=booleanOrExpression[implicit_var_right] | {isSingle = true;})
        //This first one runs in the case of a single expression with no ||.
        -> {isSingle}? ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}]) 
           $a ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[implicit_var_left] IDENT[var])) //Need to think of a better way.
        //This second one runs in the case of a || b.
        -> ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_left}])
           ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_right}])
           $a $b ^(CONSTRAINT_INSTANTIATION IDENT["testOr"] ^('(' IDENT[var] IDENT[implicit_var_left] IDENT[implicit_var_right])) ;

testExpression[const char* var]
    :  'test(' result=booleanOrExpression[var] ')' -> $result ;

enforceStatement
@init {
   CREATE_VAR(implicit_var_return);
}
    :  'enforce' '(' result=booleanOrExpression[implicit_var_return] ')' ';' -> 
        ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_return}]) 
        $result ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[implicit_var_return] 'true'))
    | result=booleanOrExpression[implicit_var_return] ';' ->
        ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_return}]) 
        $result ^(CONSTRAINT_INSTANTIATION IDENT["eq"] ^('(' IDENT[implicit_var_return] 'true'));
///

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
@init {
   CREATE_VAR(implicit_var_name);
}
    :   (   qualified ('in' | '=') initializer ';'
                 -> ^('=' qualified initializer)
        |   var=qualified ('in' | '=') set=testExpression[implicit_var_name] ';'
            -> ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_name}])
                 $set ^(CONSTRAINT_INSTANTIATION IDENT["eq"] 
                        ^('(' $var IDENT[{(ANTLR3_UINT8*)implicit_var_name}]))
        )
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
    |   enforceStatement
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
	:	IDENT variableArgumentList ';'
			-> ^(CONSTRAINT_INSTANTIATION IDENT variableArgumentList)
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
   CREATE_VAR(implicit_var_return);
   bool hasElse = false;
}

    :	'if' '(' result=booleanOrExpression[implicit_var_return] ')' a=ruleBlock ('else' b=ruleBlock {hasElse = true;}|) 
        -> {hasElse == false}? ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_return}])
           $result ^('if' ^('test' IDENT[implicit_var_return]) $a)
        -> ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_return}])
           $result ^('if' ^('test' IDENT[implicit_var_return]) $a $b)

    |   'if' 'test' '(' result=booleanOrExpression[implicit_var_return] ')' a=ruleBlock ('else' b=ruleBlock {hasElse = true;}|) 
        -> {hasElse == false}? ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_return}])
           $result ^('if' ^('test' IDENT[implicit_var_return]) $a)
        -> ^(VARIABLE IDENT["bool"] IDENT[{(ANTLR3_UINT8*)implicit_var_return}])
           $result ^('if' ^('test' IDENT[implicit_var_return]) $a $b)
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
    
// TODO: this is ugly and very inflexible, need to provide extensible method exporting mechanism  
methodName
    :   'specify'
    |   'reset'
    |   'constrain'
    |   'free'
    |   'activate'
    |   'merge'
    |   'reject'
    |   'cancel'
    |   'close'
    ;
         
methodInvocation
    :   (qualified '.')? methodName variableArgumentList ';'
            -> ^(METHOD_CALL methodName qualified? variableArgumentList)
	;

tokenNameList
	:  '('^ (tokenNames)? ')'!
	;

tokenNames
	:	IDENT (','! IDENT)*
    ;

noopstatement
	:	';'!
	;

constraintSignature
    :   'constraint' c=IDENT args=typeArgumentList
            ('extends' x=IDENT xargs=typeArgumentList)? 
            (sb=signatureBlock | ';')
                -> ^('constraint' $c $args ^('extends' $x $xargs)? $sb?)
    ;

signatureBlock
    :   '{'^ signatureExpression? '}'!
    ;

signatureExpression
    :   signatureAtom (('&&'^ | '||'^) signatureAtom)*
    ;

signatureAtom
    :   '('^ signatureExpression ')'!
    |   IDENT '<:'^ (type | 'numeric' )
    ;

INCLUDE :	'#include' WS+ file=STRING 
                {
                        std::string fullName = std::string((const char*)($file.text->chars));
                        fullName = CTX->parserObj->getFilename(fullName);
                        if (!CTX->parserObj->queryIncludeGuard(fullName)) {
                            CTX->parserObj->addInclude(fullName);
                            // Look for the included file in include path

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
                            } else {
                                
                                // Create a new input stream and take advantage of built in stream stacking
                                // in C target runtime.
                                
                                pANTLR3_STRING_FACTORY factory = antlr3StringFactoryNew();
                                pANTLR3_STRING fName = factory->newStr(factory,(ANTLR3_UINT8 *)fullName.c_str());
                                //factory->close(factory);
                                
                                pANTLR3_INPUT_STREAM in = antlr3AsciiFileStreamNew(fName->chars);
                                //fName->free(fName);
                                PUSHSTREAM(in);
                                CTX->parserObj->addInputStream(in);
                            }
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
