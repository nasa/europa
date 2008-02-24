%module %Project%
%include "std_string.i"

%{
  #include "%Project%CustomCode.hh"
%}


// Example of how to incorporate the ExampleConstraint declared in %Project%CustomCode.hh

//%nodefaultctor ExampleConstraint;   
 
//class ExampleConstraint {
//  public:
//    static void ExampleConstraint::registerSelf(std::string name, std::string propagator);
//};
 	       