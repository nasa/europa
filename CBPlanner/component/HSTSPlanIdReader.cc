#include "HSTSPlanIdReader.hh"
#include <iostream>
#include <fstream>

namespace PLASMA {

  HSTSPlanIdReader::HSTSPlanIdReader(HSTSNoBranch& noBranchSpec) : m_noBranchSpec(noBranchSpec) { }

  void HSTSPlanIdReader::read(const std::string& configFile) {
    check_error(configFile != "", "File name is empty.");
    //    std::cout << "Reading " << configFile << std::endl;

    std::ifstream cf(configFile.c_str());
    std::string msg("Failed to open " + configFile + ".");
    check_error(cf.is_open(), msg);

    std::string line;
    while (!cf.eof()) {
      getline(cf,line);
      //      std::cout << line << std::endl;
      LabelStr theLine(line);
      const char* delim = " ";
      int numElems = theLine.countElements(delim);
      check_error(numElems <= 2, "Expected at most two strings separated by a space.");
      if (numElems == 2) {
	LabelStr pred(theLine.getElement(0, delim));
	int index= atoi(theLine.getElement(1, delim).c_str());
	check_error(index>=0, "Expected index >= 0.");
	m_noBranchSpec.addNoBranch(pred,index);
      }
      else if (numElems == 1)
	m_noBranchSpec.addNoBranch(theLine);
      else
	check_error(false, "Expected one or two elements per line.");
    }
    
    /*
    std::cout << "Read the following data from NoBranch.pi" << std::endl;
    std::set<LabelStr>::iterator it(m_noBranchSpec.getNoBranchSpec().begin());
    for (; it != m_noBranchSpec.getNoBranchSpec().end(); ++it) 
      std::cout << (*it).c_str() << std::endl;
    */
  }

  HSTSPlanIdReader::~HSTSPlanIdReader() { }
}
