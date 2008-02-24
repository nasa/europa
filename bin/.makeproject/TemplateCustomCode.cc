#include "%Project%CustomCode.hh"

// Put any C++ project-specific custom code here:

// ... 

// For example, here's how you could implement a custom constraint:
// (not sure you need all these includes)

//#include "ConstraintEngine.hh"
//#include "ConstraintLibrary.hh"
//#include "ConstrainedVariable.hh"
//#include "IntervalIntDomain.hh"
//#include "BoolDomain.hh"
//#include "EnumeratedDomain.hh"
//#include "Utils.hh"
//#include "Debug.hh"
//#include "TypeFactory.hh"
//#include "NumericDomain.hh"
//
//#include <fstream>
//#include <sstream>
//
//using namespace std;
//
//ExampleConstraint::ExampleConstraint(const LabelStr& name,
//				   const LabelStr& propagatorName,
//				   const ConstraintEngineId& constraintEngine,
//				   const vector<ConstrainedVariableId>& variables)
//  : Constraint(name, propagatorName, constraintEngine, variables)
//{
//	check_error(variables.size() == (unsigned int) ARG_COUNT);	
//}
//
//
//void ExampleConstraint::registerSelf(string name, string propagator)  
//{ 
//    REGISTER_CONSTRAINT(ExampleConstraint, name.c_str(), propagator.c_str()); 
//} 
//
//
///**
// * @brief Restrict interval bounds to be integer values
// */
//void ExampleConstraint::handleExecute() {
//  check_error(isActive());
//
//  IntervalDomain& dom = static_cast<IntervalDomain&>(getCurrentDomain(m_variables[X]));
//  
//  // Discontinue if either domain is open.
//  if (dom.isOpen())
//    return;
//
//  double min = ceil(dom.getLowerBound());
//  double max = floor(dom.getUpperBound());
//  dom.intersect(min, max);
//}

