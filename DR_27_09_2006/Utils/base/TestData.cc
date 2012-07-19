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
   @file TestData.cc
   @author Will Edgington

   @brief Numerous declarations related to testing.
*/

#ifndef _NO_ERROR_EXCEPTIONS_
/* Contains the rest of this file */

#include "TestData.hh"

void TestData::failedCompare(const std::string& macro, const std::string& one,
                             const std::string& two,
                             const std::string& file, const int& line) {
  std::cerr << file << ':' << line << ": comparison failed: "
            << one << " is not equal to " << two << std::endl;
}

void TestData::unexpectedException(const std::string& macro, const Error& exception,
                                   const std::string& file, const int& line) {
  std::cerr << file << ':' << line << ": unexpected exception " << exception << std::endl;
}

void TestData::missingException(const std::string& macro, const std::string& msg,
                                const std::string& file, const int& line) {
  std::cerr << file << ':' << line << ": unexpected success; " << msg << std::endl;
}

void TestData::missingException(const std::string& macro, const Error& exception,
                                const std::string& file, const int& line) {
  std::cerr << file << ':' << line << ": unexpected success; expected exception: "
            << exception << std::endl;
}

void TestData::wrongException(const std::string& macro,
                              const Error& caughtException,
                              const Error& expectedException,
                              const std::string& file, const int& line) {
  std::cerr << file << ':' << line << ": unexpected exception "
            << caughtException << " is not the expected " << expectedException << std::endl;
}

void TestData::areEqual(const std::string& macro,
                        const std::string& one,
                        const std::string& two) {
  std::cerr << one << " equals " << two << '\n';
}

void TestData::correctException(const std::string& macro,
                                const Error& exception) {
  std::cout << "Caught expected exception " << exception << '\n';
}

#endif /* _NO_ERROR_EXCEPTIONS_ */
