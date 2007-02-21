
header "post_include_hpp" {
#include "Debug.hh"
#include "antlr/ASTFactory.hpp"
#include "ANMLTranslator.hh"

}

// run antlr.Tool on this file to generate a tree parser
options {
	language="Cpp";
}

{
	ANML2NDDL::ANML2NDDL(std::ostream& nddl) 
		: antlr::TreeParser()
		, m_translator()
		, nddl(nddl) 
    {
	}
  /**
   * Custom traceIn for Antlr which uses debugMsg.
   */
  void ANML2NDDL::traceIn(const char* rname, antlr::RefAST t) {
		int indentsize = LONG_RULE_SIZE-strlen(rname)+traceDepth+2;
    char indent[indentsize+1];
		for(int i=0; i < indentsize; ++i)
			indent[i] = ' ';
		indent[LONG_RULE_SIZE-strlen(rname)+1] = '|';
		indent[indentsize] = '\0';
    ++traceDepth;

		debugMsg((std::string("ANML2NDDL:traceIn:")+rname).c_str(), indent << "> " << rname << 
						 ((inputState->guessing > 0)? "; [guessing]" : ";") <<
						 " LA==" << ((t == antlr::nullAST)? "null" : t->getText()));
  }

  /**
   * Custom traceOut for Antlr which uses debugMsg.
   */
  void ANML2NDDL::traceOut(const char* rname, antlr::RefAST t) {
    --traceDepth;
		int indentsize = LONG_RULE_SIZE-strlen(rname)+traceDepth+1;
    char indent[indentsize+1];
		for(int i=0; i < indentsize; ++i)
			indent[i] = ' ';
		indent[LONG_RULE_SIZE-strlen(rname)] = '|';
		indent[indentsize] = '\0';

		debugMsg((std::string("ANML2NDDL:traceOut:")+rname).c_str(),
		         indent << "< " << rname << ((inputState->guessing > 0)? "; [guessing]" : ";") <<
						 " LA==" << ((t == antlr::nullAST)? "null" : t->getText()));
  }
}

class ANML2NDDL extends TreeParser;

options {
  importVocab = ANML;
	noConstructors = true;
}

{
	#define LONG_RULE_SIZE 26
	public:
		ANML2NDDL(std::ostream& nddl);
		
		ANML::ANMLTranslator& getTranslator() { return m_translator; }
		
	protected:
		void traceIn(const char* rname, antlr::RefAST t);
		void traceOut(const char* rname, antlr::RefAST t);

        ANML::ANMLTranslator m_translator;
		const std::ostream& nddl;
}

anml
{
	ANML::ANMLElement* stmt;
}
  : #(ANML
	    {
           debugMsg("ANML2NDDL:anml", "Parsing...");
        }
      (stmt=anml_stmt { m_translator.getContext().addElement(stmt); debugMsg("ANML2NDDL:anml", "Added Element " << stmt->getType());})*
	    {
	       debugMsg("ANML2NDDL:anml", "Done parsing");
	    }
    )
  ;

anml_stmt returns [ANML::ANMLElement* element]
    : element=declaration
    | element=problem_stmt
    | element=transition_constraint
    | element=action_def            // <- implicit global object
;

declaration returns [ANML::ANMLElement* element]
    : element=objtype_decl
    | element=vartype_decl
    | element=var_obj_declaration
    | (#(FUNCTION IDENT LPAREN)) => element=predicate_declaration
    | element=function_declaration
;

problem_stmt returns [ANML::ANMLElement* element]
    : element=fact
    | element=goal
;
   
vartype_decl returns [ANML::ANMLElement* element]
    : #(VARTYPE user_defined_type_name var_type)
;

var_obj_declaration! returns [ANML::ANMLElement* element]
{
	ANML::Type* type;
	std::vector<ANML::VarInit*> varInit;
}    
    : #(VARIABLE type=var_type varInit=var_init_list)
{
    element = new ANML::VarDeclaration(*type,varInit);	
}    
;

var_init_list returns [std::vector<ANML::VarInit*> varInit;]
{
	ANML::VarInit* vi;
}
	: (vi=var_init { varInit.push_back(vi); })+
;

var_type returns [ANML::Type* t;]
{
	std::vector<std::string> values;
	ANML::ANMLContext& context = m_translator.getContext();
}
    : BOOL { t = context.getType("bool"); }
    | s:IDENTIFIER { t = context.getType(s->getText(),true); }
    | #(INT { t = context.getType("int"); } 
         (  values=range     { t = new ANML::Range(*t,values[0],values[1]); context.addType(t); } 
           |values=enum_body { t = new ANML::Enumeration(*t,values); context.addType(t); }
         )?
      )
	| #(FLOAT { t = context.getType("float"); }
	     (  values=range     { t = new ANML::Range(*t,values[0],values[1]); context.addType(t); }
	       |values=enum_body { t = new ANML::Enumeration(*t,values); context.addType(t); }
	     )?
	  )
    | #(STRING { t = context.getType("string"); }
         (values=enum_body { t = new ANML::Enumeration(*t,values); context.addType(t);}
         )?
      )
    | #(VECTOR vector_body)   { throw Error("Vector data type not suported yet"); }
;

enum_body returns [std::vector<std::string> values;]
{
	std::string v;
}
    : #(LCURLY (constant { values.push_back(v); })+)
;

vector_body
    : parameters
;

range returns [std::vector<std::string> values;]
{
	std::string lb,ub;
}
    : #(LBRACK lb=signed_literal ub=signed_literal)
{
	// TODO: validate range
	values.push_back(lb);
	values.push_back(ub);	
}    
;

var_init returns [ANML::VarInit* vi;]
{
	std::string name;
	std::string value;
}
    : #(EQUAL name=var_name (value=constant)?)
	| name=var_name
{
	// TODO: do type checking for initialization!
	// TODO: Add variable to context
	vi = new ANML::VarInit(name,value);
}	
;

parameters returns [std::vector<ANML::Variable*> params;]
{
	ANML::Variable* p;
}
    : #(LPAREN (p=parameter_decl { params.push_back(p); })*)
;

parameter_decl! returns [ANML::Variable* v;]
{
	ANML::Type* type;
}
    : #(PARAMETER type=var_type name:var_name)
    {
    	v = new ANML::Variable(*type,name->getText());
    }
;

objtype_decl! returns [ANML::ANMLElement* element]
{
	ANML::ObjType* newType;
	ANML::ANMLElement* childElement;
}
    : #(OBJTYPE #(objtypeName:IDENTIFIER parent:(IDENTIFIER | OBJECT)?)
      {
   		std::string parentType("object");
   		if (parent.get() != NULL)
   		    parentType = parent->getText();
       	newType = m_translator.getContext().addObjType(objtypeName->getText(),parentType);
       	m_translator.pushContext(newType);
      }
        #(LCURLY (childElement=objtype_body_stmt { newType->addElement(childElement); } )*))
      {
      	element = newType;
        m_translator.popContext();      	
      }
;

objtype_body_stmt returns [ANML::ANMLElement* element=NULL]
    : element=declaration
    | element=action_def
    | element=transition_constraint
;

function_declaration returns [ANML::ANMLElement* element]
    : #(FUNCTION var_type (function_signature)+)
;

function_signature 
    : function_symbol parameters
;

predicate_declaration returns [ANML::ANMLElement* element]
    : #(FUNCTION (function_signature)+)
;

fact returns [ANML::ANMLElement* element]
{
    element = new ANML::ANMLElement("FACT");	
}    
    : #(FACT (effect_proposition | effect_proposition_list))
;

goal returns [ANML::ANMLElement* element]
{
    element = new ANML::ANMLElement("GOAL");	
}    
    : #(GOAL (condition_proposition | condition_proposition_list))
;

effect_proposition 
    : proposition
;

effect_proposition_list 
    : #(LCURLY (effect_proposition)+)
;

condition_proposition 
    : proposition
;

condition_proposition_list
    : #(LCURLY (condition_proposition)+)
;

proposition!
    : qualif_fluent
    | #(WHEN #(LCURLY condition_proposition) #(LCURLY effect_proposition))
    | #(FROM time_pt qualif_fluent_list)
    | #(FOR object_name #(LCURLY proposition))
;

qualif_fluent
    : #(FLUENT temporal_qualif fluent_list)
;

qualif_fluent_list 
    : #(LCURLY (qualif_fluent)+)
;

fluent_list
    : #(LCURLY (fluent)+)
;

fluent
    : #(LPAREN fluent)
		| #(OR fluent fluent)
		| #(AND fluent fluent)
		| relational_fluent 
    | quantif_clause fluent
;

quantif_clause
    : #(FORALL var_list)
    | #(EXISTS var_list)
;

var_list
    : #(LPAREN var_type (var_name)+)
;

// NOTE: if the rhs is not present, that means we're stating a predicate to be true
relational_fluent 
    : #(EQUAL lhs_expr expr)
		| lhs_expr
;

// NOTE: removed start(fluent), end(fluent) from the grammar, it has to be taken care of by either functions or dot notation
// TODO: antlr is complaining about non-determinism here, but I don't see it, LPAREN should never be in follow(lhs_expr). anyway, order of subrules means parser does the right thing
lhs_expr
    : #(FUNCTION function_symbol arguments)
    | qualified_var_name
;

// TODO: we should allow for full-blown expressions (logical and numerical) at some point
expr 
    : constant
		| arguments
    | lhs_expr
;

// TODO: For effects and facts:
// - Temporal qualifiers IN, BEFORE, AFTER (and CONTAINS??) are not allowed. What about FROM?
// check and throw semantic exception if necessary
temporal_qualif 
    : #(AT time_pt)
    | #(OVER interval)
    | #(IN interval (numeric_expr)?)
    | #(AFTER time_pt (numeric_expr)?)
    | #(BEFORE time_pt (numeric_expr)?)
    | #(CONTAINS interval)
;

// all(action) is shorthand for the interval [action.start,action.end]
interval 
    : #(ALL (action_label_arg)?)
    | #(LBRACK time_pt time_pt)
;

action_label_arg
    : #(LPAREN action_instance_label)
;

time_pt 
    : numeric_expr
;

numeric_expr 
    :     (#(PLUS numeric_expr numeric_expr)) => #(PLUS numeric_expr numeric_expr)
		| (#(MINUS numeric_expr numeric_expr)) => #(MINUS numeric_expr numeric_expr)
		| #(MULT numeric_expr numeric_expr)
		| #(DIV numeric_expr numeric_expr)
		| #(LPAREN numeric_expr)
		| #(PLUS numeric_expr)
		| #(MINUS numeric_expr)
		| numeric_literal
		| lhs_expr
;

arguments
    : #(LPAREN (arg_list)?)
;

arg_list 
    : (expr)+
;

action_def returns [ANML::ANMLElement* element]
{
	std::vector<ANML::ANMLElement*> body;
	std::vector<ANML::Variable*> params;
    ANML::Action* a;
}
    : #(ACTION name:action_symbol params=parameters 
        {
            a = new ANML::Action((ANML::ObjType&)(m_translator.getContext()),name->getText(),params);
            m_translator.pushContext(a);
        }    
        body=action_body
      )
    {
      a->setBody(body);	
      element = a;
      m_translator.popContext();
    }
;

action_body returns [std::vector<ANML::ANMLElement*> body] 
{
	ANML::ANMLElement* stmt;
}
    : #(LCURLY (stmt=action_body_stmt { body.push_back(stmt); })*)
;

action_body_stmt returns [ANML::ANMLElement* element]
    : element=duration_stmt 
    | element=condition_stmt
    | element=effect_stmt 
    | element=change_stmt 
    | element=decomp_stmt 
;

// TODO: semantic layer to enforce that only one duration statement is allowed    
duration_stmt returns [ANML::ANMLElement* element]
{
    element = new ANML::ANMLElement("DURATION");	
}    
    : #(DURATION (numeric_expr | range))
;

condition_stmt returns [ANML::ANMLElement* element]
{
    element = new ANML::ANMLElement("CONDITION");	
}    
    : #(CONDITION (condition_proposition | condition_proposition_list))
;
    
effect_stmt returns [ANML::ANMLElement* element]
    : #(EFFECT (effect_proposition | effect_proposition_list))
;

change_stmt returns [ANML::ANMLElement* element]
    : #(CHANGE (change_proposition | change_proposition_list))
;

change_proposition_list
    : #(LCURLY (change_proposition)+)
; 

change_proposition
    : #(FLUENT temporal_qualif change_fluent)
    | #(WHEN condition_proposition change_proposition)
;

change_fluent 
    : #(AND change_fluent change_fluent) 
    | #(LPAREN change_fluent)
    | quantif_clause change_fluent
    | resource_change
    | transition_change
;

resource_change 
    : #(CONSUMES var_name (numeric_expr)?)
    | #(PRODUCES var_name (numeric_expr)?)
    | #(USES var_name (numeric_expr)?)
;

transition_change
    : #(EQUAL qualified_var_name directed_expr_list)
;

directed_expr_list
    : #(ARROW directed_expr_list directed_expr_list)
		| expr
;

decomp_stmt returns [ANML::ANMLElement* element]
    : #(DECOMPOSITION (decomp_step | #(LCURLY (decomp_step)+)))
;

decomp_step
    : #(ACTIONS temporal_qualif action_set)
		| constraint
;

action_set
    : #(ORDERED (quantif_clause)? action_set_element_list)
    | #(UNORDERED (quantif_clause)? action_set_element_list)
    | #(DISJUNCTION (quantif_clause)? action_set_element_list)
;

action_set_element_list
    : #(LPAREN (action_set_element)+)
;

action_set_element
    : #(ACTION qualified_action_symbol arguments (action_instance_label)?)
    | action_set
;

qualified_action_symbol
    : (#(IDENTIFIER DOT))=> #(IDENTIFIER (#(DOT action_symbol))?)
		| action_symbol
;

constraint 
    : #(CONSTRAINT constraint_symbol arguments)
;

transition_constraint returns [ANML::ANMLElement* element]
    : #(TRANSITION var_name trans_pair_list)
;

trans_pair_list
    : #(LCURLY (trans_pair)+)
;

trans_pair 
    : #(ARROW constant (constant | #(LCURLY (constant)+)))
;

unsigned_constant 
    : numeric_literal | string_literal | bool_literal
;
constant returns [std::string s;]
    : s=signed_literal 
    | s=string_literal 
    | s=bool_literal
;

signed_literal returns [std::string s;]
    : t1:numeric_literal           { s = t1->getText(); }
	| #(MINUS t2:numeric_literal)  { s = "-" + t2->getText(); }
;

numeric_literal returns [std::string s;]
    : t1:NUMERIC_LIT { s = t1->getText(); }
    | t2:INF         { s = t2->getText(); }
;

bool_literal returns [std::string s;]
    : t1:TRUE  { s = t1->getText(); }
    | t2:FALSE { s = t2->getText(); }
;

string_literal returns [std::string s;]
    : t:STRING_LIT { s = t->getText(); }
;

action_symbol           : IDENTIFIER;
action_instance_label   : IDENTIFIER;
constraint_symbol       : IDENTIFIER;
function_symbol         : IDENTIFIER;

object_name             : IDENTIFIER;
var_name returns [std::string s;]               
    : t1:IDENTIFIER { s = t1->getText(); } 
    | t2:START      { s = t2->getText(); } 
    | t3:END        { s = t3->getText(); }
;

qualified_var_name!
    : #(DOT qualified_var_name qualified_var_name)
		| var_name
;

user_defined_type_name  : IDENTIFIER;
