namespace EUROPA {

        const std::string indent = "  ";
        const std::string replan = "Abort(), Replan(), Fail()?";
        const std::string xmlVersion ="<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        const std::string begin_plexil_plan="<PlexilPlan xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=";
	const std::string namespaceLocation = "\"../schema/plexil_Schema.xsd\">";
        const std::string end_plexil_plan = "</PlexilPlan>";
        const std::string bnodetype="<Node NodeType=";
        const std::string enodetype=">";
        const std::string low_start ="<Low>";
        const std::string low_end = "</Low>";
        const std::string high_start ="<High>";
        const std::string high_end = "</High>";
        const std::string begin_nodebody="<NodeBody>";
        const std::string end_nodebody ="</NodeBody>";
        const std::string commandNodetype="\"command\"";
	const std::string bnode = "<Node>";
 	const std::string enode = "</Node>";
        const std::string nodeidPrefix="_";  // node names cannot be numbers, they must start with a letter or symbol
 	const std::string bnodeid = "<NodeId>";
 	const std::string enodeid = "</NodeId>";
 	const std::string bnodelist = "\"NodeList\"";
        const std::string begin_full_nodelist ="<NodeList>";
 	const std::string enodelist = "</NodeList>";
 	const std::string bnodetimeval = "<NodeTimepointValue>";
 	const std::string enodetimeval = "</NodeTimepointValue>";
 	const std::string bnodestateval = "<NodeStateValue>";
 	const std::string enodestateval = "</NodeStateValue>";
 	const std::string bcomment = "<Comment>";
 	const std::string ecomment = "</Comment>";
 	const std::string bstc = "<StartCondition>";
 	const std::string estc = "</StartCondition>";
 	const std::string bpc = "<PreCondition>";
 	const std::string epc = "</PreCondition>";
 	const std::string bic = "<InvariantCondition>";
 	const std::string eic = "</InvariantCondition>";
 	const std::string bpoc = "<PostCondition>";
 	const std::string epoc = "</PostCondition>";
 	const std::string band = "<AND>";
 	const std::string eand = "</AND>";
 	const std::string bor = "<OR>";
 	const std::string eor = "</OR>";
 	const std::string bge = "<GE>";
 	const std::string ege = "</GE>";
 	const std::string ble = "<LE>";
 	const std::string ele = "</LE>";
 	const std::string blookfreq = "<LookupWithFrequency>";
 	const std::string elookfreq = "</LookupWithFrequency>";
 	const std::string blooknow = "<LookupNow>";
 	const std::string elooknow = "</LookupNow>";
        const std::string timeValue_begin = "<TimeValue>";
        const std::string timeValue_end = "</TimeValue>";
        const std::string units_begin = "<Units>";
        const std::string units_end = "</Units>";
 	const std::string bfreq = "<Frequency>";
 	const std::string efreq = "</Frequency>";
	const std::string integerValue_begin = "<IntegerValue>";
 	const std::string integerValue_end  = "</IntegerValue>";
 	const std::string breal = "<RealVariable>";
 	const std::string ereal = "</RealVariable>";
        const std::string brealValue ="<RealValue>";
        const std::string erealValue ="</RealValue>";
        const std::string bboolValue ="<BooleanValue>";
        const std::string eboolValue ="</BooleanValue>";
        const std::string bstringValue ="<StringValue>";
        const std::string estringValue ="</StringValue>";
 	const std::string bstate = "<StateName>";
 	const std::string estate = "</StateName>";
 	const std::string binf = "<PlusInfinity>";
 	const std::string einf = "</PlusInfinity>";
 	const std::string bneginf = "<MinusInfinity>";
 	const std::string eneginf = "</MinusInfinity>";
 	const std::string btp = "<Timepoint>";
 	const std::string etp = "</Timepoint>";
 	const std::string badd = "<ADD>";
 	const std::string eadd = "</ADD>";
 	const std::string beqbool = "<EQBoolean>";
 	const std::string eeqbool = "</EQBoolean>";
 	const std::string bboolvar = "<BooleanVariable>";
 	const std::string eboolvar = "</BooleanVariable>";
 	const std::string bbool = "<Boolean>";
 	const std::string ebool = "</Boolean>";
 	const std::string bcmd = "<Command>";
 	const std::string ecmd = "</Command>";
 	const std::string bcmdname = "<CommandName>";
 	const std::string ecmdname = "</CommandName>";
 	const std::string bargs = "<Arguments>";
 	const std::string eargs = "</Arguments>";
	const std::string end = "END";
	const std::string start = "START";
 	const std::string inf = "INF";
 	const std::string neginf = "-INF";
	const std::string fin = "FINISHED";
	const std::string exec = "EXECUTING";
	const std::string finin = "FINISHING";
	const std::string failed = "FAILED";
	const std::string booltrue = "1";
	const std::string boolfalse = "0";
        const std::string time="time";

//DEPRECATED

        const std::string stc = "StartCondition:AND{";
	const std::string prec = "PreCondition:AND{";
	const std::string ic = "InvariantCondition:AND{";
	const std::string poc = "PostCondition:AND{";

	const std::string lookupf = "LookupWithFrequency{";
	const std::string lookupn = "LookupNow{";
	const std::string curtime = "CurrentTimeWithin{";
	const std::string abstime = "AbsoluteTimeWithin{";

	const std::string node = "Node:{";
	const std::string nodeid = "NodeId:{";
	const std::string nodelist = "NodeList:{";
	const std::string vars = "Variables:{";
	const std::string inter = "Interface:{";

	const std::string infty = "PLUS_INFINIFY";
        const std::string mininfty = "MINUS_INFINITY";

        const std::string state = "state";	
	const std::string dot =".";	
	const std::string comma =",";	
	const std::string rbracket ="]";	
	const std::string lbracket ="[";	
	const std::string endbrace = "}";
	const std::string plus = "+";
	
	const std::string andstr = "AND";
	const std::string orstr = "OR";

	const std::string freq = "1.0";
}
