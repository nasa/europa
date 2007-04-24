#ifndef ANMLTRANSLATOR_H_
#define ANMLTRANSLATOR_H_

#include <map>
#include <string>
#include <vector>

namespace ANML
{

class Action;
class ANMLContext;
class ANMLElement;
class ConstraintDef;
class Expr;
class Fluent;
class LHSExpr;
class LHSPlannerConfig;
class Proposition;
class ObjType;
class TemporalQualifier;
class Type;
class Variable;

class ANMLTranslator
{
  public:
      ANMLTranslator();
      virtual ~ANMLTranslator();
      
      virtual void pushContext(ANMLContext* context);
      virtual void popContext();      

      virtual ANMLContext& getContext() { return *m_context; }
      
      virtual LHSPlannerConfig* getPlannerConfig() { return m_plannerConfig; }
            
      virtual void toNDDL(std::vector<ANML::ANMLElement*>& program,std::ostream& os) const;
      
	  virtual std::string toString() const;      
      
  protected:
      ANMLContext* m_context;
      LHSPlannerConfig* m_plannerConfig;
      
      ANMLContext* createGlobalContext();          
};

class ANMLContext
{
  public:
	ANMLContext(const ANMLContext* parent=NULL);
	virtual ~ANMLContext();

    virtual std::string getContextDesc() const { return "Global Context"; }

    virtual const ANMLContext* getParentContext() { return m_parentContext; }
    virtual void setParentContext(const ANMLContext* parent) { m_parentContext = parent; }
    	
	virtual void     addType(Type* type);
	virtual ObjType* addObjType(const std::string& name,const std::string& parentName);

	virtual Type*    getType(const std::string& name,bool mustExist=false) const;
	virtual ObjType* getObjType(const std::string& name) const;
	virtual const std::map<std::string,Type*>& getTypes() const { return m_types; }
	
	virtual void addAction(Action* a);
	virtual Action* getAction(const std::string& name,bool mustExist=false) const;
	virtual const std::map<std::string,Action*>& getActions() const { return m_actions; }
	
	// Variables, Functions and Predicates are included here, 
	// a function is just a var with args, a predicate is just a function on the boolean domain
	virtual void addVariable(Variable* v);
	virtual Variable* getVariable(const std::string& name,bool mustExist=false) const;
	virtual const std::map<std::string,Variable*>& getVariables() const { return m_variables; }

    virtual void addConstraint(ConstraintDef* c);
    virtual ConstraintDef* getConstraint(const std::string& name,const std::vector<const Type*>& argTypes,bool mustExist=false) const;
    	
	virtual std::string toString() const;
		
  protected:
    const ANMLContext*        m_parentContext;

    // maps to quickly get to elements by name
    std::map<std::string,Type*>      m_types;    
    std::map<std::string,Action*>    m_actions;
    std::map<std::string,Variable*>  m_variables;
    
    // map to get to constraints by name and signature
    std::map<std::string,ConstraintDef*>  m_constraints;        
};

class ANMLElement
{
  public:
    ANMLElement(const std::string& type) : m_elementType(type) {}
    ANMLElement(const std::string& type,const std::string& name) : m_elementType(type), m_elementName(name) {}
    virtual ~ANMLElement() {}    

    virtual const std::string& getElementType() { return m_elementType; }
    virtual const std::string& getElementName() const { return m_elementName; }
    virtual void setName(const std::string& n) { m_elementName = n; }
    
    // Steps to support translation
    virtual void preProcess (ANMLContext& context) {}
    virtual bool validate   (ANMLContext& context,std::vector<std::string>& problems) { return true; }
    virtual void toNDDL     (ANMLContext& context,std::ostream& os) const;

    virtual std::string toString() const;
    
  protected:    
    std::string m_elementType;
    std::string m_elementName;	  
};

class ANMLElementList : public ANMLElement
{
  public:
    ANMLElementList() : ANMLElement("Element list") {}
    
    void addElement(ANMLElement* e) { m_elements.push_back(e); }
    const std::vector<ANMLElement*>& getElements() const { return m_elements; }
    
    virtual void preProcess (ANMLContext& context);
    virtual bool validate(ANMLContext& context,std::vector<std::string>& problems);
    virtual void toNDDL(ANMLContext& context,std::ostream& os) const;
    
  protected:
    std::vector<ANMLElement*> m_elements;  	
};

// Class used to keep track of all elements needed to translate relational fluents that may appear in 
// conditions or effects
class ExplanatoryAction
{
  public:
    ExplanatoryAction(Action* a,TemporalQualifier* tq) : m_action(a), m_tq(tq) {}

    const Action* getAction() const { return m_action; }
    const TemporalQualifier* getTemporalQualifer() const { return m_tq; }
     
  protected:
    Action* m_action;
    TemporalQualifier* m_tq;
};

class ValueSetter
{
  public: 
    ValueSetter(int idx);
    virtual ~ValueSetter();
    
    const std::string& getName() const { return m_name; }
    void addExplanatoryAction(Action* a,TemporalQualifier* tq);
    
    void toNDDL(ANMLContext& context,std::ostream& os,const std::string& typeName) const;
  
  protected:
    std::string m_name;
    ObjType* m_objType;
    
    std::vector<ExplanatoryAction> m_explanatoryActions;  
};

class Type 
{
  public:
	Type(const std::string& name);
	virtual ~Type();
	    
	virtual std::string getName() const { return m_typeName; }    
	virtual bool isPrimitive() const { return true; }
	
	virtual bool canBeResourceType() const { return false; }
    virtual bool isResourceType() const { return false; }	
	virtual void becomeResourceType();
	
	virtual bool isAssignableFrom(const Type& rhs) const { return getName() == rhs.getName(); }
	    
    virtual void generateCopier(std::ostream& os,
                              const std::string& lhs,
                              const std::string& predName,
                              const std::string& tokenName,
                              const std::string& rhs) const;
                                 
    virtual void toNDDL(ANMLContext& context,std::ostream& os) const;
    
    virtual ValueSetter* getValueSetter(const ObjType* objType,const std::string& varName);
    virtual const std::string& getValueSetterName(const ObjType* objType,const std::string& varName) const; 
    
    static Type* VOID;    
    static Type* BOOL;    
    static Type* INT;    
    static Type* FLOAT;    
    static Type* STRING;    
    static ObjType* OBJECT;    
    
  protected:
    std::string m_typeName;     
    std::vector<ValueSetter*> m_valueSetters;
    std::map<std::string,ValueSetter*> m_valueSettersByKey;
};

class TypeAlias : public Type
{
  public:
	TypeAlias(const std::string& name,const Type& t);
	virtual ~TypeAlias();
	    
	virtual bool isPrimitive() const { return m_wrappedType.isPrimitive(); }
	    
    virtual void toNDDL(ANMLContext& context,std::ostream& os) const;    

  protected:    
    const Type& m_wrappedType;
};

class Range : public Type
{
  public:
	Range(const std::string& name,const Type& dataType,const std::string& lb,const std::string& ub);
	virtual ~Range();
	    
	virtual bool canBeResourceType() const;
    virtual bool isResourceType() const { return m_isResourceType; }	
	virtual void becomeResourceType();
	
	virtual bool isPrimitive() const { return !m_isResourceType; }
	    
    virtual void toNDDL(ANMLContext& context,std::ostream& os) const;
    
  protected:
    bool m_isResourceType;
    const Type& m_dataType;
    std::string m_lb;
    std::string m_ub;
};

class Enumeration : public Type
{
  public:
	Enumeration(const std::string& name,const Type& dataType,const std::vector<Expr*>& values);
	virtual ~Enumeration();
	    
	virtual bool isPrimitive() const { return false; }
	    
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:
    const Type& m_dataType;
    std::vector<Expr*> m_values;
};
	
class ObjType : public Type, public ANMLContext, public ANMLElementList
{
  public:
    ObjType(const std::string& name,ObjType* parentObjType);
    virtual ~ObjType();
    
    virtual std::string getContextDesc() const { return m_typeName; }
    
    virtual ObjType* getParentObjType() const { return m_parentObjType; }
        
	virtual bool isPrimitive() const { return false; }

    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:
    ObjType* m_parentObjType; 
    
    void declareValueSetters(std::ostream& os) const;    
};

class VectorType : public ObjType
{
  public:
    VectorType(const std::string& name);
    
    virtual void generateCopier(std::ostream& os,
                              const std::string& lhs,
                              const std::string& predName,
                              const std::string& tokenName,
                              const std::string& rhs) const;   
    
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;  
};

class ConstraintDef 
{
  public:
    ConstraintDef(const std::string& name, const std::vector<const Type*>& argTypes) : m_name(name), m_argTypes(argTypes) {}   

    const std::string& getName() const { return m_name; }
    const std::vector<const Type*>& getArgTypes() const { return m_argTypes; }
    
  protected:
	std::string m_name;
	std::vector<const Type*> m_argTypes;
};

class Arg
{
  public:
    Arg(const std::string& name,const Type& type) : m_name(name), m_dataType(type) {}    
    virtual ~Arg() {}

    const std::string& getName() const { return m_name; }    
    const Type& getType() const { return m_dataType; }

  protected:
    std::string m_name;    
    const Type& m_dataType;
};	
	
class Variable 
{
  public:
    Variable(const Type& type, const std::string& name);
    Variable(const Type& type, const std::string& name, const std::vector<Arg*>& args);
    
    virtual ~Variable();
    
    const std::string& getName() const { return m_name; }

    const ObjType* getObjType() const { return m_objType; }
    void setObjType(const ObjType* ot) { m_objType = ot; }
    
    const Type& getDataType() const { return m_dataType; }
    const std::vector<Arg*>& getArgs() const { return m_args; }
    
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:
    std::string m_name;
    const ObjType* m_objType; // this is != NULL iif the variable is an ObjType member
	const Type& m_dataType;
	std::vector<Arg*> m_args;
};
    
class VarInit
{
  public:
      VarInit(Variable* v, const Expr* value)
          : m_var(v)
          , m_value(value)
      {
      }
      
      Variable* getVar() { return m_var; }
      const std::string& getName() const { return m_var->getName(); }
      const std::string getValue() const;
      
  protected:
      Variable* m_var;
      const Expr* m_value;	
};
    
class TypeDefinition : public ANMLElement
{
  public:
    TypeDefinition(Type* anmlType) : ANMLElement("TYPE",anmlType->getName()), m_anmlType(anmlType) {}
    virtual ~TypeDefinition() {}

    virtual void toNDDL(ANMLContext& context,std::ostream& os) const { m_anmlType->toNDDL(context,os); }
    
  protected:
    Type* m_anmlType;        
};

class VarDeclaration : public ANMLElement
{
  public:
    VarDeclaration(const Type& type, const std::vector<VarInit*>& init);
    virtual ~VarDeclaration();

    const Type& getDataType() const { return m_dataType; }
    const std::vector<VarInit*>& getInit() const { return m_init; }
    
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
      
  protected:
	const Type& m_dataType;
	std::vector<VarInit*> m_init;
};

// TODO: figure out whether this should be integrated with VarDeclaration
class FreeVarDeclaration : public ANMLElement
{
  public:
    FreeVarDeclaration(const std::vector<Variable*>& vars);
    virtual ~FreeVarDeclaration();
    
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
      
  protected:
	std::vector<Variable*> m_vars;    
};

class Action : public ANMLContext, public ANMLElementList
{
  public:
    Action(ObjType& objType,const std::string& name,const std::vector<Variable*>& params);
    virtual ~Action();

    const std::string& getName() const { return m_name; }
    
    virtual std::string getContextDesc() const { return m_objType.getName() + "::" + m_elementName; }
    
    const std::vector<Variable*>& getParams() const { return m_params; }
    
    void setBody(const std::vector<ANMLElement*>& body);
  
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
        
  protected:
    std::string m_name;
    ObjType& m_objType;
    std::vector<Variable*> m_params;
};

class ActionDuration : public ANMLElement
{
  public:
    ActionDuration(const std::vector<Expr*>& values);
    virtual ~ActionDuration();
    
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:  
    std::vector<Expr*> m_values;    	
};


class Proposition 
{
  public:
    enum Context {GOAL,CONDITION,FACT,EFFECT};
    
    Proposition(TemporalQualifier* tq,Fluent* f); 
    Proposition(TemporalQualifier* tq,const std::vector<Fluent*>& fluents);
    virtual ~Proposition();
    
    Action* getParentAction() { return m_parentAction; }
    void setParentAction(Action* parent) { m_parentAction = parent; }
    
    TemporalQualifier* getTemporalQualifier() { return m_temporalQualifier; }
    
    void setContext(Context c) { m_context = c; }
    Context getContext() const { return m_context; }
    
    virtual void preProcess(ANMLContext& context);
    virtual bool validate(ANMLContext& context,std::vector<std::string>& problems);
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:  
    Context m_context;    	
    TemporalQualifier* m_temporalQualifier;
    std::vector<Fluent*> m_fluents;
    Action* m_parentAction;
    
    void commonInit();    
};

class PropositionSet : public ANMLElement
{
  public:
    PropositionSet(const std::vector<Proposition*>& propositions, Action* parent,Proposition::Context context,const std::string& type);
    virtual ~PropositionSet();
    
    virtual void preProcess (ANMLContext& context);
    virtual bool validate(ANMLContext& context,std::vector<std::string>& problems);
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:  
    std::vector<Proposition*> m_propositions;    	
};

class Condition : public PropositionSet
{
  public:
    Condition(const std::vector<Proposition*>& propositions);
    virtual ~Condition();
};

class Effect : public PropositionSet
{
  public:
    Effect(const std::vector<Proposition*>& propositions,Action* parentAction);
    virtual ~Effect();
};

class Goal : public PropositionSet
{
  public:
    Goal(const std::vector<Proposition*>& props);
    virtual ~Goal();
};

class Fact : public PropositionSet
{
  public:
    Fact(const std::vector<Proposition*>& props);
    virtual ~Fact();
};

class Change : public ANMLElement
{
  public:
    Change(Proposition* when_condition,Proposition* change_stmt);
    virtual ~Change();
    
    void setWhenCondition(Proposition* p) { m_whenCondition = p; }
    
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:  
    Proposition* m_whenCondition;
    Proposition* m_changeStmt;
};

class PropositionComponent
{
  public:
    PropositionComponent() {}
    
    void setProposition(Proposition* p) { m_parentProp = p; }
  
  protected:
    Proposition* m_parentProp;	
};

class TemporalQualifier : public PropositionComponent
{
  public:
    TemporalQualifier(const std::string& op,const std::vector<Expr*>& args);
    virtual ~TemporalQualifier();
     
    virtual void toNDDL(std::ostream& os,Proposition::Context context) const;
    virtual void toNDDL(std::ostream& os,const std::string& ident,const std::string& fluentName) const;
    
  protected:
    std::string m_operator;
    std::vector<Expr*> m_args;
    mutable std::vector<std::string> m_argValues;
};

class Fluent : public PropositionComponent
{
  public:
    Fluent() {}
    virtual ~Fluent() {}
    
    virtual void preProcess(ANMLContext& context) {}
    virtual bool validate(ANMLContext& context,std::vector<std::string>& problems) { return true; }
    virtual void toNDDL(std::ostream& os, TemporalQualifier* tq) const = 0;
    virtual std::string toString() const = 0;
};

class Constraint : public Fluent
{
  public:
    Constraint(const std::string& name,const std::vector<ANML::Expr*>& args);
    virtual ~Constraint();
    
    virtual void toNDDL(std::ostream& os, TemporalQualifier* tq) const;

    virtual std::string toString() const { return m_name; }
    
  protected:
    std::string m_name;    
    std::vector<ANML::Expr*> m_args;        	
};

class CompositeFluent : public Fluent
{
  public:
    CompositeFluent(const std::string& op,Fluent* lhs, Fluent* rhs) : m_op(op), m_lhs(lhs), m_rhs(rhs) {}
    virtual ~CompositeFluent() {}
    
    virtual void toNDDL(std::ostream& os, TemporalQualifier* tq) const { os << m_op; }

    virtual std::string toString() const;
    
  protected:
    std::string m_op;
    Fluent* m_lhs;
    Fluent* m_rhs;  
};

class RelationalFluent : public Fluent
{
  public:
    RelationalFluent(LHSExpr* lhs,Expr* rhs);
    virtual ~RelationalFluent();
    
    virtual void preProcess(ANMLContext& context);    
    virtual void toNDDL(std::ostream& os, TemporalQualifier* tq) const;
    
    virtual std::string toString() const;
    
  protected:  
    LHSExpr* m_lhs;
    Expr* m_rhs;    	
};

class ResourceChangeFluent : public Fluent
{
  public:
    ResourceChangeFluent(const std::string& op,Expr* var,Expr* qty);
    virtual ~ResourceChangeFluent();

    virtual void toNDDL(std::ostream& os, TemporalQualifier* tq) const;

    virtual std::string toString() const;

  protected:
	std::string m_op;
	Expr* m_var;
	Expr* m_qty;      
};


class TransitionChangeFluent : public Fluent
{
  public :
    TransitionChangeFluent(Expr* var, const std::vector<Expr*>& states);
    virtual ~TransitionChangeFluent();

    virtual void toNDDL(std::ostream& os, TemporalQualifier* tq) const;
    
    virtual std::string toString() const;

  protected:    
    Expr* m_var;
	std::vector<Expr*> m_states;
};


class ActionSet;
class SubAction;

class Decomposition : public ANMLElement
{
  public:
    Decomposition();
    virtual ~Decomposition();
    
    void addActionSet(ActionSet* as, TemporalQualifier* tq);
    void addConstraint(Constraint* c) { m_constraints.push_back(c); }
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;
    
  protected:  
    std::vector<ActionSet*> m_actionSets;  
    std::vector<Constraint*> m_constraints;  
    	
};

class ActionSetElement
{
  public:
      ActionSetElement() {}
      virtual ~ActionSetElement() {}
      
      virtual const std::string getType() const = 0;
      virtual const std::string& getLabel() const { return m_label; }
      virtual void toNDDL(ANMLContext& context, std::ostream& os, const std::string& ident) const = 0;

  protected:
      std::string m_label;        
};

class ActionSet : public ActionSetElement
{
  public:
      ActionSet(const std::string& op,const std::vector<ANML::ActionSetElement*>& elements);
      virtual ~ActionSet();

      virtual const std::string getType() const { return m_type; }
      
      virtual void setTemporalQualifier(TemporalQualifier* tq) { m_tq = tq; }

      virtual void toNDDL(ANMLContext& context, std::ostream& os, const std::string& ident) const;
  
  protected:
      std::string m_operator;
      TemporalQualifier* m_tq;
      std::vector<ActionSetElement*> m_elements;
      std::string m_type;
};

class LHSAction;

class SubAction : public ActionSetElement
{
  public:
      SubAction(ANML::LHSAction* action, const std::vector<ANML::Expr*>& args, const std::string& label);		
      virtual ~SubAction();

      virtual const std::string getType() const;
      
      virtual void toNDDL(ANMLContext& context, std::ostream& os, const std::string& ident) const;
  
  protected:
      LHSAction* m_action;
      std::vector<ANML::Expr*> m_args;      
};

class Expr 
{
  public:
    Expr() {}
    virtual ~Expr() {}
        
    virtual bool needsVar() { return false; }
    virtual std::string toString() const = 0; 
    virtual const Type& getDataType() const = 0;
    
    virtual void toNDDLasExpr(std::ostream& os,
                             const std::string& varName) const {}    
                             
    virtual void toNDDLasLHS(std::ostream& os,
                             Proposition::Context context,
                             const std::string& varName) const {}    
                             
    virtual void toNDDLasRHS(std::ostream& os,
                             Proposition::Context context,
                             LHSExpr* lhs,
                             const std::string& tokenName) const {}                                                           
};

class LHSExpr : public Expr
{
  public:
    LHSExpr() {}
    virtual ~LHSExpr() {}
        
    virtual bool isVariableExpr() const=0;
    virtual const Variable* getVariable() const=0;
        
    virtual void setArgs(const std::vector<Expr*>& args) { m_args = args; }
    virtual const std::vector<Expr*>& getArgs() const { return m_args; } 
    
  protected:  
    std::vector<Expr*> m_args;    	
};

class LHSPlannerConfig : public LHSExpr
{
  public:
	LHSPlannerConfig();
	virtual ~LHSPlannerConfig();
	
    virtual bool isVariableExpr() const { return true; }
    virtual const Variable* getVariable() const { return NULL; } // TODO: throw exception

    virtual void setArgs(const std::string& predicate,const std::vector<Expr*>& args);
    virtual std::string toString() const  { return "PlannerConfig"; }
    virtual const Type& getDataType() const { return *Type::BOOL; }
    virtual void toNDDL(ANMLContext& context, std::ostream& os) const;

  protected:
    mutable bool m_translated;
    Expr* m_startHorizon;
    Expr* m_endHorizon;
    Expr* m_maxSteps;
    Expr* m_maxDepth;     	
};

class LHSAction : public LHSExpr
{
  public:
	LHSAction(Action* a,const std::string& path) : m_action(a), m_path(path) {}
	virtual ~LHSAction() {}
	
    virtual bool isVariableExpr() const { return false; }
    virtual const Variable* getVariable() const { return NULL; } // TODO: throw exception
    
    virtual const Type& getDataType() const { return *Type::VOID; }
    virtual std::string toString() const  { return m_path; }
    
    virtual void toNDDLasLHS(std::ostream& os,
                             Proposition::Context context,
                             const std::string& varName) const;    
        
  protected:
    Action* m_action;  	  
    std::string m_path;
};

class LHSVariable : public LHSExpr
{
  public:
	LHSVariable(Variable* v,const std::string& path) : m_var(v), m_path(path) {}
	virtual ~LHSVariable() {}
	
    virtual bool isVariableExpr() const { return true; }
    virtual const Variable* getVariable() const { return m_var; } 

    virtual const Type& getDataType() const { return m_var->getDataType(); }
    virtual std::string toString() const  { return m_path; }
    virtual void toNDDLasRHS(std::ostream& os,
                             Proposition::Context context,
                             LHSExpr* lhs,
                             const std::string& tokenName) const;    
    
  protected:
    Variable* m_var;  	  
    std::string m_path;
};

class ExprConstant : public Expr
{
  public:
    ExprConstant(const Type& dataType,const std::string& value) : m_dataType(dataType),m_value(value) {}
    virtual ~ExprConstant() {}
        
    virtual const Type& getDataType() const { return m_dataType; }
    virtual std::string toString() const 
    { 
    	return (&m_dataType != Type::STRING 
          ? m_value 
          : "\""+m_value+"\""); 
    } 
    
    virtual void toNDDLasRHS(std::ostream& os,
                             Proposition::Context context,
                             LHSExpr* lhs,
                             const std::string& varName) const;    
  
  protected:
    const Type& m_dataType;
    std::string m_value;    
};

class ExprArithOp : public Expr
{
  public:
    ExprArithOp(const std::string& op, Expr* operand1, Expr* operand2) : m_operator(op), m_op1(operand1), m_op2(operand2) {}
    virtual ~ExprArithOp() {}
        
    virtual bool needsVar() { return true; }
    virtual const Type& getDataType() const { return m_op1->getDataType(); }
    virtual std::string toString() const { return m_op1->toString() + m_operator + m_op2->toString(); } 
    virtual void toNDDLasExpr(std::ostream& os,
                             const std::string& varName) const;    
  
  protected:
    std::string m_operator;
    Expr* m_op1;
    Expr* m_op2;        
};

class ExprVector : public Expr
{
  public:
    ExprVector(const std::vector<Expr*>& values);
    virtual ~ExprVector();
        
    virtual std::string toString() const; 
    virtual const Type& getDataType() const { return *m_dataType; }
    virtual void toNDDLasRHS(std::ostream& os,
                             Proposition::Context context,
                             LHSExpr* lhs,
                             const std::string& varName) const;    
    
  protected:
    VectorType* m_dataType;
    std::vector<Expr*> m_values;        
};

}

#endif /*ANMLTRANSLATOR_H_*/
