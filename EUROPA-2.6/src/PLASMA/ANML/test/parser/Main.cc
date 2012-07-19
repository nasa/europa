#include <iostream>
#include <fstream>
#include "Error.hh"
#include "Debug.hh"
#include "AnmlTestEngine.hh"

/**
 * Interface for testing ANML models for parsability.
 * Called with one filename: [ANML model]
 */
int main(int argc, char** argv) 
{
	assertTrue(argc == 2, "Expected exactly one filename argument: [ANML Model]");
	std::string filename(argv[1]);
	std::cout << "Testing ANML parser with: " << filename << std::endl;

	try {
		AnmlTestEngine engine;
		engine.init();
		std::string result = engine.executeScript("anml",filename,true /*isFile*/);
		std::cout << result << std::endl;
	}
	catch (Error& e) {
		std::cerr << "Failed translating " << filename << " : " << e.getMsg() << std::endl;
		return -1;
	}

	return 0;
}

