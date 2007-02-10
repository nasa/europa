#ifndef SYMBOLTABLE_H_
#define SYMBOLTABLE_H_

#include <string>
#include <map>

namespace ANML
{
	
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

class Class
{
public:
    Class(const std::string& className,Class* parentClass);
    virtual ~Class();
    
    Class* getParent() const { return m_parent; }
    const std::string& getName() const { return m_name; }
    
    std::string toString() const;
    
protected:
    std::string m_name;
    Class* m_parent;   	
};

class SymbolTable
{
public:
	SymbolTable();
	virtual ~SymbolTable();
	
	virtual void addClass(const std::string& className,const std::string& parentClass);
	virtual Class* getClass(const std::string& className);
	
	virtual std::string toString() const;
	
protected:
    std::map<std::string,Class*> m_classes;
};

}

#define check_runtime_error(cond,msg) \
if (!(cond)) \
    throw RuntimeException(msg);\

#endif /*SYMBOLTABLE_H_*/
