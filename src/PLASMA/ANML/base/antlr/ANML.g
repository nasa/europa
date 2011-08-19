/* vim: set ts=8 ft=antlr3: */

grammar ANML;

options {
language=C;
output=AST;
ASTLabelType=pANTLR3_BASE_TREE;
}

tokens {
	ANML;
	Types;
	Constants; // includes constant functions (i.e. constant mappings to constant values)
	Fluents; // includes functions mapping to fluents
	Actions;
	Parameters;
	Arguments;
	Stmts;
	ProblemStmts;
	Block;
	Decompositions;
	//List;
	
	TypeRef;
	LabelRef;
	Ref;
	Bind;
	Access;
	
	TypeRefine;

	Type;
	Fluent;
	FluentFunction;
	Constant;
	ConstantFunction;
	Parameter;
	Action;	
	//Stmt;
	
	Label; // primarily for fluent and action references -- an interval with a label
	DefiniteInterval;
	DefinitePoint;
	IndefiniteInterval;
	IndefinitePoint;
	Bra;
	Ket;
	Before;
	At;
	After;
	//Dots; //for extending to processes
	// this
	TBra;
	TKet;
	TStart;
	TEnd;
	TDuration;
	
	// but only the following are valid combinations
	/*	
	IntervalOO;  // default?  enables point/interval chaining on defaults
				// i.e., in the way ANML decomposes compound intervals
				// or PDDL's 'all' (interim).
	IntervalOC;
	IntervalCO;
	IntervalCC; // can degenerate into a point
			// should the others be trivially true if they degenerate...
			// or should it be an error?
	
	// limits are evaluated over intervals, but they are about
	// "the value that would have been" at a given point
	// nonetheless, when the other side is closed they break down into pieces
	IntervalLO;
	IntervalLC;
	IntervalOL;
	IntervalCL;
	
	Point; // CC
	*/
	
	Chain;
	
	TimedStmt;
	TimedExpr;
	ContainsSomeStmt; //special case of indefinite
	ContainsAllStmt; //special case of indefinite
	ContainsSomeExpr;
	ContainsAllExpr;
	
	ForAllExpr;
	ForAllStmt;
	ExistsExpr;
	ExistsStmt;
	//Expr;
	//Constraints; // several conditions
	//Condition;

	When;
	WhenElse;
	
	Enum;
	Range;
	
	ID;
	
	This;
	
	//Zero;	
}


@parser::includes
{
#include "AnmlInterpreter.hh"

using namespace EUROPA;
}

@lexer::includes
{
#include "AnmlInterpreter.hh" 

using namespace EUROPA;
}

anml	
	: 
	( t+=type_decl 
	| c+=const_decl 
	| f+=fluent_decl 
	| a+=action_decl
	| s+=stmt
	| ps+=problem_stmt 
	)*
	-> 
	^(Block 
		^(Types $t*)
		^(Constants $c*)
		^(Fluents $f*)
		^(Actions $a*)
		^(Stmts $s*)
		^(ProblemStmts $ps*)
	)
;

type_decl 
	: Type l+=type_decl_helper (Comma l+=type_decl_helper)* Semi
	  -> $l+
	| type_refine
	;

type_decl_helper
	: ID
	  ( LessThan s+=type_reference
	  | Assign d+=type_spec
	  | With p+=object_block
	  )*
	-> ^(Type ID ^(Assign["Assign"] $d*) ^(LessThan["LessThan"] $s*) ^(With["With"] $p*))
	;

type_reference
	: (type_name set?) 
	  -> ^(TypeRef type_name set?)
	;
		 
type_name
	: builtinType
	| ID
	;

type_ref 
	: (builtinType set?) => builtinType set
	  -> ^(TypeRef builtinType set?)
	| (ID set?) => ID set
	  -> ^(TypeRef ID set?)
;

type_spec
	: type_ref
	| Vector param_list
	  -> ^(Vector param_list)
	| type_enumeration
;

// TODO JRB: Fact??? this looks incorrect
type_refine
	: Fact
		( LeftC type_refine_helper+ RightC
			-> type_refine_helper+
		| type_refine_helper
			-> type_refine_helper
		)
;
		

type_refine_helper
	: user_type_ref LessThan type_ref Semi
		-> ^(TypeRefine[$LessThan] user_type_ref ^(LessThan type_ref))
	| user_type_ref Assign type_spec Semi
		-> ^(TypeRefine[$Assign] user_type_ref ^(Assign type_spec))
	| enumerated_type_ref i=ID (Comma ID)* Semi
		-> ^(TypeRefine[$i] enumerated_type_ref ID+)
;
	
// _constrained_ type references
user_type_ref 
	: ID -> ^(TypeRef ID)
;

enumerated_type_ref
	: Symbol -> ^(TypeRef Symbol)
	| Object -> ^(TypeRef Object)
	| ID -> ^(TypeRef ID)
;

predicate_helper
	: Predicate
	  -> ^(TypeRef[$Predicate] Boolean[$Predicate])
;

param_helper : ID  
	-> ^(Parameter ID)
;

var_decl_helper : ID init? 
	-> ^(Fluent ID init?)
;

fun_decl_helper : ID param_list 
	-> ^(FluentFunction ID param_list)
;

const_var_decl_helper : ID init? 
	-> ^(Constant ID init?)
;

const_fun_decl_helper : ID param_list 
	-> ^(ConstantFunction ID param_list)
;

problem_stmt
	:	fact_decl
	|	goal_decl
	;

init 
	: Assign! expr
	| Assign! Undefined!
	| Undefine!
;

param_list :	
	( LeftP p+=param (Comma p+=param)* RightP 
		-> ^(Parameters $p+)
	| LeftP RightP
		-> ^(Parameters)
	)
;

/* Declarations */	
param 
	: type_ref param_helper (Comma param_helper)* 
	  -> param_helper+
;

const_decl 
	: 	Constant type_ref
		//(l+=const_decl_helper (Comma l+=const_decl_helper)* Semi)
	  	//-> $l+
	;

const_decl_helper
	: 	const_var_decl_helper
	|	const_fun_decl_helper
	;

fluent_decl 
	: 	fluent_fluent_decl
	|	fluent_var_decl 
	|	fluent_fun_decl 
	|	fluent_predicate_decl
	;

fluent_fluent_decl
	: 	(Fluent type_ref
		l+=decl_helper (Comma l+=decl_helper)* Semi
		) -> $l+
	;
	
fluent_var_decl
	:	(Variable type_ref 
		l+=var_decl_helper (Comma l+=var_decl_helper)* Semi
		) -> $l+
	;
	
fluent_fun_decl
	:	(Function type_ref 
		l+=fun_decl_helper (Comma l+=fun_decl_helper)* Semi
		) -> $l+
	;
	
fluent_predicate_decl
	:	(predicate_helper 
		l+=fun_decl_helper (Comma l+=fun_decl_helper)* Semi	
		) -> $l+
	;
	
decl_helper
	: var_decl_helper
	| fun_decl_helper
	;
	

fact_decl_helper 
	: (ref Semi)=> ref Semi
	  -> ^(TimedStmt[$ref.tree] ^(DefinitePoint[$ref.tree] ^(TStart Start)) ^(Assign[$Semi] ref True))
	| ((NotLog|NotBit) ref Semi)=> (n=NotLog|n=NotBit) ref Semi
	  -> ^(TimedStmt[$n] ^(DefinitePoint[$n] ^(TStart Start)) ^(Assign[$Semi] ref False))
	| ref (o=EqualLog|o=Equal) expr Semi
	  -> ^(TimedStmt[$ref.tree] ^(DefinitePoint[$ref.tree] ^(TStart Start)) ^(Assign[$o] ref expr))
	| Semi!
;

goal_decl_helper
	: expr Semi
	  -> ^(TimedStmt[$expr.tree] ^(DefinitePoint[$expr.tree] ^(TStart End)) expr)
	| Semi!
; 

fact_decl : Fact
    ( LeftC fact_decl_helper* RightC
      -> fact_decl_helper*
    | fact_decl_helper
      -> fact_decl_helper
    )
;
 
goal_decl : Goal 
	( LeftC goal_decl_helper* RightC
	  -> goal_decl_helper*
	| goal_decl_helper
	  -> goal_decl_helper 
	)
;

/* object definitions */

object_block : 
	LeftC
		( t+=type_decl 
		| c+=const_decl 
		| f+=fluent_decl 
		| a+=action_decl
		| s+=stmt
		| ps+=problem_stmt
		)+  
	RightC
		-> 
		^(Block[$LeftC,"ObjectBlock"]
			^(Types $t*)
			^(Constants $c*)
			^(Fluents $f*)
			^(Actions $a*)
			^(Stmts $s*)
			^(ProblemStmts $ps*)
		)
;
		
/* actions */

// the declarations of actions nested within decompositions fails
// also within statement blocks and object blocks
// well, partially fails -- the declaration happens in the nearest
// action or domain level scope, instead of the most enclosing scope.
// So name clashes can occur.  Other than that, the tree grammar can fix
// the scope pointer on reparsing so that nested action definitions will be able to
// access declarations between the nearest action/domain declaration and most-enclosing block
		
// second argument is how you dispatch the action; give a start and an end time
// or just a start time
action_decl 
:
	Action ID 
	param_list
	(LeftB Duration RightB)? 
	action_block
	-> ^(Action ID param_list Duration? action_block)
;

// this would be interesting:
// "action [start] : foo... {...}"
// "5: foo...;" ==> "(5,10] foo...-executing;"
// "action [all] : foo... {...}"
// i.e. "action [start, end] : foo... {...}"
// "5, ^3: foo...;"
// "5, 8: foo...;"
// "^3, 8: foo...;"
		
//		[3] foo(...);
//		[3,8] foo(...);
//		[3,^5] foo(...);

durative_action_block
: 
		LeftB! Duration RightB! action_block_h
;

action_block
:
		action_block_h
;

action_block_h
  : LeftC
		( t+=type_decl 
		| c+=const_decl 
		| f+=fluent_decl 
		| a+=action_decl
		| s+=stmt
		| ps+=problem_stmt
		)*
		(d+=decomp_block)* 
	RightC
		-> 
			^(Types $t*)
			^(Constants $c*)
			^(Fluents $f*)
			^(Actions $a*)
			^(Stmts $s*)
			^(ProblemStmts $ps*)
			^(Decompositions $d*)
;
					   
decomp_block 
	: Decomposition
	// ID param_list
		( t+=type_decl 
		| c+=const_decl 
		| f+=fluent_decl 
		| a+=action_decl
		| s+=stmt
		| ps+=problem_stmt
		)* 
	  -> 
		^(Block[$Decomposition,"DecompositionBlock"]
		// ID param_list
			^(Types $t*)
			^(Constants $c*)
			^(Fluents $f*)
			^(Actions $a*)
			^(Stmts $s*)
			^(ProblemStmts $ps*)
		)
;

stmt_block 
	: LeftC
		( t+=type_decl 
		| c+=const_decl 
		| f+=fluent_decl 
		| a+=action_decl
		| s+=stmt
		| ps+=problem_stmt
		)*  
	  RightC  
	  ->
		^(Block[$LeftC]
			^(Types $t*)
			^(Constants $c*)
			^(Fluents $f*)
			^(Actions $a*)
			^(Stmts $s*)
			^(ProblemStmts $ps*)
		)
;
			
stmt //options {memoize=true;}
	: (stmt_primitive)=> stmt_primitive
	| (stmt_block)=> stmt_block
	| (stmt_timed)=> stmt_timed
	| stmt_contains
	| stmt_when
	| stmt_forall
	| stmt_exists
;

stmt_contains
	: Contains 
		((exist_time stmt)=> exist_time stmt
			-> ^(ContainsSomeStmt[$Contains] exist_time stmt)
		| stmt
			-> ^(ContainsAllStmt[$Contains] stmt)
		)
;

stmt_when 
	: When guard stmt  
		( (Else)=> Else stmt
			-> ^(WhenElse[$When] guard stmt stmt)
		| 
			-> ^(When guard stmt)
		)
;

stmt_forall
	: ForAll param_list stmt
		-> ^(ForAllStmt[$ForAll] param_list stmt)
;

stmt_exists
	: Exists param_list stmt
		-> ^(ExistsStmt[$Exists] param_list stmt)
;

stmt_timed
	: interval stmt
		-> ^(TimedStmt interval stmt)
;
	
stmt_primitive 
	: (expr Semi)=> expr Semi!
  	| (stmt_chain Semi)=> stmt_chain Semi!
	| (stmt_delta_chain Semi)=> stmt_delta_chain Semi!
  	| (stmt_timeless Semi)=> stmt_timeless Semi!
  	| Semi -> Skip
;

stmt_chain
	: ref e+=stmt_chain_1+
  		-> ^(Chain ^(DefiniteInterval ^(TBra Bra) ^(TStart Start) ^(TDuration Duration) ^(TEnd End) ^(TKet Ket)) ref $e+) 
	| (interval ref stmt_chain_1+)=> interval ref e+=stmt_chain_1+
		-> ^(Chain interval ref $e+)
;
stmt_chain_1
    : Comma!? Assign^ e_num
    | Comma? o=Change b=e_num
    	->	Undefine 
    		^(Assign $b)
    | Comma!? o=Produce^ b=e_num
    | Comma!? o=Consume^ b=e_num
    | Comma? o=Lend b=e_num
    	->	^(Produce $b)
    		Skip
    		^(Consume $b)
    | Comma? o=Use b=e_num
    	->	^(Consume $b) 
    		Skip
    		^(Produce $b)
    | Comma!? (o=Within|o=SetAssign)^ s=set
    | Comma!? i=num_relop^ b=e_num 
	| Comma? (o=Equal Skip | o=Skip)
		-> Skip
    | Comma? (o=Assign Undefined | o=Undefine )
    	-> Undefine
; 

stmt_delta_chain
	: Delta ref e+=stmt_delta_chain_1+ 
  		-> ^(Chain ^(DefiniteInterval ^(TBra Bra) ^(TStart Start) ^(TDuration Duration) ^(TEnd End) ^(TKet Ket)) ^(Delta ref) $e+) 
	| (interval ref stmt_delta_chain_1+)=> interval ref e+=stmt_delta_chain_1+
		-> ^(Chain interval ^(Delta ref) $e+)
;

// don't allow iterated deltas, in particular not implicitly through [start] ^f :produce 2;
stmt_delta_chain_1
    : Comma? Assign^ b=e_num
    | Comma? o=Change b=e_num
    	->	Undefine
    		^(Assign $b)
    | Comma? SetAssign^ s=set
	| Comma? (o=Equal Skip | o=Skip)
		-> Skip
    | Comma? (o=Assign Undefined | o=Undefine )
    	-> Undefine
;

stmt_timeless
	: time_primitive Assign^ expr
;
	
guard 
//	: '('! expr ')'!
	: expr
;


interval
	: (univ_time)=>univ_time
	| (exist_time)=>exist_time
;

// for intervals:
// no definition means no shadowing?
// and indefinites explicitly map to undefined?
// or no definition means must choose, and inheritance has to be explicit?
univ_time 
 //options {memoize=true;}   //memoization breaks things for no good reason
	: (bra All ket)=> bra All ket 
	    -> ^(DefiniteInterval bra ^(TStart Start) ^(TDuration Duration) ^(TEnd End) ket)
	| (LeftB expr RightB)=> LeftB e=expr RightB 
	  -> ^(DefinitePoint ^(TStart $e))
	| bra 
	  ( d=delta_time Comma e=expr ket
		-> ^(DefiniteInterval bra ^(TDuration $d) ^(TEnd $e) ket)
	  | e=expr Comma 
		( d=delta_time ket
		  -> ^(DefiniteInterval bra ^(TStart $e) ^(TDuration $d) ket)
		| f=expr ket
		  -> ^(DefiniteInterval bra ^(TStart $e) ^(TEnd $f) ket)
		)
	  )
;

exist_time 
 //options {memoize=true;} // see above
	: (LeftB Skip RightB)=> LeftB Skip RightB -> ^(IndefinitePoint)
	| (bra expr rLimit)=> bra e=expr rLimit -> ^(IndefiniteInterval bra ^(TStart $e) rLimit)
	| (lLimit expr ket)=> lLimit e=expr ket -> ^(IndefiniteInterval lLimit ^(TEnd $e) ket)
	| bra 
	  ( (Delta)=> d=delta_time Comma Skip? ket -> ^(IndefiniteInterval bra ^(TDuration $d) ket)
	  | (Skip)=> Skip Comma 
	  	( (Delta)=> d=delta_time ket -> ^(IndefiniteInterval bra ^(TDuration $d) ket)
		| (Skip)=> Skip ket -> ^(IndefiniteInterval bra ket) // only constraint is non-degeneracy; start and end must be different (maybe?)
		| e=expr ket -> ^(IndefiniteInterval bra ^(TEnd $e) ket)
		)
	  | e=expr Comma Skip ket -> ^(IndefiniteInterval bra ^(TStart $e) ket)
	  )
;

delta_time : Delta! e_num_1 ;

bra 
	: LeftB -> ^(TBra At[$LeftB,"At"]) 
	| LeftP -> ^(TBra After[$LeftP,"After"])
//	| Dots -> ^(TBra Dots)
;
ket 
	: RightB -> ^(TKet At[$RightB,"At"]) 
	| RightP -> ^(TKet Before[$RightP,"Before"])
//	| Dots -> ^(TKet Dots);
;

lLimit : Dots -> ^(TBra Before[$Dots,"Before"]);
rLimit : Dots -> ^(TKet After[$Dots,"After"]);

// parallel language; comparisons and assignments evaluate to left hand side.

expr
	: (e_prefix)=>e_prefix
	| e_log_1
;

e_prefix 
//options {memoize=true;}
	: ID Colon e=expr
		-> ^(Label[$Colon] ID $e)
	| (interval expr)=> interval e=expr
		-> ^(TimedExpr interval $e)
//	| (exist_time expr)=> exist_time e=expr
//		-> ^(TimedExpr exist_time $e)
	| (Contains)=> Contains //{System.err.println("In contains expression.");}
		((exist_time expr)=> exist_time e=expr //{System.err.println(((ANMLToken)$e.tree).toStringTree());}
			-> ^(ContainsSomeExpr[$Contains] exist_time $e)
		| e=expr //{System.err.println(((ANMLToken)$e.tree).toStringTree());}
			-> ^(ContainsAllExpr[$Contains] $e)
		)	
	| ForAll param_list e=expr
		-> ^(ForAllExpr[$ForAll] param_list $e)
	| Exists param_list e=expr
		-> ^(ExistsExpr[$Exists] param_list $e)
;

// what about colons...?
// [all] : foo
// [start, ^end] : forall ... : exists ... : blah and bar;
    	
// if...then..., and, either...or...
// have low precedence, albeit, not as low as the chain-expr operators.
e_log_1
	: e_log_2 ((Implies)=>Implies^ ((e_prefix)=>e_prefix | e_log_1))?
;

e_log_2
	: e_log_3 ((EqualLog)=>EqualLog^ ((e_prefix)=>e_prefix | e_log_3))*
;

e_log_3
	: e_log_4 ((XorLog)=>XorLog^ ((e_prefix)=>e_prefix | e_log_4))*
;
	
// albeit switching and/or matches CNF precedence, which is a very normal
// way to write a boolean expression (a set of constraints, instead of
// a list of models)
// i.e.,  a & b | c & d | e & f | g might be a,bc,de,fg (cnf) or ab,cd,ef,g (dnf)
// and since dnf is exponentially larger, also from personal experience, cnf is perhaps
// a better default.  Despite and ~= times.
// then again...in active-low logic, clipping + is & and clipping * is |
// also return codes from main() are active-low...interesting.
// the anecdotal evidence would be:
//   (ptr != null && ptr->property1 == v1 || ptr->property1 == v2 && ptr->property2 == v3)
// which is meant as (and (and (not null) (or v1 v2)) v3)
// but actually means (or (and (not null) v1) (and v2 v3)) 
e_log_4
	: e_log_5 ((OrLog)=>OrLog^ ((e_prefix)=>e_prefix | e_log_5))*
;

e_log_5 
	: e_log_6 ((AndLog)=>AndLog^ ((e_prefix)=>e_prefix | e_log_6))*
;

e_log_6
	: (NotLog)=>NotLog^ ((e_prefix)=>e_prefix | e_log_6)
	| e_log_7
;

e_log_7
	: e_num_1 ((num_relop)=>num_relop^ ((e_prefix)=>e_prefix | e_num_1))?
;

e_num
	: (e_prefix)=>e_prefix
	| e_num_1
;

e_num_1
	: e_num_2 ((XorBit)=>XorBit^ ((e_prefix)=>e_prefix | e_num_2))*
;

e_num_2
	: e_num_3 ((Plus|Minus|OrBit)=>(Plus^|Minus^|OrBit^) ((e_prefix)=>e_prefix | e_num_3))*
;

e_num_3
	: e_num_4 
		((Times|Divide|AndBit)=>(Times^|Divide^|AndBit^) 
			((e_prefix)=>e_prefix 
			| e_num_4
			)
		)*
;

e_num_4
	: (Minus|NotBit)=>(Minus^|NotBit^) 
		((e_prefix)=>e_prefix 
		| e_num_4
		)
	| e_atomic
;

e_atomic
	: '('! expr ')'!
	| time_primitive
	| time_complex
	| literal
	| ref
;
		
time_complex :
	(Unordered^|Ordered^) '('! expr (Comma! expr)* ')'!
;

ref
	:	(i=ID -> ^(Ref[$i,"ReferenceID"] $i))
	// TODO: enable this part of the rule
	//	( Dot ID
	//		-> ^(Access[$Dot,"AccessField"] $ref ^(Ref ID))
	//	| (arg_list)=> arg_list
	//		-> ^(Bind[$arg_list.tree,"BindParameters"] $ref arg_list)
	//	)*
;

time_primitive
	:	(Start LeftP ID RightP)=> Start LeftP ID RightP
    	-> ^(LabelRef ID Start)
	| 	Start
    	-> ^(LabelRef This Start)
	|	(End LeftP ID RightP)=> End LeftP ID RightP
		-> ^(LabelRef ID End)
	| 	End
    	-> ^(LabelRef This End)
	|	(Duration LeftP ID RightP)=> Duration LeftP ID RightP
		-> ^(LabelRef ID Duration)
	| 	Duration
    	-> ^(LabelRef This Duration)
//	|	(Bra LeftP ID RightP)=> Bra LeftP ID RightP
//    	-> ^(LabelRef ID Bra)
//	| 	Bra
//	   	-> ^(LabelRef This Bra)
//	| 	(Ket LeftP ID RightP)=> Ket LeftP ID RightP
//    	-> ^(LabelRef ID Ket)
//	| 	Ket
//    	-> ^(LabelRef This Ket)
	;
	
set: enumeration | range;

enumeration: 
	LeftC expr (Comma? expr)* RightC -> ^(Enum[$LeftC,"Enumeration"] expr+)
;

range: 
	  LeftB a=expr Comma? b=expr RightB -> ^(Range[$LeftB,"Range"] $a $b) 
;

type_enumeration: 
	LeftC type_enumeration_element (Comma? type_enumeration_element)* RightC 
	-> ^(Enum[$LeftC,"TypeElementEnumeration"] type_enumeration_element+)
;
	  
type_enumeration_element:
	ID | literal
;

arg_list :
	  LeftP (expr (Comma? expr)*)? RightP -> ^(Arguments[$LeftP,"Arguments"] expr*) 
;    

/* primitives */	  
builtinType
	: Boolean
	| Integer
	| Float
	| Symbol
	| String
	| Object
;
	
Boolean 
	: 'bool'
	| 'boolean'
;
Integer : 'int' | 'integer';
Float : 'float' ;
Symbol : 'symbol' ;
String : 'string' ;
Object : 'object' ;

Vector : 'vector';

literal 
	: INT
	| FLOAT
	| STRING
	| True
	| False
	| Infinity
//	| boolean_literal 
//	| vector_literal
;
//vector_literal : '('! literal (',' literal)* ')'!;
//vector_init : arg_list;

	  
num_relop : Equal | NotEqual | LessThan | GreaterThan | LessThanE | GreaterThanE;
  
	  
NotLog: 'not' 
	| 'Not' | 'NOT' 
	;
AndLog : 'and' 
	| 'And' | 'AND'
	;
OrLog : 'or' 
	| 'Or' | 'OR'
	;
XorLog : 'xor' 
	| 'Xor' | 'XOR'
	;
EqualLog: 'equals' 
	| 'iff' | 'Equals' | 'EQUALS'
	;
Implies: 'implies' 
	| 'Implies' | 'IMPLIES'
	;

NotBit: '!' | '~';
AndBit: '&';
OrBit: '|';				
XorBit : '~&|';  
  // 'a ~&| b' is supposed to suggest both '(a~&b)' and '(a|b)', i.e., '(~(a&b) and a|b)'
  // '^' is taken by Delta, so a^b could be confusing, albeit, ^ only means Delta if a unary operator applied
  // on the left, so perhaps it would be okay to use '^' for XorBit instead, in the same way that '-'
  // means inversion and subtraction depending on unary/binary 

Times: '*';
Divide: '/';
Plus: '+';
Minus: '-';

Equal : '==';
NotEqual : '!=' | '~=';
LessThan : '<' ;
LessThanE : '<=';
GreaterThan: '>' ; 
GreaterThanE: '>=';

SetAssign: ':in';

Assign : ':=';

Consume: ':consume';
Produce: ':produce';
Use: ':use';
Lend: ':lend';

Change: ':->';
// _ and :_ are special

When: 'when';
Else: 'else';
Exists: 'exists';
ForAll: 'forall';

With: 'with';
Within: 'in';
Contains: 'contains';
	  
Constant: 'constant' | 'const';
Fluent: 'fluent';
Function: 'function';
Predicate: 'predicate';
Variable: 'variable' | 'var';
Type: 'type';
Action: 'action';
Fact: 'fact';
Goal: 'goal';
Decomposition: ':decomposition';
Ordered: 'ordered';
Unordered: 'unordered';	  
	  
Delta : '^';

Undefined: 'undefined' 
	| 'U' | 'UNDEFINED' | 'Undefined'
	;
All: 'all';
Start: 'start';
End: 'end';
Duration: 'duration';
Infinity: 'infinity' | 'Infinity' | 'INFINITY' | 'infty' | 'INFTY' | 'inff' | 'INFF' ;

// cannot lex SymbolLiteral or VectorLiteral or ObjectLiteral
// cannot parse SymbolLiteral or ObjectLiteral
True:	('T' | 'TRUE' | 'true' | 'True');
False:	('F' | 'FALSE' | 'false' | 'False');
INT: DIGIT+;
FLOAT
	: DIGIT+ Dot DIGIT+
	| DIGIT+ 'f'
;
STRING:	 '"' ESC* '"';	


ID : LETTER (LETTER | DIGIT | '_')*;

fragment ESC : '\\'. | ~'\"';
fragment LETTER  : 	'a'..'z'|'A'..'Z' ;
fragment DIGIT : 	'0'..'9' ;

WS  :  (' '|'\r'|'\t'|'\u000C'|'\n')+ {$channel=HIDDEN;} ;
SLC : '//' (~'\n')* '\n' {$channel=HIDDEN;} ;
MLC : '/*' (~'*' | '*'+(~('*'|'/')))* '*'+'/' {$channel=HIDDEN;} ;

Dot: '.';
Dots: '...';
Comma: ',';
Semi: ';';
Colon: ':';
LeftP: '(';
RightP: ')';
LeftC: '{';
RightC: '}';
LeftB: '[';
RightB: ']';
Undefine: ':_';
Skip: '_';

//WS  :  (' '|'\r'|'\t'|'\u000C'|'\n')+ {skip();} ;
//COMMA : ',' {skip();} ;
// WS  :   (' '|'\t')+ {skip();} ;
//NEWLINE :'\r'? '\n' ;



    