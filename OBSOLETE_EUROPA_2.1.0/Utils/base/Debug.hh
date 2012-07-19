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
  @file DebugDefs.hh
  @author Europa project

  @brief Numerous declarations related to debugging.
*/

#ifndef _H_Debug
#define _H_Debug

#ifdef NO_DEBUG_MESSAGE_SUPPORT

#define debugMsg(marker, data)
#define condDebugMsg(cond, marker, data)
#define debugStmt(marker, stmt)
#define condDebugStmt(cond, marker, stmt)
#define initDebug()

#else

# include <iostream>
# include <string>
# include <list>

#include "Error.hh"

/**
  @brief Use the debugMsg() macro to create a debug message that
  will be printed when the code is executed if and only if this
  particular debug message has been 'enabled'.
  @param marker A string that "marks" the message to enable it by.
  @param data The data to be printed when the message is enabled.
  @brief The data argument can be any "chain" of C++ expressions to
  output to the debug stream returned by DebugMessage::getStream()
  when this debug message is enabled (via, e.g. DebugMessage::enable()
  or DebugMessage::enableAll()).
  @see condDebugMsg
  @see debugStmt
  @see condDebugStmt
  @see DebugMessage
*/
#define debugMsg(marker, data) condDebugMsg(true, marker, data)

/**
  @brief Create a conditional debug message, which will
  only be created or used when the given condition is true at run time.
  @param cond An additional condition to be checked before printing the message,
         which can be any C/C++ expression that could be used in an if statement.
  @param marker A string that "marks" the message to enable it by.
  @param data The data to be printed when the message is enabled.
  @see debugMsg
  @see condDebugMsg
  @see debugStmt
  @see condDebugStmt
  @see DebugMessage
*/
#define condDebugMsg(cond, marker, data) { \
  static DebugMessage *dmPtr = DebugMessage::addMsg(__FILE__, __LINE__, marker); \
  if (dmPtr->isEnabled() && (cond)) { \
    try { \
      DebugMessage::getStream().exceptions(std::ios_base::badbit); \
      DebugMessage::getStream() << /*dmPtr[0] << */ "[" << marker << "]" << data << std::endl; \
    } \
    catch(std::ios_base::failure& exc) { \
      checkError(ALWAYS_FAIL, exc.what()); \
      throw; \
    } \
  } \
}

/**
  @brief Add code to be executed only if the DebugMessage is enabled.
  @param marker A string that "marks" the message to enable it by.
  @param stmt The code to be executed when the message is enabled.
  @see debugMsg
  @see condDebugMsg
  @see condDebugStmt
  @see DebugMessage
*/
#define debugStmt(marker, stmt) condDebugStmt(true, marker, stmt)

/**
  @brief Add code to be executed only if the DebugMessage is enabled and
         the condition is true.
  @param cond An additional condition to be checked before printing the message,
         which can be any C/C++ expression that could be used in an if statement.
  @param marker A string that "marks" the message to enable it by.
  @param stmt The code to be executed when the message is enabled and the condition
         is true.
  @see debugMsg
  @see condDebugMsg
  @see debugStmt
  @see DebugMessage
*/
#define condDebugStmt(cond, marker, stmt) { \
  static DebugMessage *dmPtr = DebugMessage::addMsg(__FILE__, __LINE__, marker); \
  if (dmPtr->isEnabled() && (cond)) { \
    stmt ; \
  } \
}

class DebugErr {
public:
  DECLARE_ERROR(DebugStreamError);
  DECLARE_ERROR(DebugMessageError);
  DECLARE_ERROR(DebugMemoryError);
  DECLARE_ERROR(DebugConfigError);
};

/**
  @class DebugMessage Debug.hh
  @brief Implements support for debugMsg() macro, which should be used
  to create all instances.
*/
class DebugMessage {

  typedef std::ostream* oSptr;

private:

  /**
   * @brief Are all debug messages enabled?
   * @note Individual ones could be even when this is false.
   */
  static bool& allEnabled() {
    static bool s_allEnabled = false;
    return(s_allEnabled);
  }

  /**
   * @brief Construct a DebugMessage.
   * @param file File containing the debug message instance.
   * @param line Line on which it is declared/created.
   * @param marker Name for the particular instance (not required to be unique within the process).
   * @param enabled Whether the instance is enabled at creation.
   * @note Only constructor that should be used.
   * @note Should only be called from static member functions.
   */
  DebugMessage(const std::string& file, const int& line,
               const std::string& marker,
               const bool& enabled = DebugMessage::allEnabled());

  /**
   * @class DebugPattern Debug.hh
   * @brief Used to store the "patterns" of presently enabled debug messages.
   * @see DebugMessage::enableMatchingMsgs
   */
  class DebugPattern {
  public:
    /**
     * @brief Zero argument constructor.
     * @note Should not be used except implicitly (e.g., by std::list<DebugPattern>).
     */
    inline DebugPattern();

    /**
     * @brief Destructor.
     */
    inline virtual ~DebugPattern() {
    }

    /**
     * @brief Constructor with data.
     * @note Should be the only constructor called explicitly.
     */
    DebugPattern(const std::string& f, const std::string& m)
      : m_file(f), m_pattern(m) {
    }

    /**
     * @brief The source file(s) that match the pattern.
     */
    const std::string m_file;

    /**
     * @brief The markers that match the pattern.
     * @note Markers refer to those of class DebugMessage.
     * @see class DebugMessage
     */
    const std::string m_pattern;

    bool operator== (const DebugPattern& other) const {return m_file == other.m_file && m_pattern == other.m_pattern;}
  };

  /**
   * @brief Destroy a DebugMessage.
   * @note Should only be called implicitly (e.g., by std::list<DebugMessage>).
   */
  inline virtual ~DebugMessage() {
  }

public:

  /**
    @brief Create a new DebugMessage.  Should only be called from the
    debugMsg() macro and readConfigFile().
    @param file
    @param line
    @param marker
    @par Errors thrown:
    @li If no debug stream has been assigned.
    @see DebugMessage::enable
    @see DebugMessage::setStream
  */
  static DebugMessage *addMsg(const std::string& file,
                              const int& line,
                              const std::string& marker);

  /**
    @brief Find any matching DebugMessage.
    @param file
    @param pattern
  */
  static DebugMessage *findMsg(const std::string& file,
                               const std::string& pattern);

  /**
    @brief Find all matching DebugMessages and appends them to matches parameter
    without emptying it first.
    @param file
    @param pattern
    @param matches
  */
  static void findMatchingMsgs(const std::string& file,
                               const std::string& pattern,
                               std::list<DebugMessage*>& matches);

  /**
    @brief Get list of all debug known messages.
   */
  static const std::list<DebugMessage*>& getAllMsgs();

  /**
    @brief Enable all debug messages, including ones not yet created.
    @param val
    @par Errors thrown:
    @li If no debug stream has been assigned.
    @see DebugMessage::setStream
  */
  static void enableAll();

  /**
    @brief Assign a stream to which all debug messages will be sent.
    @param os
   */
  inline static void setStream(std::ostream& os) {
    streamPtr() = &os;
  }

  /**
    @brief Return the stream being used for debug messages.
   */
  inline static std::ostream& getStream() {
    return(*(streamPtr()));
  }

  /**
    @brief Read a list of debug message enablements from the
    stream argument.
    @param is
    @par Errors thrown:
    @li If the stream is not good.
    @li If setStream() has not been called
    and some existing debug messages should be enabled.
   */
  static bool readConfigFile(std::istream& is);

  /**
    @brief Return the file used to create the debug message.
   */
  inline const std::string& getFile() const {
    return(m_file);
  }

  /**
    @brief Return the line used to create the debug message.
   */
  inline int getLine() const {
    return(m_line);
  }

  /**
    @brief Return the marker used to create the debug message.
   */
  inline const std::string& getMarker() const {
    return(m_marker);
  }

  /**
    @brief Return whether the debug message is currently enabled.
   */
  inline bool isEnabled() const {
    return(m_enabled);
  }

  /**
    @brief Enable the debug message.
    @par Errors thrown:
    @li If the stream has not been set.
   */
  inline void enable() {
    checkError(streamPtr()->good(), 
	       "cannot enable debug message(s) without a good debug stream: " << 
	       (streamPtr()->rdstate() & std::ostream::badbit ? " bad " : "") <<
	       (streamPtr()->rdstate() & std::ostream::eofbit ? " eof " : "") <<
	       (streamPtr()->rdstate() & std::ostream::failbit ? " fail " : "") <<
	       (streamPtr()->rdstate() & std::ostream::goodbit ? " good???" : ""));
    /*
    check_error(streamPtr()->good(), 
	       "cannot enable debug message(s) without a good debug stream:",DebugErr::DebugStreamError());
    */
    m_enabled = true;
  }

  /**
    @brief Disable the debug message.
   */
  inline void disable() {
    m_enabled = false;
  }

  /**
    @brief Print the data members of the debug message in a format
    that Emacs can use to display the corresponding source code.
    @param
   */
  inline void print(std::ostream *os = streamPtr()) const {
    try {
      os[0].exceptions(std::ostream::badbit);
      os[0] << m_file << ':' << m_line << ": " << m_marker << ' ';
    }
    catch(std::ios_base::failure& exc) {
      checkError(ALWAYS_FAIL, exc.what());
      throw;
    }
  }

  /**
    @brief Enable matching debug messages, including those created later.
    @param file
    @param marker
    @par Errors thrown:
    @li If a message would be enabled but no debug stream has been set.
    @see DebugMessage::setStream
  */
  static void enableMatchingMsgs(const std::string& file,
                                 const std::string& marker);

  /**
     @brief Disable matching debug messages, including those created later.
     @param file
     @param marker
   */

  static void disableMatchingMsgs(const std::string& file,
                                 const std::string& marker);


  /**
     @brief Whether the message is matched by the pattern.
  */
  bool matches(const DebugPattern& pattern) const {
   return(markerMatches(getFile(), pattern.m_file) &&
          markerMatches(getMarker(), pattern.m_pattern));
  }

  static bool isGood() {
    return streamPtr()->good();
  }

private:

  /**
    @brief The pointer to the stream being used.
    @note Has to be a pointer because some C++ compiler
    implementations have a private operator=().
  */
  inline static oSptr& streamPtr() {
    static oSptr s_debugStream = &(std::cerr);
    return(s_debugStream);
  }

  /**
    @brief List of pointers to all debug messages.
  */
  static std::list<DebugMessage*>& allMsgs() {
    static std::list<DebugMessage*> s_msgs;
    return(s_msgs);
  }

  /**
    @brief List of all enabled debug patterns.
  */
  static std::list<DebugPattern>& enabledPatterns() {
    static std::list<DebugPattern> s_patterns;
    return(s_patterns);
  }

  /**
    @brief File given when this instance was created.
  */
  std::string m_file;

  /**
    @brief Line given when this instance was created.
  */
  int m_line;

  /**
    @brief Marker given when this instance was created.
  */
  std::string m_marker;

  /**
    @brief Whether this instance is 'enabled' or not.
  */
  bool m_enabled;

  /**
    @brief Whether the given marker matches the "pattern".
    Exists solely to ensure the same method is always used to check
    for a match.
   */
  inline static bool markerMatches(const std::string& marker,
                                   const std::string& pattern) {
    if (pattern.length() < 1)
      return(true);
    if (marker.length() < pattern.length())
      return(false);
    return(marker.find(pattern) < marker.length());
  }

  /**
     @class PatternMatches DebugDefs.hh
     @brief Helper function for addMsg()'s use of STL find_if().
  */
  template<class U>
  class PatternMatches : public std::unary_function<U, bool> {
  private:
    const DebugMessage& dm;

  public:
    explicit PatternMatches(const DebugMessage& debugMsg) : dm(debugMsg) {
    }

    bool operator() (const U& y) const {
      return(dm.matches(y));
    }
  };

  /**
     @class MatchesPattern DebugDefs.hh
     @brief Helper class to use markerMatches via STL find_if().
  */
  template<class T>
  class MatchesPattern : public std::unary_function<T, bool> {
  private:
    const DebugPattern pattern;

  public:
    explicit MatchesPattern(const std::string& f, const std::string& p)
      : pattern(f, p) {
    }

    explicit MatchesPattern(const DebugPattern& p)
      : pattern(p) {
    }

    bool operator() (const T& dm) const {
      return(dm->matches(pattern));
    }
  };

  /**
     @class GetMatches DebugDefs.hh
     @brief Helper class to gather matching messages via STL for_each().
  */
  class GetMatches {
  private:

    const DebugPattern pattern;

    std::list<DebugMessage*>& matches;

  public:
    explicit GetMatches(const std::string& f, const std::string& p,
                        std::list<DebugMessage*>& m)
      : pattern(f, p), matches(m) {
    }

    explicit GetMatches(const DebugPattern& p, std::list<DebugMessage*>& m)
      : pattern(p), matches(m) {
    }

    void operator() (DebugMessage* dm) {
      if (dm->matches(pattern))
        matches.push_back(dm);
    }
  };

  /**
     @class EnableMatches DebugDefs.hh
     @brief Helper class to enable matching messages via STL for_each().
  */
  class EnableMatches {
  private:

    const DebugPattern& pattern;

  public:

    explicit EnableMatches(const DebugPattern& p)
      : pattern(p) {
    }

    void operator() (DebugMessage* dm) {
      if (dm->matches(pattern))
        dm->enable();
    }
  };


  class DisableMatches {
  private:
    const DebugPattern& pattern;
  public:
    explicit DisableMatches(const DebugPattern& p) : pattern(p) {}
    void operator() (DebugMessage* dm) {
      if(dm->matches(pattern))
	dm->disable();
    }
  };

  /**
    @brief Should not be used.
  */
  DebugMessage();

  /**
    @brief Should not be used.
  */
  DebugMessage(const DebugMessage&);

  /**
    @brief Should not be used.
  */
  DebugMessage& operator=(const DebugMessage&);

  /**
    @brief Should not be used.
  */
  bool operator==(const DebugMessage&) const;

};

inline std::ostream& operator<<(std::ostream& os, const DebugMessage& dm) {
  dm.print(&os);
  return(os);
}

#endif /* DEBUG_MESSAGE_SUPPORT */

#endif /* _H_Debug */
