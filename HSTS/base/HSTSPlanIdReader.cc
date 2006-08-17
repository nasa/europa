#include "HSTSPlanIdReader.hh"
#include <iostream>
#include <fstream>

namespace EUROPA {

  HSTSPlanIdReader::HSTSPlanIdReader(HSTSNoBranchId& noBranchSpec) : m_noBranchSpec(noBranchSpec) { }

  void HSTSPlanIdReader::read(const std::string& configFile) {
    check_error(configFile != "", "File name is empty.");
    //    std::cout << "Reading " << configFile << std::endl;

    std::ifstream cf(configFile.c_str());
    std::string msg("Failed to open " + configFile + ".");
    check_error(cf.is_open(), msg);

    std::string line;
    while (!cf.eof()) {
      getline(cf,line);
      LabelStr theLine(line);
      const char* delim = " ";
      int numElems = theLine.countElements(delim);
      if (numElems == 0) continue; // empty line, ignore
      check_error(numElems == 2 || numElems == 1, "Expected one string or two strings separated by a space.");
      if (numElems == 2) {
	LabelStr pred(theLine.getElement(0, delim));
	int index= atoi(theLine.getElement(1, delim).c_str());
	check_error(index>=0, "Expected index >= 0.");
	m_noBranchSpec->addNoBranch(pred,index);
      }
      else if (numElems == 1)
	m_noBranchSpec->addNoBranch(theLine);
      else
	check_error(false, "Expected one or two elements per line.");
    }
  }

  HSTSPlanIdReader::~HSTSPlanIdReader() { check_error(m_noBranchSpec.isValid()); m_noBranchSpec.remove(); }
}
