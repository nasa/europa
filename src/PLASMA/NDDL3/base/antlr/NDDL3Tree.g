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
		  |	child=classDeclaration
                  |     child=allocation
		  |	child=rule
		  |	problemStmt
                  |     child=relation
		  |	child=methodInvocation
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

variableDeclaration returns [ExprVarDeclaration* result]
        :       ^(VARIABLE dataType=type initExpr=variableInitialization)
{
    if (dataType != NULL)
        result = new ExprVarDeclaration(initExpr->getLhs()->toString().c_str(),dataType->getTypeName().c_str(),initExpr);
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
                        objType=IDENT 
                        variableArgumentList[args]?
                )
                {
                    result = new ExprNewObject(
                        CTX->SymbolTable->getPlanDatabase()->getClient(),
                        c_str($objType.text->chars), // objectType
                        "", // TODO!: object name?
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

constraintInstantiation returns [ExprConstraint* result]
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
@init {
const char* newClass = NULL;
const char* parentClass = "Object";
ObjectType* objType = NULL;
}
	:	^('class'
                   className=IDENT { newClass = c_str($className.text->chars); }
                   
		   (^('extends' superClass=IDENT { parentClass = c_str($superClass.text->chars); }))?
		   
                   {
                       objType = new ObjectType(newClass,parentClass);
                       
                   }
                   
                   (
		       classBlock[objType] { result = new ExprObjectTypeDefinition(objType->getId()); } 
		       | ';' { result = new ExprObjectTypeDeclaration(objType->getId()); }
	           )
		)
  ;

classBlock[ObjectType* objType]
	:	^('{'
			componentTypeEntry[objType]*
		)
	;

componentTypeEntry[ObjectType* objType]
	:	classVariable[objType]
	|	constructor[objType]
	|	predicate[objType]
	;

classVariable[ObjectType* objType]
        :       ^(VARIABLE dataType=type name=IDENT)
        {
            objType->addMember(dataType->getTypeName().c_str(),c_str($name.text->chars)); // TODO!!: this won't work for inlined types
        }
        ;
        
constructor[ObjectType* objType]
@init {
    std::vector<std::string> argNames;
    std::vector<std::string> argTypes;
    std::vector<Expr*> body;
}
	:	^(CONSTRUCTOR
			name=IDENT
			^('(' constructorArgument[argNames,argTypes]*)
			^('{' superCallExpr=constructorSuper[objType]?
			      (child=assignment {body.push_back(child);})*)
		)
		{
		    std::ostringstream signature;
		    signature << objType->getName().c_str();
		    
		    for (unsigned int i=0;i<argTypes.size();i++)
		        signature << ":" << argTypes[i];
		        
                    objType->addObjectFactory(
                        (new InterpretedObjectFactory(
                            objType->getId(),
                            signature.str(),
                            argNames,
                            argTypes,
                            superCallExpr,
                            body)
                        )->getId()
                    );
		}
	;

constructorArgument[std::vector<std::string>& argNames,std::vector<std::string>& argTypes]
	:	^(VARIABLE
			argName=IDENT
			argType=type
		)
		{
		    argNames.push_back(std::string(c_str($argName.text->chars)));
		    argTypes.push_back(argType->getTypeName().toString());
		}
	;

constructorSuper[ObjectType* objType] returns [ExprConstructorSuperCall* result]
@init {
    std::vector<Expr*> args;
}
	:	^('super'
			variableArgumentList[args]
		)
		{
		    result = new ExprConstructorSuperCall(objType->getParent(),args);  
		}
	;
  
assignment returns [ExprAssignment* result]
	:	^('='
			lhs=qualified
			rhs=initializer
		)
		{
		    result = new ExprAssignment(lhs,rhs);
		}
	;

predicate[ObjectType* objType]
@init {
    InterpretedTokenFactory* tokenFactory;
    std::string predName;
}
	:	^('predicate'
			pred=IDENT 
			{ 
			    predName = objType->getName().toString() + "." + c_str($pred.text->chars);   
			    tokenFactory = new InterpretedTokenFactory(predName,objType->getId()); 
			}
			predicateStatements[tokenFactory]
		)
		{
                    objType->addTokenFactory(tokenFactory->getId());
		}
	;

// TODO: allow assignments to inherited parameters
predicateStatements[InterpretedTokenFactory* tokenFactory]
	:	^('{'
			( predicateParameter[tokenFactory] 
                        | predicateParameterAssignment[tokenFactory] 
			| standardConstraint[tokenFactory] 
			)*
		)
	;
	
// Note: Allocations are not legal here.        
predicateParameter[InterpretedTokenFactory* tokenFactory]
        :
        child=variableDeclaration { tokenFactory->addParameter(child); }
        ;       

predicateParameterAssignment[InterpretedTokenFactory* tokenFactory]
        :
        child=assignment { tokenFactory->addVarAssignment(child); }
        ;       

standardConstraint[InterpretedTokenFactory* tokenFactory]
        :
        child = constraintInstantiation { tokenFactory->addConstraint(child); } 
        ;	

rule returns [Expr* result]
@init {
    std::vector<Expr*> ruleBody;
}
 	:	^('::'
			className=IDENT
			predicateName=IDENT
			ruleBlock[ruleBody]
		)
		{
		    std::string predName = std::string(c_str($className.text->chars)) + "." + std::string(c_str($predicateName.text->chars));
		    std::string source=""; // TODO: get this from the antlr parser
		    result = new ExprRuleTypeDefinition((new InterpretedRuleFactory(predName,source,ruleBody))->getId());
		}
	;

ruleBlock[std::vector<Expr*>& ruleBody]
	:	^('{'
			(child=ruleStatement { ruleBody.push_back(child); })*
		)
	;

ruleStatement returns [Expr* result]
	: (	child=constraintInstantiation
	  |	child=assignment
	  |	child=variableDeclaration
	  |	child=ifStatement
	  |	child=loopStatement
	  |     child=relation
	  )
	  {
	      result = child;
	  }
	;


ifStatement returns [Expr* result]
@init {
std::vector<Expr*> ifBody;
std::vector<Expr*> elseBody;
}
	:	^('if'
			guard=guardExpression
			ruleBlock[ifBody]
			ruleBlock[elseBody]?
		)
		{
		    result = new ExprIf(guard,ifBody,elseBody);
		}
  ;

guardExpression returns [ExprIfGuard* result]
@init
{
    const char* relopStr = "==";
}
        : ( ^(relop=guardRelop {relopStr=c_str($relop.text->chars);} lhs=anyValue rhs=anyValue )
          | lhs=anyValue
          )
          {
              result = new ExprIfGuard(relopStr,lhs,rhs);
          }
        ;

guardRelop 
    : '==' | '!='
;

loopStatement returns [Expr* result]
@init {
std::vector<Expr*> loopBody;
}
	:	^('foreach'
			name=IDENT
			val=qualified 
			ruleBlock[loopBody]
		)
		{
		    result = new ExprLoop(c_str($name.text->chars),val->toString().c_str(),loopBody); // TODO : modify ExprLoop to pass val Expr instead
		}
	;

problemStmt returns [Expr* result] 
@init {
    std::vector<PredicateInstanceRef*> tokens;    
}

        :       ^(problemStmtType predicateInstanceList[tokens])
        ;
        
problemStmtType
        :       'goal' 
        |       'rejectable'
        |       'fact'
        ;
        
relation returns [Expr* result]
@init {
    const char* relationType=NULL;
    PredicateInstanceRef* source=NULL;
    std::vector<PredicateInstanceRef*> targets;    
}
	:	^(TOKEN_RELATION
			(i=IDENT { source = new PredicateInstanceRef(NULL,c_str($i.text->chars)); })?
			tr=temporalRelation { relationType = c_str($tr.text->chars); } 
			predicateInstanceList[targets]
		)
		{
		    result = new ExprRelation(relationType,source,targets);
		}
	;

predicateInstanceList[std::vector<PredicateInstanceRef*>& instances]
	:	^('('
			(child=predicateInstance { instances.push_back(child); })*
		)
		|	i=IDENT 
		        { instances.push_back(new PredicateInstanceRef(NULL,c_str($i.text->chars))); } // TODO: check predicate type and pass it along
	;

predicateInstance returns [PredicateInstanceRef* pi]
@init {
    const char* name = NULL;
}
	:	^(qt=qualifiedToken (i=IDENT { name = c_str($i.text->chars); })?)
	        {
	            pi = new PredicateInstanceRef(qt->toString().c_str(),name);
	            delete qt;
	        }
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

qualifiedToken returns [Expr* result]
        :       e=qualified
                {
                    // TODO: type checking !
                    result = e;
                }
        ;
        
temporalRelation
        :       'after'
        |       'any'
        |       'before'
        |       'contained_by'
        |       'contains'
        |       'contains_end'
        |       'contains_start'
        |       'ends'
        |       'ends_after'
        |       'ends_after_start'
        |       'ends_before'
        |       'ends_during'
        |       'equal'
        |       'equals'
        |       'meets'
        |       'met_by'
        |       'parallels'
        |       'paralleled_by'
        |       'starts'
        |       'starts_after'
        |       'starts_before'
        |       'starts_before_end'
        |       'starts_during'
        ;
  
methodInvocation returns [Expr* result]
	:
	(	child=variableMethod
        |       child=tokenMethod
        )
        {
            result = child;
        }
	;

variableMethod returns [Expr* result]
@init {
    std::vector<Expr*> args;
}
        :       ^(op=variableOp v=qualified variableArgumentList[args]?)
                {
                    // TODO!: close() can also apply to the database or to a class
                    result = new ExprVariableMethod(c_str($op.text->chars),v,args);
                }
        ;
   
variableOp
        :       'specify'
        |       'reset'
        |       'close'
        ;
                
tokenMethod returns [Expr* result]    
@init {
    std::vector<Expr*> args;
}
        :       ^(op=tokenOp tok=IDENT variableArgumentList[args]?)
                {
                    result = new ExprTokenMethod(c_str($op.text->chars),c_str($tok.text->chars),args); 
                }
        ;

tokenOp
        :       'activate'
        |       'merge'
        |       'reject'
        |       'cancel'
        |       'free'
        |       'constrain'
        ;        
