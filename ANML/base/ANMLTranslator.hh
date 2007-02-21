#ifndef ANMLTRANSLATOR_H_
#define ANMLTRANSLATOR_H_

#include <map>
#include <string>
#include <vector>

namespace ANML
{

class ANMLContext;
class ANMLElement;
class Type;
class ObjType;

class ANMLTranslator
{
  public:
      ANMLTranslator();
      virtual ~ANMLTranslator();
      
      virtual void pushContext(ANMLContext* context);
      virtual void popContext();      

      virtual ANMLContext& getContext() { return *m_context; }
            
      virtual void toNDDL(std::ostream& os) const;
      
	  virtual std::string toString() const;      
      
  protected:
      ANMLContext* m_context;
      
      ANMLContext* createGlobalContext();          
};

class ANMLContext
{
  public:
	ANMLContext(const ANMLContext* parent=NULL);
	virtual ~ANMLContext();

    virtual const ANMLContext* getParent() { return m_parent; }
    virtual void setParent(const ANMLContext* parent) { m_parent = parent; }
    	
    virtual void addElement(ANMLElement* element);
    
	virtual void     addType(Type* type);
	virtual ObjType* addObjType(const std::string& name,const std::string& parentName);

	virtual Type*    getType(const std::string& name,bool mustExist=false) const;
	virtual ObjType* getObjType(const std::string& name) const;
	
    virtual void toNDDL(std::ostream& os) const;
    
	virtual std::string toString() const;
		
  protected:
    const ANMLContext* m_parent;
    std::vector<ANMLElement*> m_elements;      
    std::map<std::string,Type*> m_types;    
};

class ANMLElement
{
  public:
    ANMLElement(const std::string& type) : m_type(type) {}
    ANMLElement(const std::string& type,const std::string& name) : m_type(type), m_name(name) {}
    virtual ~ANMLElement() {}    

    virtual const std::string& getType() { return m_type; }
    virtual const std::string& getName() const { return m_name; }
    virtual void toNDDL(std::ostream& os) const { os << toString(); }
    virtual std::string toString() const { return m_type+" "+m_name;}
    
  protected:    
    std::string m_type;
    std::string m_name;	  
};

class Type : public ANMLElement
{
  public:
	Type(const std::string& name);
	virtual ~Type();
	    
	virtual bool isPrimitive() const { return true; }
	    
    virtual void toNDDL(std::ostream& os) const {}
    
    virtual std::string toString() const;
};

class Range : public Type
{
  public:
	Range(const Type& dataType,const std::string& lb,const std::string& ub);
	virtual ~Range();
	    
	virtual bool isPrimitive() const { return true; }
	    
    virtual void toNDDL(std::ostream& os) const;
    
    virtual std::string toString() const;

  protected:
    const Type& m_dataType;
    std::string m_lb;
    std::string m_ub;
};

class Enumeration : public Type
{
  public:
	Enumeration(const Type& dataType,const std::vector<std::string>& values);
	virtual ~Enumeration();
	    
	virtual bool isPrimitive() const { return m_dataType.isPrimitive(); }
	    
    virtual void toNDDL(std::ostream& os) const;
    
    virtual std::string toString() const;

  protected:
    const Type& m_dataType;
    std::vector<std::string> m_values;
};
	
class Variable : public ANMLElement
{
  public:
    Variable(const Type& type, const std::string& name);
    ~Variable();

    virtual void toNDDL(std::ostream& os) const;
    
    virtual std::string toString() const;
  
  protected:
	const Type& m_dataType;
};
    
class VarInit
{
  public:
      VarInit(const std::string& name, const std::string& value)
          : m_name(name)
          , m_value(value)
      {
      }
      
      const std::string& getName() const { return m_name; }
      const std::string& getValue() const { return m_value; }
      
  protected:
      std::string m_name;
      std::string m_value;	
};
    
class VarDeclaration : public ANMLElement
{
  public:
    VarDeclaration(const Type& type, const std::vector<VarInit*>& init);
    virtual ~VarDeclaration();

    virtual void toNDDL(std::ostream& os) const;
    
    virtual std::string toString() const;
  
  protected:
	const Type& m_dataType;
	std::vector<VarInit*> m_init;
};

class ObjType : public Type, public ANMLContext
{
  public:
    ObjType(const std::string& name,ObjType* parentObjType);
    virtual ~ObjType();
    
    ObjType* getParent() const { return m_parent; }
        
	virtual bool isPrimitive() const { return false; }

    virtual void toNDDL(std::ostream& os) const;
    
    virtual std::string toString() const;
    
  protected:
    ObjType* m_parent;   	
};

class Action : public ANMLElement, public ANMLContext
{
  public:
    Action(ObjType& objType,const std::string& name,const std::vector<Variable*>& params);
    virtual ~Action();
    
    const std::vector<Variable*> getParams() const { return m_params; }
    
    void setBody(const std::vector<ANMLElement*>& body);
  
    virtual void toNDDL(std::ostream& os) const;
    
    virtual std::string toString() const;
    
  protected:
    ObjType& m_objType;
    std::vector<Variable*> m_params;
    std::vector<ANMLElement*> m_body;
};

/*
class RuntimeException
{
  public:
    RuntimeException(const char* msg) : m_msg(msg) {}
    RuntimeException(const std::string& msg) : m_msg(msg) {}
    virtual ~RuntimeException() {}
    
    virtual std::string toString() const { return m_msg; }

  protected:
    std::string m_msg;    
};
*/	

}

/*
#define check_runtime_error(cond,msg) \
if (!(cond)) \
    throw RuntimeException(msg);\
*/

#endif /*ANMLTRANSLATOR_H_*/
