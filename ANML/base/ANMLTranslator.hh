#ifndef ANMLTRANSLATOR_H_
#define ANMLTRANSLATOR_H_

#include <map>
#include <string>
#include <vector>

namespace ANML
{

class ANMLElement;
class ObjType;

class SymbolTable
{
  public:
	SymbolTable();
	virtual ~SymbolTable();
	
	virtual ObjType* addObjType(const std::string& className,const std::string& parentObjType);
	virtual ObjType* getObjType(const std::string& className);
	
	virtual std::string toString() const;
	
  protected:
    std::map<std::string,ObjType*> m_classes;
};

class ANMLTranslator
{
  public:
      ANMLTranslator();
      virtual ~ANMLTranslator();
      
      virtual ObjType* addObjType(const std::string& className,const std::string& parentObjType);
      
      virtual void toNDDL(std::ostream& os);
      
	  virtual std::string toString() const;      
      
  protected:
      SymbolTable m_symbolTable;
      std::vector<ANMLElement*> m_elements;      
};

class ANMLElement
{
  public:
    const std::string& getType() { return m_type; }
    virtual void toNDDL(std::ostream& os) {}
    virtual std::string toString() const { return "";};
    
  protected:
    ANMLElement(const std::string& type) : m_type(type) {}
    virtual ~ANMLElement() {}    
    
    std::string m_type;
};

class Type : public ANMLElement
{
  public:
	Type(const std::string& name);
	virtual ~Type();
	
    const std::string& getName() const { return m_name; }
    
    virtual void toNDDL(std::ostream& os) {}
    
    virtual std::string toString() const;
    
  protected:
    std::string m_name;	  
};
	
class Variable : public ANMLElement
{
  public:
    Variable(const Type& type, const std::string& name);
    ~Variable();

    virtual void toNDDL(std::ostream& os);
    
    virtual std::string toString() const;
  
  protected:
	const Type& m_type;
	std::string m_name;
};
    
class ObjType : public Type
{
  public:
    ObjType(const std::string& className,ObjType* parentObjType);
    virtual ~ObjType();
    
    ObjType* getParent() const { return m_parent; }
    
    virtual void addElement(ANMLElement* element) { /* TODO: make this a SymbolTable?*/ }
    
    virtual void toNDDL(std::ostream& os) {}
    
    virtual std::string toString() const;
    
  protected:
    ObjType* m_parent;   	
};

class Action : public ANMLElement
{
  public:
    Action(const std::string& name,const std::vector<Variable*>& params);
    virtual ~Action();
    
    void setBody(const std::vector<ANMLElement*>& body);
  
    virtual void toNDDL(std::ostream& os) {}
    
    virtual std::string toString() const;
    
  protected:
    std::string m_name;
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
