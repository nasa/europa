#include <iostream>
#include <fstream>
#include "Error.hh"
#include "Debug.hh"

/**
 * Interface for testing ANML models for parsability.
 * Called with one filename: [ANML model] without the extension (.anml)
 */
int main(int argc, char** argv) 
{
  assertTrue(argc == 2, "Expected exactly one filename pattern argument: [ANML Model], without the extension");
  std::string filename(argv[1]);  

  try {
  }
  catch (Error& e) {
      std::cerr << "Failed translating " << filename << " : " << e.getMsg() << std::endl;
      return -1;	  
  }
  
  return 0;
}

