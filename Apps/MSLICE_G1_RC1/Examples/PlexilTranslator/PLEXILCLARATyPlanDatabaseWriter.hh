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

#include "PLEXILKeywords.hh";

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

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
    void PLEXILCLARATyCmdSpec::writeSpec()
   {
      std::cout << "Spec:\n";
      std::cout << name << "\n";
      //Wow!  std::set has no output operator.
      for(std::set<int>::const_iterator sit = params.begin(); sit != params.end(); ++sit){
	int i=*sit;
	std::cout << i << ",";
      }
      std::cout << "\n";
    }

  private:
    PlexilCmdId m_id; 
  };

  typedef std::set<std::string> StringSet;
  typedef std::set<PlexilCmdId> SpecSet;

  class PLEXILCLARATyPlanDatabaseWriter {

  public:
	TemporalAdvisorId advisor;
        SpecSet actionSpecs;
        SpecSet stateSpecs;
	StringSet actionPreds;
	StringSet statePreds; 

       int indentCount;

    PLEXILCLARATyPlanDatabaseWriter::PLEXILCLARATyPlanDatabaseWriter():indentCount(1){}
	PLEXILCLARATyPlanDatabaseWriter::~PLEXILCLARATyPlanDatabaseWriter(){}



      void PLEXILCLARATyPlanDatabaseWriter::writePLEXILCLARATyPlan(PlanDatabaseId& db,
							std::ostream& os){

	StringSet childNodes;
	TokenSet actionTokens;
	TokenSet stateTokens;

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
		plan = xmlVersion + "\n"; 
                plan = plan + begin_plexil_plan + namespaceLocation + "\n";
	
                plan= plan + indentTo(indentCount++) + bnodetype + bnodelist + enodetype + "\n";
		plan= plan + indentTo(indentCount) + bnodeid + "DEMORoverPlan" + enodeid + "\n";
	        
                plan = plan + indentTo(indentCount) + begin_nodebody + "\n";
                plan = plan + indentTo(indentCount++) + begin_full_nodelist + "\n";

		//Write PLEXILCLARATy nodes for all action tokens 
     		for(TokenSet::const_iterator tokit = actionTokens.begin(); tokit != actionTokens.end(); ++tokit){
		        startcondCount=0;
			precondCount=0;
			invcondCount=0;
			postcondCount=0;

	    		TokenId t = *tokit;
			//nodekey = itoa(t->getKey(),10);
                        nodekey = toString(t->getKey());
			childNodes.insert(nodekey);

			//Node header
		        //nodeHeader= node + nodeid;
			//nodeHeader=nodeHeader + nodeidPrefix + nodekey;
			//nodeHeader=nodeHeader + endbrace + "\n";

			plan=plan + indentTo(indentCount++) + bnode + "\n";
			plan=plan + indentTo(indentCount) + bnodeid + nodeidPrefix + nodekey+enodeid +"\n";

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
			//os << nodeHeader; 	

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
                      	preCondition = preCondition + "," + toString(d1.getUpperBound());
			preCondition = preCondition + endbrace;

			precond=precond + indentTo(indentCount++) + bpc + "\n";
			precondCount++;
			precond = precond + indentTo(indentCount++) + band + "\n";
			precond = precond + indentTo(indentCount++) + bge + "\n";
			precond = precond + indentTo(indentCount++) + blookfreq + "\n";
			precond = precond + indentTo(indentCount) + bstate + time + estate + "\n";
			precond = precond + indentTo(indentCount++) + bfreq + "\n";
                        precond = precond + indentTo(indentCount) + low_start + brealValue + freq + erealValue + low_end + "\n";
                        precond = precond + indentTo(indentCount--) + high_start + brealValue + freq + erealValue + high_end + "\n";
                        precond = precond + indentTo(indentCount--)+ efreq + "\n";
			precond = precond + indentTo(indentCount) + elookfreq + "\n";			
			precond = precond + indentTo(indentCount--) + bneginf + neginf + eneginf + "\n";
			precond = precond + indentTo(indentCount) + ege + "\n";

			precondCount++;
			precondSet=precond;
			precond="";
			precond = precond + indentTo(indentCount++) + ble + "\n";
			precond = precond + indentTo(indentCount++) + blookfreq +"\n";
			precond = precond + indentTo(indentCount) + bstate + time + estate + "\n";
			precond = precond + indentTo(indentCount++) + bfreq + "\n";
                        precond = precond + indentTo(indentCount) + low_start + brealValue + freq + erealValue + low_end + "\n";
                        precond = precond + indentTo(indentCount--) + high_start + brealValue + freq + erealValue + high_end + "\n";
                        precond = precond + indentTo(indentCount--) + efreq + "\n";
			precond = precond + indentTo(indentCount) + elookfreq + "\n";

                        precond = precond + indentTo(indentCount++) + timeValue_begin + units_begin + "\n";
                        precond = precond + indentTo(indentCount--) + integerValue_begin + toString(d1.getLowerBound()) + integerValue_end +"\n";
                        precond = precond + indentTo(indentCount--) + units_end + timeValue_end + "\n";
	
			precond=precond + indentTo(indentCount)+ ele + "\n";

			//Start condition for absolute start time
			startCondition = startCondition + abstime;
			startCondition = startCondition + toString(d1.getLowerBound());
			startCondition = startCondition + "," + infty + endbrace;

			startcond = startcond + indentTo(indentCount++) + bstc+"\n";
			startcondCount++;
			startcond=startcond+ indentTo(indentCount++) + band+"\n";
			startcond = startcond + indentTo(indentCount++) + bge + "\n";
			startcond = startcond + indentTo(indentCount++) + blookfreq + "\n";
			startcond = startcond + indentTo(indentCount) + bstate + time + estate + "\n";
			startcond = startcond + indentTo(indentCount++) + bfreq + "\n";
                        startcond = startcond + indentTo(indentCount) + low_start + brealValue + freq + erealValue + low_end + "\n";
                        startcond = startcond + indentTo(indentCount--) + high_start + brealValue + freq + erealValue + high_end + "\n";
                        startcond = startcond + indentTo(indentCount--) + efreq +"\n";
			startcond = startcond + indentTo(indentCount) + elookfreq+"\n";	
             		
                        startcond = startcond + indentTo(indentCount++) + timeValue_begin + units_begin + "\n";
                        startcond = startcond + indentTo(indentCount--) + integerValue_begin + toString(d1.getLowerBound()) + integerValue_end +"\n";
                        startcond = startcond + indentTo(indentCount--) + units_end + timeValue_end + "\n";

			startcond=startcond + indentTo(indentCount) + ege+"\n";

			startcondSet=startcond;
			startcond="";
			startcondCount++;
			startcond=startcond + indentTo(indentCount++) + ble+"\n";
			startcond=startcond + indentTo(indentCount++) + blookfreq+"\n";
			startcond=startcond + indentTo(indentCount) + bstate + time + estate+"\n";
			
                        startcond = startcond + indentTo(indentCount++) + bfreq + "\n";
                        startcond = startcond + indentTo(indentCount) + low_start + brealValue + freq + erealValue+ low_end + "\n"; 
                        startcond = startcond + indentTo(indentCount--) + high_start + brealValue + freq + erealValue+ high_end + "\n"; 
                        startcond = startcond + indentTo(indentCount--) + efreq+"\n";
			startcond = startcond + indentTo(indentCount) + elookfreq+"\n";
			startcond = startcond + indentTo(indentCount--) + binf+inf+einf+"\n";		
		
                   	startcond=startcond + indentTo(indentCount--) + ele+"\n";

			//Invariant condition for token duration
			d1=advisor->getTemporalDistanceDomain(tstart,tend,true);
			invariantCondition = invariantCondition + curtime;
			invariantCondition = invariantCondition + nodekey +".start.time";
			invariantCondition = invariantCondition + "," + nodekey +".start.time";
			//Time in E2 is always an int so this hack works.
			invariantCondition = invariantCondition + "+" + toString(d1.getUpperBound());
			invariantCondition = invariantCondition + endbrace;

			invcond=invcond+ indentTo(indentCount++)+bic+"\n";
			invcondCount++;
			invcond=invcond+ indentTo(indentCount++) + band+"\n";
			invcond=invcond + indentTo(indentCount++) + bge+"\n";
			invcond=invcond + indentTo(indentCount++) + blookfreq+"\n";
			invcond=invcond + indentTo(indentCount) + bstate + time + estate+"\n";
			invcond=invcond + indentTo(indentCount++) + bfreq + "\n";
                        invcond=invcond + indentTo(indentCount) + low_start + brealValue + freq + erealValue + low_end +"\n";
                        invcond=invcond + indentTo(indentCount--) + high_start + brealValue + freq + erealValue + high_end +"\n";
                        invcond=invcond + indentTo(indentCount--)  +efreq+"\n";
			invcond=invcond + indentTo(indentCount--) + elookfreq+"\n";			
			invcond=invcond + indentTo(indentCount++) + bnodetimeval+"\n";
			invcond=invcond + indentTo(indentCount) + bnodeid + nodeidPrefix + nodekey + enodeid +"\n";
			invcond=invcond + indentTo(indentCount) + bnodestateval+exec+enodestateval+"\n";
			invcond=invcond + indentTo(indentCount--) + btp + start + etp+"\n";
			invcond=invcond + indentTo(indentCount--) + enodetimeval+"\n";
			invcond=invcond + indentTo(indentCount) + ege+"\n";
			invcondSet=invcond;
			invcond="";
			invcondCount++;
			invcond=invcond + indentTo(indentCount++) + ble+"\n";
			invcond=invcond + indentTo(indentCount++) + blookfreq+"\n";
	          	invcond=invcond + indentTo(indentCount) + bstate + time + estate+"\n";
			invcond=invcond + indentTo(indentCount++) + bfreq + "\n";
                        invcond=invcond + indentTo(indentCount) + low_start + brealValue + freq + erealValue + low_end +"\n";
                        invcond=invcond + indentTo(indentCount--) + high_start + brealValue + freq + erealValue + high_end +"\n";
                        invcond=invcond + indentTo(indentCount--)  +efreq+"\n"; 
			invcond=invcond + indentTo(indentCount--) +elookfreq+"\n";
		
                 	invcond=invcond + indentTo(indentCount++) + badd+"\n";
			invcond=invcond + indentTo(indentCount++) + bnodetimeval+"\n";
			invcond=invcond + indentTo(indentCount) + bnodeid + nodeidPrefix + nodekey+enodeid+"\n";
			invcond=invcond + indentTo(indentCount) + bnodestateval+exec+enodestateval+"\n";
			invcond=invcond + indentTo(indentCount--) + btp+start+etp+"\n";
			invcond=invcond + indentTo(indentCount) +enodetimeval+"\n";
			invcond=invcond + indentTo(indentCount--) + brealValue +toString(d1.getUpperBound())+erealValue+"\n";
			invcond=invcond + indentTo(indentCount--) + eadd+"\n";
			invcond=invcond + indentTo(indentCount) + ele+"\n";

			postcond=postcond + indentTo(indentCount) +bpoc+"\n";

			//Handle conditions from action-action pairs
			
     			for(TokenSet::const_iterator tokit2 = actionTokens.begin(); tokit2 != actionTokens.end(); ++tokit2){
	    			TokenId t2 = *tokit2;
				if(t != t2){
					if(mustPrecede(t,t2,d1)){
						//In this case know t2 must be finished
						//StartCondition

					        nodekey = toString(t2->getKey());
						startCondition = startCondition + ",\n ";
						startCondition = startCondition + nodekey + " " + fin + ",\n";
						startCondition = startCondition + curtime;
						startCondition = startCondition + nodekey +".end.time";
						//Time in E2 is always an int so this hack works.
						startCondition = startCondition + "+" + toString(d1.getLowerBound());
 						startCondition = startCondition + "," + infty + endbrace;

						//know 2 startconds already, add another preceeding and
						startcondCount++;
						startcondSet=startcondSet+ indentTo(indentCount++) + band+"\n";
						startcondSet=startcondSet+ indentTo(indentCount++) +startcond+"\n";
						startcond="";
						startcond=startcond + indentTo(--indentCount) + bge+"\n";
						startcond=startcond + indentTo(indentCount++) + blookfreq+"\n";
						startcond=startcond + indentTo(indentCount) +bstate+time+estate+"\n";
						startcond=startcond + indentTo(indentCount++) + bfreq +"\n";
                                                startcond=startcond + indentTo(indentCount) + low_start + brealValue +freq + erealValue + low_end + "\n";
                                                startcond=startcond + indentTo(indentCount--) + high_start + brealValue +freq + erealValue + high_end + "\n";
                                                startcond=startcond + indentTo(indentCount--) +efreq+"\n";
						startcond=startcond + indentTo(indentCount) + elookfreq+"\n";			
						startcond=startcond + indentTo(indentCount++) + badd+"\n";
						startcond=startcond + indentTo(indentCount++) + bnodetimeval+"\n";
						startcond=startcond + indentTo(indentCount) + bnodeid+ nodeidPrefix + nodekey + enodeid+"\n";
						startcond=startcond + indentTo(indentCount) + bnodestateval+exec+enodestateval+"\n";
						startcond=startcond + indentTo(indentCount--) + btp+end+etp+"\n";
						startcond=startcond + indentTo(indentCount) + enodetimeval+"\n";
						startcond=startcond + indentTo(indentCount--) + brealValue+toString(d1.getLowerBound())+erealValue+"\n";
						startcond=startcond + indentTo(indentCount--) + eadd+"\n";
						startcond=startcond + indentTo(indentCount) +ege+"\n";
						
						startcondCount++;
						startcondSet=startcondSet + indentTo(indentCount) +band+"\n"+startcond+"\n";
						startcond="";
						startcond=startcond + indentTo(indentCount++) +ble+"\n";
						startcond=startcond + indentTo(indentCount++)  +blookfreq+"\n";
						startcond=startcond + indentTo(indentCount) + bstate+time+estate+"\n";
						startcond=startcond + indentTo(indentCount++) + bfreq +"\n";
                                                startcond=startcond + indentTo(indentCount) +low_start + brealValue + freq + erealValue + low_end +"\n";
                                                startcond=startcond + indentTo(indentCount--) + high_start + brealValue + freq + erealValue + high_end +"\n";
                                                startcond=startcond + indentTo(indentCount--) + efreq+"\n";
						startcond=startcond + indentTo(indentCount) + elookfreq+"\n";
						startcond=startcond + indentTo(indentCount--) + binf+inf+einf+"\n";		
						startcond=startcond + indentTo(indentCount) + ele+"\n";

						//PreCondition
						preCondition=preCondition+",\n ";
						preCondition = preCondition + curtime;
						preCondition = preCondition + nodekey +".end.time,";
						//Time in E2 is always an int so this hack works.
						preCondition = preCondition + nodekey +".end.time";
						preCondition = preCondition + "+" + toString(d1.getUpperBound());
 						preCondition = preCondition + endbrace;						

						//have 2 preconds, so just add another and
						precondCount++;
						precondSet=precondSet+ indentTo(indentCount) + band+"\n"+precond+"\n";
						precond="";
						precond=precond+indentTo(--indentCount)+bge+"\n";
                                                indentCount++;
						precond=precond+ indentTo(indentCount++) + blookfreq+"\n";
						precond=precond+ indentTo(indentCount) + bstate+time+estate+"\n";
						precond=precond+ indentTo(indentCount++) +bfreq + "\n";
                                                precond=precond+ indentTo(indentCount) + low_start + brealValue + freq + erealValue + low_end + "\n";
                                                precond=precond+ indentTo(indentCount--) + high_start + brealValue + freq + erealValue + high_end + "\n";
                                                precond=precond+ indentTo(indentCount--) +efreq+"\n";
						precond=precond+ indentTo(indentCount) + elookfreq+"\n";	
						precond=precond+ indentTo(indentCount++) +bnodetimeval+"\n";
						precond=precond+ indentTo(indentCount) + bnodeid + nodeidPrefix + nodekey+enodeid+"\n";
						precond=precond+ indentTo(indentCount) + bnodestateval+exec+enodestateval+"\n";
						precond=precond+ indentTo(indentCount--) + btp+start+etp+"\n";
						precond=precond+ indentTo(indentCount--) +enodetimeval+"\n";
						precond=precond+ indentTo(indentCount) + ege+"\n";

						precondCount++;
						precondSet=precondSet+band+"\n"+precond+"\n";
						precond="";
						precond=precond+ indentTo(indentCount++) + ble+"\n";
						precond=precond+ indentTo(indentCount++) +blookfreq+"\n";
						precond=precond+ indentTo(indentCount) + bstate+time+estate+"\n";
						precond=precond+ indentTo(indentCount++) +bfreq+"\n";
					        precond=precond+ indentTo(indentCount) +low_start + brealValue +freq + erealValue + low_end +"\n";
                                                precond=precond+ indentTo(indentCount--) +high_start + brealValue +freq + erealValue + high_end +"\n";
                                                precond=precond+ indentTo(indentCount--) + efreq +"\n";
						precond=precond+ indentTo(indentCount) + elookfreq+"\n";	
						precond=precond+badd+"\n";		
						precond=precond+ indentTo(indentCount++) +bnodetimeval+"\n";
						precond=precond+ indentTo(indentCount) + bnodeid + nodeidPrefix + nodekey+enodeid+"\n";
						precond=precond+ indentTo(indentCount) + bnodestateval+exec+enodestateval+"\n";
						precond=precond+ indentTo(indentCount--)+ btp+start+etp+"\n";
						precond=precond+ indentTo(indentCount) +enodetimeval+"\n";
						precond=precond+ indentTo(indentCount--) + brealValue +toString(d1.getUpperBound())+erealValue+"\n";
						precond=precond+ indentTo(indentCount--) + eadd+"\n";
						precond=precond+ indentTo(indentCount) + ele+"\n";
					}

					else if(mustStartBeforeStart(t2,t,d1)){
						if(mustEndAfter(t,t2,d2)){
							//in this case know t2 contained by t 
							if(firstPost==false){
								postCondition=postCondition+",";
							}
							firstPost=false;
							nodekey = toString(t2->getKey());
							postCondition = postCondition + curtime;
							postCondition = postCondition + nodekey +".end.time,";
							postCondition = postCondition + nodekey +".end.time";
							//Time in E2 is always an int so this hack works.
							postCondition = postCondition + "+" + toString(d2.getUpperBound())+ endbrace;

							//This might be first post cond, so check count 
							//If it is, dump this postcond with a previous band
							postcondCount++;
							if(postcondCount>1){
							  postcondSet=postcondSet+band+"\n"+postcond+"\n";
							  postcond="";
							}
							postcond=postcond+bge+"\n";
							postcond=postcond+ "55555" + blookfreq+"\n";
							postcond=postcond+bstate+time+estate+"\n";
							postcond=postcond+bfreq+breal+freq+ereal+efreq+"\n";
							postcond=postcond+elookfreq+"\n";	
							postcond=postcond+bnodetimeval+"\n";
							postcond=postcond+bnodeid + nodeidPrefix + nodekey+enodeid+"\n";
							postcond=postcond+bnodestateval+exec+enodestateval+"\n";
							postcond=postcond+btp+end+etp+"\n";
							postcond=postcond+enodetimeval+"\n";
							postcond=postcond+ege+"\n";
							
							postcondSet=postcondSet+band+"\n"+postcond+"\n";
							postcond="";
							postcondCount++;
							postcond=postcond+ble+"\n";
							postcond=postcond+ "66666" +blookfreq+"\n";
							postcond=postcond+bstate+time+estate+"\n";
							postcond=postcond+bfreq+breal+freq+ereal+efreq+"\n";
							postcond=postcond+elookfreq+"\n";	
							postcond=postcond+badd+"\n";		
							postcond=postcond+bnodetimeval+"\n";
							postcond=postcond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
							postcond=postcond+bnodestateval+exec+enodestateval+"\n";
							postcond=postcond+btp+end+etp+"\n";
							postcond=postcond+enodetimeval+"\n";
							postcond=postcond+breal+toString(d2.getUpperBound())+ereal+"\n";
							postcond=postcond+eadd+"\n";
							postcond=postcond+ele+"\n";
						}
					}

					else if(mustStartBeforeStart(t,t2,d1)){

					        nodekey = toString(t->getKey());
						preCondition=preCondition+",\n ";
						preCondition = preCondition + curtime;
						preCondition = preCondition + nodekey +".start.time,";
						//Time in E2 is always an int so this hack works.
						preCondition = preCondition + nodekey +".start.time";
						preCondition = preCondition + "+" + toString(d1.getUpperBound());
 						preCondition = preCondition + endbrace;

						precondSet=precondSet+band+"\n"+precond+"\n";
						precond="";
						precondCount++;
						precond=precond+bge+"\n";
						precond=precond+ "77777" + blookfreq+"\n";
						precond=precond+bstate+time+estate+"\n";
						precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
						precond=precond+elookfreq+"\n";	
						precond=precond+bnodetimeval+"\n";
						precond=precond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
						precond=precond+bnodestateval+exec+enodestateval+"\n";
						precond=precond+btp+start+etp+"\n";
						precond=precond+enodetimeval+"\n";
						precond=precond+ege+"\n";

						precondSet=precondSet+band+"\n"+precond+"\n";
						precond="";
						precondCount++;
						precond=precond+ble+"\n";
						precond=precond+ "88888"+ blookfreq+"\n";
						precond=precond+bstate+time+estate+"\n";
						precond=precond+bfreq+breal+freq+ereal+efreq+"\n";
						precond=precond+elookfreq+"\n";	
						precond=precond+badd+"\n";		
						precond=precond+bnodetimeval+"\n";
						precond=precond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
						precond=precond+bnodestateval+exec+enodestateval+"\n";
						precond=precond+btp+start+etp+"\n";
						precond=precond+enodetimeval+"\n";
						precond=precond+breal+toString(d1.getUpperBound())+ereal+"\n";
						precond=precond+eadd+"\n";
						precond=precond+ele+"\n";

						if(mustEndAfter(t,t2,d2)){

							//in this case know t contained by t2 
						        nodekey = toString(t2->getKey());
							startCondition = startCondition + ",\n ";
							startCondition = startCondition + nodekey + " " + exec + ",\n ";
							startCondition = startCondition + curtime;
							startCondition = startCondition + nodekey +".start.time";
							//Time in E2 is always an int so this hack works.
							startCondition = startCondition + "+" + toString(d1.getLowerBound());
 							startCondition = startCondition + "," + infty + endbrace;

							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+bge+"\n";
							startcond=startcond+ "9999" + blookfreq+"\n";
							startcond=startcond+bstate+time+estate+"\n";
							startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
							startcond=startcond+elookfreq+"\n";	
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid + nodeidPrefix + nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+btp+start+etp+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+ege+"\n";
							
							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+ble+"\n";
							startcond=startcond+ "1000000"+ blookfreq+"\n";
							startcond=startcond+bstate+time+estate+"\n";
							startcond=startcond+bfreq+breal+freq+ereal+efreq+"\n";
							startcond=startcond+elookfreq+"\n";	
							startcond=startcond+badd+"\n";		
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+btp+start+etp+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+breal+toString(d1.getLowerBound())+ereal+"\n";
							startcond=startcond+eadd+"\n";
							startcond=startcond+ele+"\n";

							if(firstPost==false){
								postCondition=postCondition+",";
							}
							firstPost=false;
							nodekey = toString(t2->getKey());
							postCondition = postCondition + curtime;
							postCondition = postCondition + nodekey +".end.time,";
							postCondition = postCondition + nodekey +".end.time";
							//Time in E2 is always an int so this hack works.
							postCondition = postCondition + "+" + toString(d2.getLowerBound())+ endbrace;

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
							postcond=postcond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
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
							postcond=postcond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
							postcond=postcond+bnodestateval+exec+enodestateval+"\n";
							postcond=postcond+btp+end+etp+"\n";
							postcond=postcond+enodetimeval+"\n";
							postcond=postcond+breal+toString(d2.getLowerBound())+ereal+"\n";
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
							startCondition = startCondition + "+" + toString(d1.getLowerBound());
 							startCondition = startCondition + "," + infty + endbrace + "\n";
							startCondition = startCondition + endbrace +"\n";



							startcondSet=startcondSet+band+"\n"+startcond+"\n";
							startcond="";
							startcondCount++;
							startcond=startcond+bor+"\n";
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+bor+"\n";
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+fin+enodestateval+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+bnodetimeval+"\n";
							startcond=startcond+bnodeid+ nodeidPrefix + nodekey+enodeid+"\n";
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
							startcond=startcond+bnodeid + nodeidPrefix + nodekey+enodeid+"\n";
							startcond=startcond+bnodestateval+exec+enodestateval+"\n";
							startcond=startcond+btp+start+etp+"\n";
							startcond=startcond+enodetimeval+"\n";
							startcond=startcond+breal+toString(d1.getLowerBound())+ereal+"\n";
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
					state_appendParamsToString(id2,varList,preCondition);
					preCondition=preCondition+ ")" + endbrace;

					std::string stateDescr;
					stateDescr = fqn.substr(lastdot+1,slen-lastdot)+"(";
					state_appendParamsToString(id2,varList,stateDescr);
					stateDescr = stateDescr + ")"; 


					precondCount++;
					precondSet=precondSet+band+"\n"+precond+"\n";
					precond="";					
					precond=precond + indentTo(indentCount++) + beqbool+"\n";
					precond=precond + indentTo(indentCount++) + blooknow + "\n";
					precond=precond + indentTo(indentCount--) + bstate;
					precond=precond + stateDescr;
					precond=precond + estate+"\n";
					precond=precond + indentTo(indentCount) + elooknow+"\n";
					precond=precond + indentTo(indentCount--) + bboolValue + boolfalse + eboolValue + "\n";
					precond=precond + indentTo(indentCount--) +eeqbool + "\n";
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
					state_appendParamsToString(id2,varList,postCondition);					
					postCondition = postCondition + ")" + endbrace;

					std::string stateDescr;
					stateDescr = fqn.substr(lastdot+1,slen-lastdot)+"(";
					state_appendParamsToString(id2,varList,stateDescr);
					stateDescr = stateDescr + ")";

					//This might be first post cond, so check count 
					//If it is, dump this postcond with a previous band
					postcondCount++;
					if(postcondCount>1){
					  postcondSet=postcondSet+band+"\n"+postcond+"\n";
					  postcond="";
					}
					postcond=postcond + indentTo(indentCount++) +beqbool+"\n";
					postcond=postcond + indentTo(indentCount++) +blooknow+"\n";
					postcond=postcond + indentTo(indentCount--) + bstate ;
					postcond=postcond + stateDescr;
					postcond=postcond + estate+"\n";
					postcond=postcond + indentTo(indentCount) + elooknow+"\n";
					postcond=postcond + indentTo(indentCount--) + bboolValue +boolfalse + eboolValue+"\n";
					postcond=postcond + indentTo(indentCount--) + eeqbool + "\n";
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
					state_appendParamsToString(id2,varList,invariantCondition);					
					invariantCondition = invariantCondition+"," + freq + endbrace;

					std::string stateDescr;
					stateDescr = fqn.substr(lastdot+1,slen-lastdot)+"(";
					state_appendParamsToString(id2,varList,stateDescr);
					stateDescr = stateDescr + ")"; 

					//know 2 invconds already, add another preceeding and
					invcondCount++;
					invcondSet = invcondSet + indentTo(indentCount++) + 
					invcondSet = invcondSet + indentTo(indentCount++) + band + "\n";
					invcondSet = invcondSet + indentTo(indentCount++) + invcond + "\n";
					invcond="";
					invcond=invcond+ indentTo(indentCount++) + beqbool+"\n";
					invcond=invcond + indentTo(indentCount++) +blookfreq+"\n";
					invcond=invcond+ indentTo(indentCount++) + bstate + time + estate + "\n";
					invcond=invcond + indentTo(indentCount--) +bfreq + freq + efreq + "\n";
					invcond=invcond + indentTo(indentCount--) + elookfreq+"\n";
					invcond=invcond+ indentTo(indentCount) + bstate+"\n";
					invcond=invcond + indentTo(indentCount--) +stateDescr+"\n";
					invcond=invcond + indentTo(indentCount--) +estate+"\n";
					invcond=invcond + indentTo(indentCount--) +elookfreq+"\n";
					invcond=invcond + indentTo(indentCount--) + bboolValue +boolfalse+ ebool +eboolValue+"\n";
					invcond=invcond + indentTo(indentCount) + eeqbool+"\n";
				}
			}


			//OK.  We have to append the last condition to each set.
			//We then have to append the correct number of eands to each set; this is *condcount-1.

			int q;
			startcondSet=startcondSet+startcond+"\n";
			for(q=1;q<startcondCount;q++){
			  startcondSet=startcondSet+ indentTo(indentCount--) +eand+"\n";
			}
			startcondSet=startcondSet+ indentTo(indentCount) + estc+"\n";

			precondSet=precondSet + precond +"\n";
			for(q=1;q<precondCount;q++){
			  precondSet=precondSet + indentTo(indentCount) +eand+"\n";
			}
			precondSet=precondSet+ indentTo(indentCount) + epc +"\n";

			invcondSet=invcondSet+invcond+"\n";
			for(q=1;q<invcondCount;q++){
			  invcondSet=invcondSet+ indentTo(indentCount) + eand+"\n";
			}
			invcondSet=invcondSet + indentTo(indentCount) + eic+"\n";

			if(firstPost==false){
			  postcondSet=postcond;
			  for(q=1;q<postcondCount;q++){
			    postcondSet=postcondSet+ indentTo(indentCount) + eand+"\n";
			  }
			  postcondSet=postcondSet+ indentTo(indentCount) + epoc+"\n";
			}
			

			//The predicate name is currently a "fully qualified name" including the attribute name.
			//E2 doesn't provide a "simple" output for this yet.
			//To handle this, exploit fact that what we want is really the substring from the position of 
			//the last . in the fully qualified name to the end of the string.
			std::string fqn=t->getPredicateName().toString();
			int slen=fqn.length();
			int lastdot=fqn.find_last_of(".");
			//	os << fqn.substr(lastdot+1,slen-lastdot) +"(";
			std::string par;
			varList = t->getParameters();
			action_appendParamsToString(id,varList,par);			
		    			
			plan=plan+startcondSet;
			plan=plan+precondSet;
			plan=plan+invcondSet;
			if(firstPost==false){
			  plan=plan+postcondSet;
			}
			std::string cmdDescr;
			cmdDescr=fqn.substr(lastdot+1,slen-lastdot);
                        plan = plan + indentTo(indentCount++) + begin_nodebody + "\n"; 
			plan = plan + indentTo(indentCount++) + bcmd + "\n"; 
		        plan = plan + indentTo(indentCount) + bcmdname;
                        plan = plan  + cmdDescr;
                        plan = plan + ecmdname + "\n";
	                cmdDescr="";
			action_appendParamsToString(id,varList,cmdDescr);
			plan = plan + indentTo(indentCount++) + bargs+"\n";
			plan = plan + indentTo(indentCount--) + cmdDescr+"\n";
			plan = plan + indentTo(indentCount--) + eargs+"\n";
                        plan = plan + indentTo(indentCount--) + ecmd + "\n";
                        plan = plan + indentTo(indentCount--) + end_nodebody + "\n";
			plan = plan + indentTo(indentCount--) + enode+"\n";
		}

		//Write PLEXILCLARATy top level node with list of children

		nodekey = "0";
		nodeHeader=node + nodeid;
		nodeHeader=nodeHeader + nodekey;

		nodeHeader=nodeHeader + endbrace;
		std::string s;
		first=true;
     		for(StringSet::const_iterator sit = childNodes.begin(); sit != childNodes.end(); ++sit){
			if (first==true)
	    			s = *sit;
			else s = ","+*sit;
			first=false;
		}

		//Write PLEXILCLARATy top level failure node

		nodekey = "-1";
		nodeHeader=node + nodeid;
		nodeHeader=nodeHeader + nodekey + endbrace + "AND{";
		//os << nodeHeader; 	
		first=true;
    		for(StringSet::const_iterator sit = childNodes.begin(); sit != childNodes.end(); ++sit){
			if (first==true)
	    			s = *sit;
			else s = ","+*sit;
			s=s+" "+failed;
			//os << s;
			first=false;
		}

                //plan=plan+enodelist + "\n";
           

                plan = plan + indentTo(2) + enodelist + "\n";
                plan = plan + indentTo(2) + end_nodebody + "\n";
                plan = plan + indentTo(1) + enode + "\n";
               
                plan = plan + end_plexil_plan + "\n";


		os << plan;
	}		
	
  private:

	//Helper functions

    bool PLEXILCLARATyPlanDatabaseWriter::mustPrecede(const TokenId& t,const TokenId& prec, IntervalDomain& d1) {
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
		  std::cerr << "Malformed EUROPA-PLEXILCLARATy translation configuration file " << inputFileName << std::endl;
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


    std::string  PLEXILCLARATyPlanDatabaseWriter::indentTo(int level) {
      std::string result = "";
      for (int i=1; i <= level; ++i) {
        result = result + indent;
      }
      return result;
    }

    std::string PLEXILCLARATyPlanDatabaseWriter::toString(double value) {
          std::string returnResult;
          std::stringstream temp;
          temp << value;	
	  temp >> returnResult;
          return returnResult;
    }


       void PLEXILCLARATyPlanDatabaseWriter::action_appendParamsToString(PlexilCmdId cmd,
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
            }
	      ConstrainedVariableId v= (*varit);
            
              std::string bTypeQualifier = "";
	      std::string eTypeQualifier = "";
              if (v->derivedDomain().isNumeric()) {
		  bTypeQualifier= brealValue;
                  eTypeQualifier= erealValue;
              } else if (v->derivedDomain().isBool()) {
                  bTypeQualifier= bboolValue;
                  eTypeQualifier= eboolValue;
              } else {
		  bTypeQualifier= bstringValue;
                  eTypeQualifier= estringValue;
              }

	      condString=condString + indentTo(indentCount) + bTypeQualifier + v->derivedDomain().toString(v->derivedDomain().getSingletonValue()) + eTypeQualifier + "\n";
                
              j++;
	  }

       }


        void PLEXILCLARATyPlanDatabaseWriter::state_appendParamsToString(PlexilCmdId cmd,
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
	      else condString=condString+ "\n ";
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
		    condString=condString+  cvid->derivedDomain().toString(cvid->derivedDomain().getSingletonValue())+", ";
		  }
		  if(std::string::npos != s.find("position_y",0)){
		    condString=condString+  cvid->derivedDomain().toString(cvid->derivedDomain().getSingletonValue());
		  }
		}
	      }
	      else {
		std::string bTypeQualifier = "";
	        std::string eTypeQualifier = "";
                if (v->derivedDomain().isNumeric()) {
		  bTypeQualifier= brealValue;
                  eTypeQualifier= erealValue;
                } else if (v->derivedDomain().isBool()) {
                  bTypeQualifier= bboolValue;
                  eTypeQualifier= eboolValue;
                } else {
		  bTypeQualifier= bstringValue;
                  eTypeQualifier= estringValue;
                }

		condString=condString + bTypeQualifier + v->derivedDomain().toString(v->derivedDomain().getSingletonValue()) + eTypeQualifier;
              }
	      
	    }
	    j++;
	  }
	}



 };
}

#endif /* #ifndef _H_PLEXILCLARATyPlanDatabaseWriter */



