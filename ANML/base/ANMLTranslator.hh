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
      
      virtual void addObjType(const std::string& className,const std::string& parentObjType);
      
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
    virtual void toNDDL(std::ostream& nddl) {}
    virtual std::string toString() const { return "";};
    
  protected:
    ANMLElement(const std::string& type) : m_type(type) {}
    virtual ~ANMLElement() {}    
    
    std::string m_type;
};
	
class ObjType : public ANMLElement
{
  public:
    ObjType(const std::string& className,ObjType* parentObjType);
    virtual ~ObjType();
    
    ObjType* getParent() const { return m_parent; }
    const std::string& getName() const { return m_name; }
    
    virtual void toNDDL(std::ostream& nddl) {}
    
    virtual std::string toString() const;
    
  protected:
    std::string m_name;
    ObjType* m_parent;   	
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
