/**
   @file StringErrorStream.hh
   @author Tatiana Kichkaylo

   @brief Wrapper for string stream to be passed to Error so that the output
   of engine/parser could be captured and passed through Swig interfaces.
*/

#ifndef H_String_Error
#define H_String_Error

#include "Error.hh"
#include "Mutex.hh"
#include <iostream>
#include <sstream>

/**
   @class StringErrorStream
   @brief Static string buffer to collect output from Error printing.
*/
class StringErrorStream {
 public:
  /** @brief Set static output stream of Error to this class's string stream */
  inline static void setErrorStreamToString() {
    std::cout << "Taking over error stream\n";
    Error::setStream(stringStream);
  }

  /**
     @brief Get string accumulated in the string stream and clean up the stream.
  */
  static std::string retrieveString() {
    EUROPA::MutexGrabber grabber(s_mutex);
    std::string copy = stringStream.str();
    stringStream.str("");
    return copy;
  }

 private:
  /** Stream for accumulation of messages */
  static std::stringstream stringStream;
  static pthread_mutex_t s_mutex;
};

std::stringstream StringErrorStream::stringStream;
pthread_mutex_t StringErrorStream::s_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif /* H_String_Error */

