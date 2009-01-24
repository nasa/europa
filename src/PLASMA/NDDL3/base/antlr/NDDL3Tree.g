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
#include "Debug.hh"
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

nddl :               
	^(NDDL
		(
		  (	child=typeDefinition
                  |     child=variableDeclaration
                  |     child=assignment
                  |     constraintInstantiation
		  |	classDeclaration
                  |     allocation
		  |	rule
		  |	goal
		  |	fact
                  |     relation
		  |	invocation
		  ) 
		  {
		      if (child != NULL) { 
		          debugMsg("NddlInterpreter:nddl","Evaluating:" << child->toString());
		          child->eval(*(CTX->SymbolTable));
		          delete child;
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
		    if (dataType != NULL) {
		        if (!dataType->getIsRestricted()) // then this is just an alias for another type
		            dataType = dataType->copy();
		        
		        dataType->setTypeName(c_str($name.text->chars));    		            
		        result = new ExprTypedef(c_str($name.text->chars),dataType);
		     }
		     else {
                         result = NULL;              
                         reportSemanticError(CTX,
                            "Incorrect typedef. Unknown data type for : " + std::string(c_str($name.text->chars)));
		     }   
		}
	;

variableDeclaration returns [Expr* result]
        :       ^(VARIABLE dataType=type initExpr=variableInitialization)
{
    if (dataType != NULL)
        result = new ExprVarDeclaration(initExpr->getLhs()->toString().c_str(),dataType,initExpr);
    else { 
        result = NULL;
        reportSemanticError(CTX,
            "Incorrect variable declaration. Unknown data type for var : " + initExpr->getLhs()->toString());
    }
}       
        ;
        
variableInitialization returns [ExprAssignment* result]
        : (      name=IDENT
          |       ^('=' name=IDENT rhs=initializer)
          )
          {
              Expr* lhs = new ExprVarRef(c_str($name.text->chars));
              //std::cout << "read var initialization for:" << $name.text->chars << std::endl;
              result = new ExprAssignment(lhs,rhs);
          }
        ;       

initializer returns [Expr* result]
@init {
    result = NULL; // TODO: implement this
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
  
assignment returns [Expr* result]
	:	^('='
			lhs=qualified
			rhs=initializer
		)
		{
		    result = new ExprAssignment(lhs,rhs);
		}
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
	
qualified returns [Expr* result]
@init {
    std::string varName;
}
        :  (      name=identifier { varName=c_str($name.text->chars); }                 
           |       ^('.' name=identifier { if (varName.length()>0) varName+="."; varName+=c_str($name.text->chars); } qualified*)
           )
           {
               // TODO!!: do type checking at each "."
               result = new ExprVarRef(varName.c_str());
           }
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

