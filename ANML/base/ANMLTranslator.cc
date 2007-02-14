#include "ANMLTranslator.hh"

#include "Debug.hh"

#include <sstream>

namespace ANML
{	
    ANMLTranslator::ANMLTranslator()
    {
    }
    
    ANMLTranslator::~ANMLTranslator()
    {
    }
      
    void ANMLTranslator::addObjType(const std::string& className,const std::string& parentObjType)
    {
    	ObjType* newType = m_symbolTable.addObjType(className,parentObjType);
    	m_elements.push_back(newType);
    }
      
    void ANMLTranslator::toNDDL(std::ostream& os)
    {
    	for (unsigned int i=0; i<m_elements.size(); i++)
    	    m_elements[i]->toNDDL(os);
    }
	
    std::string ANMLTranslator::toString() const
    {
    	std::ostringstream os;
        
        os << m_symbolTable.toString();
        
        return os.str();    	
    }

    ObjType::ObjType(const std::string& className,ObjType* parentObjType)
        : ANMLElement("OBJTYPE")
        , m_name(className)
        , m_parent(parentObjType)
    {
    }
    
    ObjType::~ObjType()
    {
    }
    
    std::string ObjType::toString() const
    {
    	std::ostringstream os;
        
        std::string parent = (m_parent != NULL ? (std::string(" extends ") + m_parent->getName()) : "");
        os << "objtype " << m_name << parent << " {" << std::endl;
        os << "}" << std::endl;
        
        return os.str();    	
    }

    SymbolTable::SymbolTable()
	{
		m_classes["object"] = new ObjType("object",NULL);
	}
	
    SymbolTable::~SymbolTable()
    {
    }

    ObjType* SymbolTable::addObjType(const std::string& className,const std::string& parentObjType)
    {
    	check_runtime_error(getObjType(className) == NULL,"class "+className+" already defined");
    	    
    	ObjType* parent = getObjType(parentObjType);
    	check_runtime_error(parent != NULL,"parent class "+parentObjType+" has not been defined");
    	
    	ObjType* newType = new ObjType(className,parent);
    	m_classes[className] = newType;
    	debugMsg("SymbolTable","Added class:" << className);
    	
    	return newType;
    }
	
	ObjType* SymbolTable::getObjType(const std::string& className)
	{
		std::map<std::string,ObjType*>::iterator it = m_classes.find(className);
		
		if (it != m_classes.end())
		    return it->second;
		   
		 return NULL;
	}
	
    std::string SymbolTable::toString() const
    {
    	std::ostringstream os;
    	
    	std::map<std::string,ObjType*>::const_iterator it = m_classes.begin();
    	
    	for(;it != m_classes.end(); ++it) {
    		os << it->second->toString();
    	}    	
    	
    	return os.str();
    }
}
