#include "ANMLTranslator.hh"

#include "Debug.hh"
#include "LabelStr.hh"

#include <sstream>

namespace ANML
{	
    ANMLTranslator::ANMLTranslator()
    {
    	m_context = createGlobalContext();
    	m_plannerConfig = new LHSPlannerConfig();
    }
    
    ANMLTranslator::~ANMLTranslator()
    {
    	delete m_context;
    	delete m_plannerConfig;
    }
    
    ANMLContext* ANMLTranslator::createGlobalContext()
    {
    	ANMLContext* context = new ANMLContext();
    	
    	context->addType(Type::BOOL);
    	context->addType(Type::INT);    	
    	context->addType(Type::FLOAT);
    	context->addType(Type::STRING);    	   	
    	context->addType(Type::OBJECT);
    	
    	// start and end of the planning horizon
    	context->addVariable(new Variable(*Type::INT,"start"));
    	context->addVariable(new Variable(*Type::INT,"end"));
    	
    	{
    		std::vector<Arg*> args;
    		args.push_back(new Arg("start_horizon",*Type::INT));
    		args.push_back(new Arg("end_horizon"  ,*Type::INT));    		
    	    context->addVariable(new Variable(*Type::BOOL,"PlanningHorizon",args));
    	}
    	
    	{
    		std::vector<Arg*> args;
    		args.push_back(new Arg("max_steps",*Type::INT));
    		args.push_back(new Arg("max_depth",*Type::INT));
    	    context->addVariable(new Variable(*Type::BOOL,"PlannerConfig",args));
    	}
    	
    	{
    		std::vector<Arg*> args;
    		args.push_back(new Arg("resource",*Type::INT));
    		args.push_back(new Arg("quantity",*Type::INT));
    	    context->addVariable(new Variable(*Type::BOOL,"uses",args));
    	}
    	
    	{
    		std::vector<const Type*> argTypes;
    		argTypes.push_back(Type::INT);
    		argTypes.push_back(Type::INT);
    	    context->addConstraint(new ConstraintDef("eq",argTypes));
    	}
    	
    	{
    		std::vector<const Type*> argTypes;
    		argTypes.push_back(Type::STRING);
    		argTypes.push_back(Type::STRING);
    	    context->addConstraint(new ConstraintDef("eq",argTypes));
    	}

    	{
    		std::vector<const Type*> argTypes;
    		argTypes.push_back(Type::INT);
    		argTypes.push_back(Type::INT);
    	    context->addConstraint(new ConstraintDef("neq",argTypes));
    	}
    	
    	{
    		std::vector<const Type*> argTypes;
    		argTypes.push_back(Type::STRING);
    		argTypes.push_back(Type::STRING);
    	    context->addConstraint(new ConstraintDef("neq",argTypes));
    	}
    	
    	return context;
    }
    
    void ANMLTranslator::pushContext(ANMLContext* context)
    {
    	context->setParentContext(m_context);
    	m_context = context;
    }
 
    void ANMLTranslator::popContext()
    {
    	m_context = (ANMLContext*)(m_context->getParentContext());
    	check_runtime_error(m_context != NULL,"ANMLTranslator context can't be NULL");
    }      
            
    void ANMLTranslator::toNDDL(std::vector<ANML::ANMLElement*>& program, std::ostream& os) const
    {
    	for (unsigned int i=0; i<program.size(); i++) 
    	    program[i]->preProcess(*m_context);

        std::vector<std::string> problems;
    	for (unsigned int i=0; i<program.size(); i++) 
    	    program[i]->validate(*m_context,problems);
        
        if (problems.size() == 0) {    	        	
        	m_plannerConfig->toNDDL(*m_context,os);    	
        	for (unsigned int i=0; i<program.size(); i++) 
        	    program[i]->toNDDL(*m_context,os);
        }   
        else {
        	// TODO: Return problems instead
        	std::cerr << "Validation ERRORS translating into NDDL:" << std::endl;
        	for (unsigned int i=0; i<problems.size(); i++) 
        	    std::cerr << problems[i] << std::endl;        	
        } 	            
    }

    std::string ANMLTranslator::toString() const
    {
    	std::ostringstream os;    
    	os << "ANMLTranslator {" << std::endl;
    	os << m_context->toString();
    	os << "}" << std::endl;            	
    	return os.str();
    }

    ANMLContext::ANMLContext(const ANMLContext* parent)
        : m_parentContext(parent)
	{
	}
	
    ANMLContext::~ANMLContext()
    {
    }

    ObjType* ANMLContext::addObjType(const std::string& name,const std::string& parentObjType)
    {
    	check_runtime_error(getType(name) == NULL,"data type "+name+" already defined");
    	    
    	ObjType* parent = getObjType(parentObjType);
    	check_runtime_error(parent != NULL,"parent class "+parentObjType+" has not been defined");
    	
    	ObjType* newType = new ObjType(name,parent);
    	addType(newType);
    	
    	return newType;
    }
	
    void ANMLContext::addType(Type* type)
    {
    	m_types[type->getName()] = type;
    	debugMsg("ANML:ANMLContext",getContextDesc() << " Added type:" << type->getName());
    }
	
	Type* ANMLContext::getType(const std::string& name,bool mustExist) const
	{
		std::map<std::string,Type*>::const_iterator it = m_types.find(name);
		
		if (it != m_types.end()) 
		    return it->second;
		
		if (m_parentContext != NULL)
		    return m_parentContext->getType(name,mustExist);
		       
		if (mustExist)
    		check_runtime_error(false, "Type "+name+" has not been defined");
		   
    	return NULL;
	}

	ObjType* ANMLContext::getObjType(const std::string& name) const
	{
		Type* type = getType(name);
		check_runtime_error(type != NULL, "Object type "+name+" has not been defined");
		check_runtime_error(!(type->isPrimitive()),name+" is a primitive type, not an Object type");
		   
		return (ObjType*)type;
	}
		
	void ANMLContext::addAction(Action* a)
	{
    	check_runtime_error(getAction(a->getName()) == NULL,"Action "+a->getName()+" already defined");
    	check_runtime_error(getVariable(a->getName()) == NULL,"Variable "+a->getName()+" already defined, can't define an Action with the same name");
		m_actions[a->getName()] = a;
    	debugMsg("ANML:ANMLContext",getContextDesc() << " Added action:" << a->getName());		
	}
	
	Action* ANMLContext::getAction(const std::string& name,bool mustExist) const
	{
		std::map<std::string,Action*>::const_iterator it = m_actions.find(name);
		
		if (it != m_actions.end()) 
		    return it->second;
		   
		if (m_parentContext != NULL)
		    return m_parentContext->getAction(name,mustExist);
		       
		if (mustExist)
    		check_runtime_error(false, "Action "+name+" has not been defined");
		   
    	return NULL;
	}
	
	void ANMLContext::addVariable(Variable* v)
	{
    	check_runtime_error(getVariable(v->getName()) == NULL,"Variable "+v->getName()+" already defined");
    	check_runtime_error(getAction(v->getName()) == NULL,"Action "+v->getName()+" already defined, can't define a Variable with the same name");
		m_variables[v->getName()] = v;
    	debugMsg("ANML:ANMLContext",getContextDesc() << " Added variable:" << v->getName());		
	}
	
	Variable* ANMLContext::getVariable(const std::string& name,bool mustExist) const
	{
		std::map<std::string,Variable*>::const_iterator it = m_variables.find(name);
		
		if (it != m_variables.end()) 
		    return it->second;
		   
		if (m_parentContext != NULL)
		    return m_parentContext->getVariable(name,mustExist);
		       
		if (mustExist)
    		check_runtime_error(false, "Variable "+name+" has not been defined");
		   
    	return NULL;
	}
		
	std::string makeKey(const std::string& name,const std::vector<const Type*>& argTypes)
	{
		std::ostringstream os;
		
		os << name;
		
		for (unsigned int i=0;i<argTypes.size();i++) 
		    os << ":" << argTypes[i]->getName();
		
		return os.str();
	}
	 	
    void ANMLContext::addConstraint(ConstraintDef* c)
    {
    	std::string key = makeKey(c->getName(),c->getArgTypes());
    	check_runtime_error(getConstraint(c->getName(),c->getArgTypes()) == NULL,"Constraint "+key+" already defined");
    	m_constraints[key] = c;
    	debugMsg("ANML:ANMLContext",getContextDesc() << " Added Constraint " << key);
    }
    
    ConstraintDef* ANMLContext::getConstraint(const std::string& name,const std::vector<const Type*>& argTypes,bool mustExist) const
    {
    	std::string key = makeKey(name,argTypes);
		std::map<std::string,ConstraintDef*>::const_iterator it = m_constraints.find(key);
		
		if (it != m_constraints.end()) 
		    return it->second;
        // TODO: check for generic matches, like numeric vs int,float		    
		   
		if (m_parentContext != NULL)
		    return m_parentContext->getConstraint(name,argTypes,mustExist);
				     
		if (mustExist)
    		check_runtime_error(false, "Constraint "+key+" has not been defined");
		   
    	return NULL;
    }
	
    std::string ANMLContext::toString() const
    {
    	std::ostringstream os;
    	
        os << getContextDesc() << "{" << std::endl;
        // TODO
        os << "}" << std::endl;
    	
    	return os.str();
    }

    void ANMLElement::toNDDL(ANMLContext& context, std::ostream& os) const 
    { 
    	os << "    //" << m_elementType << " " << m_elementName << std::endl; 
    }
    
    std::string ANMLElement::toString() const 
    { 
    	std::ostringstream os;
    	ANMLContext context; 
    	toNDDL(context,os); 
    	return os.str(); 
    }
    
    void ANMLElementList::preProcess(ANMLContext& context)
    {
    	for (unsigned int i=0;i<m_elements.size();i++) {
    	    m_elements[i]->preProcess(context);
    	    debugMsg("ANML:ANMLElementList", getElementType() << "::" << getElementName() << " preProcessed " 
    	        << m_elements[i]->getElementType() << m_elements[i]->getElementName());    
        }
    }
    
    bool ANMLElementList::validate(ANMLContext& context,std::vector<std::string>& problems)
    {   
    	for (unsigned int i=0;i<m_elements.size();i++) {
    	    if (!m_elements[i]->validate(context,problems))
    	        return false;
    	    
    	    debugMsg("ANML:ANMLElementList", getElementType() << "::" << getElementName() << " validated " 
    	        << m_elements[i]->getElementType() << m_elements[i]->getElementName());    
    	}
    	    
    	return true;
    }

    void ANMLElementList::toNDDL(ANMLContext& context,std::ostream& os) const
    {
    	for (unsigned int i=0;i<m_elements.size();i++)
    	    m_elements[i]->toNDDL(context,os);
    }

    std::string defaultValueSetterName="setValue";
        
    std::string makeValueSetterName(int idx)
    {
    	std::ostringstream os;
    	os << defaultValueSetterName << "_" << idx;
    	
    	return os.str();
    }
        
    ValueSetter::ValueSetter(int idx)
        : m_name(makeValueSetterName(idx))
    {
    }
    
    ValueSetter::~ValueSetter()
    {
    }
    
    void ValueSetter::addExplanatoryAction(Action* a,TemporalQualifier* tq)
    {
    	check_error(a != NULL, "Explanatory Action can't be NULL");
    	m_explanatoryActions.push_back(ExplanatoryAction(a,tq));
    	ExplanatoryAction& ea = m_explanatoryActions[m_explanatoryActions.size()-1];
    	debugMsg("ANML:ValueSetter","added ExplanatoryAction:" << ea.getAction()->getName());
    }
    
    void ValueSetter::toNDDL(ANMLContext& context,std::ostream& os,const std::string& typeName) const
    {
    	//check_runtime_error(m_explanatoryActions.size() >= 1,"Must not generate setter if there are no actions that can cause it");
    	
    	// TODO: generate comment with objType::varName
    	os << "// ValueSetter for TODO:objType::varName" << std::endl
           << typeName << "::" << getName() << std::endl
           << "{" << std::endl
           // TODO: if there is only one action don't generate disjunction
           << "    int option = [1 " << m_explanatoryActions.size() << "];" << std::endl;
        
        for (unsigned int i=0;i<m_explanatoryActions.size();i++) {
        	// TODO: generate obj parameter in predicate declaration
        	// TODO: generate appropriate operator depending on temporal qualifier
        	os << "    if (option == " << (i+1) << ") {" << std::endl
        	   << "        met_by(obj." << m_explanatoryActions[i].getAction()->getName() << ");" << std::endl
        	   << "    }" << std::endl;
        }
        
        os << "}" << std::endl << std::endl;
    }
    
    Type* Type::VOID   = new Type("void");
    Type* Type::BOOL   = new Type("bool");
    Type* Type::INT    = new Type("int");
    Type* Type::FLOAT  = new Type("float");
    Type* Type::STRING = new Type("string");
    ObjType* Type::OBJECT = new ObjType("object",NULL);
    
	Type::Type(const std::string& name)
	    : m_typeName(name)
	{
	}
	
	Type::~Type()
	{
	}
	
	void Type::becomeResourceType()
	{
		check_runtime_error(ALWAYS_FAIL,m_typeName + " can't become a resource type");
	}
	
	std::string makeValueSetterKey(const std::string& objTypeName,const std::string& varName)
	{
		if (objTypeName != "")
		    return objTypeName+"::"+varName;
		else
		    return "default";
	}
	    
    ValueSetter* Type::getValueSetter(const ObjType* objType,const std::string& varName)
    {    	
    	const std::string& objTypeName = (objType != NULL ? objType->getName() : "");
    	std::string key = makeValueSetterKey(objTypeName,varName);
    	std::map<std::string,ValueSetter*>::iterator it = m_valueSettersByKey.find(key);
    	
    	if (it != m_valueSettersByKey.end()) {
    		return it->second;
    	}
    	else {
        	ValueSetter* retval = new ValueSetter(m_valueSetters.size()+1);
        	m_valueSetters.push_back(retval);
        	m_valueSettersByKey[key] = retval;
        	debugMsg("ANML:Type","Added value setter " << key << " to " << getName());    	
            return retval;
    	}
    }
    
    const std::string& Type::getValueSetterName(const ObjType* objType,const std::string& varName) const
    {
    	const std::string& objTypeName = (objType != NULL ? objType->getName() : "");
    	std::string key = makeValueSetterKey(objTypeName,varName);
    	std::map<std::string,ValueSetter*>::const_iterator it = m_valueSettersByKey.find(key);
    	
    	if (it != m_valueSettersByKey.end()) {
    		debugMsg("ANML:Type","Found ValueSetterName for " << key << " returning " << it->second->getName()); 
    		return it->second->getName();
    	}
    	else {
    		debugMsg("ANML:Type","Didn't find ValueSetterName for " << key << " returning defaultValueSetterName:" << defaultValueSetterName); 
    		return defaultValueSetterName;
    	}
    }	
    
    void Type::generateCopier(std::ostream& os,
                              const std::string& lhs,
                              const std::string& predName,
                              const std::string& tokenName,
                              const std::string& rhs) const   
    {
    	check_error(ALWAYS_FAIL,"generateCopier not supported for type:" + getName());
    }
	        
    void Type::toNDDL(ANMLContext& context,std::ostream& os) const 
    { 
    	for (unsigned int i=0;i<m_valueSetters.size();i++)    	
    	    m_valueSetters[i]->toNDDL(context,os,getName()); 
    }
   
    std::string autoIdentifier(const char* base)
    {
    	static int cnt=0;
    	std::ostringstream os;
        
        os << base << "_" << cnt++;
        
        return os.str();    	
    }
    
	TypeAlias::TypeAlias(const std::string& name,const Type& t) 
	    : Type(name)
	    , m_wrappedType(t) 
	{
	}
	
	TypeAlias::~TypeAlias() 
	{
	}
	    	    
    void TypeAlias::toNDDL(ANMLContext& context, std::ostream& os) const 
    { 
    	os << "typedef " << m_wrappedType.getName() << " " << m_typeName << ";" << std::endl; 
    }    
    
	Range::Range(const std::string& name,const Type& dataType,const std::string& lb,const std::string& ub)	
	    : Type(name!="" ? name : autoIdentifier("Range"))
	    , m_dataType(dataType)
	    , m_lb(lb)
	    , m_ub(ub)
	{
		// TODO: convert lb,ub to typed values
	}
	
	Range::~Range()
	{
	}

	bool Range::canBeResourceType() const
	{
		return &m_dataType == Type::FLOAT;
	}

	void Range::becomeResourceType()
	{
		check_runtime_error(canBeResourceType(),m_typeName + " can't become a resource type. only a float range can.");
		m_isResourceType = true;
	}	        
	        
    void Range::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	if (m_isResourceType) {
    		os << "class " << getName() << " extends Reusable" << std::endl
    		   << "{" << std::endl
    		   << "    " << getName() << "()" << std::endl
    		   << "    {" << std::endl
    		   << "        super(" << m_ub << "," << m_lb << ");" << std::endl
    		   << "    }" << std::endl
    		   << "}" << std::endl << std::endl;    		
    	}
    	else {
    		// TODO: this isn't right, still need to generate Timeline
            os << "typedef " << m_dataType.getName() << " [" << m_lb << " " << m_ub <<  "] " << getName() << ";"<< std::endl << std::endl;
            // TODO: Type::toNDDL(context,os);                                
    	}      
    }
    
	Enumeration::Enumeration(const std::string& name,const Type& dataType,const std::vector<Expr*>& values)
	    : Type(name!="" ? name : autoIdentifier("Enumeration"))
	    , m_dataType(dataType)
	    , m_values(values)
	{
	}
	
	Enumeration::~Enumeration()
	{
	}
	        
    void Enumeration::toNDDL(ANMLContext& context, std::ostream& os) const
    {
        os << "enum " << getName() << "Enum {";
        
        for (unsigned int i=0;i<m_values.size(); i++) {
            if (i>0)
                os << ",";
            // TODO: translate different types differently?    
            os << m_values[i]->toString();
        }   
        os << "};" << std::endl << std::endl;
        
        os << "class " << getName() << " extends Timeline" << std::endl;
        os << "{" << std::endl;
        os << "    predicate setValue { " << getName() << "Enum value; }" << std::endl;     
        os << "}" << std::endl << std::endl;
        
        os << getName() << "::setValue {}" << std::endl << std::endl;
        
        Type::toNDDL(context,os);                                
    }
    
    ObjType::ObjType(const std::string& name,ObjType* parentObjType)
        : Type(name)
        , m_parentObjType(parentObjType)
    {
    }
    
    ObjType::~ObjType()
    {
    }
    
    void ObjType::declareValueSetters(std::ostream& os) const
    {
        for (unsigned int i=0; i < m_valueSetters.size(); i++) {
            os  << "    predicate " << m_valueSetters[i]->getName() << " { ";    	

            std::map<std::string,Variable*>::const_iterator varIt = m_variables.begin();              
            for (; varIt != m_variables.end() ; ++varIt) {
    	        Variable* v = varIt->second;
    	        os << " " << v->getDataType().getName() << " " << v->getName() << ";";
            }
        
            os << " }" << std::endl;
        }     	
    }
    
    void ObjType::toNDDL(ANMLContext& context, std::ostream& os) const
    {
        std::string parent = (
            (m_parentObjType != NULL && m_parentObjType->getName() != "object") 
                ? (std::string(" extends ") + m_parentObjType->getName()) 
                : ""
        );
        
        os << "class " << getName() << parent << std::endl 
           << "{" << std::endl;
        
        // Declare Variables
        std::map<std::string,Variable*>::const_iterator varIt = m_variables.begin();              
        for (; varIt != m_variables.end() ; ++varIt) {
    	    Variable* v = varIt->second;
    	    os << "    " << v->getDataType().getName() << " " << v->getName() << ";" << std::endl;
        }

        // Declare Actions
        std::map<std::string,Action*>::const_iterator actIt = m_actions.begin();              
        for (; actIt != m_actions.end() ; ++actIt) {
    	    	Action* a = actIt->second;
    	    	os << "    predicate " << a->getName() << " {";
    	    	
    	    	const std::vector<Variable*>& params = a->getParams();
                for (unsigned int j=0; j<params.size(); j++) {
        	        params[j]->toNDDL(context,os);
       	            os << ";";
                }    	
    	    	
    	        os << "}" << std::endl;
    	}
    	
    	declareValueSetters(os);
        
        // Generate constructor and initialize members that require contructors themselves
        os << std::endl << "    " << getName() << "()" << std::endl 
           << "    {" << std::endl;
        varIt = m_variables.begin();              
        for (; varIt != m_variables.end() ; ++varIt) {
   	    	Variable* v = varIt->second;
   	    	if (!v->getDataType().isPrimitive()) {
   	    	    os << "        " << v->getName() 
   	    	       << " = new " << v->getDataType().getName() << "();" << std::endl;
   	    	}
        }
        os << "    }" << std::endl;
           
        os << "}" << std::endl << std::endl;
        
        for (unsigned int i=0; i<m_elements.size(); i++) {
    	    if (m_elements[i]->getElementType() != "VAR_DECLARATION") 
    	        m_elements[i]->toNDDL(context,os);
        }   
        
        Type::toNDDL(context,os); 	     
    }
 
    VectorType::VectorType(const std::string& name)
        : ObjType(name,Type::OBJECT)
    {
    }
    
    void VectorType::generateCopier(std::ostream& os,
                              const std::string& lhs,
                              const std::string& predName,
                              const std::string& tokenName,
                              const std::string& rhs) const   
    {
    	// TODO: do it differently depending on whether it's a condition/effect or a goal
    	std::string ident = "    ";
        std::map<std::string,Variable*>::const_iterator varIt = m_variables.begin();              
        for (; varIt != m_variables.end() ; ++varIt) {
    	    Variable* v = varIt->second;
    	    os << ident << "eq(" << tokenName << "." << v->getName() << "," << rhs << "." << v->getName() << ");" << std::endl;
        }
    }
	        

    void VectorType::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	os << "class " << getName() << " extends Timeline" << std::endl
           << "{" << std::endl;
        
        declareValueSetters(os);   
        
        os << "}" << std::endl << std::endl;
                
        Type::toNDDL(context,os);                                                         
    }
    
    Variable::Variable(const Type& dataType, const std::string& name)
        : m_name(name)
        , m_dataType(dataType)
    {
    }

    Variable::Variable(const Type& dataType, const std::string& name, const std::vector<Arg*>& args)
        : m_name(name)
        , m_dataType(dataType)
        , m_args(args)
    {
    }

    Variable::~Variable()
    {
    }

    void Variable::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	os << m_dataType.getName() << " " << m_name;
    }
    
    const std::string VarInit::getValue() const 
    { 
    	return (m_value != NULL ? m_value->toString() : ""); 
    }
    
    VarDeclaration::VarDeclaration(const Type& type, const std::vector<VarInit*>& init)
        : ANMLElement("VAR_DECLARATION")
        , m_dataType(type)
        , m_init(init)
    {
    }
    
    VarDeclaration::~VarDeclaration()
    {
    }

    void VarDeclaration::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	os << m_dataType.getName() << " ";
    	for (unsigned int i=0; i < m_init.size(); i++) {
    		if (i>0)
    		    os << "," << std::endl << "    ";
    		os << m_init[i]->getName();
    		if (m_init[i]->getValue().length() > 0) {
    			os << "=" << m_init[i]->getValue();
    		}
    		else if (!m_dataType.isPrimitive()) {
    			os << " = new " << m_dataType.getName() << "()";
    		}
    	}
    	os << ";" << std::endl << std::endl;
    }
    
    FreeVarDeclaration::FreeVarDeclaration(const std::vector<Variable*>& vars)
        : ANMLElement("FREE_VAR_DECLARATION")
        , m_vars(vars)
    {
    }
    
    FreeVarDeclaration::~FreeVarDeclaration()
    {
    }

    void FreeVarDeclaration::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	for (unsigned int i=0; i < m_vars.size(); i++) 
    		os << "    " << m_vars[i]->getDataType().getName() << " " << m_vars[i]->getName() << ";" << std::endl;
    	
    	os << std::endl;	
    }
    
    Action::Action(ObjType& objType,const std::string& name, const std::vector<Variable*>& params)
        : m_name(name) 
        , m_objType(objType)        
        , m_params(params)
    {
    	m_elementType="ACTION";
    }
    
    Action::~Action()
    {
    }
    
    void Action::setBody(const std::vector<ANMLElement*>& body)
    {
    	for (unsigned int i=0;i<body.size();i++)
    	    addElement(body[i]);
    }
    
    void Action::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	os << m_objType.getName() << "::" << getName() << std::endl;

    	os << "{" << std::endl;
        for (unsigned int i=0; i<m_elements.size(); i++) {
    		debugMsg("ANML:Action", getName() << " Action::toNDDL " << i << " " << m_elements[i]->getElementType()); 
        	m_elements[i]->toNDDL(context,os);
        }    	
    	os << "}" << std::endl << std::endl;
    }
    
    ActionDuration::ActionDuration(const std::vector<Expr*>& values)
        : ANMLElement("ACTION_DURATION")
        , m_values(values)
    {
    }
    
    ActionDuration::~ActionDuration()
    {    	
    }
    
    void ActionDuration::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	os << "    eq(duration,";
    	
    	if (m_values.size() == 2) 
    		os << "[" << m_values[0]->toString() << " " << m_values[1]->toString() << "]";
    	else 
    	    os << m_values[0]->toString();
    	    
    	os << ");" << std::endl  << std::endl;
    }    
    
    
    TemporalQualifier::TemporalQualifier(const std::string& op,const std::vector<Expr*>& args) 
        : m_operator(op)
        , m_args(args)
        , m_argValues(args.size())
    {
    }
    
    TemporalQualifier::~TemporalQualifier() 
    {
    }
     
    void TemporalQualifier::toNDDL(std::ostream& os,Proposition::Context context) const 
    {
        // Evaluate all args, cache var names if necessary.
        for (unsigned int i=0;i<m_args.size();i++) {
            if (m_args[i]->needsVar()) {
            	std::string varName = autoIdentifier("_v");
            	m_argValues[i] = varName;
            	m_args[i]->toNDDLasExpr(os,varName);
            }
            else {
            	m_argValues[i] = m_args[i]->toString();
            }
        }                 
    }
     
    void TemporalQualifier::toNDDL(std::ostream& os, const std::string& ident,const std::string& fluentName) const 
    { 
  		// TODO: determine context for all time points, eval expr if necessary

    	if (m_operator == "at") {
    		const std::string& timePoint = m_argValues[0];
    		os << ident << "leq(" << fluentName << ".start," << timePoint << ");" << std::endl;
    		os << ident << "leq(" << timePoint << "," << fluentName << ".end);"<< std::endl;    		
    	}
    	else if (m_operator == "over") {
    		const std::string& lb = m_argValues[0];
    		const std::string& ub = m_argValues[1];
    		os << ident << "leq(" << fluentName << ".start," << lb << ");" << std::endl;
    		os << ident << "leq(" << ub << "," << fluentName << ".end);" << std::endl;    		                		
    	}
    	else if (m_operator == "in") {
    		const std::string& lb = m_argValues[0];
    		const std::string& ub = m_argValues[1];
    		os << ident << "leq(" << lb << "," << fluentName << ".start);" << std::endl;
    		os << ident << "leq(" << fluentName << ".end," << ub << ");" << std::endl;    		                		
    	}
    	else if (m_operator == "after") {
    		const std::string& timePoint = m_argValues[0];
    		os << ident << "leq(" << timePoint << "," << fluentName << ".start);" << std::endl;
    	}
    	else if (m_operator == "before") {
    		const std::string& timePoint = m_argValues[0];
    		os << ident << "leq(" << fluentName << ".end," << timePoint << ");" << std::endl;
    	}
    	// TODO: should contains operate with a single point? it is currently the same as over
    	else if (m_operator == "contains") {
    		const std::string& lb = m_argValues[0];
    		const std::string& ub = m_argValues[1];
    		os << ident << "leq(" << fluentName << ".start," << lb << ");" << std::endl;
    		os << ident << "leq(" << ub << "," << fluentName << ".end);" << std::endl;
    	}
    	
    	os << std::endl;
    }
    
    RelationalFluent::RelationalFluent(LHSExpr* lhs,Expr* rhs) 
        : m_lhs(lhs)
        , m_rhs(rhs) 
    {
    	debugMsg("ANML:RelationalFluent","Created " << toString());
    }
    
    RelationalFluent::~RelationalFluent() 
    {
    }
     
    void RelationalFluent::preProcess(ANMLContext& context) 
    {
    	// TODO: make sure nddl for Functions and Predicates is generated so that this doesn't break    	
    	
    	if (m_parentProp->getContext() == Proposition::EFFECT &&
    	    m_lhs->isVariableExpr()) {
    	    	
        	Type* t = context.getType(m_lhs->getDataType().getName(),true);
        	// TODO: capture varName when lhs is parsed
        	EUROPA::LabelStr ls(m_lhs->toString());
    	    unsigned int cnt = ls.countElements(".");
    	    EUROPA::LabelStr varName = ls.getElement(cnt-1,".");
    	    
    	    ValueSetter* vs = t->getValueSetter(m_lhs->getVariable()->getObjType(),varName.toString());
    	    vs->addExplanatoryAction(m_parentProp->getParentAction(),m_parentProp->getTemporalQualifier());
    	}
    	
    	// TODO: do we need to do anything else for GOAL, FACT or CONDITION?
    	// TODO: how do we generate termination axioms??
    	// TODO: ValueSetter::toNDDL must take the value as a parameter and generate the set of explanatory axioms
    	// with the appropriate temporal qualifiers
    	
    	// TODO: need to eliminate copiers, only setValue can be used
    	debugMsg("ANML:RelationalFluent","preProcessed " << toString());
    }
    
    // TODO: temporal qualifier should not be passed in a parameter, fluent can go to the parent proposition for it    
    void RelationalFluent::toNDDL(std::ostream& os, TemporalQualifier* tq) const 
    {
    	if (m_lhs->toString() == "PlannerConfig")
    	    return;
    	    
    	std::string ident = (m_parentProp->getContext() == Proposition::GOAL || 
    	                     m_parentProp->getContext() == Proposition::FACT ? "" : "    ");
    	 
    	// A token is always created for lhs                     
   	    std::string tokenName = autoIdentifier("_v");   	    
    	m_lhs->toNDDLasLHS(os,m_parentProp->getContext(),tokenName);
   		   		
    	if (m_rhs != NULL) {
    	    m_rhs->toNDDLasRHS(os,m_parentProp->getContext(),m_lhs,tokenName);
    	}
    	else {
    		// TODO: if lhs is an action, then it's ok to do nothing
    		// if it's a predicate, we need to generate the assignment to TRUE
    	}
    	
   		tq->toNDDL(os,ident,tokenName);    	     
    }    
    
    std::string RelationalFluent::toString() const
    {
    	std::ostringstream os;
    	
    	os << "RelationalFluent : " << m_lhs->toString() << (m_rhs != NULL ? "=" + m_rhs->toString() : "");
    	
    	return os.str();
    }
    
    std::string CompositeFluent::toString() const { return m_op + " " + m_lhs->toString() + " " + m_rhs->toString(); }
    std::string TransitionChangeFluent::toString() const { return "transition change : " + m_var->toString(); }
    std::string ResourceChangeFluent::toString() const { return m_op + " " + m_var->toString() + " " + m_qty->toString(); }
            
    Constraint::Constraint(const std::string& name,const std::vector<ANML::Expr*>& args) 
        : m_name(name)
        , m_args(args)
    {
    } 
       
    Constraint::~Constraint() 
    {
    }
    
    void Constraint::toNDDL(std::ostream& os, TemporalQualifier* tq) const
    {
    	// TODO: deal with the temporal qualifier??
    	os << "    " << m_name << "(";
    	
    	for (unsigned int i=0;i<m_args.size();i++) {
    		if (i>0)
    		    os << ",";
    		os << m_args[i]->toString();
    	}    	
    	
    	os << ");" << std::endl;
    }

    Condition::Condition(const std::vector<Proposition*>& propositions) 
        : PropositionSet(propositions,NULL,Proposition::CONDITION,"CONDITION")
    {
    }
    
    Condition::~Condition() 
    {
    }
    
    Effect::Effect(const std::vector<Proposition*>& propositions,Action* parentAction) 
        : PropositionSet(propositions,parentAction,Proposition::EFFECT,"EFFECT")
    {
    }
    
    Effect::~Effect() 
    {
    }
    
    Decomposition::Decomposition() 
        : ANMLElement("DECOMPOSITION") 
    {
    }
    
    Decomposition::~Decomposition() 
    {
    }    
    
    void Decomposition::addActionSet(ActionSet* as, TemporalQualifier* tq) 
    {
    	as->setTemporalQualifier(tq);
    	m_actionSets.push_back(as); 
    }

    void Decomposition::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	std::string ident="    ";
        
    	os << "    // Decomposition start" << std::endl;
    	for (unsigned int i=0;i<m_actionSets.size();i++) {
    		m_actionSets[i]->toNDDL(context,os,ident);
    	}
    	
    	for (unsigned int i=0;i<m_constraints.size();i++) {
    		m_constraints[i]->toNDDL(os,NULL);
    	}
    	os << "    // Decomposition end" << std::endl;
    }
    

    ActionSet::ActionSet(const std::string& op,const std::vector<ANML::ActionSetElement*>& elements)
        : m_operator(op)
        , m_tq(NULL)
        , m_elements(elements)
        , m_type(autoIdentifier("ActionSet"))
    {    	
        m_label = autoIdentifier("_ActionSet");
    }
    
    ActionSet::~ActionSet()
    {
    }

    void ActionSet::toNDDL(ANMLContext& context, std::ostream& os, const std::string& ident) const
    {
    	os << ident << "any(Decomposition " << getLabel() << ");" << std::endl;
    	
    	if (m_operator == "or") {
    		check_error(m_elements.size() == 2, "OR decomposition must have exactly 2 branches");
    		std::string varName = autoIdentifier("_v");
    		os << ident << "int " << varName << "= [0 1];" << std::endl;
    		for (unsigned int i=0;i<m_elements.size();i++) {
    		    os << ident << "if (" << varName << " == " << i << ") { " << std::endl;
    		    m_elements[i]->toNDDL(context,os,ident+"    ");
    		    os << ident << "   " << getLabel() << " contains " << m_elements[i]->getLabel() << ";" << std::endl;
    		    os << ident << "}" << std::endl;
    		} 
    	}
    	else {
    		for (unsigned int i=0;i<m_elements.size();i++) {
    		    m_elements[i]->toNDDL(context,os,ident);
    		    os << ident << getLabel() << " contains " << m_elements[i]->getLabel() << ";" << std::endl;
    		}
    		
    		std::string lastLabel = "";
    		for (unsigned int i=0;i<m_elements.size();i++) {
    		    if (m_operator == "ordered") {
    		    	if (lastLabel != "") 
    		    		os << ident << lastLabel << " before " << m_elements[i]->getLabel() << ";" << std::endl;
   		    		lastLabel = m_elements[i]->getLabel();
    		    }    			
    		}    		
    	}
    	
    	// Deal with temporal qualifier
    	if (m_tq != NULL) {
    		// TODO: this seems awkward, look into improving API for TemporalQualifier
    		m_tq->toNDDL(os,Proposition::EFFECT);
    	    m_tq->toNDDL(os,ident,getLabel());
    	}
    }
  
    SubAction::SubAction(ANML::LHSAction* action, const std::vector<ANML::Expr*>& args, const std::string& label)
        : m_action(action)
        , m_args(args)        
    {
    	if (label.length() == 0)
    	    m_label = autoIdentifier("_v");
    	else 
    	    m_label = label;
    }
    
    SubAction::~SubAction()
    {
    }

    const std::string SubAction::getType() const 
    { 
    	return m_action->toString(); 
    }

    void SubAction::toNDDL(ANMLContext& context, std::ostream& os, const std::string& ident) const
    {
    	os << ident << "any(" << m_action->toString() << " " << m_label << ");" << std::endl;
   		for (unsigned int i=0;i<m_args.size();i++) {  	
    	    // TODO: specify args
   		}
    }
    
    Goal::Goal(const std::vector<Proposition*>& propositions) 
        : PropositionSet(propositions,NULL,Proposition::GOAL,"GOAL")
    {
    }
    
    Goal::~Goal() 
    {
    }
    
    Fact::Fact(const std::vector<Proposition*>& propositions) 
        : PropositionSet(propositions,NULL,Proposition::FACT,"FACT")
    {
    }
    
    Fact::~Fact() 
    {
    }
    
    Proposition::Proposition(TemporalQualifier* tq,Fluent* f) 
       : m_temporalQualifier(tq)
    {
    	m_fluents.push_back(f);
    	commonInit();
    }
    
    Proposition::Proposition(TemporalQualifier* tq,const std::vector<Fluent*>& fluents) 
       : m_temporalQualifier(tq)
       , m_fluents(fluents) 
    {
    	commonInit();
    }
    
    void Proposition::commonInit()
    {
  	    debugMsg("ANML:Proposition","Creating  Proposition with " << m_fluents.size() << " fluents");
    	m_temporalQualifier->setProposition(this);
    	for (unsigned int i=0;i<m_fluents.size();i++) 
    	    m_fluents[i]->setProposition(this); 
    }
    
    Proposition::~Proposition() 
    {
    }
    
    void Proposition::preProcess(ANMLContext& context) 
    {
    	for (unsigned int i=0;i<m_fluents.size();i++) {
    		Fluent* f = m_fluents[i];
    	    f->preProcess(context);    	
        	debugMsg("ANML:Proposition","preProcessed:" << m_fluents[i]->toString());
    	}
    }
    
    bool Proposition::validate(ANMLContext& context,std::vector<std::string>& problems)
    {   
    	for (unsigned int i=0;i<m_fluents.size();i++) {
    	    if (!m_fluents[i]->validate(context,problems))
    	        return false;
        	debugMsg("ANML:Proposition","Validated:" << m_fluents[i]->toString());
    	}
    	    
    	return true;
    }
    
    void Proposition::toNDDL(ANMLContext& context, std::ostream& os) const 
    {
    	// output any expressions needed for the parameters of the temporal qualifier
    	m_temporalQualifier->toNDDL(os,m_context);
    	 
    	for (unsigned int i=0;i<m_fluents.size();i++) 
    	    m_fluents[i]->toNDDL(os,m_temporalQualifier);
    }
    
    PropositionSet::PropositionSet(const std::vector<Proposition*>& propositions,
                                   Action* parent,
                                   Proposition::Context context,
                                   const std::string& type) 
        : ANMLElement(type)
        , m_propositions(propositions) 
    {
    	for (unsigned int i=0;i<m_propositions.size();i++) { 
    		m_propositions[i]->setParentAction(parent);
    		m_propositions[i]->setContext(context);
    	}
    }
    
    PropositionSet::~PropositionSet()
    {
    	for (unsigned int i=0;i<m_propositions.size();i++) 
    		delete m_propositions[i];
    }
    
    void PropositionSet::preProcess(ANMLContext& context) 
    {
    	for (unsigned int i=0;i<m_propositions.size();i++) {
    	    m_propositions[i]->preProcess(context);    	
        	debugMsg("ANML:PropositionSet","preProcessed Proposition:" << i);
    	}
    }
    
    bool PropositionSet::validate(ANMLContext& context,std::vector<std::string>& problems)
    {   
    	for (unsigned int i=0;i<m_propositions.size();i++) {
    	    if (!m_propositions[i]->validate(context,problems))
    	        return false;
        	debugMsg("ANML:PropositionSet","validated Proposition:" << i);
    	}
    	    
    	return true;
    }
    
    void PropositionSet::toNDDL(ANMLContext& context, std::ostream& os) const 
    { 
    	os << "    // " << m_elementType << " start" << std::endl;
    	for (unsigned int i=0; i<m_propositions.size(); i++) {
    		m_propositions[i]->toNDDL(context,os);
    	}
    	os << "    // " << m_elementType << " end" << std::endl << std::endl;
    }
    
    Change::Change(Proposition* when_condition,Proposition* change_stmt) 
        : ANMLElement("CHANGE")
        , m_whenCondition(when_condition)
        , m_changeStmt(change_stmt)
    {
    }
   
    Change::~Change()
    {
       	if (m_whenCondition != NULL)
       	    delete m_whenCondition;
       	
       	delete m_changeStmt;    
    }
   
    void Change::toNDDL(ANMLContext& context, std::ostream& os) const
    {
        if (m_whenCondition != NULL) {
            // TODO: deal with this
            check_runtime_error(ALWAYS_FAIL,"When clause not supported for changes yet");
        }
       
        m_changeStmt->toNDDL(context,os);
    }    
        
    ResourceChangeFluent::ResourceChangeFluent(const std::string& op,Expr* var,Expr* qty)
        : m_op(op)
        , m_var(var)
        , m_qty(qty)
    {
    }
    
    ResourceChangeFluent::~ResourceChangeFluent()
    {
    }

    void ResourceChangeFluent::toNDDL(std::ostream& os, TemporalQualifier* tq) const
    {
    	os << "    // TODO:  generate NDDL for ResourceChangeFluent" << std::endl;
    	std::string v1 = autoIdentifier("_v");
    	os << "    contains(" << m_var->toString() << "." << m_op << " " << v1 << ");" << std::endl;    	  
    	os << "    eq(" << v1 << ".quantity," << (m_qty != NULL ? m_qty->toString() : "1.0") << ");" << std::endl;
    }


    TransitionChangeFluent::TransitionChangeFluent(Expr* var, const std::vector<Expr*>& states)
        : m_var(var)
        , m_states(states)
    {
    	
    }
    
    TransitionChangeFluent::~TransitionChangeFluent()
    {
    }

    void TransitionChangeFluent::toNDDL(std::ostream& os, TemporalQualifier* tq) const
    {
    	os << "    // TODO:  generate NDDL for TransitionChangeFluent " << std::endl;
    	os << "    // " << m_var->toString();
    	
    	for (unsigned int i=0;i<m_states.size();i++) {
    		std::string state = m_states[i]->toString(); 
    	    os << "->" << state;
    	}
    	
    	os << std::endl;        	
    }

    void LHSAction::toNDDLasLHS(std::ostream& os,Proposition::Context context,const std::string& varName) const 
    { 
    	switch (context) {
    		case Proposition::GOAL : 
    		    os << "goal(" << m_action->getName() << " " << varName << ");" << std::endl;
    		    break; 
    		case Proposition::CONDITION : 
    		    os << "    any(" << m_action->getName() << " " << varName << ");" << std::endl;
    		    break; 
    		case Proposition::FACT : 
    		    check_runtime_error(ALWAYS_FAIL,"ERROR! LHSAction not supported for FACTS");
    		    break; 
    		case Proposition::EFFECT : 
    		    check_runtime_error(ALWAYS_FAIL,"ERROR! LHSAction not supported for EFFECTS");
    		    break;
    		default:
    		    check_error(ALWAYS_FAIL,"Unexpected error");
    		    break;
    	}
    }          
    
    void LHSVariable::toNDDLasRHS(std::ostream& os,
                                  Proposition::Context context,
                                  LHSExpr* lhs,
                                  const std::string& tokenName) const 
    { 
    	std::string ident;
    	
    	// TODO: implement this correctly
    	if (context == Proposition::GOAL || context == Proposition::FACT) {
    	    ident = "";
    	    os << "goal(";
        }    
        else {
        	ident = "    ";
        	os << ident << "any(";
        }
        
        // TODO: store varName when this Expr is created        
       	EUROPA::LabelStr ls(lhs->toString());
  	    unsigned int cnt = ls.countElements(".");
  	    EUROPA::LabelStr varName = ls.getElement(cnt-1,".");
    	    
    	const std::string& predName = lhs->getVariable()->getDataType().getValueSetterName(lhs->getVariable()->getObjType(),varName.toString());    	
    	os << lhs->toString() << "." << predName << " " << tokenName << ");" << std::endl;
    	
        m_var->getDataType().generateCopier(os,lhs->toString(),predName,tokenName,m_path);
    }
    
    void ExprConstant::toNDDLasRHS(std::ostream& os,
                                   Proposition::Context context,
                                   LHSExpr* lhs,
                                   const std::string& tokenName) const 
    { 
    	std::string ident;
    	
    	// TODO: implement this properly
    	if (context == Proposition::GOAL || context == Proposition::FACT) {
    	    ident = "";
    	    os << "goal(";
        }    
        else {
        	ident = "    ";
        	os << ident << "any(";
        }
        
        os << lhs->toString() << ".setValue " << tokenName << ");" << std::endl
           << ident << "eq(" << tokenName << ".value," << toString() << ");" << std::endl << std::endl;
    }
    
    
   void ExprArithOp::toNDDLasExpr(std::ostream& os,const std::string& varName) const 
   {
       std::string op1,op2;
       
       if (m_op1->needsVar()) {
         op1 = autoIdentifier("_v");
         m_op1->toNDDLasExpr(os,op1);
       }
       else {
       	 op1 = m_op1->toString();
       }
       	
       if (m_op2->needsVar()) {
         op2 = autoIdentifier("_v");
         m_op2->toNDDLasExpr(os,op2);
       }
       else {
       	 op2 = m_op2->toString();
       }
       
       std::string ident="    ";
       if (m_operator == "-") {
       	   os << ident; getDataType().getName(); os << " " << varName << ";" << std::endl;
           // addEq(x,y,z) means x+y=z which implies x=z-y
           os << ident << "addEq(" << varName << "," << op2 << "," << op1 << ");" << std::endl << std::endl;
       }
       else {
           check_runtime_error(ALWAYS_FAIL,"Don't know how to deal with operator : " + m_operator);
       }
   }    
   
   ExprVector::ExprVector(const std::vector<Expr*>& values)
       : m_values(values) 
   {
   	   // TODO dataType needs to make sure there are no leaks
   	   m_dataType = new VectorType(autoIdentifier("VectorType_"));
       for (unsigned int i=0;i<m_values.size();i++)  {
       	   std::ostringstream name; name << "member_" << i;
       	   std::vector<VarInit*> init;
       	   init.push_back(new VarInit(new Variable(m_values[i]->getDataType(),name.str()),NULL));
       	   
           m_dataType->addElement(
             new VarDeclaration(
               m_values[i]->getDataType(),
               init             
             )  
           );
       }
   }
   
   ExprVector::~ExprVector() 
   {
       delete m_dataType;
   }
  
   std::string ExprVector::toString() const
   {
       std::ostringstream os;
    
       os << "(";   
       for (unsigned int i=0;i<m_values.size();i++) {
           if (i>0)
             os << ",";
           os << m_values[i]->toString();
       }
       os << ")" << std::endl;
       
       return os.str();
   }
    
   void ExprVector::toNDDLasRHS(std::ostream& os,
                                Proposition::Context context,
                                LHSExpr* lhs,
                                const std::string& tokenName) const
   {
   	   std::string ident = (context == Proposition::GOAL || 
    	                    context == Proposition::FACT ? "" : "    ");
    	                        	
       if (context == Proposition::GOAL || context == Proposition::FACT)	
           os << "goal(";
       else
           os << "any(";
           
       os << lhs->toString() << ".setValue " << tokenName << ");" << std::endl;
       
       VectorType* lhsType = (VectorType*)&(lhs->getDataType());       
       const std::vector<ANMLElement*>& elements = lhsType->getElements();
       int varIdx=0;
       for (unsigned int i=0; i<elements.size(); i++) {
    	    if (elements[i]->getElementType() == "VAR_DECLARATION") {
    	    	const VarDeclaration* vd = (const VarDeclaration*)elements[i];
        	    for (unsigned j=0;j<vd->getInit().size();j++) {
        	    	os << "eq(" << tokenName << "." << vd->getInit()[j]->getName()
        	    	   << "," << m_values[varIdx++]->toString() << ");" << std::endl;
        	    }    	    	
    	    }
        }    
        
        os << std::endl;                
   }   
    
    // Special expression to handle PlannerConfig
    LHSPlannerConfig::LHSPlannerConfig()
        : m_translated(false)
        , m_startHorizon(NULL)
        , m_endHorizon(NULL)
        , m_maxSteps(NULL)
        , m_maxDepth(NULL)
    {
    }
	
	LHSPlannerConfig::~LHSPlannerConfig()
	{
	}
	
    void LHSPlannerConfig::setArgs(const std::string& predicate,const std::vector<Expr*>& args)
    {
    	if (predicate == "PlannerConfig") {
    		m_maxSteps = args[0]; 
    		m_maxDepth = args[1]; 
    	}
    	else { // PlanningHorizon
    		m_startHorizon = args[0]; 
    		m_endHorizon = args[1]; 
    	}    	  
    }
    
    void LHSPlannerConfig::toNDDL(ANMLContext& context, std::ostream& os) const
    {
    	if (m_translated)
    	    return;
    	    
    	std::string sh = (m_startHorizon != NULL ? m_startHorizon->toString() : "0");
    	std::string eh = (m_endHorizon != NULL ? m_endHorizon->toString() : "1");
    	std::string ms = (m_maxSteps != NULL ? m_maxSteps->toString() : "+inf");
    	std::string md = (m_maxDepth != NULL ? m_maxDepth->toString() : "+inf");
     	
   	    os << "int start=" << sh << ";" << std::endl;
   	    os << "int end="   << eh << ";" << std::endl;
   	    os << "int solver_maxSteps=" << ms << ";" << std::endl;
   	    os << "int solver_maxDepth=" << md << ";" << std::endl;
    	
    	os << "PlannerConfig plannerConfiguration = new PlannerConfig(start,end,solver_MaxSteps,solver_maxDepth);" 
    	   << std::endl << std::endl;
    	   
    	m_translated = true;    	
    }            
}
