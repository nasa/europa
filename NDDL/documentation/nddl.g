// NDDL Grammar (Without semantic checks)

nddl: (nddlStatement)*
    ;

nddlStatement: inclusion | enumeration | typeDefinition SEMICOLON | constraintSignature | allocation SEMICOLON
             | variableDeclaration SEMICOLON | classDeclaration | rule | goal SEMICOLON | relation SEMICOLON
             | function SEMICOLON | assignment SEMICOLON
             | constraintInstantiation SEMICOLON | SEMICOLON
             ;

inclusion: INCLUDE_DECL STRING
         ;

enumeration: ENUM_KEYWORD IDENT symbolSet 
           ;

symbolSet: LBRACE (symbolDefinitions)? RBRACE
         ;

symbolDefinitions: symbolDefinition (COMMA symbolDefinition)*
                 ;

symbolDefinition: IDENT
                ;

typeDefinition: TYPEDEF_KEYWORD typeWithBaseDomain IDENT
              ;

typeWithBaseDomain: INT_KEYWORD    (intervalIntDomain|enumeratedIntDomain)?
                  | FLOAT_KEYWORD  (intervalFloatDomain|enumeratedFloatDomain)?
                  | BOOL_KEYWORD   (enumeratedBoolDomain)?
                  | STRING_KEYWORD (enumeratedStringDomain)?
                  | IDENT          (enumeratedSymbolDomain|enumeratedObjectDomain)?
                  ;

constraintSignature: CONSTRAINT_KEYWORD IDENT typeArgumentList
                     (EXTENDS_KEYWORD IDENT typeArgumentList)? 
                     (signatureBlock | SEMICOLON) 
                   ;

signatureBlock: LBRACE (signatureExpression)? RBRACE
              ;

signatureExpression: signatureAtom ((DAMP | DPIPE) signatureAtom)*
                   ;

signatureAtom: LPAREN signatureExpression RPAREN
             | IDENT IS_A (type | NUMERIC_KEYWORD | IDENT )
             ;

classDeclaration: CLASS_KEYWORD IDENT (
                    ((EXTENDS_KEYWORD IDENT)? classBlock)
                    | SEMICOLON )
                ;

classBlock: LBRACE (classStatement)* RBRACE
          ;

classStatement: constructor | predicate | variableDeclaration SEMICOLON | SEMICOLON
              ;

constructor: IDENT constructorParameterList constructorBlock
           ;

constructorBlock: LBRACE (constructorStatement)* RBRACE
                ;

constructorStatement: (assignment | superInvocation) SEMICOLON | flowControl | SEMICOLON
                    ;

constructorParameterList: LPAREN (constructorParameters)? RPAREN
                        ;

constructorParameters: constructorParameter  (COMMA constructorParameters)?
                     ;

constructorParameter: type IDENT
                    ;

predicate: PREDICATE_KEYWORD IDENT predicateBlock 
         ;

predicateBlock: LBRACE (predicateStatement)* RBRACE
              ;

// Note: Allocations are not legal here.
predicateStatement: ( variableDeclaration | constraintInstantiation | assignment)? SEMICOLON
                  ;

rule: IDENT DCOLON IDENT ruleBlock
    ;

ruleBlock: LBRACE (ruleStatement)* RBRACE | ruleStatement
         ;

ruleStatement: (relation | variableDeclaration | constraintInstantiation) SEMICOLON | flowControl | SEMICOLON
             ;

type: INT_KEYWORD | FLOAT_KEYWORD | BOOL_KEYWORD | STRING_KEYWORD | IDENT
    ;

relation: (IDENT | THIS_KEYWORD)? temporalRelation predicateArgumentList
        ;

goal: (REJECTABLE_KEYWORD | GOAL_KEYWORD) predicateArgumentList
    ;

predicateArgumentList: IDENT | LPAREN (predicateArguments)? RPAREN
                     ;

predicateArguments: predicateArgument (COMMA predicateArgument)*
                  ;

predicateArgument: qualified (IDENT)?
                 ;

constraintInstantiation: IDENT variableArgumentList
                       ;

constructorInvocation: IDENT variableArgumentList
                     ;

superInvocation: SUPER_KEYWORD variableArgumentList
               ;

variableArgumentList: LPAREN (variableArguments)? RPAREN
                    ;

variableArguments: variableArgument (COMMA variableArgument)*
                 ;

// Note: Allocation not legal here
variableArgument: anyValue
                ;

typeArgumentList: LPAREN (typeArguments)? RPAREN
                ;

typeArguments: typeArgument (COMMA typeArgument)*
             ;

typeArgument: IDENT
            ;

domain : intLiteral | intervalIntDomain| enumeratedIntDomain | floatLiteral | intervalFloatDomain| enumeratedFloatDomain
       | enumeratedStringDomain | enumeratedBoolDomain | enumeratedSymbolDomain
       ;

intervalIntDomain: LBRACKET intLiteral (COMMA)? intLiteral RBRACKET
                 ;

intervalFloatDomain: LBRACKET floatLiteral (COMMA)? floatLiteral RBRACKET
                   ;

enumeratedIntDomain: LBRACE intSet RBRACE
                   ;

intSet: intLiteral (COMMA intLiteral)*
      ;

enumeratedFloatDomain: LBRACE floatSet RBRACE
                     ;

floatSet: floatLiteral (COMMA floatLiteral)*
        ;

enumeratedObjectDomain: LBRACE objectSet RBRACE
                      ;

objectSet: (constructorInvocation|qualified) (COMMA (constructorInvocation|qualified))*
         ;

enumeratedSymbolDomain: LBRACE qsymbolSet RBRACE
                      ;

qsymbolSet: qualified (COMMA qualified)*
          ;

enumeratedStringDomain: LBRACE stringSet RBRACE
                      ;

stringSet: STRING (COMMA STRING)*
         ;

enumeratedBoolDomain: LBRACE boolSet RBRACE
                    ;

boolSet: boolLiteral (COMMA boolLiteral)*
       ;

flowControl: (IF_KEYWORD expression ruleBlock (ELSE_KEYWORD  ruleBlock )?)
           | (FOREACH_KEYWORD LPAREN IDENT IN_KEYWORD qualified RPAREN ruleBlock )
           ;

// Note: Allocation not legal here
expression: LPAREN anyValue ((DEQUALS|NEQUALS) anyValue)? RPAREN
          ;

allocation: NEW_KEYWORD constructorInvocation
          ;

variableDeclaration: (FILTER_KEYWORD)? type nameWithBase (COMMA nameWithBase)* 
                   ;

nameWithBase: IDENT (LPAREN anyValue RPAREN)?
            | IDENT EQUALS anyValue
            ;

assignment: qualified (IN_KEYWORD|EQUALS) anyValue
          ;

modifiers: (FILTER_KEYWORD)*
         ;

anyValue: STRING | boolLiteral | qualified | domain | allocation
        ;

qualified: THIS_KEYWORD
         | (THIS_KEYWORD DOT)? IDENT (DOT IDENT)*
         ;

temporalRelation: TR_ANY_KEYWORD | TR_ENDS_KEYWORD | TR_STARTS_KEYWORD | TR_EQUALS_KEYWORD | TR_EQUAL_KEYWORD
                | TR_BEFORE_KEYWORD | TR_AFTER_KEYWORD | TR_CONTAINS_KEYWORD | TR_CONTAINED_BY_KEYWORD
                | TR_ENDS_BEFORE_KEYWORD | TR_ENDS_AFTER_KEYWORD | TR_STARTS_BEFORE_END_KEYWORD | TR_ENDS_AFTER_START_KEYWORD
                | TR_CONTAINS_START_KEYWORD | TR_STARTS_DURING_KEYWORD | TR_CONTAINS_END_KEYWORD | TR_ENDS_DURING_KEYWORD
                | TR_MEETS_KEYWORD | TR_MET_BY_KEYWORD | TR_PARALLELS_KEYWORD | TR_PARALLELED_BY_KEYWORD
                | TR_STARTS_BEFORE_KEYWORD | TR_STARTS_AFTER_KEYWORD
                ;

intLiteral: INT | PINF | NINF 
          ;

floatLiteral: FLOAT | PINFF | NINFF 
            ;

boolLiteral: TRUE_KEYWORD | FALSE_KEYWORD 
           ;

function : qualifiedName DOT 
           ( SPECIFY_KEYWORD variableArgumentList
           | FREE_KEYWORD variableArgumentList
           | CONSTRAIN_KEYWORD variableArgumentList
           | MERGE_KEYWORD variableArgumentList
           | ACTIVATE_KEYWORD LPAREN RPAREN
           | RESET_KEYWORD LPAREN RPAREN
           | REJECT_KEYWORD LPAREN RPAREN
           | CANCEL_KEYWORD LPAREN RPAREN)
         | (IDENT DOT)? CLOSE_KEYWORD LPAREN RPAREN
         ;

tokenNameList: LPAREN (tokenNames)? RPAREN
             ;

tokenNames: IDENT (COMMA IDENT)*
          ;
