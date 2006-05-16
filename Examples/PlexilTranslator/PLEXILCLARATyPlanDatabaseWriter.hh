#ifndef _H_PLEXILCLARATyPlanDatabaseWriter
#define _H_PLEXILCLARATyPlanDatabaseWriter

#include "Timeline.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "ConstraintEngine.hh"
#include "IntervalIntDomain.hh"
#include "PlanDatabaseDefs.hh"
#include "TokenVariable.hh"
#include "ConstrainedVariable.hh"
#include "TestSupport.hh"
#include "TemporalAdvisor.hh"
#include <string>
#include <iostream.h>
#include <fstream>
#include <stdlib.h>
#include <strstream>

/*
PLEXIL Plan Writer.
0. Code basically cribbed from PlanDatabaseWriter.hh. Heavily modified, of course.
1. Writes E2 plans as PLEXIL plans.
2. Right now, only handles tokens inserted on a timeline.
	2a. Will always ignore tokens not on objects (timelines or resources).
 	2b. In future will need to handle tokens on resource objects.
3. Right now, prints every active token on timelines.
	3a. In future, will have a designated set of token predicates that must be printed.
	3b. Options: ignore-timeline, ignore-predicate.
4. Assume planner did NOT establish all-pairs shortest paths; need to do that ourselves before printing.
	4a. Rather than do that for all tokens, do it only for pairs of tokens you care about.  MUCH cheaper!
5. In order to avoid supreme annoyance of pre-defined limits on strings,
don't use strcat; just print appropriate string directly.
Means some of our pre-defined strings are overkill.  TOobad.
	5a. Handle this by compiling lists of tokens satisfying the various
conditions.
	5b. In the future, Do The Right THing and have strings with the various conditions in
	them; this will prevent multiple loops over sets of tokens and make it cheaper.  WIll also
	lead to vast simplification of writexyz conditioon functions.
6. pseudocode:

initialize temporal advisor, list of strings for pred names
read the predicates PLEXIL will ignore (or keep)
for every token
	keep it if it's active and is to be exported
for every kept token t
	empty strings containing the conditions
	for every other kept token o
		determine whether it:
			precedes t
			follows t
			meets t
			contains t
		append to the strings containing the
			precondition
			start condition
			invariant condition
			post condition
	write the node preamble
	write the conditions
	


Data structures needed
	either a list of predicates, list of objects, or both to check isIgnoredByPlexil
	if we want to assemble strings, we could have such a data structure too.

Barf if we get a badly formed plan (it has resources, it has active tokens not on an object,
 it is otherwise ill-behaved or appears evil)



for printing constant classes:
can grovel over objects to extract objects that are instances of the correct class
method getvariables that are fields of 

*/


namespace EUROPA {

        std::string replan = "Abort(), Replan(), Fail()?";
        std::string preamble= "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        std::string preamble2="<PlexilPlan xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\"/Users/vandi/plexil_Schema_051026.xsd\">";
 	std::string bnode = "<Node>";
 	std::string enode = "</Node>";
 	std::string bnodeid = "<NodeId>";
 	std::string enodeid = "</NodeId>";
 	std::string bnodelist = "<NodeList>";
 	std::string enodelist = "</NodeList>";
 	std::string bnodetimeval = "<NodeTimepointValue>";
 	std::string enodetimeval = "</NodeTimepointValue>";
 	std::string bnodestateval = "<NodeStateValue>";
 	std::string enodestateval = "</NodeStateValue>";
 	std::string bcomment = "<Comment>";
 	std::string ecomment = "</Comment>";
 	std::string bstc = "<StartCondition>";
 	std::string estc = "</StartCondition>";
 	std::string bpc = "<PreCondition>";
 	std::string epc = "</PreCondition>";
 	std::string bic = "<InvariantCondition>";
 	std::string eic = "</InvariantCondition>";
 	std::string bpoc = "<PostCondition>";
 	std::string epoc = "</PostCondition>";
 	std::string band = "<AND>";
 	std::string eand = "</AND>";
 	std::string bor = "<OR>";
 	std::string eor = "</OR>";
 	std::string bge = "<GE>";
 	std::string ege = "</GE>";
 	std::string ble = "<LE>";
 	std::string ele = "</LE>";
 	std::string blookfreq = "<LookupWithFrequency>";
 	std::string elookfreq = "</LookupWithFrequency>";
 	std::string blooknow = "<LookupNow>";
 	std::string elooknow = "</LookupNow>";
 	std::string bfreq = "<Frequency>";
 	std::string efreq = "</Frequency>";
 	std::string breal = "<RealVariable>";
 	std::string ereal = "</RealVariable>";
 	std::string bstate = "<StateName>";
 	std::string estate = "</StateName>";
 	std::string binf = "<PlusInfinity>";
 	std::string einf = "</PlusInfinity>";
 	std::string bneginf = "<MinusInfinity>";
 	std::string eneginf = "</MinusInfinity>";
 	std::string btp = "<Timepoint>";
 	std::string etp = "</Timepoint>";
 	std::string badd = "<ADD>";
 	std::string eadd = "</ADD>";
 	std::string beqbool = "<EQBoolean>";
 	std::string eeqbool = "</EQBoolean>";
 	std::string bboolvar = "<BooleanVariable>";
 	std::string eboolvar = "</BooleanVariable>";
 	std::string bbool = "<Boolean>";
 	std::string ebool = "</Boolean>";
 	std::string bcmd = "<Command>";
 	std::string ecmd = "</Command>";
 	std::string bcmdname = "<CommandName>";
 	std::string ecmdname = "</CommandName>";
 	std::string bargs = "<Arguments>";
 	std::string eargs = "</Arguments>";
	std::string end = "END";
	std::string start = "START";
 	std::string inf = "INF";
 	std::string neginf = "-INF";
	std::string fin = "FINISHED";
	std::string exec = "EXECUTING";
	std::string finin = "FINISHING";
	std::string failed = "FAILED";
	std::string booltrue = "1";
	std::string boolfalse = "0";
  std::string time="time";


//DEPRECATED


        std::string stc = "StartCondition:AND{";
	std::string prec = "PreCondition:AND{";
	std::string ic = "InvariantCondition:AND{";
	std::string poc = "PostCondition:AND{";

	std::string lookupf = "LookupWithFrequency{";
	std::string lookupn = "LookupNow{";
	std::string curtime = "CurrentTimeWithin{";
	std::string abstime = "AbsoluteTimeWithin{";

	std::string node = "Node:{";
	std::string nodeid = "NodeId:{";
	std::string nodelist = "NodeList:{";
	std::string vars = "Variables:{";
	std::string inter = "Interface:{";



	std::string infty = "PLUS_INFINIFY";
  std::string mininfty = "MINUS_INFINITY";

  std::string state = "state";	
	std::string dot =".";	
	std::string comma =",";	
	std::string rbracket ="]";	
	std::string lbracket ="[";	
	std::string endbrace = "}";
	std::string plus = "+";
	
	std::string andstr = "AND";
	std::string orstr = "OR";

	std::string freq = "1.0";

	//following works: string = string + string!


  //Need to fill this out enough to put it in a std::set
  //Conor's recommendation: use Ids
  class PLEXILCLARATyCmdSpec;
  typedef EUROPA::Id<PLEXILCLARATyCmdSpec> PlexilCmdId;
  class PLEXILCLARATyCmdSpec{
  public:
    std::string name;
    std::set<int> params;
    PLEXILCLARATyCmdSpec::PLEXILCLARATyCmdSpec():m_id(this){}
    PLEXILCLARATyCmdSpec::~PLEXILCLARATyCmdSpec(){
      m_id.remove();
    }
    PLEXILCLARATyCmdSpec::PLEXILCLARATyCmdSpec(PLEXILCLARATyCmdSpec &inSpec){
      name=inSpec.name;
      params=inSpec.params;
    }
    void PLEXILCLARATyCmdSpec::setName(std::string inName){
      name=inName;
    }
    std::string PLEXILCLARATyCmdSpec::getName(){
      return name;
    }
    void PLEXILCLARATyCmdSpec::addParam(int inParam){
      params.insert(inParam);
    }
    bool PLEXILCLARATyCmdSpec::isParamPrinted(int printParam){
      if(params.count(printParam)>0){
	return true;
      }
      else{
	return false;
      }
    }
    bool PLEXILCLARATyCmdSpec::operator =(PLEXILCLARATyCmdSpec &inSpec){
      if(name !=inSpec.name){
	return false;
      }	
      if(params !=inSpec.params){
	return false;
      }
      else return true;
    }
    const PlexilCmdId& getId() const{ 
      return m_id; 
    } 

    //Meant for debugging purposes only; deprecate as soon as things work
    void PLEXILCLARATyCmdSpec::writeSpec(){
      cout << "Spec:\n";
      cout << name << "\n";
      //Wow!  std::set has no output operator.
      for(std::set<int>::const_iterator sit = params.begin(); sit != params.end(); ++sit){
	int i=*sit;
	cout << i << ",";
      }
      cout << "\n";
    }

  private:
    PlexilCmdId m_id; 
  };

  typedef std::set<std::string> StringSet;
  typedef std::set<PlexilCmdId> SpecSet;

  class PLEXILCLARATyPlanDatabaseWriter {

  public:


	TemporalAdvisorId advisor;
	StringSet actionPreds;
	StringSet statePreds;
	StringSet childNodes;
	TokenSet actionTokens;
	TokenSet stateTokens;
        SpecSet actionSpecs;
        SpecSet stateSpecs;
 
	std::string nodeHeader;
	std::string startCondition;
	std::string postCondition;
	std::string preCondition;
	std::string invariantCondition;
	std::string nodekey;

    //temporary string to hang on to one condition
    std::string startcond;
    std::string precond;
    std::string postcond;
    std::string invcond;

    //string to hang on to all conditions, with connective logical expressions
    std::string startcondSet;
    std::string precondSet;
    std::string postcondSet;
    std::string invcondSet;

	PLEXILCLARATyPlanDatabaseWriter::PLEXILCLARATyPlanDatabaseWriter(){}
	PLEXILCLARATyPlanDatabaseWriter::~PLEXILCLARATyPlanDatabaseWriter(){}


	//Helper functions

	bool PLEXILCLARATyPlanDatabaseWriter::mustPrecede(const TokenId& t,const TokenId& prec, IntervalDomain& d1){
		//Test to see if prec must precede t.  If so, then pend must precede tstart
		TempVarId tstart= t->getStart();
		TempVarId pend = prec->getEnd();
		d1 = advisor->getTemporalDistanceDomain(pend,tstart,true);
		if(d1.getLowerBound()>0){
			return true;
		}
		else{
			return false;
		}
	}


	bool PLEXILCLARATyPlanDatabaseWriter::mustEndAfter(const TokenId& t,const TokenId& prec,IntervalDomain& d1){
		//Test to see if pend must precede tend
		TempVarId tend= t->getEnd();
		TempVarId pend= prec->getEnd();
		d1 = advisor->getTemporalDistanceDomain(pend,tend,true);
		if(d1.getLowerBound()>=0){
			return true;
		}
		else{
			return false;
		}
	}

	bool PLEXILCLARATyPlanDatabaseWriter::mustStartBeforeStart(const TokenId& t,const TokenId& prec, IntervalDomain& d1){
		//Test to see if pstart must precede tstart
		TempVarId tstart= t->getStart();
		TempVarId pstart = prec->getStart();
		d1 = advisor->getTemporalDistanceDomain(pstart,tstart,true);
		if(d1.getLowerBound()>=0){
			return true;
		}
		else{
			return false;
		}
	}

	 bool PLEXILCLARATyPlanDatabaseWriter::isContainedBy(const TokenId& t,
							const TokenId& conc){
		//Returns true if cstart must precede tstart, tend must precede cend
		TempVarId tend= t->getEnd();
		TempVarId tstart= t->getStart();
		TempVarId cstart = conc->getStart();
		TempVarId cend = conc->getEnd();
		IntervalDomain d1,d2;
		d1 = advisor->getTemporalDistanceDomain(cstart,tstart,true);
		d2 = advisor->getTemporalDistanceDomain(tend,cend,true);
		if((d1.getLowerBound()>=0)&&(d2.getLowerBound()>=0)){
			return true;
		}
		else{
			return false;
		}
	}

	 bool PLEXILCLARATyPlanDatabaseWriter::meets(const TokenId& t,const TokenId& meets){
		TempVarId tend= t->getEnd();
		TempVarId sstart = meets->getStart();
		IntervalDomain d1 = advisor->getTemporalDistanceDomain(tend,sstart,true);
		if((d1.getLowerBound()==0)&&(d1.getUpperBound()==0)){
			return true;
		}
		else{
			return false;
		}
	}


	 bool PLEXILCLARATyPlanDatabaseWriter::gatherTokens(PlanDatabaseId db,TokenSet& actionTokens, TokenSet& stateTokens){
		check_error(!db->getConstraintEngine()->provenInconsistent());
      		const TokenSet& alltokens = db->getTokens();
     		for(TokenSet::const_iterator tokit = alltokens.begin(); tokit != alltokens.end(); ++tokit){
	    		TokenId t = *tokit;
			if (t->isActive()){
				if (isPlexilState(t)){
	    				stateTokens.insert(t);
				}
				else if (isPlexilAction(t)){
	    				actionTokens.insert(t);
				}
			}
	  	}
		check_error(!(actionTokens.empty()&&(stateTokens.empty())));
		return true;
	}

	 void PLEXILCLARATyPlanDatabaseWriter::readIgnoredByPlexil(StringSet& actionPreds,StringSet& statePreds){
		bool error=false;
		bool sawState=false;
		char *inputFileName = "PLEXILCLARATy-Ignore-Predicates.txt";
		std::ifstream in(inputFileName);
		std::string nextPred;
		int nextParam;
		/*Can't use this method to test for sane opening of file!  Why not?
		if (in.is_open()) {
			  cerr << "Can't open input file " << inputFileName << endl;
		  exit(1);
		}*/

		PLEXILCLARATyCmdSpec *spec;
		PlexilCmdId id;

		//File format expected to be:
		//Action: {list of cmd specs}
		//State: {list of cmd specs}
		//CmdSpec: name, {list of params, last is -1 (yech!)}
		//Lists could be empty
		//Later on add comments
		//Need some more bulletproofing

		in >> nextPred;
		if(nextPred == "Action:"){
			while(in.good()){
				in >> nextPred;
				if(nextPred == "State:")
					break;
				else {
				  actionPreds.insert(nextPred);
				  spec=new PLEXILCLARATyCmdSpec;
				  spec->setName(nextPred);
				  while(in.good()){
				    in >> nextParam;
				    if(nextParam!=-1){
				      spec->addParam(nextParam);
				    }
				    else break;
				  }
				  id=spec->getId();
				  actionSpecs.insert(id);
				}
			}
			while(in.good()){
				sawState=true;
				in >> nextPred;
				statePreds.insert(nextPred);
				spec=new PLEXILCLARATyCmdSpec;
				spec->setName(nextPred);
			        while(in.good()){
				  in >> nextParam;
				  if(nextParam!=-1){
				    spec->addParam(nextParam);
				  }
				  else break;
				}
				id=spec->getId();
				stateSpecs.insert(id);
			}
			in.close();
		}
		else {
			error=true;
		}
		if((error==true)||(sawState==false)){	
			  cerr << "Malformed EUROPA-PLEXILCLARATy translation configuration file " << inputFileName << endl;
		}
	}

	 bool PLEXILCLARATyPlanDatabaseWriter::isPlexilState(const TokenId& t){
		std::string pred = t->getPredicateName().toString();
		if(statePreds.count(pred) >0)
			return true;
		else return false;
	}


	 bool PLEXILCLARATyPlanDatabaseWriter::isPlexilAction(const TokenId& t){
		std::string pred = t->getPredicateName().toString();
		if(actionPreds.count(pred) >0)
			return true;
		else return false;
	}


	//I can't believe I need to do this.
	//C++ on this machine doesn't appear to have itoa or ltoa or other functions to handle this.
	//Stolen wholesale from the web

	std::string PLEXILCLARATyPlanDatabaseWriter::itoa(int value, int base) {		
		enum { kMaxDigits = 35 };	
		std::string buf;		
		buf.reserve( kMaxDigits ); // Pre-allocate enough space.	
			
		// check that the base if valid	
		// replace this with checkerror one of these days
		if (base < 2 || base > 16) return buf;	
		
		int quotient = value;	
	
		// Translating number to string with base:	
		
		do {	
			buf += "0123456789abcdef"[ std::abs( quotient % base ) ];	
			quotient /= base;	
		} while ( quotient );	
		
		// Append the negative sign for base 10	
		
		if ( value < 0 && base == 10) buf += '-';	
			std::reverse( buf.begin(), buf.end() );	
		return buf;		
	}	

        void PLEXILCLARATyPlanDatabaseWriter::appendParamsToString(PlexilCmdId cmd,
							       std::vector<ConstrainedVariableId> &varList, 
							       std::string &condString){
	  int j=1;
	  bool first=true;
	  for (std::vector<ConstrainedVariableId>::const_iterator varit = varList.begin(); varit != varList.end(); ++varit) {
	    //Check to see if we really want this var
	    if(cmd->isParamPrinted(j)==true){
	      if(first==true){
		first=false;
	      }
	      else condString=condString+ ",\n ";
	      ConstrainedVariableId v= (*varit);
	      
	      
	      //Check to see if this is a Location; if so, decompose to it's X,Y coordinates.
	      //Clean up PLEXILcmdspec to capture this later and make it more generic.
	      //Consider breaking this out as a function.
	      
	      if(v->derivedDomain().getTypeName().toString() == "Location"){
		double d = v->derivedDomain().getSingletonValue();
		//EVIL!
		ObjectId oid = (ObjectId)d;
		std::vector<ConstrainedVariableId> vv=oid->getVariables();
		for (std::vector<ConstrainedVariableId>::const_iterator varit2 = vv.begin(); varit2 != vv.end(); ++varit2) {
		  ConstrainedVariableId cvid=*varit2;
		  
		  //Asking PLASMA for variable name provides the "fully qualified name".
		  //That is, we get "rock2.position_x".
		  //We will do substring check and assume we don't have a problem.
		  //To do this right, we simply find position of the last period, 
		  //and replace the 0 below with that position.
		  
		  std::string s=cvid->getName().toString();
		  if(std::string::npos != s.find("position_x",0)){
		    condString=condString+  cvid->derivedDomain().toString(cvid->derivedDomain().getSingletonValue())+",\n";
		  }
		  if(std::string::npos != s.find("position_y",0)){
		    condString=condString+  cvid->derivedDomain().toString(cvid->derivedDomain().getSingletonValue());
		  }
		}
	      }
	      else 
		condString=condString+  v->derivedDomain().toString(v->derivedDomain().getSingletonValue());
	      
	    }
	    j++;
	  }
	}

      void PLEXILCLARATyPlanDatabaseWriter::writePLEXILCLARATyPlan(PlanDatabaseId& db,
							std::ostream& os){
		check_error(db.isValid() && advisor.isNoId());
		IntervalIntDomain d1,d2;
		std::vector<ConstrainedVariableId> varList;
		ConstrainedVariableId v; 
		bool first;
		advisor = db->getTemporalAdvisor();
		readIgnoredByPlexil(actionPreds,statePreds);
		gatherTokens(db,actionTokens,stateTokens);
		
		int startcondCount;
		int precondCount;
		int invcondCount;
		int postcondCount;

		std::string plan;
		plan=preamble + "\n"+preamble2 +"\n";
		plan=plan+bnode+"\n";
		plan=plan+bnodeid+"DEMORoverPlan"+enodeid+"\n";
		plan=plan+bnodelist+"\n";

		
		//Write PLEXILCLARATy nodes for all action tokens
     		for(TokenSet::const_iterator tokit = actionTokens.begin(); tokit != actionTokens.end(); ++tokit){
		        startcondCount=0;
			precondCount=0;
			invcondCount=0;
			postcondCount=0;

	    		TokenId t = *tokit;
			nodekey = itoa(t->getKey(),10);
			childNodes.insert(nodekey);

			//Node header
			nodeHeader=node + nodeid;
			nodeHeader=nodeHeader + nodekey;
			nodeHeader=nodeHeader + endbrace + "\n";

			plan=plan+bnode+"\n";
			plan=plan+bnodeid+nodekey+enodeid +"\n";


			first=true;
			varList = t->getParameters();
			//Retrieve cmd spec to ensure we know what variables we need
			PlexilCmdId id;
			for(SpecSet::const_iterator ait=actionSpecs.begin(); ait!=actionSpecs.end(); ++ait){
			  id=*ait;
			  if(id->getName()==t->getPredicateName().toString()){
			    break;
			  }
			}
			os << nodeHeader; 	

			startCondition=stc;
			preCondition=prec;			
			postCondition=poc;
			invariantCondition=ic;

			startcond="";
			precond="";			
			postcond="";
			invcond="";

			startcondSet="";
			precondSet="";			
			postcondSet="";
			invcondSet="";

			bool firstPost=true;

			TempVarId tend= t->getEnd();
			TempVarId tstart = t->getStart();
			d1=tstart->derivedDomain();

			//Pre condition for absolute start time
			preCondition = preCondition + abstime;
			preCondition = preCondition + mininfty;
			preCondition = preCondition + "," + itoa((int)d1.getUpperBound(),10);
			preCondition = preCondition + endbrace;

			precond=precond+bpc+"\n";
			precondCount++;
			precond=precond+band+"\n";
			precond=precond+bge+"\n";
			precond=precond+blookfreq+"\n";
			precond=precond+bstate+time+estate+"\n";
			precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
			precond=precond+elookfreq+"\n";			
			precond=precond+bneginf+neginf+eneginf+"\n";
			precond=precond+ege+"\n";

			precondCount++;
			precondSet=precond;
			precond="";
			precond=precond+ble+"\n";
			precond=precond+blookfreq+"\n";
			precond=precond+bstate+time+estate+"\n";
			precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
			precond=precond+elookfreq+"\n";
			precond=precond+breal+itoa((int)d1.getUpperBound(),10)+ereal+"\n";		
			precond=precond+ele+"\n";

			//Start condition for absolute start time
			startCondition = startCondition + abstime;
			startCondition = startCondition + itoa((int)d1.getLowerBound(),10);
			startCondition = startCondition + "," + infty + endbrace;


			startcond=startcond+bstc+"\n";
			startcondCount++;
			startcond=startcond+band+"\n";
			startcond=startcond+bge+"\n";
			startcond=startcond+blookfreq+"\n";
			startcond=startcond+bstate+time+estate+"\n";
			startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
			startcond=startcond+elookfreq+"\n";			
			startcond=startcond+breal+itoa((int)d1.getLowerBound(),10)+ereal+"\n";
			startcond=startcond+ege+"\n";

			startcondSet=startcond;
			startcond="";
			startcondCount++;
			startcond=startcond+ble+"\n";
			startcond=startcond+blookfreq+"\n";
			startcond=startcond+bstate+time+estate+"\n";
			startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
			startcond=startcond+elookfreq+"\n";
			startcond=startcond+binf+inf+einf+"\n";		
			startcond=startcond+ele+"\n";

			//Invariant condition for token duration
			d1=advisor->getTemporalDistanceDomain(tstart,tend,true);
			invariantCondition = invariantCondition + curtime;
			invariantCondition = invariantCondition + nodekey +".start.time";
			invariantCondition = invariantCondition + "," + nodekey +".start.time";
			//Time in E2 is always an int so this hack works.
			invariantCondition = invariantCondition + "+" + itoa((int)d1.getUpperBound(),10);
			invariantCondition = invariantCondition + endbrace;


			invcond=invcond+bic+"\n";
			invcondCount++;
			invcond=invcond+band+"\n";
			invcond=invcond+bge+"\n";
			invcond=invcond+blookfreq+"\n";
			invcond=invcond+bstate+time+estate+"\n";
			invcond=invcond+bfreq+breal+freq+ereal+efreq+"\n";
			invcond=invcond+elookfreq+"\n";			
			invcond=invcond+bnodetimeval+"\n";
			invcond=invcond+bnodeid+nodekey+enodeid+"\n";
			invcond=invcond+bnodestateval+exec+enodestateval+"\n";
			invcond=invcond+btp+start+etp+"\n";
			invcond=invcond+enodetimeval+"\n";
			invcond=invcond+ege+"\n";
			invcondSet=invcond;
			invcond="";
			invcondCount++;
			invcond=invcond+ble+"\n";
			invcond=invcond+blookfreq+"\n";
			invcond=invcond+bstate+time+estate+"\n";
			invcond=invcond+bfreq+breal+freq+ereal+efreq+"\n";
			invcond=invcond+elookfreq+"\n";
			invcond=invcond+badd+"\n";
			invcond=invcond+bnodetimeval+"\n";
			invcond=invcond+bnodeid+nodekey+enodeid+"\n";
			invcond=invcond+bnodestateval+exec+enodestateval+"\n";
			invcond=invcond+btp+start+etp+"\n";
			invcond=invcond+enodetimeval+"\n";
			invcond=invcond+breal+itoa((int)d1.getUpperBound(),10)+ereal+"\n";
			invcond=invcond+eadd+"\n";
			invcond=invcond+ele+"\n";

			postcond=postcond+bpoc+"\n";

			//Handle conditions from action-action pairs
			
     			for(TokenSet::const_iterator tokit2 = actionTokens.begin(); tokit2 != actionTokens.end(); ++tokit2){
	    			TokenId t2 = *tokit2;
				if(t != t2){


					if(mustPrecede(t,t2,d1)){
						//In this case know t2 must be finished
						//StartCondition

						nodekey = itoa(t2->getKey(),10);
						startCondition = startCondition + ",\n ";
						startCondition = startCondition + nodekey + " " + fin + ",\n";
						startCondition = startCondition + curtime;
						startCondition = startCondition + nodekey +".end.time";
						//Time in E2 is always an int so this hack works.
						startCondition = startCondition + "+" + itoa((int)d1.getLowerBound(),10);
 						startCondition = startCondition + "," + infty + endbrace;

						//know 2 startconds already, add another preceeding and
						startcondCount++;
						startcondSet=startcondSet+band+"\n"+startcond+"\n";
						startcond="";
						startcond=startcond+bge+"\n";
						startcond=startcond+blookfreq+"\n";
						startcond=startcond+bstate+time+estate+"\n";
						startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
						startcond=startcond+elookfreq+"\n";			
						startcond=startcond+badd+"\n";
						startcond=startcond+bnodetimeval+"\n";
						startcond=startcond+bnodeid+nodekey+enodeid+"\n";
						startcond=startcond+bnodestateval+exec+enodestateval+"\n";
						startcond=startcond+btp+end+etp+"\n";
						startcond=startcond+enodetimeval+"\n";
						startcond=startcond+breal+itoa((int)d1.getLowerBound(),10)+ereal+"\n";
						startcond=startcond+eadd+"\n";
						startcond=startcond+ege+"\n";
						
						startcondCount++;
						startcondSet=startcondSet+band+"\n"+startcond+"\n";
						startcond="";
						startcond=startcond+ble+"\n";
						startcond=startcond+blookfreq+"\n";
						startcond=startcond+bstate+time+estate+"\n";
						startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
						startcond=startcond+elookfreq+"\n";
						startcond=startcond+binf+inf+einf+"\n";		
						startcond=startcond+ele+"\n";

						//PreCondition
						preCondition=preCondition+",\n ";
						preCondition = preCondition + curtime;
						preCondition = preCondition + nodekey +".end.time,";
						//Time in E2 is always an int so this hack works.
						preCondition = preCondition + nodekey +".end.time";
						preCondition = preCondition + "+" + itoa((int)d1.getUpperBound(),10);
 						preCondition = preCondition + endbrace;						

						//have 2 preconds, so just add another and
						precondCount++;
						precondSet=precondSet+band+"\n"+precond+"\n";
						precond="";
						precond=precond+bge+"\n";
						precond=precond+blookfreq+"\n";
						precond=precond+bstate+time+estate+"\n";
						precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
						precond=precond+elookfreq+"\n";	
						precond=precond+bnodetimeval+"\n";
						precond=precond+bnodeid+nodekey+enodeid+"\n";
						precond=precond+bnodestateval+exec+enodestateval+"\n";
						precond=precond+btp+start+etp+"\n";
						precond=precond+enodetimeval+"\n";
						precond=precond+ege+"\n";

						precondCount++;
						precondSet=precondSet+band+"\n"+precond+"\n";
						precond="";
						precond=precond+ble+"\n";
						precond=precond+blookfreq+"\n";
						precond=precond+bstate+time+estate+"\n";
						precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
						precond=precond+elookfreq+"\n";	
						precond=precond+badd+"\n";		
						precond=precond+bnodetimeval+"\n";
						precond=precond+bnodeid+nodekey+enodeid+"\n";
						precond=precond+bnodestateval+exec+enodestateval+"\n";
						precond=precond+btp+start+etp+"\n";
						precond=precond+enodetimeval+"\n";
						precond=precond+breal+itoa((int)d1.getUpperBound(),10)+ereal+"\n";
						precond=precond+eadd+"\n";
						precond=precond+ele+"\n";

					}


					else if(mustStartBeforeStart(t2,t,d1)){
						if(mustEndAfter(t,t2,d2)){
							//in this case know t2 contained by t 
							if(firstPost==false){
								postCondition=postCondition+",";
							}
							firstPost=false;
							nodekey = itoa(t2->getKey(),10);
							postCondition = postCondition + curtime;
							postCondition = postCondition + nodekey +".end.time,";
							postCondition = postCondition + nodekey +".end.time";
							//Time in E2 is always an int so this hack works.
							postCondition = postCondition + "+" + itoa((int)d2.getUpperBound(),10)+ endbrace;

							//This might be first post cond, so check count 
							//If it is, dump this postcond with a previous band
							postcondCount++;
							if(postcondCount>1){
							  postcondSet=postcondSet+band+"\n"+postcond+"\n";
							  postcond="";
							}
							postcond=postcond+bge+"\n";
							postcond=postcond+blookfreq+"\n";
							postcond=postcond+bstate+time+estate+"\n";
							postcond=postcond+bfreq+breal+freq+ereal+efreq+"\n";
							postcond=postcond+elookfreq+"\n";	
							postcond=postcond+bnodetimeval+"\n";
							postcond=postcond+bnodeid+nodekey+enodeid+"\n";
							postcond=postcond+bnodestateval+exec+enodestateval+"\n";
							postcond=postcond+btp+end+etp+"\n";
							postcond=postcond+enodetimeval+"\n";
							postcond=postcond+ege+"\n";
							
							postcondSet=postcondSet+band+"\n"+postcond+"\n";
							postcond="";
							postcondCount++;
							postcond=postcond+ble+"\n";
							postcond=postcond+blookfreq+"\n";
							postcond=postcond+bstate+time+estate+"\n";
							postcond=postcond+bfreq+breal+freq+ereal+efreq+"\n";
							postcond=postcond+elookfreq+"\n";	
							postcond=postcond+badd+"\n";		
							postcond=postcond+bnodetimeval+"\n";
							postcond=postcond+bnodeid+nodekey+enodeid+"\n";
							postcond=postcond+bnodestateval+exec+enodestateval+"\n";
							postcond=postcond+btp+end+etp+"\n";
							postcond=postcond+enodetimeval+"\n";
							postcond=postcond+breal+itoa((int)d2.getUpperBound(),10)+ereal+"\n";
							postcond=postcond+eadd+"\n";
							postcond=postcond+ele+"\n";

						}
					}

					else if(mustStartBeforeStart(t,t2,d1)){

						nodekey = itoa(t->getKey(),10);
						preCondition=preCondition+",\n ";
						preCondition = preCondition + curtime;
						preCondition = preCondition + nodekey +".start.time,";
						//Time in E2 is always an int so this hack works.
						preCondition = preCondition + nodekey +".start.time";
						preCondition = preCondition + "+" + itoa((int)d1.getUpperBound(),10);
 						preCondition = preCondition + endbrace;

						precondSet=precondSet+band+"\n"+precond+"\n";
						precond="";
						precondCount++;
						precond=precond+bge+"\n";
						precond=precond+blookfreq+"\n";
						precond=precond+bstate+time+estate+"\n";
						precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
						precond=precond+elookfreq+"\n";	
						precond=precond+bnodetimeval+"\n";
						precond=precond+bnodeid+nodekey+enodeid+"\n";
						precond=precond+bnodestateval+exec+enodestateval+"\n";
						precond=precond+btp+start+etp+"\n";
						precond=precond+enodetimeval+"\n";
						precond=precond+ege+"\n";

						precondSet=precondSet+band+"\n"+precond+"\n";
						precond="";
						precondCount++;
						precond=precond+ble+"\n";
						precond=precond+blookfreq+"\n";
						precond=precond+bstate+time+estate+"\n";
						precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
						precond=precond+elookfreq+"\n";	
						precond=precond+badd+"\n";		
						precond=precond+bnodetimeval+"\n";
						precond=precond+bnodeid+nodekey+enodeid+"\n";
						precond=precond+bnodestateval+exec+enodestateval+"\n";
						precond=precond+btp+start+etp+"\n";
						precond=precond+enodetimeval+"\n";
						precond=precond+breal+itoa((int)d1.getUpperBound(),10)+ereal+"\n";
						precond=precond+eadd+"\n";
						precond=precond+ele+"\n";

						if(mustEndAfter(t,t2,d2)){

							//in this case know t contained by t2 
							nodekey = itoa(t2->getKey(),10);
							startCondition = startCondition + ",\n ";
							startCondition = startCondition + nodekey + " " + exec + ",\n ";
							startCondition = startCondition + curtime;
							startCondition = startCondition + nodekey +".start.time";
							//Time in E2 is always an int so this hack works.
							startCondition = startCondition + "+" + itoa((int)d1.getLowerBound(),10);
 							startCondition = startCondition + "," + infty + endbrace;


							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+bge+"\n";
							startcond=startcond+blookfreq+"\n";
							startcond=startcond+bstate+time+estate+"\n";
							startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
							startcond=startcond+elookfreq+"\n";	
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+btp+start+etp+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+ege+"\n";
							
							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+ble+"\n";
							startcond=startcond+blookfreq+"\n";
							startcond=startcond+bstate+time+estate+"\n";
							startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
							startcond=startcond+elookfreq+"\n";	
							startcond=startcond+badd+"\n";		
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+btp+start+etp+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+breal+itoa((int)d1.getLowerBound(),10)+ereal+"\n";
							startcond=startcond+eadd+"\n";
							startcond=startcond+ele+"\n";

							if(firstPost==false){
								postCondition=postCondition+",";
							}
							firstPost=false;
							nodekey = itoa(t2->getKey(),10);
							postCondition = postCondition + curtime;
							postCondition = postCondition + nodekey +".end.time,";
							postCondition = postCondition + nodekey +".end.time";
							//Time in E2 is always an int so this hack works.
							postCondition = postCondition + "+" + itoa((int)d2.getLowerBound(),10)+ endbrace;

							postcondCount++;
							if(postcondCount>1){
							  postcondSet=postcondSet+band+"\n"+postcond+"\n";
							  postcond="";
							}
							postcond=postcond+bge+"\n";
							postcond=postcond+blookfreq+"\n";
							postcond=postcond+bstate+time+estate+"\n";
							postcond=postcond+bfreq+breal+freq+ereal+efreq+"\n";
							postcond=postcond+elookfreq+"\n";	
							postcond=postcond+bnodetimeval+"\n";
							postcond=postcond+bnodeid+nodekey+enodeid+"\n";
							postcond=postcond+bnodestateval+exec+enodestateval+"\n";
							postcond=postcond+btp+end+etp+"\n";
							postcond=postcond+enodetimeval+"\n";
							postcond=postcond+ege+"\n";
							
							postcondCount++;
							postcondSet=postcondSet+band+"\n"+postcond+"\n";
							postcond="";
							postcond=postcond+ble+"\n";
							postcond=postcond+blookfreq+"\n";
							postcond=postcond+bstate+time+estate+"\n";
							postcond=postcond+bfreq+breal+freq+ereal+efreq+"\n";
							postcond=postcond+elookfreq+"\n";	
							postcond=postcond+badd+"\n";		
							postcond=postcond+bnodetimeval+"\n";
							postcond=postcond+bnodeid+nodekey+enodeid+"\n";
							postcond=postcond+bnodestateval+exec+enodestateval+"\n";
							postcond=postcond+btp+end+etp+"\n";
							postcond=postcond+enodetimeval+"\n";
							postcond=postcond+breal+itoa((int)d2.getLowerBound(),10)+ereal+"\n";
							postcond=postcond+eadd+"\n";
							postcond=postcond+ele+"\n";


						}
						else{
							//In this case we need to construct the disjunction, since t2 could
							//either be executing or finishing or finished

							startCondition = startCondition + ",\n ";
							startCondition = startCondition + "OR{";
							startCondition = startCondition + nodekey + " " + fin + ",";
							startCondition = startCondition + nodekey + " " + finin + ",";
							startCondition = startCondition + nodekey + " " + exec + endbrace +",\n ";

							//After disjunction over state, refer only to start time for temporal
							//condition
							
							startCondition = startCondition + curtime;
							startCondition = startCondition + nodekey +".start.time";
							//Time in E2 is always an int so this hack works.
							startCondition = startCondition + "+" + itoa((int)d1.getLowerBound(),10);
 							startCondition = startCondition + "," + infty + endbrace + "\n";
							startCondition = startCondition + endbrace +"\n";



							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+bor+"\n";
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+bor+"\n";
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+fin+enodestateval+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+finin+enodestateval+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+eor+"\n";
							startcond=startcond+eor+"\n";


							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+bge+"\n";
							startcond=startcond+blookfreq+"\n";
							startcond=startcond+bstate+time+estate+"\n";
							startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
							startcond=startcond+elookfreq+"\n";
							startcond=startcond+badd+"\n";		
							startcond=startcond+bnodetimeval+"\n";	
							startcond=startcond+bnodeid+nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+btp+start+etp+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+breal+itoa((int)d1.getLowerBound(),10)+ereal+"\n";
							startcond=startcond+eadd+"\n";
							startcond=startcond+ege+"\n";
							

							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+ble+"\n";
							startcond=startcond+blookfreq+"\n";
							startcond=startcond+bstate+time+estate+"\n";
							startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
							startcond=startcond+elookfreq+"\n";	
							startcond=startcond+binf+inf+einf+"\n";
							startcond=startcond+ele+"\n";


						}
					}
				}
			}		


			//Handle tokens from action-state token pairs
			
     			for(TokenSet::const_iterator tokit2 = stateTokens.begin(); tokit2 != stateTokens.end(); ++tokit2){
				TokenId t2 = *tokit2;
				varList = t2->getParameters();
				PlexilCmdId id2;
				for(SpecSet::const_iterator bit=stateSpecs.begin(); bit!=stateSpecs.end(); ++bit){
				  id2=*bit;
				  if(id2->getName()==t2->getPredicateName().toString()){
				    break;
				  }
				}
				if(meets(t2,t)){
					//precondtion
					preCondition = preCondition+",\n ";
					preCondition = preCondition + lookupn;
					//The predicate name is currently a "fully qualified name" including the attribute name.
					//E2 doesn't provide a "simple" output for this yet.
					//To handle this, exploit fact that what we want is really the substring from the position of 
					//the last . in the fully qualified name to the end of the string.
					std::string fqn=t2->getPredicateName().toString();
					int slen=fqn.length();
					int lastdot=fqn.find_last_of(".");
					preCondition = preCondition + fqn.substr(lastdot+1,slen-lastdot) +"(";
					appendParamsToString(id2,varList,preCondition);
					preCondition=preCondition+ ")" + endbrace;

					std::string stateDescr;
					stateDescr = fqn.substr(lastdot+1,slen-lastdot)+"(";
					appendParamsToString(id2,varList,stateDescr);
					stateDescr = stateDescr + ")"; 


					precondCount++;
					precondSet=precondSet+band+"\n"+precond+"\n";
					precond="";					
					precond=precond+beqbool+"\n";
					precond=precond+blooknow+"\n";
					precond=precond+bstate+"\n";
					precond=precond+stateDescr+"\n";
					precond=precond+estate+"\n";
					precond=precond+elooknow+"\n";
					precond=precond+bboolvar+bbool +boolfalse+ ebool + eboolvar+"\n";
					precond=precond+eeqbool+"\n";


				}
				else if(meets(t,t2)){
					//postcondition
					if(firstPost==false){
						postCondition=postCondition+",\n ";
					}
					firstPost=false;
					postCondition = postCondition + lookupn;

					//The predicate name is currently a "fully qualified name" including the attribute name.
					//E2 doesn't provide a "simple" output for this yet.
					//To handle this, exploit fact that what we want is really the substring from the position of 
					//the last . in the fully qualified name to the end of the string.
					std::string fqn=t2->getPredicateName().toString();
					int slen=fqn.length();
					int lastdot=fqn.find_last_of(".");
					postCondition = postCondition + fqn.substr(lastdot+1,slen-lastdot) +"(";
					appendParamsToString(id2,varList,postCondition);					
					postCondition = postCondition + ")" + endbrace;



					std::string stateDescr;
					stateDescr = fqn.substr(lastdot+1,slen-lastdot)+"(";
					appendParamsToString(id2,varList,stateDescr);
					stateDescr = stateDescr + ")"; 

					//This might be first post cond, so check count 
					//If it is, dump this postcond with a previous band
					postcondCount++;
					if(postcondCount>1){
					  postcondSet=postcondSet+band+"\n"+postcond+"\n";
					  postcond="";
					}
					postcond=postcond+beqbool+"\n";
					postcond=postcond+blooknow+"\n";
					postcond=postcond+bstate+"\n";
					postcond=postcond+stateDescr+"\n";
					postcond=postcond+estate+"\n";
					postcond=postcond+elooknow+"\n";
					postcond=postcond+bboolvar+bbool +boolfalse+ ebool + eboolvar+"\n";
					postcond=postcond+eeqbool+"\n";


				}
				else if(isContainedBy(t,t2)){
					//Invariantcondition
					invariantCondition = invariantCondition+",\n ";
					invariantCondition = invariantCondition + lookupf;


					//The predicate name is currently a "fully qualified name" including the attribute name.
					//E2 doesn't provide a "simple" output for this yet.
					//To handle this, exploit fact that what we want is really the substring from the position of 
					//the last . in the fully qualified name to the end of the string.
					std::string fqn=t2->getPredicateName().toString();
					int slen=fqn.length();
					int lastdot=fqn.find_last_of(".");
					invariantCondition = invariantCondition + fqn.substr(lastdot+1,slen-lastdot) +"(";
					appendParamsToString(id2,varList,invariantCondition);					
					invariantCondition = invariantCondition+"," + freq + endbrace;




					std::string stateDescr;
					stateDescr = fqn.substr(lastdot+1,slen-lastdot)+"(";
					appendParamsToString(id2,varList,stateDescr);
					stateDescr = stateDescr + ")"; 

					//know 2 invconds already, add another preceeding and
					invcondCount++;
					invcondSet=invcondSet+band+"\n"+invcond+"\n";
					invcond="";
					invcond=invcond+beqbool+"\n";
					invcond=invcond+blookfreq+"\n";
					invcond=invcond+bstate + time + estate + "\n";
					invcond=invcond+bfreq + freq + efreq + "\n";
					invcond=invcond+elookfreq+"\n";
					invcond=invcond+bstate+"\n";
					invcond=invcond+stateDescr+"\n";
					invcond=invcond+estate+"\n";
					invcond=invcond+elookfreq+"\n";
					invcond=invcond+bboolvar+bbool +boolfalse+ ebool +eboolvar+"\n";
					invcond=invcond+eeqbool+"\n";



				}
			}


			//OK.  We have to append the last condition to each set.
			//We then have to append the correct number of eands to each set; this is *condcount-1.

			int q;
			startcondSet=startcondSet+startcond+"\n";
			for(q=1;q<startcondCount;q++){
			  startcondSet=startcondSet+eand+"\n";
			}
			startcondSet=startcondSet+estc+"\n";

			precondSet=precondSet+precond+"\n";
			for(q=1;q<precondCount;q++){
			  precondSet=precondSet+eand+"\n";
			}
			precondSet=precondSet+epc+"\n";

			invcondSet=invcondSet+invcond+"\n";
			for(q=1;q<invcondCount;q++){
			  invcondSet=invcondSet+eand+"\n";
			}
			invcondSet=invcondSet+eic+"\n";


			if(firstPost==false){
			  postcondSet=postcond+"\n";
			  for(q=1;q<postcondCount;q++){
			    postcondSet=postcondSet+eand+"\n";
			  }
			  postcondSet=postcondSet+epoc+"\n";
			}

			//All conditions gathered, write them out!

			if(startCondition!=stc){	
				os << startCondition;
				os << endbrace+ "\n";
			}	
			if(invariantCondition!=ic){
				os << invariantCondition;
				os << endbrace+ "\n";
			}	
			if(postCondition!=poc){
				os << postCondition;
				os << endbrace+ "\n";
			}	
			if(preCondition!=prec){
				os << preCondition;
				os << endbrace+ "\n";
			}	

			//The predicate name is currently a "fully qualified name" including the attribute name.
			//E2 doesn't provide a "simple" output for this yet.
			//To handle this, exploit fact that what we want is really the substring from the position of 
			//the last . in the fully qualified name to the end of the string.
			std::string fqn=t->getPredicateName().toString();
			int slen=fqn.length();
			int lastdot=fqn.find_last_of(".");
      			os << fqn.substr(lastdot+1,slen-lastdot) +"(";
			std::string par;
			varList = t->getParameters();
			appendParamsToString(id,varList,par);			
			os << par;
			os << ");" + endbrace + "\n\n";
			
			
			plan=plan+startcondSet;
			plan=plan+precondSet;
			plan=plan+invcondSet;
			if(firstPost==false){
			  plan=plan+postcondSet;
			}
			std::string cmdDescr;
			cmdDescr=fqn.substr(lastdot+1,slen-lastdot);
			plan= plan + bcmd + "\n" + cmdDescr + "\n" + ecmd + "\n";
			cmdDescr="";
			appendParamsToString(id,varList,cmdDescr);
			plan=plan+bargs+"\n" +cmdDescr+"\n"+eargs+"\n";
			plan=plan+enode+"\n";
		}

		//Write PLEXILCLARATy top level node with list of children

		nodekey = "0";
		nodeHeader=node + nodeid;
		nodeHeader=nodeHeader + nodekey;

		nodeHeader=nodeHeader + endbrace;
		os << nodeHeader; 	
		os << nodelist;
		std::string s;
		first=true;
     		for(StringSet::const_iterator sit = childNodes.begin(); sit != childNodes.end(); ++sit){
			if (first==true)
	    			s = *sit;
			else s = ","+*sit;
			os << s;
			first=false;
		}
		os << endbrace + endbrace + "\n\n";

		//Write PLEXILCLARATy top level failure node

		nodekey = "-1";
		nodeHeader=node + nodeid;
		nodeHeader=nodeHeader + nodekey + endbrace + "AND{";
		os << nodeHeader; 	
		first=true;
    		for(StringSet::const_iterator sit = childNodes.begin(); sit != childNodes.end(); ++sit){
			if (first==true)
	    			s = *sit;
			else s = ","+*sit;
			s=s+" "+failed;
			os << s;
			first=false;
		}
		os << endbrace + "\n";
		os << replan;
		os << endbrace;

                plan=plan+enodelist + "\n";
                plan=plan+enode + "\n";
                os << plan;


	}		
	


	


  private:

 };
}

#endif /* #ifndef _H_PLEXILCLARATyPlanDatabaseWriter */



