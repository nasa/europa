#include "ANMLTranslator.hh"

#include "Debug.hh"

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
    	
    	context->addType(new Type("bool"));
    	context->addType(new Type("int"));
    	context->addType(new Type("float"));
    	context->addType(new Type("string"));
    	context->addType(new ObjType("object",NULL));
    	
    	{
    		std::vector<Arg*> args;
    		const Type& intType = *(context->getType("int"));
    		args.push_back(new Arg("start_horizon",intType));
    		args.push_back(new Arg("end_horizon"  ,intType));
    		
    		const Type& boolType = *(context->getType("bool"));
    	    context->addVariable(new Variable(boolType,"PlanningHorizon",args));
    	}
    	
    	{
    		std::vector<Arg*> args;
    		const Type& intType = *(context->getType("int"));
    		args.push_back(new Arg("max_steps",intType));
    		args.push_back(new Arg("max_depth",intType));
    		
    		const Type& boolType = *(context->getType("bool"));
    	    context->addVariable(new Variable(boolType,"PlannerConfig",args));
    	}
    	
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
            
    void ANMLTranslator::toNDDL(std::ostream& os) const
    {
    	m_plannerConfig->toNDDL(os);
    	m_context->toNDDL(os);        
    }

    std::string ANMLTranslator::toString() const
    {
    	std::ostringstream os;    
    	toNDDL(os);            	
    	return os.str();
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
    	debugMsg("ANMLContext","Added type:" << type->getName());
    }
	
	Type* ANMLContext::getType(const std::string& name,bool mustExist) const
	{
		std::map<std::string,Type*>::const_iterator it = m_types.find(name);
		
		if (it != m_types.end()) 
		    return it->second;
		
		if (m_parent != NULL)
		    return m_parent->getType(name,mustExist);
		       
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
		// TODO: check name against functions as well?
		// TODO: warn if it hides an element in a parent context?
    	check_runtime_error(getAction(a->getName()) == NULL,"Action "+a->getName()+" already defined");
		m_actions[a->getName()] = a;
	}
	
	Action* ANMLContext::getAction(const std::string& name,bool mustExist) const
	{
		std::map<std::string,Action*>::const_iterator it = m_actions.find(name);
		
		if (it != m_actions.end()) 
		    return it->second;
		   
		if (m_parent != NULL)
		    return m_parent->getAction(name,mustExist);
		       
		if (mustExist)
    		check_runtime_error(false, "Action "+name+" has not been defined");
		   
    	return NULL;
	}
	
	void ANMLContext::addVariable(Variable* v)
	{
		// TODO: check name against actions as well?
		// TODO: warn if it hides an element in a parent context?
    	check_runtime_error(getVariable(v->getName()) == NULL,"Variable "+v->getName()+" already defined");
		m_variables[v->getName()] = v;
	}
	
	Variable* ANMLContext::getVariable(const std::string& name,bool mustExist) const
	{
		std::map<std::string,Variable*>::const_iterator it = m_variables.find(name);
		
		if (it != m_variables.end()) 
		    return it->second;
		   
		if (m_parent != NULL)
		    return m_parent->getVariable(name,mustExist);
		       
		if (mustExist)
    		check_runtime_error(false, "Variable "+name+" has not been defined");
		   
    	return NULL;
	}
	
    void ANMLContext::toNDDL(std::ostream& os) const
    {
    	for (unsigned int i=0; i<m_elements.size(); i++) 
    	    m_elements[i]->toNDDL(os);
    }
	
    std::string ANMLContext::toString() const
    {
    	std::ostringstream os;
    	
    	for (unsigned int i=0; i<m_elements.size(); i++) {
    		debugMsg("ANMLContext", "toString:" << i << " " << m_elements[i]->getType()); 
    	    os << m_elements[i]->toString() << std::endl; 
    	}   	     
    	
    	return os.str();
    }

    void ANMLElement::toNDDL(std::ostream& os) const 
    { 
    	os << m_type << " " << m_name; 
    }
    
    std::string ANMLElement::toString() const 
    { 
    	std::ostringstream os; 
    	toNDDL(os); 
    	return os.str(); 
    }
    
	Type::Type(const std::string& name)
	    : ANMLElement("TYPE",name)
	{
	}
	
	Type::~Type()
	{
	}
	        
    std::string autoIdentifier(const char* base)
    {
    	static int cnt=0;
    	std::ostringstream os;
        
        os << base << "_" << cnt++;
        
        return os.str();    	
    }

	Range::Range(const Type& dataType,const std::string& lb,const std::string& ub)	
	    : Type(autoIdentifier("Range"))
	    , m_dataType(dataType)
	    , m_lb(lb)
	    , m_ub(ub)
	{
		// TODO: convert lb,ub to typed values
	}
	
	Range::~Range()
	{
	}
	        
    void Range::toNDDL(std::ostream& os) const
    {
        os << m_dataType.getName() << " [" << m_lb << " " << m_ub <<  "]";
    }
    
	Enumeration::Enumeration(const Type& dataType,const std::vector<std::string>& values)
	    : Type(autoIdentifier("Enumeration"))
	    , m_dataType(dataType)
	    , m_values(values)
	{
		// TODO: convert lb,ub to typed values
	}
	
	Enumeration::~Enumeration()
	{
	}
	        
    void Enumeration::toNDDL(std::ostream& os) const
    {
        os << m_dataType.getName() << " {";
        
        for (unsigned int i=0;i<m_values.size(); i++) {
            if (i>0)
                os << ",";
            os << m_values[i];
        }                                   
    }
    
    Variable::Variable(const Type& dataType, const std::string& name)
        : ANMLElement("VARIABLE",name)
        , m_dataType(dataType)
    {
    }

    Variable::Variable(const Type& dataType, const std::string& name, const std::vector<Arg*>& args)
        : ANMLElement("VARIABLE",name)
        , m_dataType(dataType)
        , m_args(args)
    {
    }

    Variable::~Variable()
    {
    }

    void Variable::toNDDL(std::ostream& os) const
    {
    	os << m_dataType.getName() << " " << m_name;
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

    void VarDeclaration::toNDDL(std::ostream& os) const
    {
    	os << m_dataType.getName() << " ";
    	for (unsigned int i=0; i < m_init.size(); i++) {
    		if (i>0)
    		    os << " , ";
    		os << m_init[i]->getName();
    		if (m_init[i]->getValue().length() > 0) {
    			os << "=" << m_init[i]->getValue();
    		}
    		else if (!m_dataType.isPrimitive()) {
    			os << " = new " << m_dataType.getName() << "()";
    		}
    	}
    	os << ";" << std::endl;
    }
    
    ObjType::ObjType(const std::string& name,ObjType* parentObjType)
        : Type(name)
        , m_parent(parentObjType)
    {
    }
    
    ObjType::~ObjType()
    {
    }
    
    void ObjType::toNDDL(std::ostream& os) const
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
           
        os << "}" << std::endl << std::endl;
        ANMLContext::toNDDL(os);
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
    
    void Action::toNDDL(std::ostream& os) const
    {
    	os << m_objType.getName() << "::" << m_name << "(";
        for (unsigned int i=0; i<m_params.size(); i++) {
        	if (i>0)
        	    os << ",";
        	m_params[i]->toNDDL(os);
        }    	
    	os << ")" << std::endl;

    	os << "{" << std::endl;
        for (unsigned int i=0; i<m_body.size(); i++) {
    		debugMsg("ANMLContext", "Action::toNDDL " << i << " " << m_body[i]->getType()); 
        	m_body[i]->toNDDL(os);
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
    
    void ActionDuration::toNDDL(std::ostream& os) const
    {
    	os << "    eq(duration,";
    	
    	if (m_values.size() == 2) 
    		os << "[" << m_values[0]->toString() << " " << m_values[1]->toString() << "]";
    	else 
    	    os << m_values[0]->toString();
    	    
    	os << ");" << std::endl;
    }    
    
    
    TemporalQualifier::TemporalQualifier(const std::string& op,const std::vector<Expr*>& args) 
        : m_operator(op)
        , m_args(args)
    {
    }
    
    TemporalQualifier::~TemporalQualifier() 
    {
    }
     
    void TemporalQualifier::toNDDL(std::ostream& os, const std::string& ident,const std::string& fluentName) const 
    { 
    	if (m_operator == "at") {
    		Expr* timePoint = m_args[0];
    		// TODO: determine context for timePoint, eval expr if necessary
    		os << ident << "leq(" << fluentName << ".start," << timePoint->toString() << ");" << std::endl;
    		os << ident << "leq(" << timePoint->toString() << "," << fluentName << ".end);"<< std::endl;    		
    	}
    	else if (m_operator == "over") {
    		Expr* lb = m_args[0];
    		Expr* ub = m_args[1];
    		// TODO: determine context for time points, eval expr if necessary
    		os << ident << "leq(" << fluentName << ".start," << lb->toString() << ");" << std::endl;
    		os << ident << "leq(" << ub->toString() << "," << fluentName << ".end);" << std::endl;    		                		
    	}
    	else if (m_operator == "in") {
    	}
    	else if (m_operator == "after") {
    	}
    	else if (m_operator == "before") {
    		Expr* timePoint = m_args[0];
    		// TODO: determine context for timePoint, eval expr if necessary
    		os << ident << "leq(" << fluentName << ".end," << timePoint->toString() << ");" << std::endl;
    	}
    	else if (m_operator == "contains") {
    	}
    }
    
    RelationalFluent::RelationalFluent(LHSExpr* lhs,Expr* rhs) 
        : m_lhs(lhs)
        , m_rhs(rhs) 
    {
    }
    
    RelationalFluent::~RelationalFluent() 
    {
    }
    
    std::string RelationalFluent::getName() const
    {
    	return m_lhs->getName();
    }
    
    void RelationalFluent::toNDDL(std::ostream& os, TemporalQualifier* tq) const 
    {
    	std::string ident = (m_parent->getContext() == Proposition::GOAL || 
    	                     m_parent->getContext() == Proposition::FACT ? "" : "    ");
    	                     
   	    std::string varName = (m_lhs->needsVar() ? autoIdentifier("_v") : "");
   	    
    	m_lhs->toNDDL(os,m_parent->getContext(),varName);
    	if (m_lhs->needsVar()) 
   		    tq->toNDDL(os,ident,varName);
   		
    	if (m_rhs != NULL)
    	    m_rhs->toNDDL(os,m_parent->getContext(),varName);     
    }    
    
    Condition::Condition(const std::vector<Proposition*>& propositions) 
        : ANMLElement("CONDITION")
        , m_propositions(propositions) 
    {
    	for (unsigned int i=0;i<m_propositions.size();i++) 
    		m_propositions[i]->setContext(Proposition::CONDITION);
    }
    
    Condition::~Condition() 
    {
    }
    
    void Condition::toNDDL(std::ostream& os) const 
    { 
    	for (unsigned int i=0; i<m_propositions.size(); i++) {
    		m_propositions[i]->toNDDL(os);
    	}
    }
    
    Goal::Goal(const std::vector<Proposition*>& propositions) 
        : ANMLElement("GOAL")
        , m_propositions(propositions) 
    {
    	for (unsigned int i=0;i<m_propositions.size();i++) 
    		m_propositions[i]->setContext(Proposition::GOAL);
    }
    
    Goal::~Goal() 
    {
    }
    
    void Goal::toNDDL(std::ostream& os) const 
    { 
    	for (unsigned int i=0; i<m_propositions.size(); i++) 
    		m_propositions[i]->toNDDL(os);
    }
    
    Fact::Fact(const std::vector<Proposition*>& propositions) 
        : ANMLElement("FACT")
        , m_propositions(propositions) 
    {
    	for (unsigned int i=0;i<m_propositions.size();i++) 
    		m_propositions[i]->setContext(Proposition::FACT);
    }
    
    Fact::~Fact() 
    {
    }
    
    void Fact::toNDDL(std::ostream& os) const 
    { 
    	for (unsigned int i=0; i<m_propositions.size(); i++) 
    		m_propositions[i]->toNDDL(os);
    }
    
    Proposition::Proposition(TemporalQualifier* tq,const std::vector<Fluent*>& fluents) 
       : ANMLElement("PROPOSITION")
       , m_temporalQualifier(tq)
       , m_fluents(fluents) 
    {
    	m_temporalQualifier->setProposition(this);
    	for (unsigned int i=0;i<m_fluents.size();i++) 
    	    m_fluents[i]->setProposition(this); 
    }
    
    Proposition::~Proposition() 
    {
    }
    
    void Proposition::toNDDL(std::ostream& os) const 
    { 
    	for (unsigned int i=0;i<m_fluents.size();i++) 
    	    m_fluents[i]->toNDDL(os,m_temporalQualifier);
    }
    
    void LHSAction::toNDDL(std::ostream& os,Proposition::Context context,const std::string& varName) const 
    { 
    	switch (context) {
    		case Proposition::GOAL : 
    		    os << "goal(" << m_action->getName() << " " << varName << ");" << std::endl;
    		    break; 
    		case Proposition::CONDITION : 
    		    os << "    any(" << m_action->getName() << " " << varName << ");" << std::endl;
    		    break; 
    		case Proposition::FACT : 
    		    check_runtime_error(false,"ERROR! LHSAction not supported for FACTS");
    		    break; 
    		case Proposition::EFFECT : 
    		    check_runtime_error(false,"ERROR! LHSAction not supported for EFFECTS");
    		    break;
    		default:
    		    check_error(false,"Unexpected error");
    		    break;
    	}
    }          
    
    void LHSVariable::toNDDL(std::ostream& os,Proposition::Context context,const std::string& varName) const 
    { 
    	// TODO: implement this
    	os << m_var->getName(); 
    }
    
    void LHSQualifiedVar::toNDDL(std::ostream& os,Proposition::Context context,const std::string& varName) const 
    { 
    	// hack! : assuming this is and Action for now, this could also be a Var
    	switch (context) {
    		case Proposition::GOAL : 
    		    os << "goal(" << m_path << " " << varName << ");" << std::endl;
    		    break; 
    		case Proposition::CONDITION : 
    		    os << "    any(" << m_path << " " << varName << ");" << std::endl;
    		    break; 
    		case Proposition::FACT : 
    		    check_runtime_error(false,"ERROR! LHSAction not supported for FACTS");
    		    break; 
    		case Proposition::EFFECT : 
    		    check_runtime_error(false,"ERROR! LHSAction not supported for EFFECTS");
    		    break;
    		default:
    		    check_error(false,"Unexpected error");
    		    break;
    	}
    }    
    
    // Special expression to handle PlannerConfig
    LHSPlannerConfig::LHSPlannerConfig()
        : m_startHorizon(NULL)
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
    
    void LHSPlannerConfig::toNDDL(std::ostream& os) const
    {
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
    }            
}
