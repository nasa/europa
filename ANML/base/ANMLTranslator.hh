#ifndef ANMLTRANSLATOR_H_
#define ANMLTRANSLATOR_H_

#include <map>
#include <string>
#include <vector>

namespace ANML
{

class ANMLContext;
class ANMLElement;
class ObjType;

class ANMLTranslator
{
  public:
      ANMLTranslator();
      virtual ~ANMLTranslator();
      
      virtual void pushContext(ANMLContext* context);
      virtual void popContext();      

      virtual ANMLContext& getContext() { return *m_context; }
            
      virtual void toNDDL(std::ostream& os);
      
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
    
	virtual ObjType* getObjType(const std::string& name) const;
	virtual ObjType* addObjType(const std::string& name,const std::string& parentName);
	
    virtual void toNDDL(std::ostream& os);
    
	virtual std::string toString() const;
		
  protected:
    const ANMLContext* m_parent;
    std::vector<ANMLElement*> m_elements;      
    std::map<std::string,ObjType*> m_objTypes;
    
	void addObjType(ObjType* objType);
	
    friend class ANMLTranslator;
};

class ANMLElement
{
  public:
    ANMLElement(const std::string& type) : m_type(type) {}
    virtual ~ANMLElement() {}    

    const std::string& getType() { return m_type; }
    virtual void toNDDL(std::ostream& os) { os << m_type; }
    virtual const std::string& getType() const { return m_type; }
    virtual std::string toString() const { return m_type;};
    
  protected:
    
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
    
class ObjType : public Type, public ANMLContext
{
  public:
    ObjType(const std::string& className,ObjType* parentObjType);
    virtual ~ObjType();
    
    ObjType* getParent() const { return m_parent; }
        
    virtual void toNDDL(std::ostream& os) {}
    
    virtual std::string toString() const;
    
  protected:
    ObjType* m_parent;   	
};

class Action : public ANMLElement, public ANMLContext
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
