#include "SymbolTable.hh"

#include "Debug.hh"

#include <sstream>

namespace ANML
{
    Class::Class(const std::string& className,Class* parentClass)
        : m_name(className)
        , m_parent(parentClass)
    {
    }
    
    Class::~Class()
    {
    }
    
    std::string Class::toString() const
    {
    	std::ostringstream os;
        
        std::string parent = (m_parent != NULL ? (std::string(" extends ") + m_parent->getName()) : "");
        os << "class " << m_name << parent << "{" << std::endl;
        os << "}" << std::endl;
        
        return os.str();    	
    }

    SymbolTable::SymbolTable()
	{
		m_classes["object"] = new Class("object",NULL);
	}
	
    SymbolTable::~SymbolTable()
    {
    }

    void SymbolTable::addClass(const std::string& className,const std::string& parentClass)
    {
    	check_runtime_error(getClass(className) == NULL,"class "+className+" already defined");
    	    
    	Class* parent = getClass(parentClass);
    	check_runtime_error(parent != NULL,"parent class "+parentClass+" has not been defined");
    	    
    	m_classes[className] = new Class(className,parent);
    	debugMsg("SymbolTable","Added class:" << className);
    }
	
	Class* SymbolTable::getClass(const std::string& className)
	{
		std::map<std::string,Class*>::iterator it = m_classes.find(className);
		
		if (it != m_classes.end())
		    return it->second;
		   
		 return NULL;
	}
	
    std::string SymbolTable::toString() const
    {
    	std::ostringstream os;
    	
    	std::map<std::string,Class*>::const_iterator it = m_classes.begin();
    	
    	for(;it != m_classes.end(); ++it) {
    		os << it->second->toString();
    	}    	
    	
    	return os.str();
    }
}
