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

  class ExprVarRef : public Expr
  {
  	public:
  	    ExprVarRef(const char* name);
  	    virtual ~ExprVarRef();

  	    virtual DataRef eval(EvalContext& context) const;
  	    virtual std::string toString() const;

  	protected:
  	    LabelStr m_varName;
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

  // Assignment inside a constructor
  // TODO: make lhs an Expr as well to make this a generic assignment
  class ExprConstructorAssignment : public Expr
  {
    public:
        ExprConstructorAssignment(const char* lhs,
                                  Expr* rhs);
        virtual ~ExprConstructorAssignment();

        virtual DataRef eval(EvalContext& context) const;

    protected:
        LabelStr m_lhs;
        Expr* m_rhs;
  };

  class ExprVarDeclaration;
  class ExprAssignment;

  // InterpretedToken is the interpreted version of NddlToken
  class InterpretedToken : public IntervalToken
  {
  	public:
  	    // Same Constructor signatures as NddlToken, see if both are needed
  	    InterpretedToken(const PlanDatabaseId& planDatabase,
  	                     const LabelStr& predicateName,
  	                     const std::vector<ExprVarDeclaration*>& parameters,
  	                     const std::vector<ExprAssignment*>& varAssignments,
                         const std::vector<ExprConstraint*>& constraints,
                         const bool& rejectable = false,
                         const bool& isFact = false,
  	                     const bool& close = false);

        InterpretedToken(const TokenId& master,
                         const LabelStr& predicateName,
                         const LabelStr& relation,
                         const std::vector<ExprVarDeclaration*>& parameters,
                         const std::vector<ExprAssignment*>& varAssignments,
                         const std::vector<ExprConstraint*>& constraints,
                         const bool& close = false);


  	    virtual ~InterpretedToken();

    protected:
        void commonInit(const std::vector<ExprVarDeclaration*>& parameters,
                        const std::vector<ExprAssignment*>& varAssignments,
                        const std::vector<ExprConstraint*>& constraints,
                        const bool& autoClose);

        friend class InterpretedTokenFactory;
  };

  class TokenEvalContext : public EvalContext
  {
  	public:
  	    TokenEvalContext(EvalContext* parent, const TokenId& tok);
  	    virtual ~TokenEvalContext();

  	    virtual ConstrainedVariableId getVar(const char* name);

  	    virtual bool isClass(const LabelStr& className) const;

  	protected:
  	    TokenId m_token;
  };

  class InterpretedTokenFactory: public TokenFactory
  {
    public:
	  InterpretedTokenFactory(const LabelStr& predicateName,
	                          const ObjectTypeId& objType);

	  void addParameter(ExprVarDeclaration* parameterDecl);
	  ExprVarDeclaration* getParameter(const LabelStr& name);

      void addVarAssignment(ExprAssignment* va);
	  void addConstraint(ExprConstraint* c);

	  virtual TokenId createInstance(const PlanDatabaseId& planDb, const LabelStr& name, bool rejectable, bool isFact) const;
	  virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const;

    protected:
      ObjectTypeId m_objType;
      std::vector<ExprVarDeclaration*> m_parameters;
      std::vector<ExprAssignment*> m_varAssignments;
      std::vector<ExprConstraint*> m_constraints;

      TokenFactoryId getParentFactory(const PlanDatabaseId& planDb) const;
  };

  class RuleExpr;

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

        void createConstraint(const LabelStr& name, std::vector<ConstrainedVariableId>& vars);

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
        friend class ExprRuleVarRef;
  };

  typedef Id<InterpretedRuleInstance> InterpretedRuleInstanceId;
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

  class ExprRuleVarRef : public RuleExpr
  {
  	public:
  	    ExprRuleVarRef(const char* name);
  	    virtual ~ExprRuleVarRef();

  	    virtual DataRef doEval(RuleInstanceEvalContext& context) const;

  	protected:
  	    std::string m_parentName;
  	    std::string m_varName;
  };

  class ExprSubgoal : public RuleExpr
  {
  	public:
  	    ExprSubgoal(const char* name,
  	                const char* predicateType,
  	                const char* predicateInstance,
  	                const char* relation);
  	    virtual ~ExprSubgoal();

  	    virtual DataRef doEval(RuleInstanceEvalContext& context) const;

  	protected:
  	    LabelStr m_name;
  	    LabelStr m_predicateType;
  	    LabelStr m_predicateInstance;
  	    LabelStr m_relation;

  	  bool isConstrained(RuleInstanceEvalContext& context, const LabelStr& predicateInstance) const;
  };

  class ExprRelation : public RuleExpr
  {
  	public:
  	    ExprRelation(const char* relation,
  	                 const char* origin,
  	                 const char* target);
  	    virtual ~ExprRelation();

  	    virtual DataRef doEval(RuleInstanceEvalContext& context) const;

  	protected:
  	    LabelStr m_relation;
  	    LabelStr m_origin;
  	    LabelStr m_target;
  };

  class ExprLocalVar : public RuleExpr
  {
  	public:
  	    ExprLocalVar(const LabelStr& name,
  	                 const LabelStr& type,
  	                 bool guarded,
  	                 Expr* domainRestriction,
  	                 const AbstractDomain& baseDomain);
  	    virtual ~ExprLocalVar();

  	    virtual DataRef doEval(RuleInstanceEvalContext& context) const;

  	protected:
  	    LabelStr m_name;
  	    LabelStr m_type;
  	    bool m_guarded;
  	    Expr* m_domainRestriction;
  	    const AbstractDomain& m_baseDomain;
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

  class ExprVarDeclaration : public Expr
  {
  public:
      ExprVarDeclaration(const char* name, const char* type, Expr* initValue);
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

}

#endif // _H_Interpreter
