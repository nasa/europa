#ifndef _H_Interpreter
#define _H_Interpreter

#include "PDBInterpreter.hh"
#include "IntervalToken.hh"
#include "Object.hh"
#include "ObjectFactory.hh"
#include "PlanDatabaseDefs.hh"
#include "Rule.hh"
#include "RuleInstance.hh"
#include "RulesEngineDefs.hh"
#include "Timeline.hh"
#include "TokenFactory.hh"
#include "Debug.hh"
#include <map>
#include <vector>


namespace EUROPA {

  class ExprConstant : public Expr
  {
  	public:
  	    ExprConstant(const DbClientId& dbClient, const char* type, const AbstractDomain* d);
  	    virtual ~ExprConstant();

  	    virtual DataRef eval(EvalContext& context) const;
        virtual std::string toString() const;

  	protected:
  	    const DbClientId m_dbClient;
  	    LabelStr m_type;
  	    const AbstractDomain* m_domain;
  };

  class TokenEvalContext;
  class RuleInstanceEvalContext;

  class ExprVarDeclaration : public Expr
  {
  public:
      ExprVarDeclaration(const char* name, const char* type, Expr* initValue, bool canBeSpecified);
      virtual ~ExprVarDeclaration();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

      const LabelStr& getName() const;
      const LabelStr& getType() const;

      const Expr* getInitValue() const;
      void setInitValue(Expr* iv);

  protected:
      LabelStr m_name;
      LabelStr m_type;
      Expr* m_initValue;
      bool m_canBeSpecified;

      ConstrainedVariableId makeGlobalVar(EvalContext& context) const;
      ConstrainedVariableId makeTokenVar(TokenEvalContext& context) const;
      ConstrainedVariableId makeRuleVar(RuleInstanceEvalContext& context) const;
  };

  class ExprVarRef : public Expr
  {
  	public:
  	    ExprVarRef(const char* name);
  	    virtual ~ExprVarRef();

  	    virtual DataRef eval(EvalContext& context) const;
  	    virtual std::string toString() const;

  	protected:
  	    std::string m_varName;
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

    protected:
        LabelStr m_name;
        std::vector<Expr*> m_args;
  };

  class ExprTypedef : public Expr
  {
  public:
      ExprTypedef(const char* name, AbstractDomain* type);
      virtual ~ExprTypedef();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      LabelStr m_name;
      AbstractDomain* m_type;
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
      ExprObjectTypeDeclaration(const ObjectTypeId& objType);
      virtual ~ExprObjectTypeDeclaration();

      virtual DataRef eval(EvalContext& context) const;
      virtual std::string toString() const;

  protected:
      const ObjectTypeId m_objType;
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
  	    ExprNewObject(const DbClientId& dbClient,
	                  const LabelStr& objectType,
	                  const LabelStr& objectName,
	                  const std::vector<Expr*>& argExprs);

	    virtual ~ExprNewObject();

  	    virtual DataRef eval(EvalContext& context) const;

  	    virtual std::string toString() const;

  	protected:
        DbClientId            m_dbClient;
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

  // InterpretedToken is the interpreted version of NddlToken
  class InterpretedToken : public IntervalToken
  {
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

        friend class InterpretedTokenFactory;
  };

  class InterpretedTokenFactory: public TokenFactory
  {
    public:
	  InterpretedTokenFactory(const LabelStr& predicateName,
	                          const ObjectTypeId& objType);

      void addBodyExpr(Expr* e);

	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;

    protected:
      ObjectTypeId m_objType;
      std::vector<Expr*> m_body;

      TokenFactoryId getParentFactory(const PlanDatabaseId& planDb) const;
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
        const std::vector<Expr*> m_ifBody;
        const std::vector<Expr*> m_elseBody;
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
        const std::vector<Expr*> m_loopBody;
  };

  class NativeTokenFactory: public TokenFactory
  {
    public:
	  NativeTokenFactory(const LabelStr& predicateName) : TokenFactory(predicateName) {}

	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const = 0;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const = 0;
  };

}

#endif // _H_Interpreter
