#include <iostream>
#include <fstream>
#include <cstdlib> // for rand()
#include <vector>
#include <sstream>

#define MAX_SATELLITES 1000

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
  out << "#include \"NddlWorld.nddl\"" << std::endl;
  out << std::endl;
  out << "class Satellite {" << std::endl;
  out << "predicate Pointing {" << std::endl;
  out << " Target observation;" << std::endl;
  for (int i=1; i <= prob.numParams; ++i) {
    out << " int " << prob.parameters[i] << ";" << std::endl;
    out << " leq(" << prob.parameters[i] << "," << prob.numParamChoices << ");" << std::endl;
    out << " leq(0," << prob.parameters[i] << ");" << std::endl;
  }
  out << "}" << std::endl;
  out << "predicate slew {" << std::endl;
  out << " Target from;" << std::endl;
  out << " Target to;" << std::endl;
  out << " neq(from, to);" << std::endl;
  out << " eq(duration, [1 10]);" << std::endl;
  for (int i=1; i <= prob.numParams; ++i) {
    out << " int " << prob.parameters[i] << ";" << std::endl;
    out << " leq(" << prob.parameters[i] << "," << prob.numParamChoices << ");" << std::endl;
    out << " leq(0," << prob.parameters[i] << ");" << std::endl;
  }
  out << "}" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
  out << "class Target {" << std::endl;
  out << " string name;" << std::endl;
  out << " int x;" << std::endl;
  out << " int y;" << std::endl;
  out << " Target(string _name, int _x, int _y) {" << std::endl;
  out << "  name = _name;" << std::endl;
  out << "  x = _x;" << std::endl;
  out << "  y = _y;" << std::endl;
  out << " }" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
  out << "class MyResource {" << std::endl;
  out << "predicate Consume {" << std::endl;
  out << " int amount;" << std::endl;
  out << "}" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
  out << "Satellite::Pointing {" << std::endl;
  out << " meets(object.slew sl1);" << std::endl;
  out << " eq(observation, sl1.from);" << std::endl;
  out << "" << std::endl;
  out << " met_by(object.slew sl2);" << std::endl;
  out << " eq(observation, sl2.to);" << std::endl;
  out << "" << std::endl;
  out << " contains(MyResource.Consume r);" << std::endl;
  out << " eq(r.amount, 10);" << std::endl;
  for (int i=1; i <= prob.numParams; ++i)
    for (int j=0; j<prob.numParamChoices; ++j)
      out << " neq(" << prob.parameters[i] << "," << j << ");" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
  out << "Satellite::slew{" << std::endl;
  out << " meets(object.Pointing p1);" << std::endl;
  out << " eq(to, p1.observation);" << std::endl;
  out << "" << std::endl;
  out << " met_by(object.Pointing p2);" << std::endl;
  out << " eq(from, p2.observation);" << std::endl;
  out << "" << std::endl;
  out << " contains(MyResource.Consume r);" << std::endl;
  out << " eq(r.amount, 100);" << std::endl;
  for (int i=1; i <= prob.numParams; ++i)
    for (int j=0; j<prob.numParamChoices; ++j)
      out << " neq(" << prob.parameters[i] << "," << j << ");" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
  out << "MyResource::Consume{" << std::endl;
  out << " meets(Consume c1);" << std::endl;
  out << " met_by(Consume c2);" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
}  

void nddloutputInitialState(std::ofstream& out, const Problem& prob) {
  out << "class World extends NddlWorld {" << std::endl;
  out << " MyResource res;" << std::endl;

  for (int i=1; i <= prob.numSatellites; ++i) {
    out << " Satellite " << prob.satellites[i] << ";" << std::endl;
  }

  for (int j=1; j <= prob.numSatellites; ++j) 
    for (int i=1; i <= prob.numTargets; ++i)
      out << " Target " << prob.targets[j*MAX_SATELLITES+i] << ";" << std::endl;

  out << "" << std::endl;
  out << " predicate initialState{}" << std::endl;
  out << "" << std::endl;
  out << " World() {" << std::endl;
  out << "  super(" << prob.horStart << "," << prob.horEnd << "," << 500*prob.numSatellites << ");" << std::endl;
  out << "  res = new MyResource();" << std::endl;

  for (int i=1; i <= prob.numSatellites; ++i)
    out << "  " << prob.satellites[i] << " = new Satellite();" << std::endl;

  for (int j=1; j <= prob.numSatellites; ++j) 
    for (int i=1; i <= prob.numTargets; ++i) {
      int rnd1 = rand()%100;
      int rnd2 = rand()%100;
      out << "  " << prob.targets[j*MAX_SATELLITES+i] << " = new Target(TARGET" << j*MAX_SATELLITES+i << ", " << rnd1 << "," << rnd2 << ");" << std::endl;
    }

  out << " }" << std::endl;
  out << "}" << std::endl;
  out << "" << std::endl;
  out << "World::initialState{" << std::endl;
  out << " leq(object.m_horizonStart, start);" << std::endl;
  out << " leq(end, object.m_horizonEnd);" << std::endl;
  out << "" << std::endl;

  for (int i = 1; i <= prob.numSatellites; ++i) {
    out << " contains(Satellite.Pointing initialPosition" << i << ");" << std::endl;
    out << " eq(initialPosition" << i << ".object, object." << prob.satellites[i] << ");" << std::endl;
    out << " eq(initialPosition" << i << ".observation, object." << prob.targets[i*MAX_SATELLITES+1] << ");" << std::endl;
    out << " eq(initialPosition" << i << ".start, object.m_horizonStart);" << std::endl;
    out << "" << std::endl;
    out << " contains(Satellite.Pointing finalPosition" << i << ");" << std::endl;
    out << " eq(finalPosition" << i << ".object, object." << prob.satellites[i] << ");" << std::endl;
    out << " eq(finalPosition" << i << ".observation, object." << prob.targets[i*MAX_SATELLITES+prob.numTargets] << ");" << std::endl;
    out << " eq(finalPosition" << i << ".end, object.m_horizonEnd);" << std::endl;
    out << "" << std::endl;
  }

  for (int j=1; j <= prob.numSatellites; ++j) 
    for (int i=1; i <= prob.numTargets; ++i) {
      std::ostringstream targetName;
      targetName << "target" << j << "_" << i;
      out << " any(Satellite.Pointing " << targetName.str() << ");" << std::endl;
      out << " eq(" << targetName.str() << ".observation, object." << prob.targets[j*MAX_SATELLITES+i] << ");" << std::endl;
      out << " eq(" << targetName.str() << ".object, object." << prob.satellites[j] << ");" << std::endl;
      out << " leq(object.m_horizonStart, " << targetName.str() << ".start);" << std::endl;
      out << " leq(" << targetName.str() << ".end, object.m_horizonEnd);" << std::endl;
      out << std::endl;
    }
  out << "}" << std::endl;
  out << "" << std::endl;
}


// FIX: need to add an Object Parameter to Pointing to equate the object to
// the possible targets.  Then add an eq constraint ?_object_ to the compat.

void ddloutputModel(std::ofstream& out, const Problem& prob) {
  //out << "Debugging_Level 1" << std::endl;
  //out << "" << std::endl;
  out << "(Define_Object_Class MyResource_Class" << std::endl;
  out << "  :state_variables" << std::endl;
  out << "  ((Controllable resource)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Predicate Consume ((Integer amount)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Member_Values ((MyResource_Class resource)) (Consume))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Object_Class Satellite_Class" << std::endl;
  out << "  :state_variables" << std::endl;
  out << "  ((Controllable satellite)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Object_Class Observation_Class" << std::endl;
  out << "  :state_variables" << std::endl;
  out << "  ((Controllable target)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Predicate CanObserve ((Satellite_Class Sat) (Observation_Class Obs)))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Predicate Pointing ((Observation_Class The_Obs) (Satellite_Class The_Sat)";
  for (int i=1; i<=prob.numParams; ++i)
    out << " (Integer [0 " << prob.numParamChoices << "] p" << i << ")";
  out << "))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Predicate Slew ((Observation_Class From_Obs) (Observation_Class To_Obs) (Satellite_Class Sat)";
  for (int i=1; i<=prob.numParams; ++i)
    out << " (Integer [0 " << prob.numParamChoices << "] p" << i << ")";
  out << "))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Member_Values ((Satellite_Class satellite)) (Pointing Slew))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Member_Values ((Observation_Class target)) (CanObserve))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Compatibility" << std::endl;
  out << " (SINGLE ((Observation_Class target)) ((CanObserve (?S ?O))))" << std::endl;
  out << " :parameter_functions ((?_object_ <- eq (?O))))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Compatibility" << std::endl;
  out << " (SINGLE ((Satellite_Class satellite)) ((Pointing (?X ?S";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?" << prob.parameters[i];
  out << "))))" << std::endl;
  out << " :duration_bounds [1 _plus_infinity_]" << std::endl;
  out << " :parameter_functions (" << std::endl;
  for (int i=1; i<=prob.numParams; ++i)
    for (int j=0; j < prob.numParamChoices; ++j)
      out << "   (?" << prob.parameters[i] << " <- neq(" << j << "))" << std::endl;
  out << "  )" << std::endl;
  out << " :compatibility_spec" << std::endl;
  out << " (AND" << std::endl;
  out << "  (met_by (SINGLE ((?_object_ satellite)) ((Slew (?_any_value_ ?X ?S";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
  out << "  (meets (SINGLE ((?_object_ satellite)) ((Slew (?X ?_any_value_ ?S";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
  out << "  (contained_by (SINGLE ((Observation_Class target)) ((CanObserve (?S ?X)))))" << std::endl;
  out << "  (contains (SINGLE ((MyResource_Class resource)) ((Consume (10)))))))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Compatibility" << std::endl;
  out << " (SINGLE ((Satellite_Class satellite)) ((Slew (?X ?Y ?S";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?" << prob.parameters[i];
  out << "))))" << std::endl;
  out << " :duration_bounds [1 10]" << std::endl;
  out << " :parameter_functions ((?X <- neq ( ?Y ))" << std::endl;
  for (int i=1; i<=prob.numParams; ++i)
    for (int j=0; j < prob.numParamChoices; ++j)
      out << "   (?" << prob.parameters[i] << " <- neq(" << j << "))" << std::endl;
  out << "  )" << std::endl;
  out << " :compatibility_spec" << std::endl;
  out << " (AND" << std::endl;
  out << "  (met_by (SINGLE ((?_object_ satellite)) ((Pointing (?X ?S";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
  out << "  (meets (SINGLE ((?_object_ satellite)) ((Pointing (?Y ?S";
  for (int i=1; i<=prob.numParams; ++i)
    out << " ?_any_value_";
  out << ")))))" << std::endl;
  out << "  (contained_by (SINGLE ((Observation_Class target)) ((CanObserve (?S ?X)))))" << std::endl;
  out << "  (contained_by (SINGLE ((Observation_Class target)) ((CanObserve (?S ?Y)))))" << std::endl;
  out << "  (contains (SINGLE ((MyResource_Class resource)) ((Consume (100)))))))" << std::endl;
  out << "" << std::endl;
  out << "(Define_Compatibility" << std::endl;
  out << " (SINGLE ((MyResource_Class resource)) ((Consume (?X)))))" << std::endl;
  out << "" << std::endl;
}

void ddloutputInitialState(std::ofstream& out, const Problem& prob) {
  //out << "Debugging_Level 1" << std::endl;
  //out << "" << std::endl;
  out << "[" << prob.horStart << " " << prob.horEnd << "]" << std::endl;
  out << "" << std::endl;
  out << "Object_Timelines MyResource_Class res1 (" << std::endl;
  out << "    resource ( Consume(*) ... Consume(*) ))" << std::endl;
  out << "" << std::endl;
  for (int i=1; i<=prob.numSatellites; ++i) {
    out << "Object_Timelines Satellite_Class " << prob.satellites[i] << " (" << std::endl;
    out << "    satellite (" << std::endl;
    out << "        Pointing(" << prob.targets[i*MAX_SATELLITES+1];
    for (int k=1; k <= prob.numParams; ++k) {
      out << " *";
    }
    out << ")" << std::endl;
    out << "		... 	" << std::endl;
    out << "        Pointing(" << prob.targets[i*MAX_SATELLITES+prob.numTargets];
    for (int k=1; k <= prob.numParams; ++k) {
      out << " *";
    }
    out << ")" << std::endl;
    out << "    ))" << std::endl;
    out << "" << std::endl;
  }

  for (int j=1; j <= prob.numSatellites; ++j) 
    for (int i=1; i <= prob.numTargets; ++i) {
      out << " Object_Timelines Observation_Class " << prob.targets[j*MAX_SATELLITES+i] << " (" << std::endl;
      out << "    target ( CanObserve(" << prob.satellites[j] << " " << prob.targets[j*MAX_SATELLITES+i] << ") ))" << std::endl;
    }

  for (int j=1; j <= prob.numSatellites; ++j) 
    for (int i=1; i <= prob.numTargets; ++i) {
      out << " Free_Token Satellite_Class satellite" << " (" << std::endl;
      out << " [" << prob.horStart << " " << prob.horEnd << "]" << std::endl;
      out << "     Pointing(" << prob.targets[j*MAX_SATELLITES+i];
      for (int k=1; k <= prob.numParams; ++k) {
      	out << " *";
      }
      out << ")" << std::endl;
      out << " [" << prob.horStart << " " << prob.horEnd << "])" << std::endl;
      out << std::endl;
    }
  out << "" << std::endl;
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

  std::cout << "Building model for " << prob.numSatellites << " satellites." << std::endl;

  prob.satellites.resize(prob.numSatellites+10, "");

  for (int i=1; i <= prob.numSatellites; ++i) {
    std::stringstream os;
    os << "sat" << i;
    prob.satellites[i] = os.str();
  }

  prob.targets.resize(prob.numSatellites*MAX_SATELLITES+prob.numTargets+10, "");

  for (int j=1; j <= prob.numSatellites; ++j) 
    for (int i=1; i <= prob.numTargets; ++i) {
      std::ostringstream os;
      os << "obs" << j << "_" << i;
      prob.targets[j*MAX_SATELLITES+i] = os.str();
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
