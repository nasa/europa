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
   @file Error.cc
   @author Will Edgington

   @brief Numerous declarations related to error generation and handling.
*/

#ifndef _NO_ERROR_EXCEPTIONS_
/* Contains the rest of this file */

#include "Error.hh"

#ifdef __BEOS__
#include <OS.h>
#endif

std::ostream *Error::s_os = 0;
bool Error::s_throw = false;
bool Error::s_printErrors = true;
bool Error::s_printWarnings = true;

Error::Error(const std::string& condition, const std::string& file, const int& line)
  : m_condition(condition), m_file(file), m_line(line), m_type("Error") {
  if (s_os == 0)
    s_os = &(std::cerr);
}

Error::Error(const std::string& condition, const std::string& msg,
             const std::string& file, const int& line)
  : m_condition(condition), m_msg(msg), m_file(file), m_line(line), m_type("Error") {
  if (s_os == 0)
    s_os = &(std::cerr);
}

Error::Error(const std::string& condition, const Error& exception,
             const std::string& file, const int& line)
  : m_condition(condition), m_msg(exception.getMsg()), m_file(file), m_line(line), m_type("Error") {
  if (s_os == 0)
    s_os = &(std::cerr);
}

Error::Error(const std::string& condition, const std::string& msg, const std::string& type,
             const std::string& file, const int& line)
  : m_condition(condition), m_msg(msg), m_file(file), m_line(line), m_type(type) {
  if (s_os == 0)
    s_os = &(std::cerr);
}

void Error::handleAssert() {
  display();
  if (throwEnabled())
    throw *this;
#ifndef __BEOS__
  assert(false); // Need the stack to work backwards and look at state in the debugger
#else
  debugger(m_condition.c_str());
#endif
}

void Error::setCause(const std::string& condition, const std::string& file, const int& line) {
  m_condition = condition;
  m_file = file;
  m_line = line;
  display();
}

void Error::display() {
  if (!printingErrors())
    return;
  std::cout.flush();
  std::cerr.flush();
  getStream() << '\n' << m_file << ':' << m_line << ": Error: " << m_condition << " is false";
  if (!m_msg.empty())
    getStream() << "\n\t" << m_msg;
  getStream() << std::endl;
}

void Error::printWarning(const std::string& msg,
                         const std::string& file,
                         const int& line) {
  if (!displayWarnings())
    return;
  getStream() << file << ':' << line << ": Warning: " << msg << std::endl;
}

void Error::print(std::ostream& os) const {
  os << "Error(\"" << m_condition << "\", \"";
  if (!m_msg.empty())
    os << m_msg << "\", \"";
  os << m_file << "\", " << m_line << ")";
}

Error::~Error() {
}

#endif /* _NO_ERROR_EXCEPTIONS_ */
