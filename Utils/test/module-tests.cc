//  Copyright Notices

//  This software was developed for use by the U.S. Government as
//  represented by the Administrator of the National Aeronautics and
//  Space Administration. No copyright is claimed in the United States
//  under 17 U.S.C. 105.

//  This software may be used, copied, and provided to others only as
//  permitted under the terms of the contract or other agreement under
//  which it was acquired from the U.S. Government.  Neither title to nor
//  ownership of the software is hereby transferred.  This notice shall
//  remain on all copies of the software.

/**
   @file error-test.cc
   @author Will Edgington

   @brief A small test of classes Error and TestData and the related
   macros.
*/

#include "Error.hh"
#include "Debug.hh"
#include "LabelStr.hh"
#include "TestData.hh"
#include <sstream>
#include <iostream>
#include <fstream>

#define runTest(test) { \
  try { \
  std::cout << "      " << #test; \
  bool result = test(); \
  if (result) \
    std::cout << " PASSED." << std::endl; \
  else \
    if (result) { \
      std::cout << " FAILED TO PASS UNIT TEST." << std::endl; \
      throw Error::GeneralUnknownError(); \
    } \
  } \
  catch (Error err){ \
   err.print(std::cout); \
  }\
  }

#define runTestSuite(test) { \
  try{ \
  std::cout << #test << "***************" << std::endl; \
  if (test()) \
    std::cout << #test << " PASSED." << std::endl; \
  else \
    std::cout << #test << " FAILED." << std::endl; \
  }\
  catch (Error err){\
   err.print(std::cout);\
  }\
  }


class TestError {
public:
  DECLARE_ERROR(BadThing);
};

class ErrorTest {
public:
  static bool test() {
    runTest(testExceptions);
    return true;
  }
private:
  static bool testExceptions() {
    int argc = 1;
    try {
      Error::doThrowExceptions();
      check_error(Error::printingErrors());
      check_error(Error::getStream() == std::cerr);
      check_error(Error::displayWarnings());
      check_error(argc == 1);
      check_error(argc == 1, "check_error(argc == 1)");
      check_error(argc == 1, Error("check_error(argc == 1)"));
      condWarn(argc == 1, "argc is not 1");
      warn("everything worked in first try() block of main()");
    } 
    catch (Error e) {
      __x__(e);
    }
    Error::doNotDisplayErrors(); // don't print purposely provoked errors
    try {
      check_error(argc == 2);
      __y__("check_error(argc == 2) did not throw an exception");
    } 
    catch (Error e) {
      __z__(e, Error("argc == 2", __FILE__, __LINE__ - 4));
    }
    try {
      check_error(argc == 2, "check_error(argc == 2)");
      __y__("check_error(argc == 2, blah) did not throw an exception");
    } 
    catch (Error e) {
      __z__(e, Error("argc == 2", "check_error(argc == 2)", __FILE__, __LINE__ - 4));
    }
    try {
      check_error(argc == 2, Error("check_error(argc == 2)"));
      __y__("check_error(argc == 2, Error(blah)) did not throw an exception");
    } 
    catch (Error e) {
      __z__(e, Error("argc == 2", "check_error(argc == 2)", __FILE__, __LINE__ - 4));
    }
    try {
      check_error(argc == 2, "check_error(argc == 2)", TestError::BadThing());
      __y__("check_eror(argc == 2, TestError::BadThing()) did not throw an exception");
    }
    catch(Error e) {
      assert(e.getType() == "BadThing");
      std::cerr << "Caught expected " << e.getType() << std::endl;
    }
    return true;
  }
};

class DebugTest {
public:
  static bool test() {
    runTest(testDebugError);
    runTest(testDebugFiles);
    return true;
  }
private:

  static bool testDebugError() {
    Error::doNotDisplayErrors();
    try {
      DebugMessage::enableAll();
      __y__("enabling all debug messages succeeded despite no debug stream");
    }
    catch(Error e) {
      __z__(e, Error("s_os != 0", "no debug stream has been assigned",
                     "../../Utils/core/Debug.cc", 91));
    }
    Error::doDisplayErrors();
    return true;
  }

  static bool testDebugFiles() {
    for(int i = 1; i < 7; i++)
      runDebugTest(i);
    return true;
  }

  static void runDebugTest(int cfgNum) {
    std::stringstream cfgName;
    cfgName << "debug" << cfgNum << ".cfg";
    std::string cfgFile(cfgName.str());
    cfgName << ".output";
    std::string cfgOut(cfgName.str());

    Error::doNotThrowExceptions();
    std::ifstream debugStream(cfgFile.c_str());
    check_error(debugStream, "could not open debug config file",
                DebugErr::DebugConfigError());
    if(!DebugMessage::readConfigFile(debugStream))
      handle_error(!DebugMessage::readConfigFile(debugStream),
                   "problems reading debug config file",
                   DebugErr::DebugConfigError());
    std::ofstream debugOutput(cfgOut.c_str());
    check_error(debugOutput, "could not open debug output file");
    DebugMessage::setStream(debugOutput);
    
    debugMsg("main1", "done opening files");
    condDebugMsg(std::cout.good(), "main1a", "std::cout is good");
    debugStmt("main2a", int s = 0; for (int i = 0; i < 5; i++) { s += i; } std::cerr << "Sum is " << s << '\n'; );
    debugMsg("main2", "primary testing done");
  }
};

int main() {
  runTestSuite(ErrorTest::test);
  runTestSuite(DebugTest::test);
  std::cout << "Finished" << std::endl;
}
