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
#include "TestData.hh"

int main(int argc, char **argv) {
  try {
    check_error(Error::printingErrors());
    std::cerr << "Error::printErrors() is true\n";
    check_error(Error::getStream() == std::cerr);
    std::cerr << "Error::getStream() == std::cerr\n";
    check_error(Error::displayWarnings());
    std::cerr << "Error::displayWarnings() is true\n";
    check_error(argc == 1);
    std::cerr << "argc is 1 per check_error()\n";
    check_error(argc == 1, "check_error(argc == 1)");
    std::cerr << "argc is 1 per check_error()\n";
    check_error(argc == 1, Error("check_error(argc == 1)"));
    std::cerr << "argc is 1 per check_error()\n";
    condWarn(argc == 1, "argc is not 1");
    warn("everything worked in first try() block of main()");
  } catch (Error e) {
    __x__(e);
  }
  Error::doNotDisplayErrors(); // don't print purposely provoked errors
  try {
    check_error(argc == 2);
    __y__("check_error(argc == 2) did not throw an exception");
  } catch (Error e) {
    __z__(e, Error("argc == 2", __FILE__, __LINE__ - 3));
  }
  try {
    check_error(argc == 2, "check_error(argc == 2)");
    __y__("check_error(argc == 2, blah) did not throw an exception");
  } catch (Error e) {
    __z__(e, Error("argc == 2", "check_error(argc == 2)", __FILE__, __LINE__ - 3));
  }
  try {
    check_error(argc == 2, Error("check_error(argc == 2)"));
    __y__("check_error(argc == 2, Error(blah)) did not throw an exception");
  } catch (Error e) {
    __z__(e, Error("argc == 2", "check_error(argc == 2)", __FILE__, __LINE__ - 3));
  }
  exit(0);
}
