header { package testLang;}


class TestLangParser extends Parser;
options{ k=2; buildAST=true; exportVocab=TestLang; defaultErrorHandler=false;}


enumeration :  OBRACE^ (single_value (COMMA!)?)+ CBRACE!
            //{System.err.println("enumeration");System.err.println(currentAST);}
            ;

range : OBRACKET^ single_value TO! single_value CBRACKET!
      //{System.err.println("range");System.err.println(currentAST);}
      ;

step_statement : //(step_qualifier)? STEP^ (single_boolean_op INTEGER | list_boolean_op list_value)?
                 ((FIRST | LAST) STEP^ | (ANY | EACH | ALL)? STEP^ (single_boolean_op INTEGER | list_boolean_op list_value)?)
               //{System.err.println("step_statement");System.err.println(currentAST);}
               ;

start_statement : START^ (single_boolean_op single_value | list_boolean_op list_value)
                //{System.err.println("start_statement");System.err.println(currentAST);}
                ;

end_statement : END^ (single_boolean_op single_value | list_boolean_op list_value)
              //{System.err.println("end_statement");System.err.println(currentAST);}
              ;

status_statement : STATUS^ (single_boolean_op single_value | (IN | OUT | INTERSECTS) list_value)
                 //{System.err.println("status_statement");System.err.println(currentAST);}
                 ;

predicate_statement : PREDICATE^ ((EQ | NE) single_value | (IN | OUT | INTERSECTS) list_value)
                    //{System.err.println("predicate_statement");System.err.println(currentAST);}
                    ;

value_statement : VALUE^ (single_boolean_op single_value | list_boolean_op list_value)
                //{System.err.println("value_statement");System.err.println(currentAST);}
                ;

variable_statement : VARIABLE^ OPAREN! name_statement (COMMA!)? value_statement CPAREN!
                    //{System.err.println("variable_statement");System.err.println(currentAST);}
                    ;

token_function : TOKENS^ OPAREN! (start_statement | end_statement | 
                                  status_statement | predicate_statement | variable_statement)* CPAREN!
                 //{System.err.println("token_function");System.err.println(currentAST);}
                 ;

name_statement : NAME^ (((EQ | NE) STRING) | ((IN | OUT | INTERSECTS) list_value))
								//{System.err.println("name_statement");System.err.println(currentAST);}
								;

object_function : OBJECTS^ OPAREN! (name_statement | variable_statement)* CPAREN!
                //{System.err.println("object_function");System.err.println(currentAST);}
                ;

type_statement : TYPE^ ((EQ | NE) STRING | (IN | OUT | INTERSECTS) list_value)
               //{System.err.println("type_statement");System.err.println(currentAST);}
               ;


trans_name_function : NAME^ EQ STRING;
trans_type_function : TYPE^ EQ STRING;
transaction_function : TRANSACTIONS^ OPAREN! (trans_name_function | trans_type_function) CPAREN!  
                     //tightened grammar for PLASMA support.  may loosen it later.  ~MJI
                     //{System.err.println("transaction_function");System.err.println(currentAST);}
                     ;

list_valued_function : token_function | object_function | transaction_function
                     //{System.err.println("list_valued_function");System.err.println(currentAST);}
                     ;

entity_function : ENTITY^ OPAREN! INTEGER (COMMA!)? list_value CPAREN!
                //{System.err.println("entity_function");System.err.println(currentAST);}
                ;

property_function : PROPERTY^ OPAREN! STRING (COMMA!)? (token_function | object_function | 
                                                    entity_function) CPAREN!
                  //{System.err.println("property_function");System.err.println(currentAST);}
                  ;

count_function : COUNT^ OPAREN! list_valued_function CPAREN!
               //{System.err.println("count_function");System.err.println(currentAST);}
               ;

single_valued_function : entity_function | property_function | count_function
                       //{System.err.println("single_valued_function");System.err.println(currentAST);}
                       ;

list_value : range | enumeration | list_valued_function
           //{System.err.println("list_value");System.err.println(currentAST);}
           ;

single_value : INTEGER | STRING | FIRST | LAST | single_valued_function
             //{System.err.println("single_value"); System.err.println(currentAST);}
             ;

single_boolean_op : (EQ^ | LT^ | GT^ | GE^ | LE^ | NE^)
                  //{System.err.println("single_boolean_op");System.err.println(currentAST);}
                  ;

list_boolean_op : (EQ^ | LT^ | GT^ | GE^ | LE^ | NE^ | IN^ | OUT^ | INTERSECTS^)
                //{System.err.println("list_boolean_op");System.err.println(currentAST);}
                ;

list_boolean_statement : list_value (EQ^ | LT^ | GT^ | GE^ | LE^ | NE^ | IN^ | OUT^ | INTERSECTS^) 
                         (list_value | single_value)
                       //{System.err.println("list_boolean_statement");System.err.println(currentAST);}
                       ;

singleton_boolean_statement : single_value ((EQ^ | LT^ | GT^ | GE^ | LE^ | NE^) single_value |
                              	            (IN^ | OUT^ | LT^ | GT^ | GE^ | LE^) list_value)
                            //{System.err.println("singleton_boolean_statement");System.err.println(currentAST);}
                            ;

boolean_statement : singleton_boolean_statement | list_boolean_statement
                  //{System.err.println("boolean_statement");System.err.println(currentAST);}
                  ;

step_qualifier : FIRST | LAST | ANY | ALL | EACH
               //{System.err.println("step_qualifier");System.err.println(currentAST);}
               ;

assertion_statement : AT^ step_statement COLON! boolean_statement
                    //{System.err.println("assertion_statement");System.err.println(currentAST);}
                    ;

test_set {getASTFactory().setASTNodeClass("testLang.LineNumberAST");}
  : TEST^ OPAREN! STRING COMMA! (test_set | assertion_statement SEMI!)+ CPAREN! SEMI!
           //{System.err.println("test_set");System.err.println(currentAST);}
           ;

class TestLangLexer extends Lexer;
options {k=11; exportVocab=TestLang; defaultErrorHandler=false;}

WS :
   (' '
   | '\t'
   | "\r\n"
   | '\n' {newline();}
   )
   {$setType(Token.SKIP);}
   ;

STRING : '\'' (ESC|~('\''|'\\'))* '\'';
ESC : '\\' ( 'n' | 't' | '\'' | '\\') ;
OPAREN : '(';
CPAREN : ')';
OBRACKET : '[';
CBRACKET : ']';
OBRACE : '{';
CBRACE : '}';
SEMI : ';';
COLON : ':';
COMMA : ',';
SQUOTE : '\'';
protected PLUS : '+';
protected MINUS : '-';
EQ : ('=' | "==");
LT : '<';
GT : '>';
GE : ">=";
LE : "<=";
NE : "!=";
TO : "..";
INTEGER : (PLUS | MINUS)? ('0'..'9')+;
TEST : "Test";
TOKENS : "Tokens";
OBJECTS : "Objects";
TRANSACTIONS : "Transactions";
ENTITY : "Entity";
PROPERTY : "Property";
COUNT : "Count";
IN : "in";
OUT : "out";
INTERSECTS : "intersects";
INF : "Inf";
STEP : "step";
START : "start";
END : "end";
DURATION : "duration";
STATUS : "status";
PREDICATE : "predicate";
OBJECT : "object";
MASTER : "master";
VARIABLE : "variable";
VALUE : "value";
NAME : "name";
TYPE : "type";
FIRST : ("FIRST" | "first");
LAST : ("LAST" | "last");
ALL : ("ALL" | "all");
ANY : ("ANY" | "any");
EACH : ("EACH" | "each");
AT : "At";

//this rule exists because ANTLR will only generate matches for characters used in other rules
DUMMY : "ghlqvwxzABCDFGHJKLMNQRSUVWXYZ1234567890!@#$%^&*(){}[]<>,./?\";:`~_-+=\\|";
