#ifndef _H_TransactionInterpreter
#define _H_TransactionInterpreter

#include "PlanDatabaseDefs.hh"
#include "Object.hh"
#include "ObjectFactory.hh"
#include <map>
#include <vector>


namespace EUROPA {

  class DataRef
  {
  	public :
  	    DataRef(AbstractDomain* d);
  	    DataRef(const AbstractDomain* d=NULL);
  	    virtual ~DataRef();
  	    
  	    AbstractDomain* getValue();
  	    const AbstractDomain* getConstValue() const;
  	    
  	    static DataRef null;

  	protected :
  	    union DataRefValue {
  	        AbstractDomain* domain;
  	        const AbstractDomain* constDomain;
  	    };
  	    
  	    DataRefValue m_value;
  };
  
  /*
   * Transaction Interpreter Variable
   */
  class TIVariable
  {
  	public:
  	    TIVariable() {} // TODO: this is so that EvalContext can add to the var map, fix it  	    
  	    TIVariable(const char* name,const AbstractDomain* d);
  	    virtual ~TIVariable();
  	    
  	    const char* getName() const;
  	    DataRef& getDataRef();
  	    
  	protected:
  	    const char* m_name;
  	    DataRef m_value;
  };
  
  class EvalContext 
  {
  	public:
  	    EvalContext(EvalContext* parent); 
  	    virtual ~EvalContext();
  	    
  	    virtual void addVar(const char* name,const AbstractDomain* value); 
  	    virtual TIVariable& getVar(const char* name);
  	
  	protected:
  	    EvalContext* m_parent;      	    
  	    std::map<std::string,TIVariable> m_variables;
  };
  
  class Expr
  {      	
  	public:
  	    virtual DataRef eval(EvalContext& context) const = 0;
  };
  
  // Call to super inside a constructor 
  class ExprConstructorSuperCall : public Expr
  {      	
  	public:
  	    ExprConstructorSuperCall(const char* className);
  	    virtual ~ExprConstructorSuperCall();
  	    
  	    virtual DataRef eval(EvalContext& context) const;
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
        const char* m_lhs;
        Expr* m_rhs;
  };
  
  class ExprConstant : public Expr
  {
  	public:
  	    ExprConstant(const AbstractDomain* d);
  	    virtual ~ExprConstant();

  	    virtual DataRef eval(EvalContext& context) const;  
  	    
  	protected:
  	    DataRef m_data;    	    
  };
  
  class ExprVariableRef : public Expr
  {
  	public:
  	    ExprVariableRef(const char* name);
  	    virtual ~ExprVariableRef();

  	    virtual DataRef eval(EvalContext& context) const;  
  	    
  	protected:
  	    const char* m_varName;    	    
  };
  
  class ExprNewObject : public Expr
  {
  	public:
  	    ExprNewObject(const PlanDatabaseId& planDb,
	                    const LabelStr& objectType, 
	                    const LabelStr& objectName,
	                    const std::vector<const AbstractDomain*>& arguments);
	    virtual ~ExprNewObject();

  	    virtual DataRef eval(EvalContext& context) const;  
  	    
  	protected:
        const PlanDatabaseId& m_planDb;
	    const LabelStr&       m_objectType; 
	    const LabelStr&       m_objectName;
	    std::vector<const AbstractDomain*> m_arguments;  
  };
  
    
  class InterpretedObjectFactory : public ConcreteObjectFactory
  {
  	public:
  	    InterpretedObjectFactory(
  	        const LabelStr& signature, 
  	        const std::vector<std::string>& constructorArgNames,
  	        const std::vector<std::string>& constructorArgTypes,
  	        const std::vector<Expr*>& constructorBody
  	    );
  	    
  	    virtual ~InterpretedObjectFactory();
  	      	    
	protected:
	    virtual ObjectId createInstance(const PlanDatabaseId& planDb,
	                            const LabelStr& objectType, 
	                            const LabelStr& objectName,
	                            const std::vector<const AbstractDomain*>& arguments) const;
	                             
	    void constructor(ObjectId& instance, const std::vector<const AbstractDomain*>& arguments) const;
  
    	ObjectId makeNewObject( 
	                        const PlanDatabaseId& planDb,
	                        const LabelStr& objectType, 
	                        const LabelStr& objectName) const;
	                        
	    bool checkArgs(const std::vector<const AbstractDomain*>& arguments) const;

        std::vector<std::string> m_constructorArgNames;	                          
        std::vector<std::string> m_constructorArgTypes;	                          
        std::vector<Expr*> m_constructorBody;	                          
  };  
  
}

#endif // _H_TransactionInterpreter
