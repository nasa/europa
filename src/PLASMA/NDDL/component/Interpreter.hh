#ifndef H_Interpreter
#define H_Interpreter

#include <vector>

#include "PDBInterpreter.hh"
#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"
#include "Object.hh"
#include "ObjectType.hh"
#include "CFunction.hh"
#include "Method.hh"
#include "IntervalToken.hh"
#include "TokenType.hh"
#include "Rule.hh"
#include "RuleInstance.hh"

#include <boost/cast.hpp>

namespace EUROPA {

class ExprConstant : public Expr {
 public:
  ExprConstant(const std::string& type, const Domain* d);
  virtual ~ExprConstant();

  virtual DataRef eval(EvalContext& context) const;
  virtual const DataTypeId getDataType() const;
  virtual std::string toString() const;
  std::string getConstantValue() const;

 protected:
  std::string m_type;
  const Domain* m_domain;
private:
  ExprConstant(const ExprConstant&);
  ExprConstant& operator=(const ExprConstant&);
};

  class TokenEvalContext;
  class RuleInstanceEvalContext;

class ExprVarDeclaration : public Expr {
 private:
  ExprVarDeclaration(const ExprVarDeclaration&);
  ExprVarDeclaration& operator=(const ExprVarDeclaration&);
 public:
  ExprVarDeclaration(const std::string& name, const DataTypeId type, Expr* initValue, bool canBeSpecified);
  virtual ~ExprVarDeclaration();

  virtual DataRef eval(EvalContext& context) const;
  virtual const DataTypeId getDataType() const;
  virtual std::string toString() const;

  const std::string& getName() const;
  const Expr* getInitValue() const;
  void setInitValue(Expr* iv);

 protected:
  std::string m_name;
  DataTypeId m_type;
  Expr* m_initValue;
  bool m_canBeSpecified;

  ConstrainedVariableId makeGlobalVar(EvalContext& context) const;
  ConstrainedVariableId makeTokenVar(TokenEvalContext& context) const;
  ConstrainedVariableId makeRuleVar(RuleInstanceEvalContext& context) const;
};

class ExprVarRef : public Expr {
 public:
  ExprVarRef(const std::string& name, const DataTypeId type);
  virtual ~ExprVarRef();

  virtual DataRef eval(EvalContext& context) const;
  virtual const DataTypeId getDataType() const;
  virtual std::string toString() const;

 protected:
  std::string m_varName;
  DataTypeId m_varType;
  std::string m_parentName;
  std::vector<std::string> m_vars;
};

class ExprAssignment : public Expr {
 private:
  ExprAssignment(const ExprAssignment&);
  ExprAssignment& operator=(const ExprAssignment&);
 public:
  ExprAssignment(Expr* lhs, Expr* rhs);
  virtual ~ExprAssignment();

  Expr* getLhs() { return m_lhs; }
  Expr* getRhs() { return m_rhs; }

  virtual DataRef eval(EvalContext& context) const;
  virtual std::string toString() const;

 protected:
  Expr* m_lhs;
  Expr* m_rhs;
};

  class ExprConstraint : public Expr
  {
    public:
        ExprConstraint(const std::string& name,const std::vector<Expr*>& args, const std::string& violationExpl);
        virtual ~ExprConstraint();

        virtual DataRef eval(EvalContext& context) const;

        const std::string getName() const { return m_name; }
        const std::vector<Expr*>& getArgs() const { return m_args; }
        virtual std::string toString() const;

    protected:
        std::string m_name;
        std::vector<Expr*> m_args;
        std::string m_violationExpl;
  };

class ExprTypedef : public Expr {
private:
  ExprTypedef(const ExprTypedef&);
  ExprTypedef& operator=(const ExprTypedef&);
 public:
  ExprTypedef(const DataTypeId baseType, const std::string& name, Domain* baseDomain);
  virtual ~ExprTypedef();

  virtual DataRef eval(EvalContext& context) const;
  virtual std::string toString() const;

 protected:
  DataTypeId m_baseType;
  std::string m_name;
  Domain* m_baseDomain;
};

  class ExprEnumdef : public Expr
  {
  public:
      ExprEnumdef(const std::string& name, const std::vector<LabelStr>& values);
      virtual ~ExprEnumdef();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      std::string m_name;
      std::vector<LabelStr> m_values;
  };

  class ExprObjectTypeDeclaration : public Expr
  {
  public:
      ExprObjectTypeDeclaration(const std::string& name);
      virtual ~ExprObjectTypeDeclaration();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      std::string m_name;
  };

  class ExprObjectTypeDefinition : public Expr
  {
  public:
      ExprObjectTypeDefinition(const ObjectTypeId objType);
      virtual ~ExprObjectTypeDefinition();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      mutable bool m_registered;
      const ObjectTypeId m_objType;
  };

  class ExprRuleTypeDefinition : public Expr
  {
  public:
      ExprRuleTypeDefinition(const RuleId rf);
      virtual ~ExprRuleTypeDefinition();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      const RuleId m_ruleFactory;
  };

class ExprMethodCall : public Expr {
private:
  ExprMethodCall(const ExprMethodCall&);
  ExprMethodCall& operator=(const ExprMethodCall&);
 public:
  ExprMethodCall(const MethodId method, Expr* varExpr, const std::vector<Expr*>& argExprs);
  virtual ~ExprMethodCall();

  virtual DataRef eval(EvalContext& context) const;
  virtual std::string toString() const;
  virtual const std::vector<Expr*>& getArgs() const {return m_argExprs;}
 protected:
  MethodId m_method;
  Expr* m_varExpr;
  std::vector<Expr*> m_argExprs;
};

class ExprVariableMethod : public Expr {
private:
  ExprVariableMethod(const ExprVariableMethod&);
  ExprVariableMethod& operator=(const ExprVariableMethod&);
 public:
  ExprVariableMethod(const std::string& name, Expr* varExpr, const std::vector<Expr*>& argExprs);
  virtual ~ExprVariableMethod();

  virtual DataRef eval(EvalContext& context) const;
  virtual std::string toString() const;

 protected:
  std::string m_methodName;
  Expr* m_varExpr;
  std::vector<Expr*> m_argExprs;

  DataRef eval(EvalContext& context, ConstrainedVariableId var, const std::vector<ConstrainedVariableId>& args) const;
};

class ExprObjectMethod : public Expr {
private:
  ExprObjectMethod(const ExprObjectMethod&);
  ExprObjectMethod& operator=(const ExprObjectMethod&);
 public:
  ExprObjectMethod(const std::string& name, Expr* objExpr, const std::vector<Expr*>& argExprs);
  virtual ~ExprObjectMethod();

  virtual DataRef eval(EvalContext& context) const;
  virtual std::string toString() const;

 protected:
  std::string m_methodName;
  Expr* m_objExpr;
  std::vector<Expr*> m_argExprs;

  DataRef eval(EvalContext& context, ObjectId var, const std::vector<ConstrainedVariableId>& args) const;
};

  class ExprTokenMethod : public Expr
  {
  public:
      ExprTokenMethod(const std::string& name, const std::string& tokenName, const std::vector<Expr*>& argExprs);
      virtual ~ExprTokenMethod();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      std::string m_methodName;
      std::string m_tokenName;
      std::vector<Expr*> m_argExprs;

      DataRef eval(EvalContext& context, TokenId tok, const std::vector<ConstrainedVariableId>& args) const;
  };

  class ExprNewObject : public Expr
  {
  	public:
  	    ExprNewObject(const std::string& objectType,
	                  const std::string& objectName,
	                  const std::vector<Expr*>& argExprs);

	    virtual ~ExprNewObject();

  	    virtual DataRef eval(EvalContext& context) const;

  	    virtual std::string toString() const;

  	protected:
	    std::string              m_objectType;
	    std::string              m_objectName;
	    std::vector<Expr*>    m_argExprs;
  };

  class InterpretedRuleInstance;

class PredicateInstanceRef {
 public:
  PredicateInstanceRef(const TokenTypeId tokenType, const std::string& predInstance,
                       const std::string& predName, const std::string& annotation);
  virtual ~PredicateInstanceRef();

  TokenId getToken(EvalContext& ctx, const std::string& relationName, bool isFact=false,
                   bool isRejectable=false);
  int     getAttributes() const;
  const TokenTypeId getTokenType();

 protected:
  TokenTypeId m_tokenType;
  std::string m_predicateInstance;
  std::string m_predicateName;
  int m_attributes;

  TokenId createSubgoal(EvalContext& ctx, InterpretedRuleInstance* rule, const std::string& relationName);
  TokenId createGlobalToken(EvalContext& context, bool isFact, bool isRejectable);
};

  class ExprProblemStmt : public Expr
  {
  public:
      ExprProblemStmt(const std::string& name, const std::vector<PredicateInstanceRef*>& tokens);
      virtual ~ExprProblemStmt();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      std::string m_name;
      std::vector<PredicateInstanceRef*> m_tokens;

      DataRef eval(EvalContext& context, TokenId tok, const std::vector<ConstrainedVariableId>& args) const;
  };

  class InterpretedTokenType;
class ExprRelation : public Expr {
private:
  ExprRelation(const ExprRelation&);
  ExprRelation& operator=(const ExprRelation&);
 public:
  ExprRelation(const std::string& relation,
               PredicateInstanceRef* origin,
               const std::vector<PredicateInstanceRef*>& targets);
  virtual ~ExprRelation();

  virtual DataRef eval(EvalContext& context) const;

  void populateCausality( InterpretedTokenType* container );

 protected:
  std::string m_relation;
  PredicateInstanceRef* m_origin;
  std::vector<PredicateInstanceRef*> m_targets;

};

  // Constraint Expressions
class CExpr : public Expr {
private:
  CExpr(const CExpr&);
  CExpr& operator=(const CExpr&);
 public:
  CExpr() : m_count(s_counter++), m_enforceContext(false), m_violationMsg(), 
            m_returnArgument(NULL) {}
  virtual ~CExpr() {}

  virtual bool hasReturnValue() const = 0;
  virtual void setEnforceContext() { m_enforceContext = true; }
  virtual void setViolationMsg(const std::string& msg) { m_violationMsg = msg; }
  virtual bool isSingleton() = 0;
  virtual bool isSingletonOptimizable() = 0; // TODO: use output node method instead? (ask JRB)
  virtual void checkType() = 0;
  void setReturnArgument(CExpr *arg) { m_returnArgument = arg; }

 protected:
  std::string createVariableName() const;

  static unsigned int s_counter;
  unsigned int m_count;
  bool m_enforceContext;
  std::string m_violationMsg;
  CExpr *m_returnArgument; // TODO: generalize this to output node (ask JRB)
};

  class CExprFunction : public CExpr
  {
    public:
        CExprFunction(const CFunctionId func, const std::vector<CExpr*>& args);
        virtual ~CExprFunction() { /* TODO: release memory */ }

        // Expr methods
        virtual DataRef eval(EvalContext& context) const;

        virtual const DataTypeId getDataType() const { return m_func->getReturnType(); }

        virtual std::string toString() const;

        // CExpr methods
        virtual bool hasReturnValue() const { return true; }
        virtual bool isSingleton() { return false; }
        virtual bool isSingletonOptimizable() { return false; }

        virtual void checkType();

    virtual const std::vector<CExpr*>& getArgs() const {return m_args;}

    protected:
        CFunctionId m_func;
        std::vector<CExpr*> m_args;
   private:
    CExprFunction() : m_func(), m_args() {}
  };

class CExprValue : public CExpr {
private:
  CExprValue(const CExprValue&);
  CExprValue& operator=(const CExprValue&);
 public:
  CExprValue(Expr* value);
  virtual ~CExprValue() { /* TODO: release memory */ }

  // Expr methods
  virtual DataRef eval(EvalContext& context) const;

  virtual const DataTypeId getDataType() const { return m_value->getDataType(); }

  virtual std::string toString() const { return m_value->toString(); }

  // CExpr methods
  virtual bool hasReturnValue() const { return true; }
  virtual bool isSingleton() { return true; /*TODO:not accurate?*/}
  virtual bool isSingletonOptimizable() { return false; }

  virtual void checkType();

 protected:
  Expr* m_value;
};

  // TODO: this still needs to be broken down more into cleaner pieces
class CExprBinary : public CExpr {
 private:
  CExprBinary(const CExprBinary&);
  CExprBinary& operator=(const CExprBinary&);

 public:
  CExprBinary(std::string op, CExpr* left, CExpr* right);
  virtual ~CExprBinary();

  // Expr methods
  virtual DataRef eval(EvalContext& context) const;

  virtual const DataTypeId getDataType() const;

  virtual std::string toString() const;

  // CExpr methods
  virtual bool hasReturnValue() const;

  virtual bool isSingleton();

  virtual bool isSingletonOptimizable();

  virtual void checkType();

  virtual const CExpr* getLhs() const {return m_lhs;}
  virtual const CExpr* getRhs() const {return m_rhs;}

 protected:
  std::string m_operator;
  CExpr *m_lhs, *m_rhs;
};

  // InterpretedToken is the interpreted version of NddlToken
  class InterpretedToken : public IntervalToken  {
  	public:
  	    // Same Constructor signatures as NddlToken, TODO: see if both are needed
  	    InterpretedToken(const PlanDatabaseId planDatabase,
  	                     const std::string& predicateName,
  	                     const std::vector<Expr*>& body,
                         const bool& rejectable = false,
                         const bool& isFact = false,
  	                     const bool& close = false);

        InterpretedToken(const TokenId master,
                         const std::string& predicateName,
                         const std::string& relation,
                         const std::vector<Expr*>& body,
                         const bool& close = false);


  	    virtual ~InterpretedToken();

    protected:
        void commonInit(const std::vector<Expr*>& body,
                        const bool& autoClose);

        friend class InterpretedTokenType;
  };

  class InterpretedRuleFactory;

class InterpretedTokenType: public TokenType {
 public:
  InterpretedTokenType(const ObjectTypeId ot,const std::string& predicateName, const std::string& kind);
  virtual ~InterpretedTokenType();

  void addBodyExpr(Expr* e);
  void addRule(InterpretedRuleFactory* rf);

  virtual TokenId createInstance(const PlanDatabaseId planDb, const std::string& name, bool rejectable, bool isFact) const;
  virtual TokenId createInstance(const TokenId master, const std::string& name, const std::string& relation) const;

 protected:
  std::vector<Expr*> m_body;
  std::vector<InterpretedRuleFactory*> m_rules;
  TokenTypeId getParentType(const PlanDatabaseId planDb) const;
  void processExpr(Expr* e);

  friend class ExprRelation;
};

class TokenEvalContext : public EvalContext {
 public:
  TokenEvalContext(EvalContext* parent, const TokenId tok);
  virtual ~TokenEvalContext();

  virtual ConstrainedVariableId getVar(const std::string& name);

  virtual void* getElement(const std::string& name) const;

  virtual bool isClass(const std::string& className) const;

  TokenId getToken();
  TokenId getToken(const std::string& name);

 protected:
  TokenId m_token;
};

  class InterpretedRuleInstance : public RuleInstance
  {
  	public:
  	    InterpretedRuleInstance(const RuleId rule,
  	                            const TokenId token,
  	                            const PlanDatabaseId planDb,
                                const std::vector<Expr*>& body);

        InterpretedRuleInstance(const RuleInstanceId parent,
                                const ConstrainedVariableId var,
                                const Domain& domain,
                                const bool positive,
                                const std::vector<Expr*>& body);

        InterpretedRuleInstance(const RuleInstanceId parent,
                                const ConstrainedVariableId var,
                                const Domain& domain,
                                const bool positive,
                                const std::vector<ConstrainedVariableId>& guardComponents,
                                const std::vector<Expr*>& body);

        InterpretedRuleInstance(const RuleInstanceId parent,
                                const std::vector<ConstrainedVariableId>& vars,
                                const bool positive,
                                const std::vector<Expr*>& body);

  	    virtual ~InterpretedRuleInstance();

        TokenId createSubgoal(
                   const std::string& name,
                   const std::string& predicateType,
                   const std::string& predicateInstance,
                   const std::string& relation,
                   bool isConstrained,
                   ConstrainedVariableId owner);

        ConstrainedVariableId addLocalVariable(
                       const Domain& baseDomain,
				       bool canBeSpecified,
				       const std::string& name);

        ConstrainedVariableId addObjectVariable(
                       const std::string& type,
                       const ObjectDomain& baseDomain,
				       bool canBeSpecified,
				       const std::string& name);

        void executeLoop(EvalContext& evalContext,
                         const std::string& loopVarName,
                         const std::string& valueSet,
                         const std::vector<Expr*>& loopBody);


    protected:
        std::vector<Expr*> m_body;

        virtual void handleExecute();

        friend class ExprIf;
        friend class ExprVarRef;
  };

  typedef Id<InterpretedRuleInstance> InterpretedRuleInstanceId;

  class InterpretedRuleFactory : public Rule
  {
    public:
        InterpretedRuleFactory(const std::string& predicate, const std::string& source, const std::vector<Expr*>& ruleBody);
        virtual ~InterpretedRuleFactory();

        virtual RuleInstanceId createInstance(const TokenId token,
                                              const PlanDatabaseId planDb,
                                              const RulesEngineId &rulesEngine) const;

        const std::vector<Expr*>& getBody() const;

    protected:
        std::vector<Expr*> m_body;
  };

  class RuleInstanceEvalContext : public EvalContext
  {
    public:
        RuleInstanceEvalContext(EvalContext* parent, const InterpretedRuleInstanceId ruleInstance);
        virtual ~RuleInstanceEvalContext();

        virtual void* getElement(const std::string& name) const;

        virtual ConstrainedVariableId getVar(const std::string& name);
        virtual InterpretedRuleInstanceId getRuleInstance() { return m_ruleInstance; }

        virtual TokenId getToken(const std::string& name);

        virtual bool isClass(const std::string& className) const;

        virtual std::string toString() const;

    protected:
        InterpretedRuleInstanceId m_ruleInstance;
  };

  /*
   * Expr that appears in the body of an interpreted rule instance
   *
   */

class RuleExpr  : public Expr {
 public:
  virtual DataRef eval(EvalContext& context) const
  {
    RuleInstanceEvalContext* ec = boost::polymorphic_cast<RuleInstanceEvalContext*>(&context);
    return doEval(*ec);
  }
  
  virtual DataRef doEval(RuleInstanceEvalContext& context) const = 0;
  virtual ~RuleExpr(){}
};

class ExprIfGuard : public Expr {
private:
  ExprIfGuard(const ExprIfGuard&);
  ExprIfGuard& operator=(const ExprIfGuard&);
 public:
  ExprIfGuard(const std::string& op, Expr* lhs,Expr* rhs);
  virtual ~ExprIfGuard();

  const std::string& getOperator();
  Expr* getLhs() const;
  Expr* getRhs() const;

  virtual DataRef eval(EvalContext& context) const;
  virtual std::string toString() const;

 protected:
  const std::string m_op;
  Expr* m_lhs;
  Expr* m_rhs;

};

class ExprIf : public RuleExpr {
private:
  ExprIf(const ExprIf&);
  ExprIf& operator=(const ExprIf&);
 public:
  ExprIf(ExprIfGuard* guard,const std::vector<Expr*>& ifBody, const std::vector<Expr*>& elseBody);
  virtual ~ExprIf();

  virtual DataRef doEval(RuleInstanceEvalContext& context) const;
  virtual std::string toString() const;

 protected:
  ExprIfGuard* m_guard;
  std::vector<Expr*> m_ifBody;
  std::vector<Expr*> m_elseBody;
};

  class ExprLoop : public RuleExpr
  {
  	public:
  	    ExprLoop(const std::string& varName, const std::string& varValue,const std::vector<Expr*>& loopBody);
  	    virtual ~ExprLoop();

  	    virtual DataRef doEval(RuleInstanceEvalContext& context) const;

    protected:
        std::string m_varName;
    std::string m_varValue;
        std::vector<Expr*> m_loopBody;
  };

  class NativeTokenType: public TokenType
  {
    public:
	  NativeTokenType(const ObjectTypeId ot,const std::string& predicateName) : TokenType(ot,predicateName) {}

	  virtual TokenId createInstance(const PlanDatabaseId planDb, const std::string& name, bool rejectable, bool isFact) const = 0;
	  virtual TokenId createInstance(const TokenId master, const std::string& name, const std::string& relation) const = 0;
  };

void getVariableReferences(const Expr* expr, EvalContext& ctx, std::vector<ConstrainedVariableId>& dest);

}

#endif // H_Interpreter
