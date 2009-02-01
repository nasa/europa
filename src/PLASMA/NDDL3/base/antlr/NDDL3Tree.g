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
#include "BoolDomain.hh"
#include "StringDomain.hh"
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

static std::string getAutoLabel(const char* prefix)
{
    static int idx = 0;
    std::ostringstream os;
    os << prefix << "_" << idx++;
    
    return os.str();   
}

static DataRef evalExpr(pNDDL3Tree treeWalker,Expr* expr)
{
    return expr->eval(*(treeWalker->SymbolTable));
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
		( {
		     // debugMsg("NddlInterpreter:nddl","Line:" << LEXER->getLine(LEXER)); 
		  }
		  (	child=typeDefinition
                  |     child=variableDeclaration
                  |     child=assignment
                  |     child=constraintInstantiation
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
		          evalExpr(CTX,child);
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
		        
		        const char* newName = c_str($name.text->chars);
		        dataType->setTypeName(newName);    		            
		        result = new ExprTypedef(newName,dataType);
		     }
		     else {
                         result = NULL;              
                         reportSemanticError(CTX,
                            "Incorrect typedef. Unknown data type for : " + std::string(c_str($name.text->chars)));
		     }   
		}
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
        : (      child=valueSet
          |      child=numericInterval
          )
{
    // TODO: type checking. ensure inline domain is consistent with base
    DataRef data=evalExpr(CTX,child);    
    result = (AbstractDomain*)&(data.getValue()->lastDomain()); 
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
              result = new ExprAssignment(lhs,rhs); // TODO: to preserve old semantics, this needs to restrict base domain instead
          }
        ;       

initializer returns [Expr* result]
        : (       child=anyValue 
          |       child=allocation
          )
          {
              result = child;
          }
        ;

anyValue returns [Expr* result]
        : (      child=value
          |      child=valueSet
          |      child=numericInterval
          )
          { 
              result = child;
          }
        ;

value returns [Expr* result]
@init {
    result = NULL;
}
        : (       child=booleanLiteral
          |       child=stringLiteral
          |       child=numericLiteral 
          |       ^(i=IDENT type?) // TODO: what is this?, a variable ref?, an object ref?
                   { result = new ExprVarRef(c_str($i.text->chars)); }
          )
          { 
              if (result == NULL)
                  result = new ExprConstant(
                      CTX->SymbolTable->getPlanDatabase()->getClient(),
                      child->getTypeName().c_str(),
                      child); 
          }
        ;

valueSet returns [Expr* result]
@init {
    bool isNumeric = false;
    std::list<double> values;
    std::string autoName = getAutoLabel("ENUM");
    const char* typeName = autoName.c_str();
}
        :       ^('{'
                        (element=value
                         {
                             DataRef elemValue = evalExpr(CTX,element);
                             const AbstractDomain& ev = elemValue.getValue()->lastDomain(); 
                             // TODO: delete element;
                             double v = ev.getSingletonValue();
                             values.push_back(v);
                             isNumeric = ev.isNumeric();
                             // TODO: make sure data types for all values are consistent
                         }
                        )*
                 )
                 {
                   AbstractDomain* newDomain = new EnumeratedDomain(values,isNumeric,typeName); 
                   result = new ExprConstant(
                       CTX->SymbolTable->getPlanDatabase()->getClient(),
                       typeName,
                       newDomain                       
                   );
                   
                   // TODO: this is necessary so that the Expr definaed above can be evaluated, see about fixing it.
                   ExprTypedef newTypedef(typeName,newDomain);
                   evalExpr(CTX,&newTypedef);
                 }
        ;

booleanLiteral returns [AbstractDomain* result]
        :       'true'  { result = new BoolDomain(true); }            
        |       'false' { result = new BoolDomain(false); }
        ;

stringLiteral returns [AbstractDomain* result]
        :    str = STRING 
             { 
                 LabelStr value = c_str($str.text->chars); 
                 result = new StringDomain((double)value);
             }
        ; 

numericLiteral returns [AbstractDomain* result]
        :       floating=FLOAT  { result = new IntervalDomain(atof(c_str($floating.text->chars))); }
        |       integer=INT     { result = new IntervalIntDomain(atoi(c_str($integer.text->chars))); }
        |       'inf'           { result = new IntervalIntDomain(PLUS_INFINITY); }   // TODO: deal with real inf vs int inf?
        |       '-inf'          { result = new IntervalIntDomain(MINUS_INFINITY); }  // TODO: deal with real inf vs int inf?
        ;

numericInterval returns [Expr* result]
        :       ^('['   
                        lower=numericLiteral
                        upper=numericLiteral
                )
                {      
                    double lb = lower->getSingletonValue();
                    double ub = upper->getSingletonValue();
                    const char* typeName;
                    AbstractDomain* baseDomain;
                    
                    if (lower->getTypeName().toString()=="float" || upper->getTypeName().toString()=="float") {
                        typeName = "float";
                        baseDomain = new IntervalDomain(lb,ub);
                    }
                    else {
                        typeName = "int";
                        baseDomain = new IntervalIntDomain((int)lb,(int)ub);
                    }
                                  
                    result = new ExprConstant(
                        CTX->SymbolTable->getPlanDatabase()->getClient(),
                        lower->getTypeName().c_str(),
                        baseDomain
                    );
                    
                    delete lower;
                    delete upper;
                }
        ;

allocation returns [Expr* result]
@init {
    std::vector<Expr*> args;
}
        :       ^(CONSTRUCTOR_INVOCATION
                        name=IDENT 
                        variableArgumentList[args]?
                )
                {
                    result = new ExprNewObject(
                        CTX->SymbolTable->getPlanDatabase()->getClient(),
                        "", // TODO!: object name?
                        c_str($name.text->chars), // objectType
                        args
                    );
                }
        ;

variableArgumentList[std::vector<Expr*>& result]
        :       ^('('
                        (arg=initializer {result.push_back(arg);})*
                )
        ;

identifier 
        : IDENT
          | 'this'
        ;

constraintInstantiation returns [Expr* result]
@init {
    std::vector<Expr*> args;
}
        :       
              ^(CONSTRAINT_INSTANTIATION
                        name=IDENT
                        variableArgumentList[args]
                )
                {
                    result = new ExprConstraint(c_str($name.text->chars),args);
                }
                
        ;

classDeclaration returns [Expr* result]
	:	^('class'
			className=IDENT
			((^('extends' superClass=IDENT)
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
@init {
    std::vector<Expr*> args;
}
	:	^('super'
			variableArgumentList[args]
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
  
invocation
@init {
    std::vector<Expr*> args;
}
	:	^('specify' i=IDENT variableArgumentList[args])
	|	^('free' i=IDENT variableArgumentList[args])
	|	^('constrain' i=IDENT variableArgumentList[args])
	|	^('merge' i=IDENT variableArgumentList[args])
	|	^('close' i=IDENT)
	|	^('activate' i=IDENT)
	|	^('reject' i=IDENT)
	|	^('cancel' i=IDENT)
	|	^('reset' i=IDENT)
	;

