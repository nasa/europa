#ifndef _H_Interpreter
#define _H_Interpreter

#include "PDBInterpreter.hh"
#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"
#include "Object.hh"
#include "ObjectFactory.hh"
#include "Method.hh"
#include "IntervalToken.hh"
#include "TokenType.hh"
#include "Rule.hh"
#include "RuleInstance.hh"
#include <vector>


namespace EUROPA {

  //TODO: Rename
  class FunctionType;
  class FunctionType
  {
  public:
    FunctionType(const char* name, const char* constraint, const char* returnType, unsigned int argumentCount, bool addToList = false);
    FunctionType(FunctionType& copy);
    ~FunctionType();
    const char* getName();
    const char* getConstraint();
    const char* getReturnType();
    unsigned int getArgumentCount();
  private:
    std::string m_name, m_constraint, m_returnType;
    unsigned int m_argumentCount;
  };

  // TODO: EUROPA doesn't have any static data, this needs to be moved to CESchema
  extern std::vector<FunctionType*> g_functionTypes;
#define DECLARE_FUNCTION_TYPE(name, constraint, returnType, argumentCount) FunctionType g_function_##name(#name, constraint, returnType, argumentCount, true);
  FunctionType* getFunction(std::string name);


  class ExprConstant : public Expr
  {
  	public:
  	    ExprConstant(const char* type, const AbstractDomain* d);
  	    virtual ~ExprConstant();

  	    virtual DataRef eval(EvalContext& context) const;
        virtual const DataTypeId getDataType() const;
        virtual std::string toString() const;
        std::string getConstantValue() const;

  	protected:
  	    LabelStr m_type;
  	    const AbstractDomain* m_domain;
  };

  class TokenEvalContext;
  class RuleInstanceEvalContext;

  class ExprVarDeclaration : public Expr
  {
  public:
      ExprVarDeclaration(const char* name, const DataTypeId& type, Expr* initValue, bool canBeSpecified);
      virtual ~ExprVarDeclaration();

      virtual DataRef eval(EvalContext& context) const;
      virtual const DataTypeId getDataType() const;
      virtual std::string toString() const;

      const LabelStr& getName() const;
      const Expr* getInitValue() const;
      void setInitValue(Expr* iv);

  protected:
      LabelStr m_name;
      DataTypeId m_type;
      Expr* m_initValue;
      bool m_canBeSpecified;

      ConstrainedVariableId makeGlobalVar(EvalContext& context) const;
      ConstrainedVariableId makeTokenVar(TokenEvalContext& context) const;
      ConstrainedVariableId makeRuleVar(RuleInstanceEvalContext& context) const;
  };

  class ExprVarRef : public Expr
  {
  	public:
  	    ExprVarRef(const char* name, const DataTypeId& type);
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

  class ExprAssignment : public Expr
  {
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
        ExprConstraint(const char* name,const std::vector<Expr*>& args);
        virtual ~ExprConstraint();

        virtual DataRef eval(EvalContext& context) const;

        const LabelStr getName() const { return m_name; }
        const std::vector<Expr*>& getArgs() const { return m_args; }
        virtual std::string toString() const;
    protected:
        LabelStr m_name;
        std::vector<Expr*> m_args;
  };

  class ExprTypedef : public Expr
  {
  public:
      ExprTypedef(const DataTypeId& baseType, const char* name, AbstractDomain* baseDomain);
      virtual ~ExprTypedef();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      DataTypeId m_baseType;
      LabelStr m_name;
      AbstractDomain* m_baseDomain;
  };

  class ExprEnumdef : public Expr
  {
  public:
      ExprEnumdef(const char* name, const std::vector<std::string>& values);
      virtual ~ExprEnumdef();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      LabelStr m_name;
      std::vector<std::string> m_values;
  };

  class ExprObjectTypeDeclaration : public Expr
  {
  public:
      ExprObjectTypeDeclaration(const LabelStr& name);
      virtual ~ExprObjectTypeDeclaration();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      LabelStr m_name;
  };

  class ExprObjectTypeDefinition : public Expr
  {
  public:
      ExprObjectTypeDefinition(const ObjectTypeId& objType);
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
      ExprRuleTypeDefinition(const RuleId& rf);
      virtual ~ExprRuleTypeDefinition();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      const RuleId m_ruleFactory;
  };

  class ExprMethodCall : public Expr
  {
  public:
      ExprMethodCall(const MethodId& method, Expr* varExpr, const std::vector<Expr*>& argExprs);
      virtual ~ExprMethodCall();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      MethodId m_method;
      Expr* m_varExpr;
      std::vector<Expr*> m_argExprs;
  };

  class ExprVariableMethod : public Expr
  {
  public:
      ExprVariableMethod(const char* name, Expr* varExpr, const std::vector<Expr*>& argExprs);
      virtual ~ExprVariableMethod();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      LabelStr m_methodName;
      Expr* m_varExpr;
      std::vector<Expr*> m_argExprs;

      DataRef eval(EvalContext& context, ConstrainedVariableId& var, const std::vector<ConstrainedVariableId>& args) const;
  };

  class ExprObjectMethod : public Expr
  {
  public:
      ExprObjectMethod(const char* name, Expr* objExpr, const std::vector<Expr*>& argExprs);
      virtual ~ExprObjectMethod();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      LabelStr m_methodName;
      Expr* m_objExpr;
      std::vector<Expr*> m_argExprs;

      DataRef eval(EvalContext& context, ObjectId& var, const std::vector<ConstrainedVariableId>& args) const;
  };

  class ExprTokenMethod : public Expr
  {
  public:
      ExprTokenMethod(const char* name, const char* tokenName, const std::vector<Expr*>& argExprs);
      virtual ~ExprTokenMethod();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      LabelStr m_methodName;
      LabelStr m_tokenName;
      std::vector<Expr*> m_argExprs;

      DataRef eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const;
  };

  class ExprNewObject : public Expr
  {
  	public:
  	    ExprNewObject(const LabelStr& objectType,
	                  const LabelStr& objectName,
	                  const std::vector<Expr*>& argExprs);

	    virtual ~ExprNewObject();

  	    virtual DataRef eval(EvalContext& context) const;

  	    virtual std::string toString() const;

  	protected:
	    LabelStr              m_objectType;
	    LabelStr              m_objectName;
	    std::vector<Expr*>    m_argExprs;
  };

  class InterpretedRuleInstance;

  class PredicateInstanceRef
  {
  public:
      PredicateInstanceRef(const char* predInstance, const char* predName);
      virtual ~PredicateInstanceRef();

      TokenId getToken(EvalContext& ctx, const char* relationName, bool isFact=false, bool isRejectable=false);

  protected:
      std::string m_predicateInstance;
      std::string m_predicateName;

      TokenId createSubgoal(EvalContext& ctx, InterpretedRuleInstance* rule, const char* relationName);
      TokenId createGlobalToken(EvalContext& context, bool isFact, bool isRejectable);
  };

  class ExprProblemStmt : public Expr
  {
  public:
      ExprProblemStmt(const char* name, const std::vector<PredicateInstanceRef*>& tokens);
      virtual ~ExprProblemStmt();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      LabelStr m_name;
      std::vector<PredicateInstanceRef*> m_tokens;

      DataRef eval(EvalContext& context, TokenId& tok, const std::vector<ConstrainedVariableId>& args) const;
  };

  class ExprRelation : public Expr
  {
    public:
        ExprRelation(const char* relation,
                     PredicateInstanceRef* origin,
                     const std::vector<PredicateInstanceRef*>& targets);
        virtual ~ExprRelation();

        virtual DataRef eval(EvalContext& context) const;

    protected:
        LabelStr m_relation;
        PredicateInstanceRef* m_origin;
        std::vector<PredicateInstanceRef*> m_targets;
  };

  // Constraint Expressions
  class CExpr : public Expr
  {
  public:
      CExpr() : m_count(s_counter++), m_enforceContext(false), m_returnArgument(NULL) {}
      virtual ~CExpr() {}

      virtual bool hasReturnValue() const = 0;
      virtual void setEnforceContext() { m_enforceContext = true; }
      virtual bool isSingleton() = 0;
      virtual bool isSingletonOptimizable() = 0; // TODO: use output node method instead? (ask JRB)
      virtual void checkType() = 0;
      void setReturnArgument(CExpr *arg) { m_returnArgument = arg; }

  protected:
      std::string createVariableName() const;

      static unsigned int s_counter;
      unsigned int m_count;
      bool m_enforceContext;
      CExpr *m_returnArgument; // TODO: generalize this to output node (ask JRB)
  };

  class CExprFunction : public CExpr
  {
    public:
        CExprFunction(FunctionType* func, std::vector<CExpr*> args, DataTypeId data);
        virtual ~CExprFunction() { /* TODO: release memory */ }

        // Expr methods
        virtual DataRef eval(EvalContext& context) const;

        virtual const DataTypeId getDataType() const { return m_dataType; }

        virtual std::string toString() const;

        // CExpr methods
        virtual bool hasReturnValue() const { return true; }
        virtual bool isSingleton() { return false; }
        virtual bool isSingletonOptimizable() { return false; }

        virtual void checkType();

    protected:
        FunctionType* m_func;
        std::vector<CExpr*> m_args;
        DataTypeId m_dataType;
  };

  class CExprValue : public CExpr
  {
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
  class ExprExpression : public CExpr
  {
    public:
        ExprExpression(std::string name, CExpr* left, CExpr* right);
        virtual ~ExprExpression();

        // Expr methods
        virtual DataRef eval(EvalContext& context) const;

        virtual const DataTypeId getDataType() const;

        virtual std::string toString() const;

        // CExpr methods
        virtual bool hasReturnValue() const;

        virtual bool isSingleton();

        virtual bool isSingletonOptimizable();

        virtual void checkType();

    protected:
        std::string m_name;
        CExpr *m_lhs, *m_rhs;
  };

  // InterpretedToken is the interpreted version of NddlToken
  class InterpretedToken : public IntervalToken  {
  	public:
  	    // Same Constructor signatures as NddlToken, TODO: see if both are needed
  	    InterpretedToken(const PlanDatabaseId& planDatabase,
  	                     const LabelStr& predicateName,
  	                     const std::vector<Expr*>& body,
                         const bool& rejectable = false,
                         const bool& isFact = false,
  	                     const bool& close = false);

        InterpretedToken(const TokenId& master,
                         const LabelStr& predicateName,
                         const LabelStr& relation,
                         const std::vector<Expr*>& body,
                         const bool& close = false);


  	    virtual ~InterpretedToken();

    protected:
        void commonInit(const std::vector<Expr*>& body,
                        const bool& autoClose);

        friend class InterpretedTokenType;
  };

  class InterpretedTokenType: public TokenType
  {
    public:
	  InterpretedTokenType(const ObjectTypeId& ot,const LabelStr& predicateName);
          virtual ~InterpretedTokenType();

          void addBodyExpr(Expr* e);

	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;

    protected:
      std::vector<Expr*> m_body;

      TokenTypeId getParentType(const PlanDatabaseId& planDb) const;
  };

  class TokenEvalContext : public EvalContext
  {
    public:
        TokenEvalContext(EvalContext* parent, const TokenId& tok);
        virtual ~TokenEvalContext();

        virtual ConstrainedVariableId getVar(const char* name);

        virtual void* getElement(const char* name) const;

        virtual bool isClass(const LabelStr& className) const;

        TokenId& getToken();

    protected:
        TokenId m_token;
  };

  class InterpretedRuleInstance : public RuleInstance
  {
  	public:
  	    InterpretedRuleInstance(const RuleId& rule,
  	                            const TokenId& token,
  	                            const PlanDatabaseId& planDb,
                                const std::vector<Expr*>& body);

        InterpretedRuleInstance(const RuleInstanceId& parent,
                                const ConstrainedVariableId& var,
                                const AbstractDomain& domain,
                                const bool positive,
                                const std::vector<Expr*>& body);

        InterpretedRuleInstance(const RuleInstanceId& parent,
                                const std::vector<ConstrainedVariableId>& vars,
                                const bool positive,
                                const std::vector<Expr*>& body);

  	    virtual ~InterpretedRuleInstance();

        TokenId createSubgoal(
                   const LabelStr& name,
                   const LabelStr& predicateType,
                   const LabelStr& predicateInstance,
                   const LabelStr& relation,
                   bool isConstrained,
                   ConstrainedVariableId& owner);

        ConstrainedVariableId addLocalVariable(
                       const AbstractDomain& baseDomain,
				       bool canBeSpecified,
				       const LabelStr& name);

        ConstrainedVariableId addObjectVariable(
                       const LabelStr& type,
                       const ObjectDomain& baseDomain,
				       bool canBeSpecified,
				       const LabelStr& name);

        void executeLoop(EvalContext& evalContext,
                         const LabelStr& loopVarName,
                         const LabelStr& valueSet,
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
        InterpretedRuleFactory(const LabelStr& predicate, const LabelStr& source, const std::vector<Expr*>& ruleBody);
        virtual ~InterpretedRuleFactory();

        virtual RuleInstanceId createInstance(const TokenId& token,
                                              const PlanDatabaseId& planDb,
                                              const RulesEngineId &rulesEngine) const;

    protected:
        std::vector<Expr*> m_body;
  };

  class RuleInstanceEvalContext : public EvalContext
  {
    public:
        RuleInstanceEvalContext(EvalContext* parent, const InterpretedRuleInstanceId& ruleInstance);
        virtual ~RuleInstanceEvalContext();

        virtual void* getElement(const char* name) const;

        virtual ConstrainedVariableId getVar(const char* name);
        virtual InterpretedRuleInstanceId& getRuleInstance() { return m_ruleInstance; }

        virtual TokenId getToken(const char* name);

        virtual bool isClass(const LabelStr& className) const;

        virtual std::string toString() const;

    protected:
        InterpretedRuleInstanceId m_ruleInstance;
  };

  /*
   * Expr that appears in the body of an interpreted rule instance
   *
   */

  class RuleExpr  : public Expr
  {
  	public:
  	    virtual DataRef eval(EvalContext& context) const
  	    {
  	    	RuleInstanceEvalContext* ec = (RuleInstanceEvalContext*)&context;
  	    	return doEval(*ec);
  	    }

  	    virtual DataRef doEval(RuleInstanceEvalContext& context) const = 0;
    virtual ~RuleExpr(){}
  };

  class ExprIfGuard : public Expr
  {
  public:
      ExprIfGuard(const char* op, Expr* lhs,Expr* rhs);
      virtual ~ExprIfGuard();

      const std::string& getOperator();
      Expr* getLhs();
      Expr* getRhs();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      const std::string m_op;
      Expr* m_lhs;
      Expr* m_rhs;

  };

  class ExprIf : public RuleExpr
  {
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
  	    ExprLoop(const char* varName, const char* varValue,const std::vector<Expr*>& loopBody);
  	    virtual ~ExprLoop();

  	    virtual DataRef doEval(RuleInstanceEvalContext& context) const;

    protected:
        LabelStr m_varName;
        LabelStr m_varValue;
        std::vector<Expr*> m_loopBody;
  };

  class NativeTokenType: public TokenType
  {
    public:
	  NativeTokenType(const ObjectTypeId& ot,const LabelStr& predicateName) : TokenType(ot,predicateName) {}

	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const = 0;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const = 0;
  };

}

#endif // _H_Interpreter
