/* vim: set ts=8 ft=antlr3: */
tree grammar NDDL3Tree;

options {
	language=C;
	tokenVocab=NDDL3;
	ASTLabelType=pANTLR3_BASE_TREE;
}

@context {
    NddlSymbolTable* SymbolTable;
}

@includes
{
#include "NddlInterpreter.hh"
using namespace EUROPA;

}

@members {

static const char* c_str(pANTLR3_UINT8 chars)
{
    // TODO: what's the best way to do this?
    return (const char*)chars;
}

// TODO: make sure we also get ANTLR errors, see apifuncs below
static void reportSemanticError(pNDDL3Tree treeWalker, const std::string& msg)
{
    // get location. see displayRecognitionError() in antlr3baserecognizer.c
    treeWalker->SymbolTable->addError(msg);
    // TODO: throw exception to abort tree walker?
}

}

@apifuncs
{
    // TODO: Install custom error message display that gathers them in CTX->SymbolTable
    //RECOGNIZER->displayRecognitionError = reportAntlrError;

    // Add a function with the following signature:
    // void reportAntlrError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames);    
    // use the meat from displayRecognitionError() in antlr3baserecognizer.c
}

nddl returns [Expr* result]
@init {
    ExprList* retval = new ExprList();
    result = retval;
}
        :               
	^(NDDL
		(
		  (	child=typeDefinition
                  |     child=variableDeclaration
                  |     allocation
                  |     assignment
                  |     constraintInstantiation
		  |	classDeclaration
		  |	rule
		  |	goal
		  |	fact
		  |	relation
		  |	invocation
		  ) 
		  {
		      if (child != NULL) { 
		          retval->addChild(child);
		          child = NULL; 
		      }
		  }
		)*  
		
	)
	;

typeDefinition returns [Expr* result]
	:	^('typedef'
			name=IDENT
			dataType=type
		)
		{
                    // TODO: can we delay evaluation to keep consistent?
		    ExprTypedef e(c_str($name.text->chars),dataType);
		    e.eval(*(CTX->SymbolTable));
		 
		    result = NULL;   
		}
	;

variableDeclaration returns [Expr* result]
        :       ^(VARIABLE dataType=type initExpr=variableInitialization)
{
    // TODO: remove this check when we deal with classes
    if (dataType != NULL)
        result = new ExprVarDeclaration(initExpr->getLhs()->toString().c_str(),dataType,initExpr);
    else { 
        result = NULL;
        reportSemanticError(CTX,
            "Skipping definition because of unknown data type for var : " + initExpr->getLhs()->toString());
    }
}       
        ;
        
variableInitialization returns [ExprAssignment* result]
        : (      name=IDENT
          |       ^('=' name=IDENT rhs=initializer)
          )
          {
              Expr* lhs = new ExprVariableRef(c_str($name.text->chars),CTX->SymbolTable->getPlanDatabase()->getSchema());
              //std::cout << "read var initialization for:" << $name.text->chars << std::endl;
              result = new ExprAssignment(lhs,rhs);
          }
        ;       

initializer returns [Expr* result]
@init {
    result = new ExprNoop("initializer"); // TODO: implement this
}
        :       anyValue 
        |       allocation
        ;

allocation
        :       ^(CONSTRUCTOR_INVOCATION
                        name=IDENT 
                        variableArgumentList?
                )
        ;

variableArgumentList
        :       ^('('
                        initializer*
                )
        ;

identifier
        :       IDENT
        |       'this'
        ;

type returns [AbstractDomain* result]
        : (      retval=simpleType 
          |       ^(baseType=simpleType retval=inlineType[baseType])
          )
          {
              result = retval;
          }
        ;

simpleType returns [AbstractDomain* result] 
        :       'int'         { result = CTX->SymbolTable->getVarType("int"); }
        |       'float'       { result = CTX->SymbolTable->getVarType("float"); }
        |       'bool'        { result = CTX->SymbolTable->getVarType("bool"); }
        |       'string'      { result = CTX->SymbolTable->getVarType("string"); }
        |       object=IDENT  { result = CTX->SymbolTable->getVarType(c_str($object.text->chars)); }
        ;
        
inlineType[AbstractDomain* baseType] returns [AbstractDomain* result]
        : (      valueSet
          |       numericInterval
          )
{
    result = baseType; // TODO: finish this implementation
}        
        ;       

anyValue
        :       value
        |       valueSet
        |       numericInterval
        ;

valueSet
        :       ^('{'
                        value*
                )
        ;

numericInterval
        :       ^('['   
                        lower=number
                        upper=number
                )
        ;

value
        :       booleanValue
        |       ^(i=IDENT type?)
        |       str=STRING
        |       element=number
        ;

booleanValue
        :       'true'
        |       'false'
        ;


number
        :       floating=FLOAT // real number
        |       integer=INT
        |       pinf='inf'
        |       ninf='-inf'
        ;

classDeclaration
	:	^('class'
			name=IDENT
			((^('extends' extends=IDENT)
				classBlock)
			|	classBlock
			|	';'
			)
		)
  ;

classBlock
	:	^('{'
			componentTypeEntry*
		)
	;

componentTypeEntry
	:	variableDeclaration
	|	constructor
	|	predicate
	|	constraintInstantiation
	;

constructor
	:	^(CONSTRUCTOR
			name=IDENT
			^('(' constructorArgument*)
			^('{'	(	constructorSuper
				|	assignment)*)
		)
	;

constructorArgument
	:	^(VARIABLE
			IDENT
			type
		)
	;

constructorSuper
	:	^('super'
			variableArgumentList
		)
	;
  
assignment
	:	^('='
			qualified
			initializer
		)
	;

predicate
	:	^('predicate'
			pred=IDENT
			predicateStatements
		)
	;

predicateStatements
	:	^('{'
			ruleStatement*
		)
	;

rule
 	:	^('::'
			className=IDENT
			predicateName=IDENT
			ruleBlock
		)
	;

ruleBlock
	:	^('{'
			ruleStatement*
		)
	;

ruleStatement
	:	relation
	|	constraintInstantiation
	|	assignment
	|	variableDeclaration
	|	ifStatement
	|	loopStatement
	|	ruleBlock
	;


ifStatement
	:	^('if'
			expression
			ruleBlock
			ruleBlock?
		)
  ;

loopStatement
	:	^('foreach'
			name=IDENT
			val=identifier
			ruleBlock
		)
	;

// ==========================================================
// expressions
// ==========================================================

expression
	:	^('=='
			anyValue
			anyValue
		)
	|	^('!='
			anyValue
			anyValue
		)
	|	anyValue
	;

goal
	:	^('goal'
			predicateArgumentList
		)
	|	^('rejectable'
			predicateArgumentList
		)
	;

fact
	:	^('fact'
			predicateArgumentList
		)
  ;

relation
	:	^(SUBGOAL
			originName=identifier?
			(	tr=temporalRelationNoInterval
			|	tr=temporalRelationOneInterval
				numericInterval?
			|	tr=temporalRelationTwoIntervals
				(numericInterval
				numericInterval?)?
			)
			predicateArgumentList
		)
	;

predicateArgumentList
	:	^('('
			predicateArgument*
		)
		|	target=IDENT
	;

predicateArgument
	:	^(qualified name=IDENT?)
	;
	
qualified
        :       identifier                 
        |       ^('.' identifier qualified*)
        ;
	

temporalRelationNoInterval
	:	'any'
	|	'equals'
	|	'meets'
	|	'met_by'
	;
  
temporalRelationOneInterval
	:	'ends'
	|	'starts'
	|	'after'
	|	'before'
 	|	'ends_after_start'
	|	'starts_before_end'
	|	'ends_after'
	|	'ends_before'
	|	'starts_after'
	|	'starts_before'
	;

temporalRelationTwoIntervals
	:	'contained_by'
	|	'contains'
	|	'paralleled_by'
	|	'parallels'
	|	'starts_during'
	|	'contains_start'
	|	'ends_during'
	|	'contains_end'
	;
  
constraintInstantiation
	:	^(CONSTRAINT_INSTANTIATION
			name=IDENT
			variableArgumentList
		)
	;

invocation
	:	^('specify' i=IDENT variableArgumentList)
	|	^('free' i=IDENT variableArgumentList)
	|	^('constrain' i=IDENT variableArgumentList)
	|	^('merge' i=IDENT variableArgumentList)
	|	^('close' i=IDENT)
	|	^('activate' i=IDENT)
	|	^('reject' i=IDENT)
	|	^('cancel' i=IDENT)
	|	^('reset' i=IDENT)
	;

