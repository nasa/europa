/* vim: set ts=8 ft=antlr3: */
//Add numbers here to force rebuild until bug fixed: 1
tree grammar NDDL3Tree;

options {
	language=C;
	tokenVocab=NDDL3;
	ASTLabelType=pANTLR3_BASE_TREE;
}

@context {
    NddlSymbolTable* SymbolTable;
    NddlInterpreter* parserObj;
}

@includes
{
#include "Debug.hh"
#include "Domains.hh"
#include "NddlInterpreter.hh"
#include "PathDefs.hh"
#include "Utils.hh"
#include "Error.hh"
#include <iostream>
#include <boost/cast.hpp>
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
    treeWalker->SymbolTable->reportError(treeWalker,msg);
    // TODO: make sure cleanup is done correctly
    throw msg;
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

static void pushContext(pNDDL3Tree treeWalker, NddlSymbolTable* st)
{
    treeWalker->SymbolTable = st; 
}

static void popContext(pNDDL3Tree treeWalker)
{
    NddlSymbolTable* st = treeWalker->SymbolTable; 
    treeWalker->SymbolTable = st->getParentST();
    delete st;                                  
    check_error(treeWalker->SymbolTable != NULL);
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

nddl      
	: ^(NDDL
		( {
		     // debugMsg("NddlInterpreter:nddl","Line:" << LEXER->getLine(LEXER)); 
		  }
		  (	child=classDeclaration
          | child=enumDefinition
		  |	child=typeDefinition
		  |	child=variableDeclarations
		  |	child=assignment
		  |	child=constraintInstantiation
		  |	child=enforceExpression
		  |	child=allocation[NULL]
		  |	child=rule
		  |	child=problemStmt
		  |	child=relation
		  |	child=methodInvocation
		  ) 
		  {
		      if (child != NULL) { 
                  boost::scoped_ptr<Expr> c(child);
		          debugMsg("NddlInterpreter:nddl","Evaluating:" << child->toString());
                  evalExpr(CTX,child);
		          debugMsg("NddlInterpreter:nddl","Done evaluating:" << child->toString());
                  CTX->SymbolTable->popFromCleanupStack();
                  checkError(CTX->SymbolTable->cleanupStackSize() == 0,
                             CTX->SymbolTable->cleanupStackSize());
		          // TODO!!: systematically deal with memory mgmt for all Exprs.
                  CTX->SymbolTable->getPlanDatabase()->getConstraintEngine()->propagate();
		      }
              CTX->SymbolTable->getPlanDatabase()->getConstraintEngine()->propagate();
              
		  }
		)*  
		
	)
	;

enumDefinition returns [Expr* result]
@init {
    std::vector<LabelStr> values;
}
    :   ^('enum' name=IDENT enumValues[values])
        {
            const char* enumName = c_str($name.text->chars);
            result = new ExprEnumdef(enumName,values);
            CTX->SymbolTable->pushToCleanupStack(result);
        }
    ;

enumValues[std::vector<LabelStr>& values]
    :       ^('{' (v=IDENT {values.push_back(c_str($v.text->chars));})+ )
    ;
                  
typeDefinition returns [Expr* result]
    :   ^('typedef'
            name=IDENT
            dataType=type
			domain=baseDomain[dataType]
        )
        {
            const char* newName = c_str($name.text->chars);
	        result = new ExprTypedef(dataType->getId(),newName,domain);
            CTX->SymbolTable->pushToCleanupStack(result);
		}
	;

type returns [DataType* result] 
    :   (   name='int'
        |   name='float'
        |   name='bool'
        |   name='string'
        |   name=IDENT
        )
        {
            const char* nameStr = c_str($name.text->chars);
            DataTypeId dt = CTX->SymbolTable->getDataType(nameStr);
            
            if (dt.isId()) 
                result = (DataType*)dt;
            else 
                reportSemanticError(CTX,"Unknown data type:"+std::string(nameStr));           
        }
    ;
        
baseDomain[const DataType* baseType] returns [Domain* result]
    :   child=baseDomainValues
    {
        DataRef data=evalExpr(CTX,child);
        CTX->SymbolTable->popFromCleanupStack();
        delete child;
        ConstrainedVariableId value = data.getValue();
        if (!baseType->isAssignableFrom(value->getDataType())) {
            reportSemanticError(CTX,
                "Can't assign "+value->toString()+" to "+baseType->getName());
        }
       
        result = value->lastDomain().copy(); 
    }        
    ;       

baseDomainValues returns [Expr* result]
    :   ( child=numericInterval
        | child=valueSet
        )
        {
            result = child;
        }
    ;
     
variableDeclarations returns [ExprList* result]
@init {
    result = new ExprList();
    CTX->SymbolTable->pushToCleanupStack(result);
}
    :   ^(VARIABLE
            dataType=type 
            (child=variableInitialization[dataType->getId()]
            {
                result->addChild(child);
                CTX->SymbolTable->popFromCleanupStack();
            }
            )+
        )
    {
        
    }
    ;
        
variableInitialization[const DataTypeId& dataType] returns [Expr* result]
@init {
   const char* varName; 
}
    :   (   name=IDENT { varName = c_str($name.text->chars); }
        |   ^('=' name=IDENT  { varName = c_str($name.text->chars); } initExpr=initializer[varName])
        )
        {
            if(initExpr != NULL)
              CTX->SymbolTable->popFromCleanupStack();
            // TODO: type check initExpr;
            result = new ExprVarDeclaration(
                 varName,
                 dataType,
                 initExpr,
                 true // canBeSpecified
            ); 
            CTX->SymbolTable->addLocalVar(varName,dataType);
            CTX->SymbolTable->pushToCleanupStack(result);
        }
        ;       

initializer[const char* varName] returns [Expr* result]
    :   (   child=anyValue 
        |   child=allocation[varName]
        )
        {
            result = child;
        }
    ;

anyValue returns [Expr* result]
    :   (   child=literalValue
        |   child=baseDomainValues
        |   child=qualified
        )
        { 
            result = child;
        }
    ;

setElement returns [Expr* result]
    :   (   child=literalValue
        |   child=qualified
        )
        { 
            result = child;
        }
    ;

valueSet returns [Expr* result]
@init {
    std::list<edouble> values;
    DataTypeId elementType;
}
    :   ^('{'
            (element=setElement
            {
                DataRef elemValue = evalExpr(CTX,element);
                CTX->SymbolTable->popFromCleanupStack();
                delete element;
                const Domain& ev = elemValue.getValue()->lastDomain();
                edouble v = ev.getSingletonValue();
                              
                if (elementType.isNoId())
                    elementType = ev.getDataType();
                else {
                    DataTypeId newElementType=ev.getDataType();
                    if (!newElementType->isAssignableFrom(elementType) &&
                        !elementType->isAssignableFrom(newElementType)) {
                        reportSemanticError(CTX,
                            "Incompatible types in value set: "+
                            elementType->toString(values.front())+"("+elementType->getName()+") "+ 
                            newElementType->toString(v)+"("+newElementType->getName()+")" 
                        );
                    }
                }
                             
                values.push_back(v);
            }
            )*
        )
        {
            Domain* newDomain = new EnumeratedDomain(elementType,values); 
            result = new ExprConstant(
                elementType->getName().c_str(),
                newDomain                       
            );
            CTX->SymbolTable->pushToCleanupStack(result);
        }
    ;

literalValue returns [Expr* result]
    :   (   child=booleanLiteral
        |   child=numericLiteral
        |   child=stringLiteral
        )
        {
            result = new ExprConstant(
                child->getTypeName().c_str(),
                child
            );
            CTX->SymbolTable->pushToCleanupStack(result);
        }
    ;
    
booleanLiteral returns [Domain* result]
    :   ('true'  { result = new BoolDomain(true); }            
    |   'false' { result = new BoolDomain(false); } )
    ;

stringLiteral returns [Domain* result]
    :    str = STRING 
         { 
             // remove quotes
             std::string s(c_str($str.text->chars));
             s = s.substr(1,s.size()-2);
                 
             LabelStr value(s); 
             result = new StringDomain(value,StringDT::instance());
         }
    ; 

numericLiteral returns [Domain* result]
    :   floating=floatLiteral  { result = CTX->SymbolTable->makeNumericDomainFromLiteral("float",c_str($floating.text->chars)); }
    |   integer=intLiteral  { result = CTX->SymbolTable->makeNumericDomainFromLiteral("int",c_str($integer.text->chars)); }
    ;

floatLiteral 
    :   FLOAT | 'inff' | '-inff'
    ;        

intLiteral 
    :   INT | 'inf' | '-inf'
    ;        
        
numericInterval returns [Expr* result]
    :   ^('['   
            lower=numericLiteral
            upper=numericLiteral
        )
        {      
            edouble lb = lower->getSingletonValue();
            edouble ub = upper->getSingletonValue();
            Domain* baseDomain;
                    
            if (lower->getTypeName()=="float" || 
                upper->getTypeName()=="float") 
                baseDomain = new IntervalDomain(lb,ub);
            else 
                baseDomain = new IntervalIntDomain((eint)lb,(eint)ub);
                                  
            result = new ExprConstant(
                        lower->getTypeName().c_str(),
                        baseDomain
                    );
            CTX->SymbolTable->pushToCleanupStack(result);     
            delete lower;
            delete upper;
        }
    ;

allocation[const char* name] returns [Expr* result]
@init {
    std::vector<Expr*> args;
}
        :       ^(CONSTRUCTOR_INVOCATION
                        objType=IDENT 
                        variableArgumentList[args]?
                )
                {
                    std::string objName = (name != NULL ? name : getAutoLabel("__Object"));
                    /*try {
                        CTX->SymbolTable->checkObjectFactory(c_str($objType.text->chars),args);
                    }
                    catch (const std::string& errorMsg) {
                        reportSemanticError(CTX,errorMsg);
                    }*/
                    CTX->SymbolTable->popFromCleanupStack(args.size());
                    result = new ExprNewObject(
                        c_str($objType.text->chars), // objectType
                        objName.c_str(),
                        args
                    );
                    args.clear();
                    CTX->SymbolTable->pushToCleanupStack(result);
                }
        ;

variableArgumentList[std::vector<Expr*>& result]
        :       '('
        |       ^('('
                        (arg=initializer[NULL] {checkError(arg != NULL); result.push_back(arg);})*
                )
        ;

identifier 
        : IDENT
        | 'this'
        ;

constraintInstantiation returns [ExprConstraint* result]
@init {
    std::vector<Expr*> args;
    std::string vmsg;
}
        :       
              ^(CONSTRAINT_INSTANTIATION
                        name=IDENT
                        variableArgumentList[args]
                        (violationMsg[vmsg])?
                )
                {
                    CTX->SymbolTable->popFromCleanupStack(args.size());
                    const char* cname = c_str($name.text->chars);
                    try {
                        CTX->SymbolTable->checkConstraint(cname,args);
                    }
                    catch (const std::string& errorMsg) {
                        cleanup(args.begin(), args.end());
                        reportSemanticError(CTX,errorMsg);
                    }
                    
                    result = new ExprConstraint(cname,args,vmsg.c_str());
                    CTX->SymbolTable->pushToCleanupStack(result);
                    args.clear();
                }
                
        ;

violationMsg [std::string& result]
    :    str = STRING 
         { 
             // remove quotes
             std::string s(c_str($str.text->chars));
             result = s.substr(1,s.size()-2);
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
		       ObjectTypeId parent = CTX->SymbolTable->getObjectType(parentClass);
		       if (parent.isNoId())
		          reportSemanticError(CTX,"class "+std::string(parentClass)+" is undefined");
		           
               objType = new ObjectType(newClass,parent);
               // TODO: do this more cleanly. Needed to deal with self-reference inside class definition
               CTX->SymbolTable->getPlanDatabase()->getSchema()->declareObjectType(newClass);
               pushContext(CTX,new NddlClassSymbolTable(CTX->SymbolTable,objType));                       
           }
                   
           (
		       classBlock[objType] 
		       {
                   dynamic_cast<NddlClassSymbolTable*>(CTX->SymbolTable)->saveObjectType();
		           result = new ExprObjectTypeDefinition(objType->getId()); 
		       } 
		       | ';' 
		         { 
		             result = new ExprObjectTypeDeclaration(objType->getName());
		             //delete objType;
		         }
	       )
	       {
               checkError(CTX->SymbolTable->cleanupStackSize() == 0,
                          CTX->SymbolTable->cleanupStackSize());
               popContext(CTX);            
               CTX->SymbolTable->pushToCleanupStack(result);
	       }
		)
	;

classBlock[ObjectType* objType]
	:	'{'
	|	^('{' componentTypeEntry[objType]* )
	;

componentTypeEntry[ObjectType* objType]
	:	classVariable[objType]
	|	constructor[objType]
	|	tokenType[objType]
	;

classVariable[ObjectType* objType]
        :       ^(VARIABLE 
                  dataType=type 
                  (name=IDENT
                  {
                      objType->addMember(dataType->getId(),c_str($name.text->chars)); 
                  }
                  )+
                 )
        ;
        
constructor[ObjectType* objType]
@init {
    std::vector<std::string> argNames;
    std::vector<std::string> argTypes;
    std::vector<Expr*> body;
    pushContext(CTX,new NddlSymbolTable(CTX->SymbolTable));                       
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
            if(superCallExpr != NULL)
                CTX->SymbolTable->popFromCleanupStack();
            CTX->SymbolTable->popFromCleanupStack(body.size());
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
            

            checkError(CTX->SymbolTable->cleanupStackSize() == 0,
                       CTX->SymbolTable->cleanupStackSize());
            popContext(CTX);
		}
	;

constructorArgument[std::vector<std::string>& argNames,std::vector<std::string>& argTypes]
	:	^(VARIABLE
			argName=IDENT
			argType=type
		)
		{
		    const char* varName = c_str($argName.text->chars);
		    argNames.push_back(varName);
		    argTypes.push_back(argType->getName());
		    CTX->SymbolTable->addLocalVar(varName,argType->getId());
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
            CTX->SymbolTable->popFromCleanupStack(args.size());
		    result = new ExprConstructorSuperCall(objType->getParent()->getName(),args);
            CTX->SymbolTable->pushToCleanupStack(result);
            args.clear();
            /*try {
                CTX->SymbolTable->checkObjectFactory(objType->getParent()->getName().c_str(),args);
            }
            catch (const std::string& errorMsg) {
                reportSemanticError(CTX,errorMsg);
            }*/
		}
	;
  
assignment returns [ExprAssignment* result]
	:	^('='
			lhs=qualified
			rhs=initializer[lhs->toString().c_str()]
		)
		{
            CTX->SymbolTable->popFromCleanupStack();
            CTX->SymbolTable->popFromCleanupStack();
		    result = new ExprAssignment(lhs,rhs);
            CTX->SymbolTable->pushToCleanupStack(result);
		}
	;

tokenType[ObjectType* objType]
@init {
    InterpretedTokenType* tokenType;
    std::string tokenTypeName;
}
	:	^(kind=('predicate' | 'action')
			ttName=IDENT 
			{ 
			    tokenTypeName = objType->getName() + "." + c_str($ttName.text->chars);
			    tokenType = new InterpretedTokenType(objType->getId(),tokenTypeName,c_str($kind.text->chars)); //NOTE: This can leak if one of the tokenStatements throws
			    pushContext(CTX,new NddlTokenSymbolTable(CTX->SymbolTable,tokenType->getId(),objType->getId())); 
			}
			tokenStatements[tokenType]
		)
		{
            objType->addTokenType(tokenType->getId());
            checkError(CTX->SymbolTable->cleanupStackSize() == 0,
                       CTX->SymbolTable->cleanupStackSize());
            popContext(CTX);
		}
	;

// TODO: allow assignments to inherited parameters
tokenStatements[InterpretedTokenType* tokenType]
	:	^('{'
            (
                ( child=tokenParameter[tokenType] 
                | child=tokenParameterAssignment
                | child=standardConstraint 
                | child=enforceExpression
                )
                {
                    tokenType->addBodyExpr(child);
                    CTX->SymbolTable->popFromCleanupStack();
                }
		    )*
		)
	;
	
// Note: Allocations are not legal here.        
tokenParameter[InterpretedTokenType* tokenType] returns [Expr* result]
        :
        child=variableDeclarations 
        { 
            const std::vector<Expr*>& vars=child->getChildren();
            for (unsigned int i=0;i<vars.size();i++) {
                ExprVarDeclaration* vd = boost::polymorphic_cast<ExprVarDeclaration*>(vars[i]);
                tokenType->addArg(vd->getDataType(),vd->getName());
            }
            result = child;
            CTX->SymbolTable->popFromCleanupStack();
            CTX->SymbolTable->pushToCleanupStack(result);
        }
        ;       

tokenParameterAssignment returns [Expr* result]
        :
        child=assignment 
        { 
            result = child; 
        }
	;       

standardConstraint returns [Expr* result]
        :
        child = constraintInstantiation 
        { 
            result = child; 
        }
        ;	

rule returns [Expr* result]
@init {
    std::vector<Expr*> ruleBody;
    std::string predName;
    InterpretedTokenType* iTokenType=NULL;
}
 	:	^('::'
			className=IDENT
			predicateName=IDENT
			{
		        std::string clazz = c_str($className.text->chars);
			    ObjectTypeId ot = CTX->SymbolTable->getObjectType(clazz.c_str());
			    if (ot.isNoId())
			        reportSemanticError(CTX,"class "+clazz+" has not been declared");
			        
                predName = clazz + "." + std::string(c_str($predicateName.text->chars));
                debugMsg("NddlInterpreter","Parsing rule for:" << predName);
                TokenTypeId tt = CTX->SymbolTable->getTokenType(predName.c_str());
                if (tt.isNoId())
                    reportSemanticError(CTX,predName+" has not been declared");            
                
                iTokenType = dynamic_cast<InterpretedTokenType*>((EUROPA::TokenType*)tt);
                
                pushContext(CTX,new NddlTokenSymbolTable(CTX->SymbolTable,tt,ot));    
			}
			ruleBlock[ruleBody]
		)
		{
            char buff[10];
            sprintf(buff, "\%u", className->getLine(className));
            std::string filename = "UNKNOWN";
            if (className->getToken(className)) {
                if (className->getToken(className)->input) {
                    if (className->getToken(className)->input->fileName) {
                        filename = c_str(className->getToken(className)->input->fileName->chars);
                    }
                }
            }
		    std::string source="\"" + filename + "," + std::string(buff) + "\"";
		    InterpretedRuleFactory* rf = new InterpretedRuleFactory(predName,source,ruleBody);
            CTX->SymbolTable->popFromCleanupStack(ruleBody.size());
		    if (iTokenType != NULL) 
		        iTokenType->addRule(rf);
		    
            checkError(CTX->SymbolTable->cleanupStackSize() == 0,
                       CTX->SymbolTable->cleanupStackSize());
		    popContext(CTX);
		    result = new ExprRuleTypeDefinition(rf->getId());
            CTX->SymbolTable->pushToCleanupStack(result);
		}
	;

ruleBlock[std::vector<Expr*>& ruleBody]
	:	^('{'
			(child=ruleStatement { ruleBody.push_back(child); })*
		)
	;

ruleStatement returns [Expr* result]
	: (	child=constraintInstantiation
      | child=enforceExpression
	  |	child=assignment
	  | child=relation
	  |	child=ifStatement
	  |	child=loopStatement
	  |	child=variableDeclarations
      | child=dummyRuleForExprType
	  )
	  {
	      result = child;
	  }
	;

dummyRuleForExprType returns [Expr* result] : ('CAN\'T MATCH') ;

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
            //The + 1 covers the guard expression
            CTX->SymbolTable->popFromCleanupStack(ifBody.size() + elseBody.size() + 1);
		    result = new ExprIf(guard,ifBody,elseBody);
            CTX->SymbolTable->pushToCleanupStack(result);
		}
  ;

// TODO: perform systematic cleanup of lhs, rhs and constant exprs throughout
// TODO: this doesn't seem to be correct
guardExpression returns [ExprIfGuard* result]
@init
{
    const char* relopStr = "==";
    BoolDomain *dom = NULL;
}
        : ( ^(relop=guardRelop {relopStr=c_str($relop.text->chars);} lhs=anyValue rhs=anyValue )
          | lhs=anyValue
          | ^(EXPRESSION_RETURN lhs=expressionCleanReturn
                { dom = new BoolDomain(true); 
                  rhs = new ExprConstant(dom->getTypeName().c_str(), dom);
                  CTX->SymbolTable->pushToCleanupStack(rhs);
                } )
          )
          {
              result = new ExprIfGuard(relopStr,lhs,rhs);
              CTX->SymbolTable->popFromCleanupStack(2);
              CTX->SymbolTable->pushToCleanupStack(result);
          }
        ;

guardRelop 
    : '==' | '!='
;

loopStatement returns [Expr* result]
@init {
    std::vector<Expr*> loopBody;
    const char* loopVarName = NULL;
}
	:	^('foreach'
			name=IDENT
			val=qualified 
			{
               loopVarName =  c_str($name.text->chars);
               CTX->SymbolTable->addLocalVar(loopVarName,val->getDataType());
			} 
			ruleBlock[loopBody]
		)
		{
		    // TODO : modify ExprLoop to take val Expr instead, otherwise delete val.
		    result = new ExprLoop(loopVarName,val->toString().c_str(),loopBody); 
            CTX->SymbolTable->popFromCleanupStack(loopBody.size() + 1); //the +1 accounts for the val above
            CTX->SymbolTable->pushToCleanupStack(result);
		    delete val;
		}
	;

problemStmt returns [Expr* result] 
@init {
    std::vector<PredicateInstanceRef*> tokens;    
}

        :       ^(t=problemStmtType predicateInstanceList[tokens])
                {
                    result = new ExprProblemStmt(c_str($t.text->chars),tokens);
                    CTX->SymbolTable->pushToCleanupStack(result);
                }   
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
			(pvr=predicateVarRef { source = pvr; })?
			tr=temporalRelation { relationType = c_str($tr.text->chars); } 
			predicateInstanceList[targets]
		)
		{		   
		    result = new ExprRelation(relationType,source,targets);
            CTX->SymbolTable->pushToCleanupStack(result);
		}
	;

predicateInstanceList[std::vector<PredicateInstanceRef*>& instances]
	:	^('('
			(child=predicateInstance { instances.push_back(child); })*
		)
		|	pvr=predicateVarRef 
		        { instances.push_back(pvr); } 
	;

predicateInstance returns [PredicateInstanceRef* pi]
@init {
    const char* name = NULL;
    const char* annotation = NULL;
    std::string tokenStr;
    TokenTypeId tokenType;
}
	:	^(PREDICATE_INSTANCE 
				qualifiedToken[tokenStr,tokenType] 
				(i=IDENT { name = c_str($i.text->chars); })?
				(ann=tokenAnnotation { annotation = c_str($ann.text->chars); })?
		)
	        {
	            pi = new PredicateInstanceRef(tokenType,tokenStr.c_str(),(name == NULL ? "" : name),(annotation == NULL ? "" : annotation));
	            if (name != NULL)
	                CTX->SymbolTable->addLocalToken(name,tokenType);
	        }
	;

predicateVarRef returns [PredicateInstanceRef* pi]
@init {
    const char* varName = NULL;
    TokenTypeId tokenType;
}
	:	i=identifier
		{
			varName = c_str($i.text->chars);
            tokenType = CTX->SymbolTable->getTypeForToken(varName);  
            if (tokenType.isNoId()) 
               reportSemanticError(CTX,std::string(varName)+" is an undefined token");

            pi = new PredicateInstanceRef(tokenType,"",varName,"");
		}
	;

tokenAnnotation
	:	'condition' | 'effect'
	;
	
qualifiedString[std::string& result]
    :
    (    name=identifier { result+=c_str($name.text->chars); }                 
    |    ^('.' prefix=qualifiedString[result] 
               (q=IDENT { result += std::string(".") + c_str($q.text->chars); })+)
           )
    ;
    	
qualified returns [Expr* result]
@init {
    std::string varName;
}
    :   qualifiedString[varName]
        {
                bool isEnum = false;
  
                //My guess is that there is a better way to do this. First check if it is an enum.
                std::string secondPart = "";
                if (varName.rfind(".") != std::string::npos) {
                   secondPart = varName.substr(varName.rfind(".") + 1, std::string::npos);
                }

                if (CTX->SymbolTable->isEnumValue(varName.c_str())) {
                   result = CTX->SymbolTable->makeEnumRef(varName.c_str());
                   CTX->SymbolTable->pushToCleanupStack(result);
                   isEnum = true;
                } else if (secondPart != "") {
                   if (CTX->SymbolTable->isEnumValue(secondPart.c_str())) {
                      result = CTX->SymbolTable->makeEnumRef(secondPart.c_str());
                      CTX->SymbolTable->pushToCleanupStack(result);
                      isEnum = true;
                   }
                }
  
  				// TODO: should check for classes last, since this won't be common
		        ObjectTypeId ot = CTX->SymbolTable->getObjectType(varName.c_str()); // Hack!
        		if (ot.isId())  { // is a class reference
            		LabelStr value(varName);
            		result = new ExprConstant("string",new StringDomain((edouble)value,StringDT::instance()));
                    CTX->SymbolTable->pushToCleanupStack(result);
        		}        		
                else if (!isEnum) {
                	std::string errorMsg;
                	DataTypeId dt = CTX->SymbolTable->getTypeForVar(varName.c_str(),errorMsg);  
                	// TODO!!: do type checking at each "."
                	if (dt.isNoId())
                    	reportSemanticError(CTX,errorMsg);
                   	result = new ExprVarRef(varName,dt);
                    CTX->SymbolTable->pushToCleanupStack(result);
                }
        }
    ;
        
qualifiedToken[std::string& tokenStr, TokenTypeId& tokenType]
    :   qualifiedString[tokenStr]  
        {
            std::string errorMsg;
            tokenType = CTX->SymbolTable->getTypeForToken(tokenStr.c_str(),errorMsg);  
            if (tokenType.isNoId())
               reportSemanticError(CTX,errorMsg);
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
@init {
    std::vector<Expr*> args;
}
	:	^(METHOD_CALL v=qualified op=methodName variableArgumentList[args]?)
    {
        std::string mName = c_str($op.text->chars);

        // Hack! data type for v should tell us whether we're dealing with a class
        ExprConstant* classNameExpr = dynamic_cast<ExprConstant*>(v);
        if (classNameExpr != NULL) {
            std::string className = classNameExpr->getConstantValue();
            debugMsg("NddlInterpreter:method","Trying class:"+className);
            ObjectTypeId ot = CTX->SymbolTable->getObjectType(className.c_str()); 
            if (ot.isId())  { // is a class method
                LabelStr value(mName);
                mName = "class__"+mName;    
            }
        }
            
        MethodId method = CTX->SymbolTable->getMethod(mName.c_str(),v,args);
        if (method.isNoId()) {
            delete v;
            cleanup(args.begin(), args.end());
            args.clear();
            reportSemanticError(CTX,"Method "+mName+" is not defined");
        }   
        result = new ExprMethodCall(method,v,args);
        //the +1 accounts for the "qualified" v
        CTX->SymbolTable->popFromCleanupStack(args.size() + 1); 
        CTX->SymbolTable->pushToCleanupStack(result);
        args.clear();
    }
    |	^(CLOSE CLOSE)
    {
        // TODO: hack!
        Expr* varExpr = NULL;
        MethodId method = CTX->SymbolTable->getMethod("pdb_close",varExpr,args);
        result = new ExprMethodCall(method,varExpr,args);
        CTX->SymbolTable->popFromCleanupStack(args.size());
        CTX->SymbolTable->pushToCleanupStack(result);
        args.clear();
    }
	;

methodName
	:	IDENT
	|	'close'
	;

cexpressionList [std::vector<CExpr*> &args]
	:  (expr=cexpression { checkError(expr != NULL); args.push_back(expr); })* 
	;

cexpression returns [CExpr* result]
@init { 
    std::vector<CExpr*> args; 
}
	:	^(cop=cexprOp leftValue=cexpression rightValue=cexpression)
        {
           std::string op = c_str($cop.text->chars);
           result = new CExprBinary(op, leftValue, rightValue);
           CTX->SymbolTable->popFromCleanupStack(2);
           CTX->SymbolTable->pushToCleanupStack(result);
        }
	|	r=anyValue
        {
           result = new CExprValue(r);
           CTX->SymbolTable->popFromCleanupStack();
           CTX->SymbolTable->pushToCleanupStack(result);
        }
	|	^(FUNCTION_CALL name=IDENT ^('(' cexpressionList[args]))
        {
            std::string fName = c_str($name.text->chars);
            CFunctionId func = CTX->SymbolTable->getCFunction(fName.c_str(),args);
            
            if (func.isNoId())  {
                cleanup(args.begin(), args.end());
                args.clear();
                reportSemanticError(CTX, "CFunction does not exist: " + fName);
            }
            try {
                CTX->SymbolTable->checkCFunction(fName, args);
            }
            catch(const std::string& errorMsg) {
                cleanup(args.begin(), args.end());
                args.clear();
                reportSemanticError(CTX, errorMsg);
            }
            result = new CExprFunction(func, args);
            CTX->SymbolTable->popFromCleanupStack(args.size());
            CTX->SymbolTable->pushToCleanupStack(result);
            args.clear();
        }
	;

cexprOp
	:	'=='
	|	'!='
	|	'<'
	|	'<='
	|	'>'
	|	'>='
	|	'||'
	|	'&&'
	|	'+'
	|	'-'
	|	'*'
;
	
enforceExpression returns [Expr* result]
@init { 
	BoolDomain* dom = NULL; 
	ExprConstant* con = NULL; 
	std::vector<Expr*> args; 
	std::string vmsg;
}
	: ^(EXPRESSION_ENFORCE value=cexpression violationMsg[vmsg]?) 
	  { 
            value->setEnforceContext();
            value->setViolationMsg(vmsg);
            
            /*try {
                value->checkType();
            } catch(std::string msg) {
                reportSemanticError(CTX, msg);
            }*/
            
            if (value->hasReturnValue()) {
                dom = new BoolDomain(true); 
                con = new ExprConstant(dom->getTypeName().c_str(), dom); 
                args.push_back(value);
                args.push_back(con);    
                result = new ExprConstraint("eq", args, "");
                CTX->SymbolTable->popFromCleanupStack();
                CTX->SymbolTable->pushToCleanupStack(result);
                args.clear();
            } else {
                result = value;
                //No change to cleanup stack--value is already on the stack
                //CTX->SymbolTable->pushToCleanupStack(result);
            }
        } ;

// TODO: this needs more behavior??
expressionCleanReturn returns [Expr* result]
	: e = cexpression { result = e; }
	;
