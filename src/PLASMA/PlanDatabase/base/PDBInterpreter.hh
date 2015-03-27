#ifndef H_PDBInterpreter
#define H_PDBInterpreter

#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include <map>
#include <vector>


namespace EUROPA {

  class Expr;

  class DataRef
  {
  	public :
  	    DataRef();
  	    DataRef(const ConstrainedVariableId v);
  	    virtual ~DataRef();

  	    const ConstrainedVariableId getValue();

  	    static DataRef null;

  	protected :
  	    ConstrainedVariableId m_value;
  };

class EvalContext {
 public:
  EvalContext(EvalContext* parent);
  virtual ~EvalContext();

  virtual void addVar(const std::string& name,const ConstrainedVariableId v);
  virtual ConstrainedVariableId getVar(const std::string& name);

  virtual void addToken(const std::string& name,const TokenId t);
  virtual TokenId getToken(const std::string& name);

  virtual void* getElement(const std::string& name) const;

  virtual std::string toString() const;

 protected:
  EvalContext* m_parent;
  std::map<std::string,ConstrainedVariableId> m_variables;
  std::map<std::string,TokenId> m_tokens;
 private:
  EvalContext(const EvalContext&);
  EvalContext& operator=(const EvalContext&);
};

  class Expr
  {
  	public:
        virtual DataRef eval(EvalContext& context) const = 0;
        virtual ~Expr(){}

        virtual const DataTypeId getDataType() const;

        virtual std::string toString() const;
  };

  class ExprList : public Expr
  {
    public:
        ExprList();
        virtual ~ExprList();

        virtual DataRef eval(EvalContext& context) const;
        void addChild(Expr* child);
        const std::vector<Expr*>& getChildren() const;

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

#endif // H_PDBInterpreter
