#include "ANMLTranslator.hh"

#include "Debug.hh"

#include <sstream>

namespace ANML
{	
    ANMLTranslator::ANMLTranslator()
    {
    	m_context = createGlobalContext();
    }
    
    ANMLTranslator::~ANMLTranslator()
    {
    	delete m_context;
    }
    
    ANMLContext* ANMLTranslator::createGlobalContext()
    {
    	ANMLContext* context = new ANMLContext();
    	
    	context->addObjType(new ObjType("object",NULL));
    	
    	return context;
    }
    
    void ANMLTranslator::pushContext(ANMLContext* context)
    {
    	context->setParent(m_context);
    	m_context = context;
    }
 
    void ANMLTranslator::popContext()
    {
    	m_context = (ANMLContext*)(m_context->getParent());
    	check_runtime_error(m_context != NULL,"ANMLTranslator context can't be NULL");
    }      
            
    void ANMLTranslator::toNDDL(std::ostream& os)
    {
    	return m_context->toNDDL(os);        
    }

    std::string ANMLTranslator::toString() const
    {
    	return m_context->toString();        
    }

    ANMLContext::ANMLContext(const ANMLContext* parent)
        : m_parent(parent)
	{
	}
	
    ANMLContext::~ANMLContext()
    {
    }

    void ANMLContext::addElement(ANMLElement* element)
    {
    	// TODO: temporarily ignore NULL elements until Translator is complete
    	// check_error(element != NULL, "Can't add a NULL element to an ANML context");
    	if (element != NULL) {
    	    m_elements.push_back(element);
    	    debugMsg("ANMLContext","Added element:" << element->getType());    	    
    	}
    	else
    	    std::cerr << "ERROR: tried to add null element to ANMLContext" << std::endl;    	    
    }

    ObjType* ANMLContext::addObjType(const std::string& className,const std::string& parentObjType)
    {
    	check_runtime_error(getObjType(className) == NULL,"class "+className+" already defined");
    	    
    	ObjType* parent = getObjType(parentObjType);
    	check_runtime_error(parent != NULL,"parent class "+parentObjType+" has not been defined");
    	
    	ObjType* newType = new ObjType(className,parent);
    	m_objTypes[className] = newType;
    	debugMsg("ANMLContext","Added class:" << className);
    	
    	return newType;
    }
	
    void ANMLContext::addObjType(ObjType* objType)
    {
    	m_objTypes[objType->getName()] = objType;
    	debugMsg("ANMLContext","Added class:" << objType->getName());
    }
	
	ObjType* ANMLContext::getObjType(const std::string& className) const
	{
		std::map<std::string,ObjType*>::const_iterator it = m_objTypes.find(className);
		
		if (it != m_objTypes.end())
		    return it->second;
		   
		 return NULL;
	}
	
    void ANMLContext::toNDDL(std::ostream& os)
    {
    	for (unsigned int i=0; i<m_elements.size(); i++) {
    	    m_elements[i]->toNDDL(os);
    	    os << std::endl;
    	}
    }
	
    std::string ANMLContext::toString() const
    {
    	std::ostringstream os;
    	
    	for (unsigned int i=0; i<m_elements.size(); i++) {
    		//debugMsg("ANMLContext", "toString:" << i << " " << m_elements[i]->getType()); 
    	    os << m_elements[i]->toString() << std::endl; 
    	}   	    
    	
    	return os.str();
    }

	Type::Type(const std::string& name)
	    : ANMLElement("TYPE",name)
	{
	}
	
	Type::~Type()
	{
	}
	        
    std::string Type::toString() const
    {
    	std::ostringstream os;
        
        os << m_name;
        
        return os.str();    	
    }

    Variable::Variable(const Type& dataType, const std::string& name)
        : ANMLElement("VARIABLE",name)
        , m_dataType(dataType)
    {
    }
    
    Variable::~Variable()
    {
    }

    void Variable::toNDDL(std::ostream& os)
    {
    	os << m_dataType.getName() << " " << m_name;
    }
    
    std::string Variable::toString() const
    {
    	std::ostringstream os;
        
        os << m_dataType.getName() << " " << m_name;
        
        return os.str();    	
    }

    ObjType::ObjType(const std::string& name,ObjType* parentObjType)
        : Type(name)
        , m_parent(parentObjType)
    {
    }
    
    ObjType::~ObjType()
    {
    }
    
    void ObjType::toNDDL(std::ostream& os)
    {
        std::string parent = ((m_parent != NULL && m_parent->getName() != "object") ? (std::string(" extends ") + m_parent->getName()) : "");
        os << "class " << m_name << parent << std::endl 
           << "{" << std::endl;
           
        for (unsigned int i=0; i<m_elements.size(); i++) {
    	    if (m_elements[i]->getType() == "ACTION") {
    	    	Action* a = (Action*) m_elements[i];
    	    	os << "    predicate " << a->getName() << " {";
    	    	
    	    	const std::vector<Variable*> params = a->getParams();
                for (unsigned int j=0; j<params.size(); j++) {
        	        params[j]->toNDDL(os);
       	            os << ";";
                }    	
    	    	
    	        os << "}" << std::endl;
    	    }
    	}
           
        os << "}" << std::endl;
        ANMLContext::toNDDL(os);
    }
    
    std::string ObjType::toString() const
    {
    	std::ostringstream os;
        
        std::string parent = (m_parent != NULL ? (std::string(" extends ") + m_parent->getName()) : "");
        os << "objtype " << m_name << parent << " {" << std::endl;
        os << ANMLContext::toString();
        os << "}";
        
        return os.str();    	
    }

    Action::Action(ObjType& objType,const std::string& name, const std::vector<Variable*>& params)
        : ANMLElement("ACTION",name)
        , m_objType(objType)        
        , m_params(params)
    {
    }
    
    Action::~Action()
    {
    }
    
    void Action::setBody(const std::vector<ANMLElement*>& body)
    {
    	m_body = body;
    }
    
    void Action::toNDDL(std::ostream& os)
    {
    	os << m_objType.getName() << "::" << m_name << "(";
    	os << ")" << std::endl;
        for (unsigned int i=0; i<m_params.size(); i++) {
        	if (i>0)
        	    os << ",";
        	m_params[i]->toNDDL(os);
        }    	
    	os << "{" << std::endl;
    	// TODO: generate NDDL for internal elements
    	os << "}" << std::endl;
    }
    
    std::string Action::toString() const
    {
    	std::ostringstream os;

        os << "action " << m_name << "(";
        for (unsigned int i=0; i<m_params.size(); i++) {
        	if (i>0)
        	    os << ",";
        	os << m_params[i]->toString();
        }
        os << ") {" << std::endl;
        
    	for (unsigned int i=0; i<m_body.size(); i++)
    	    os << m_body[i]->toString() << std::endl;
    	    
        os << "}";        
        
        return os.str();    	
    }
}
