#ifndef _H_PDBInterpreter
#define _H_PDBInterpreter

#include "ConstrainedVariable.hh"
#include "PlanDatabaseDefs.hh"
#include <map>
#include <vector>


namespace EUROPA {

  class Expr;

  class DataRef
  {
  	public :
  	    DataRef();
  	    DataRef(const ConstrainedVariableId& v);
  	    virtual ~DataRef();

  	    const ConstrainedVariableId& getValue();

  	    static DataRef null;

  	protected :
  	    ConstrainedVariableId m_value;
  };

  class EvalContext
  {
  	public:
  	    EvalContext(EvalContext* parent);
  	    virtual ~EvalContext();

  	    virtual void addVar(const char* name,const ConstrainedVariableId& v);
  	    virtual ConstrainedVariableId getVar(const char* name);

  	    virtual void addToken(const char* name,const TokenId& t);
  	    virtual TokenId getToken(const char* name);

  	    virtual void* getElement(const char* name) const { return NULL; }

        virtual std::string toString() const;

  	protected:
  	    EvalContext* m_parent;
  	    std::map<std::string,ConstrainedVariableId> m_variables;
  	    std::map<std::string,TokenId> m_tokens;
  };

  class Expr
  {
  	public:
        virtual DataRef eval(EvalContext& context) const = 0;
        virtual ~Expr(){}

        virtual std::string toString() const { return "Expr"; }
  };

  class ExprList : public Expr
  {
    public:
        ExprList();
        virtual ~ExprList();

        virtual DataRef eval(EvalContext& context) const;
        void addChild(Expr* child);
        const std::vector<Expr*>& getChildren();

        virtual std::string toString() const;

    protected:
        std::vector<Expr*> m_children;
  };

  class ExprNoop : public Expr
  {
    public:
        ExprNoop(const std::string& str);
        virtual ~ExprNoop();

        virtual DataRef eval(EvalContext& context) const;

        virtual std::string toString() const { return "ExprNoop:"+m_str; }

    protected:
        std::string m_str;
  };

}

#endif // _H_PDBInterpreter
