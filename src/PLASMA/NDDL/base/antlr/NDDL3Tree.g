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
#include "Domains.hh"
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

nddl :               
	^(NDDL
		( {
		     // debugMsg("NddlInterpreter:nddl","Line:" << LEXER->getLine(LEXER)); 
		  }
		  (	child=classDeclaration
          | child=enumDefinition
		  |	child=typeDefinition
		  |	child=variableDeclarations
		  |	child=assignment
		  |	child=constraintInstantiation
		  |	child=allocation[NULL]
		  |	child=rule
		  |	child=problemStmt
		  |	child=relation
		  |	child=methodInvocation
		  |	child=constraintSignature
		  ) 
		  {
		      if (child != NULL) { 
		          debugMsg("NddlInterpreter:nddl","Evaluating:" << child->toString());
		          evalExpr(CTX,child);
		          // TODO!!: systematically deal with memory mgmt for all Exprs.
		          delete child;
		          child = NULL; 
		          
                  CTX->SymbolTable->getPlanDatabase()->getConstraintEngine()->propagate();
		      }
              CTX->SymbolTable->getPlanDatabase()->getConstraintEngine()->propagate();
              
		  }
		)*  
		
	)
	;

enumDefinition returns [Expr* result]
@init {
    std::vector<std::string> values;
}
    :   ^('enum' name=IDENT enumValues[values])
        {
            const char* enumName = c_str($name.text->chars);
            CTX->SymbolTable->addEnumValues(enumName,values);
            result = new ExprEnumdef(enumName,values);
        }
    ;

enumValues[std::vector<std::string>& values]
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
        
baseDomain[const DataType* baseType] returns [AbstractDomain* result]
    :   child=baseDomainValues
    {
        DataRef data=evalExpr(CTX,child); 
        ConstrainedVariableId value = data.getValue();
        if (!baseType->isAssignableFrom(value->getDataType())) {
            reportSemanticError(CTX,
                "Can't assign "+value->toString()+" to "+baseType->getName().toString());
        }
       
        result = value->lastDomain().copy(); 
        // TODO: delete child;?
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
}
    :   ^(VARIABLE 
            dataType=type 
            (child=variableInitialization[dataType->getId()]
            {
                result->addChild(child);
            }
            )+
        )
    ;
        
variableInitialization[const DataTypeId& dataType] returns [Expr* result]
@init {
   const char* varName; 
}
    :   (   name=IDENT { varName = c_str($name.text->chars); }
        |   ^('=' name=IDENT  { varName = c_str($name.text->chars); } initExpr=initializer[varName])
        )
        {
            // TODO: type check initExpr;
            result = new ExprVarDeclaration(
                 varName,
                 dataType,
                 initExpr,
                 true // canBeSpecified
            ); 
            CTX->SymbolTable->addLocalVar(varName,dataType);
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
    std::list<double> values;
    DataTypeId elementType;
}
    :   ^('{'
            (element=setElement
            {
                DataRef elemValue = evalExpr(CTX,element);
                const AbstractDomain& ev = elemValue.getValue()->lastDomain();
                double v = ev.getSingletonValue();
                              
                // TODO: delete element;
                             
                if (elementType.isNoId())
                    elementType = ev.getDataType();
                else {
                    DataTypeId newElementType=ev.getDataType();
                    if (!newElementType->isAssignableFrom(elementType) &&
                        !elementType->isAssignableFrom(newElementType)) {
                        reportSemanticError(CTX,
                            "Incompatible types in value set: "+
                            elementType->toString(values.front())+"("+elementType->getName().toString()+") "+ 
                            newElementType->toString(v)+"("+newElementType->getName().toString()+")" 
                        );
                    }
                }
                             
                values.push_back(v);
            }
            )*
        )
        {
            AbstractDomain* newDomain = new EnumeratedDomain(elementType,values); 
            result = new ExprConstant(
                elementType->getName().c_str(),
                newDomain                       
            );
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
        }
    ;
    
booleanLiteral returns [AbstractDomain* result]
    :   'true'  { result = new BoolDomain(true); }            
    |   'false' { result = new BoolDomain(false); }
    ;

stringLiteral returns [AbstractDomain* result]
    :    str = STRING 
         { 
             // remove quotes
             std::string s(c_str($str.text->chars));
             s = s.substr(1,s.size()-2);
                 
             LabelStr value(s); 
             result = new StringDomain((double)value,StringDT::instance());
         }
    ; 

numericLiteral returns [AbstractDomain* result]
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
            double lb = lower->getSingletonValue();
            double ub = upper->getSingletonValue();
            AbstractDomain* baseDomain;
                    
            if (lower->getTypeName().toString()=="float" || 
                upper->getTypeName().toString()=="float") 
                baseDomain = new IntervalDomain(lb,ub);
            else 
                baseDomain = new IntervalIntDomain((int)lb,(int)ub);
                                  
            result = new ExprConstant(
                        lower->getTypeName().c_str(),
                        baseDomain
                    );
                    
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
                    result = new ExprNewObject(
                        c_str($objType.text->chars), // objectType
                        objName.c_str(),
                        args
                    );
                }
        ;

variableArgumentList[std::vector<Expr*>& result]
        :       '('
        |       ^('('
                        (arg=initializer[NULL] {result.push_back(arg);})*
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
		           result = new ExprObjectTypeDefinition(objType->getId()); 
		       } 
		       | ';' 
		         { 
		             result = new ExprObjectTypeDeclaration(objType->getName());
		             delete objType; 
		         }
	       )
	       {
               popContext(CTX);            
	       }
		)
  ;

classBlock[ObjectType* objType]
	:	'{'
	        | ^('{'
			componentTypeEntry[objType]*
		)
	;

componentTypeEntry[ObjectType* objType]
	:	classVariable[objType]
	|	constructor[objType]
	|	predicate[objType]
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
		    argTypes.push_back(argType->getName().toString());
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
		    result = new ExprConstructorSuperCall(objType->getParent()->getName(),args);  
		}
	;
  
assignment returns [ExprAssignment* result]
	:	^('='
			lhs=qualified
			rhs=initializer[lhs->toString().c_str()]
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
			    tokenFactory = new InterpretedTokenFactory(objType->getId(),predName);
			    pushContext(CTX,new NddlTokenSymbolTable(CTX->SymbolTable,tokenFactory->getId(),objType->getId())); 
			}
			predicateStatements[tokenFactory]
		)
		{
            objType->addTokenFactory(tokenFactory->getId());
            popContext(CTX);
		}
	;

// TODO: allow assignments to inherited parameters
predicateStatements[InterpretedTokenFactory* tokenFactory]
	:	^('{'
            (
                ( child=predicateParameter[tokenFactory] 
                | child=predicateParameterAssignment
                | child=standardConstraint 
                )
                {
                    tokenFactory->addBodyExpr(child);
                }
		    )*
		)
	;
	
// Note: Allocations are not legal here.        
predicateParameter[InterpretedTokenFactory* tokenFactory] returns [Expr* result]
        :
        child=variableDeclarations 
        { 
            const std::vector<Expr*>& vars=child->getChildren();
            for (unsigned int i=0;i<vars.size();i++) {
                ExprVarDeclaration* vd = dynamic_cast<ExprVarDeclaration*>(vars[i]);
                tokenFactory->addArg(vd->getDataType(),vd->getName());
            }
            result = child;            
        }
        ;       

predicateParameterAssignment returns [Expr* result]
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
                TokenFactoryId tt = CTX->SymbolTable->getTokenType(predName.c_str());
                if (tt.isNoId())
                    reportSemanticError(CTX,predName+" has not been declared");            
                
                pushContext(CTX,new NddlTokenSymbolTable(CTX->SymbolTable,tt,ot));    
			}
			ruleBlock[ruleBody]
		)
		{
		    std::string source=""; // TODO: get this from the antlr parser
		    result = new ExprRuleTypeDefinition((new InterpretedRuleFactory(predName,source,ruleBody))->getId());
		    popContext(CTX);
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
	  |	child=variableDeclarations
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

// TODO: perform systematic cleanup of lhs, rhs and constant exprs throughout
guardExpression returns [ExprIfGuard* result]
@init
{
    const char* relopStr = "==";
    BoolDomain *dom = NULL;
}
        : ( ^(relop=guardRelop {relopStr=c_str($relop.text->chars);} lhs=anyValue rhs=anyValue )
          | lhs=anyValue
          | ^('test' lhs=anyValue
                { dom = new BoolDomain(true); 
                  rhs = new ExprConstant(dom->getTypeName().c_str(), dom); 
                } )
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
		    // TODO : modify ExprLoop to pass val Expr instead, otherwise delete val.
		    result = new ExprLoop(loopVarName,val->toString().c_str(),loopBody); 
		}
	;

problemStmt returns [Expr* result] 
@init {
    std::vector<PredicateInstanceRef*> tokens;    
}

        :       ^(t=problemStmtType predicateInstanceList[tokens])
                {
                    result = new ExprProblemStmt(c_str($t.text->chars),tokens);
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
		        { instances.push_back(new PredicateInstanceRef(NULL,c_str($i.text->chars))); } 
	;

predicateInstance returns [PredicateInstanceRef* pi]
@init {
    const char* name = NULL;
    std::string tokenStr;
    TokenFactoryId tokenType;
}
	:	^(PREDICATE_INSTANCE qualifiedToken[tokenStr,tokenType] (i=IDENT { name = c_str($i.text->chars); })?)
	        {
	            pi = new PredicateInstanceRef(tokenStr.c_str(),name);
	            if (name != NULL)
	                CTX->SymbolTable->addLocalToken(name,tokenType);
	        }
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
                isEnum = true;
             } else if (secondPart.c_str() != "") {
                if (CTX->SymbolTable->isEnumValue(secondPart.c_str())) {
                   result = CTX->SymbolTable->makeEnumRef(secondPart.c_str());
                   isEnum = true;
                }
             }

             if (!isEnum) {
                std::string errorMsg;
                DataTypeId dt = CTX->SymbolTable->getTypeForVar(varName.c_str(),errorMsg);  
                // TODO!!: do type checking at each "."
                if (dt.isNoId())
                    reportSemanticError(CTX,errorMsg);
                result = new ExprVarRef(varName.c_str(),dt);
             }
        }
    ;
        
qualifiedToken[std::string& tokenStr, TokenFactoryId& tokenType]
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
 
// TODO: this is ugly, need to provide extensible method exporting mechanism  
methodInvocation returns [Expr* result]
	:
	(  child=variableMethod
	|  child=objectMethod
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
        :       ^(METHOD_CALL op=variableOp v=qualified? variableArgumentList[args]?)
                {
                    result = new ExprVariableMethod(c_str($op.text->chars),v,args);
                }
        ;
   
variableOp
        :       'specify'
        |       'reset'
        |       'close'
        ;
                
objectMethod returns [Expr* result]
@init {
    std::vector<Expr*> args;
}
        :       ^(METHOD_CALL op=objectOp obj=qualified? variableArgumentList[args]?)
                {
                    result = new ExprObjectMethod(c_str($op.text->chars),obj,args);
                }
        ;
   
objectOp
        :       'constrain'
        |       'free'
        ;
                
tokenMethod returns [Expr* result]    
@init {
    std::vector<Expr*> args;
}
        :       ^(METHOD_CALL op=tokenOp tok=IDENT variableArgumentList[args]?)
                {
                    result = new ExprTokenMethod(c_str($op.text->chars),c_str($tok.text->chars),args); 
                }
        ;

tokenOp
        :       'activate'
        |       'merge'
        |       'reject'
        |       'cancel'
        ;        
        
// This is here only for backwards compatibility, must be dropped eventually        
constraintSignature returns [Expr* result]
        :       ^('constraint' name=IDENT typeArgumentList constraintExtendsClause? signatureBlock?)
        {
            debugMsg("NddlInterpreter","Ignored constraint declaration for " << c_str($name.text->chars) 
                                        << ". Constraint Signatures are ignored in nddl3.");
            result = NULL;
        }
        ;
        
constraintExtendsClause
        :       ^('extends' IDENT typeArgumentList)
        ;        
        
typeArgumentList
        :       ^('(' IDENT*)
        ;

signatureBlock
        :       ^('{' signatureExpression*)
        ;

signatureExpression
        :       ^(('&&' | '||') signatureAtom signatureAtom)
        ;

signatureAtom
        :       ^('<:' IDENT  (type | 'numeric'))
        |       ^('(' signatureExpression)
        ;
