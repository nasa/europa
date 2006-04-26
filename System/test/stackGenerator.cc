#include <iostream>
#include <fstream>
#include <cstdlib> // for rand()
#include <vector>
#include <sstream>

#define MAX_SATELLITES 1000

//#define USING_RESOURCE 1

class Problem {
public:
  int numSatellites;
  int numTargets;
  int numParams;
  int numParamChoices;
  int horStart;
  int horEnd;
  std::vector<std::string> satellites;
  std::vector<std::string> targets;
  std::vector<std::string> parameters;

  Problem() {
    numSatellites=1;
    numTargets=1;
    numParams=1;
    numParamChoices=1;
    horStart=0;
    horEnd=0;
  }

  ~Problem() {}
};

void nddloutputModel(std::ofstream& out, const Problem& prob) {
  out << "#include \"../../NDDL/core/Plasma.nddl\"" << std::endl;
  out << "#include \"../../NDDL/core/PlannerConfig.nddl\"" << std::endl; 
  out << std::endl;
  out << "enum Target {";  
  for (int i=1; i <= prob.numTargets; ++i) {
    if (i == prob.numTargets)
      out << prob.targets[i];
    else 
      out << prob.targets[i] << ", ";
  }
  out << "};" << std::endl;
  out << "" << std::endl;
  if (prob.numParamChoices >0)  {
    out << "typedef int{";
    for (int i=0; i < prob.numParamChoices; ++i)
      out << i << ", ";
    out << prob.numParamChoices;
    out << "} Domain;" << std::endl;
    out << "" << std::endl;
  }
  out << "class Satellite {" << std::endl;
  out << "predicate Pointing {" << std::endl;
  out << " Target observation;" << std::endl;
  out << " eq(duration, [1 1]);" << std::endl;
  if (prob.numParams >= 2) {
    out << " Domain " << prob.parameters[1] << ";" << std::endl;
    out << " Domain " << prob.parameters[2] << ";" << std::endl;
    out << " perf(" << prob.parameters[1] << ", " << prob.parameters[2] << ");" << std::endl;
    out << " perf(" << prob.parameters[2] << ", " << prob.parameters[1] << ");" << std::endl;
  }
  for (int i=3; i <= prob.numParams; ++i) {
    out << " Domain " << prob.parameters[i] << ";" << std::endl;
    out << " perf(" << prob.parameters[i] << ", " << prob.parameters[1] << ");" << std::endl;
  }
  out << "}" << std::endl;
  out << "predicate slew {" << std::endl;
  out << " Target from;" << std::endl;
  out << " Target to;" << std::endl;
  out << " neq(from, to);" << std::endl;
  out << " eq(duration, [1 1]);" << std::endl;
  if (prob.numParams >= 2) {
    out << " Domain " << prob.parameters[1] << ";" << std::endl;
    out << " Domain " << prob.parameters[2] << ";" << std::endl;
    out << " perf(" << prob.parameters[1] << ", " << prob.parameters[2] << ");" << std::endl;
    out << " perf(" << prob.parameters[2] << ", " << prob.parameters[1] << ");" << std::endl;
  }
  for (int i=3; i <= prob.numParams; ++i) {
    out << " Domain " << prob.parameters[i] << ";" << std::endl;
    out << " perf(" << prob.parameters[i] << ", " << prob.parameters[1] << ");" << std::endl;
  }
  out << "}" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
#ifdef USING_RESOURCE
  out << "class MyResource {" << std::endl;
  out << "predicate Consume {" << std::endl;
  out << " int amount;" << std::endl;
  out << "}" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
#endif
  out << "Satellite::Pointing {" << std::endl;
  out << " meets(object.slew sl1);" << std::endl;
  out << " eq(observation, sl1.from);" << std::endl;
  out << "" << std::endl;
  out << " met_by(object.slew sl2);" << std::endl;
  out << " eq(observation, sl2.to);" << std::endl;
  out << "" << std::endl;
#ifdef USING_RESOURCE
  out << " contains(MyResource.Consume r);" << std::endl;
  out << " eq(r.amount, 10);" << std::endl;
#endif
  out << "}" << std::endl;
  out << "" << std::endl;
  out << "Satellite::slew{" << std::endl;
  out << " meets(object.Pointing p1);" << std::endl;
  out << " eq(to, p1.observation);" << std::endl;
  out << "" << std::endl;
  out << " met_by(object.Pointing p2);" << std::endl;
  out << " eq(from, p2.observation);" << std::endl;
  out << "" << std::endl;
#ifdef USING_RESOURCE
  out << " contains(MyResource.Consume r);" << std::endl;
  out << " eq(r.amount, 100);" << std::endl;
#endif
  out << "}" << std::endl;
  out << "" << std::endl;
}  

void nddloutputInitialState(std::ofstream& out, const Problem& prob) {
  int expectedTokens = prob.numTargets*2;
  out << "PlannerConfig world = new PlannerConfig(0," << expectedTokens << ",99999999);" << std::endl;
#ifdef USING_RESOURCE
  out << " MyResource res = new MyResource();" << std::endl;
#endif

  for (int i=1; i <= prob.numSatellites; ++i)
    out << "Satellite " << prob.satellites[i] << " = new Satellite();" << std::endl;
  out << "" << std::endl;
  out << "close();" << std::endl;

  for (int i = 1; i <= prob.numSatellites; ++i) {
    out << "goal(Satellite.Pointing initialPosition" << i << ");" << std::endl;
    out << "eq(initialPosition" << i << ".object, " << prob.satellites[i] << ");" << std::endl;
    out << "eq(initialPosition" << i << ".observation, " << prob.targets[1] << ");" << std::endl;
    out << "eq(initialPosition" << i << ".start, world.m_horizonStart);" << std::endl;
    out << "" << std::endl;
  }
}


// FIX: need to add an Object Parameter to Pointing to equate the object to
// the possible targets.  Then add an eq constraint ?_object_ to the compat.

void ddloutputModel(std::ofstream& out, const Problem& prob) {
  //out << "Debugging_Level 1" << std::endl;
  //out << "" << std::endl;
#ifdef USING_RESOURCE
  out << "(Define_Object_Class MyResource_Class" << std::endl;
  out << "  :state_variables" << std::endl;
  out << "  ((Controllable resource)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Predicate Consume ((Integer amount)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Member_Values ((MyResource_Class resource)) (Consume))" << std::endl;
  out << "" << std::endl;
#endif
  out << "(Define_Object_Class Satellite_Class" << std::endl;
  out << "  :state_variables" << std::endl;
  out << "  ((Controllable satellite)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Label_Set Observation_Label (";
  for (int i=1; i <= prob.numTargets; ++i)
    out << " " << prob.targets[i];
  out << "))" << std::endl;
  out << "" << std::endl;
  if (prob.numParamChoices > 0) {
    out << "(Define_Label_Set Domain_Label (";
    for (int i=0; i < prob.numParamChoices; ++i) {
      out << "\"" << i << "\" ";
    }
    out << "\"" << prob.numParamChoices << "\"))" << std::endl;
  out << "" << std::endl;
  }
  out << "(Define_Predicate Pointing ((Observation_Label The_Obs)";
  for (int i=1; i<=prob.numParams; ++i)
    out << " (Domain_Label p" << i << ")";
  out << "))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Predicate Slew ((Observation_Label From_Obs) (Observation_Label To_Obs)";
  for (int i=1; i<=prob.numParams; ++i)
    out << " (Domain_Label p" << i << ")";
  out << "))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Member_Values ((Satellite_Class satellite)) (Pointing Slew))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Compatibility" << std::endl;
  out << " (SINGLE ((Satellite_Class satellite)) ((Pointing (?X";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?" << prob.parameters[i];
  out << "))))" << std::endl;
  out << " :duration_bounds [1 1]" << std::endl;
  if (prob.numParams > 0)
    out << " :parameter_functions (" << std::endl;
  for (int i=1; i <= prob.numParams; ++i) 
    out << "                       (perf(?" << prob.parameters[i] << "))" << std::endl;
  if (prob.numParams > 0)
    out << ")" << std::endl;
  out << " :compatibility_spec" << std::endl;
  out << " (AND" << std::endl;
  out << "  (met_by (SINGLE ((?_object_ satellite)) ((Slew (?_any_value_ ?X";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
  out << "  (meets (SINGLE ((?_object_ satellite)) ((Slew (?X ?_any_value_";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
#ifdef USING_RESOURCE
  out << "  (contains (SINGLE ((MyResource_Class resource)) ((Consume (10)))))" << std::endl;
#endif
  out << "))\n" << std::endl;
  out << "(Define_Compatibility" << std::endl;
  out << " (SINGLE ((Satellite_Class satellite)) ((Slew (?X ?Y";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?" << prob.parameters[i];
  out << "))))" << std::endl;
  out << " :duration_bounds [1 1]" << std::endl;
  out << " :parameter_functions ((?X <- neq ( ?Y )) " << std::endl;
  for (int i=1; i <= prob.numParams; ++i) 
    out << "                       (perf(?" << prob.parameters[i] << "))" << std::endl;
  out << ")" << std::endl;
  out << "" << std::endl;
  out << " :compatibility_spec" << std::endl;
  out << " (AND" << std::endl;
  out << "  (met_by (SINGLE ((?_object_ satellite)) ((Pointing (?X";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
  out << "  (meets (SINGLE ((?_object_ satellite)) ((Pointing (?Y";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
#ifdef USING_RESOURCE
  out << "  (contains (SINGLE ((MyResource_Class resource)) ((Consume (100)))))" << std::endl;
#endif
  out << "))\n" << std::endl;
#ifdef USING_RESOURCE
  out << "(Define_Compatibility" << std::endl;
  out << " (SINGLE ((MyResource_Class resource)) ((Consume (?X)))))" << std::endl;
  out << "" << std::endl;
#endif
}

void ddloutputInitialState(std::ofstream& out, const Problem& prob) {
  //out << "Debugging_Level 1" << std::endl;
  //out << "" << std::endl;
  out << "[" << prob.horStart << " " << prob.horEnd << "]" << std::endl;
  out << "" << std::endl;
#ifdef USING_RESOURCE
  out << "Object_Timelines MyResource_Class res1 (" << std::endl;
  out << "    resource ( Consume(*) ... Consume(*) ))" << std::endl;
  out << "" << std::endl;
#endif
  for (int i=1; i<=prob.numSatellites; ++i) {
    out << "Object_Timelines Satellite_Class " << prob.satellites[i] << " (" << std::endl;
    out << "    satellite (" << std::endl;
    out << "        Pointing(" << prob.targets[1];
    for (int k=1; k <= prob.numParams; ++k) {
      out << " *";
    }
    out << ")" << std::endl;
    out << "		... 	" << std::endl;
    out << "    ))" << std::endl;
    out << "" << std::endl;
  }

}

int main (int argc, const char** argv) {

  Problem prob;
  std::string filename("satellites");

  if (argc == 5) {
    prob.numSatellites = atoi(argv[1]);
    prob.numTargets = atoi(argv[2]);
    prob.numParams = atoi(argv[3]);
    prob.numParamChoices = atoi(argv[4]);
  }
  else if (argc == 6) {
    prob.numSatellites = atoi(argv[1]);
    prob.numTargets = atoi(argv[2]);
    prob.numParams = atoi(argv[3]);
    prob.numParamChoices = atoi(argv[4]);
    filename = argv[5];
  }
  else {
    std::cerr << "Error: " << argv[0] << " requires #satellites #targets #params #paramChoices [outputFilename]." << std::endl;
    exit(-1);
  }

  if (prob.numSatellites >= MAX_SATELLITES) {
    std::cerr << "Error: " << argv[0] << " does not support more than " << MAX_SATELLITES-1 << " satellites." << std::endl;
    exit (-1);
  }

  // dumb restriction but nddl compiler does not allow single parameter constraints.
  if (prob.numParams != 0 && prob.numParams < 2) {
    std::cerr << "Error: " << argv[0] << " requires 0 or 2 or more parameters " << std::endl;
    exit(-1);
  }

  std::cout << "Building model for " << prob.numSatellites << " satellites." << std::endl;

  prob.satellites.resize(prob.numSatellites+10, "");

  for (int i=1; i <= prob.numSatellites; ++i) {
    std::stringstream os;
    os << "sat" << i;
    prob.satellites[i] = os.str();
  }

  prob.targets.resize(prob.numTargets+10, "");

  for (int i=1; i <= prob.numTargets; ++i) {
    std::ostringstream os;
    os << "obs" << i;
    prob.targets[i] = os.str();
  }

  prob.parameters.resize(prob.numParams+10, "");
  for (int i=1; i <= prob.numParams; ++i) {
    std::stringstream os;
    os << "p" << i;
    prob.parameters[i] = os.str();
  }

  prob.horStart = 0;
  prob.horEnd = 2*prob.numTargets+2;

  std::string nf(filename+".nddl");
  std::ofstream nout(nf.c_str());

  nddloutputModel(nout, prob);
  nddloutputInitialState(nout, prob);

  std::string df(filename+".ddl");
  std::ofstream dout(df.c_str());

  ddloutputModel(dout, prob);

  std::string dfi(filename+".init");
  std::ofstream douti(dfi.c_str());

  ddloutputInitialState(douti, prob);

  std::cout << "Done." << std::endl;
}
