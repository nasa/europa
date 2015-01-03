/**
   @file debug.cc
   @author William R. Edgington (wedgingt@ptolemy.arc.nasa.gov)
   @brief Define and implement variables and functions related to
   debugging and profiling.
*/

#ifndef NO_DEBUG_MESSAGE_SUPPORT

// #include "europa-config.h"

#include <fstream>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>

#include <boost/ref.hpp>

#include "DebugMsg.hh"
#include "Mutex.hh"
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

  DebugPattern(const DebugPattern& o) : m_file(o.m_file), m_pattern(o.m_pattern) {}

  DebugPattern& operator=(const DebugPattern& o) {
    const_cast<std::string&>(m_file) = o.m_file;
    const_cast<std::string&>(m_pattern) = o.m_pattern;
    return *this;
  }
  /**
   * @brief The source file(s) that matches the pattern.
   */
  const std::string m_file;

  /**
   * @brief The markers that match the pattern.
   * @note Markers refer to those of class DebugMessage.
   * @see class DebugMessage
   */
  const std::string m_pattern;

  bool operator== (const DebugPattern& other) const {return m_file == other.m_file && m_pattern == other.m_pattern;}
};  //class DebugPattern

namespace {

/**
   @class PatternMatches DebugDefs.hh
   @brief Helper function for addMsg()'s use of STL find_if().
*/
template<class U>
class PatternMatches {
 private:
  const DebugMessage& dm;

 public:
  typedef U argument_type;
  typedef bool result_type;
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
class MatchesPattern {
 private:
  const DebugPattern pattern;

 public:
  typedef T argument_type;
  typedef bool result_type;

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
 * @class DebugConfig
 * @brief Used to perform default allocation, based on 'Debug.cfg'.
 * @author Conor McGann
 * @see DebugMessage::addMsg
 */
class DebugConfig {
 public:
  static void init() {
    static DebugConfig s_instance;
  }
 private:
  DebugConfig() {
    std::ifstream config("Debug.cfg");
    if (config.good()) {
      DebugMessage::setStream(std::cout);
      DebugMessage::readConfigFile(config);
    }
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


}

class DebugMessage::DebugInternals {
 public:
  DebugInternals() 
    : m_allEnabled(false), m_msgs(), m_patterns() {}
  std::vector<DebugMessage*>& allMsgs() {return m_msgs;}
  std::vector<DebugPattern> enabledPatterns() {return m_patterns;}
  bool allEnabled() const {return m_allEnabled;}
  void enableAll() {
    m_allEnabled = true;
    m_patterns.clear();
    std::for_each(m_msgs.begin(), m_msgs.end(), std::mem_fun(&DebugMessage::enable));
  }

  void disableAll() {
    m_allEnabled = false;
    m_patterns.clear();
    std::for_each(m_msgs.begin(),
                  m_msgs.end(),
                  std::mem_fun(&DebugMessage::disable));
  }

  void enableMatchingMsgs(const std::string& file,
                          const std::string& pattern) {
    if (file.length() < 1 && pattern.length() < 1) {
      enableAll();
      return;
    }
    DebugPattern dp(file, pattern);
    m_patterns.push_back(dp);
    std::for_each(m_msgs.begin(),
                  m_msgs.end(),
                  EnableMatches(dp));
  }

  void disableMatchingMsgs(const std::string& file,
                           const std::string& pattern) {
    if(file.length() < 1 && pattern.length() < 1)
      return;

    DebugPattern dp(file, pattern);
    m_patterns.erase(std::find(m_patterns.begin(), m_patterns.end(), dp));
    std::for_each(m_msgs.begin(),
                  m_msgs.end(),
                  DisableMatches(dp));

  }


  
  DebugMessage *addMsg(const std::string &file, const int& line,
                       const std::string &marker) {
    DebugConfig::init();
    check_error(line > 0, "debug messages must have positive line numbers",
                DebugErr::DebugMessageError());
    check_error(!file.empty() && !marker.empty(), "debug messages must have non-empty file and marker",
                DebugErr::DebugMessageError());
    DebugMessage *msg = findMsg(file, marker);
    if (msg == 0) {
      msg = new DebugMessage(file, line, marker);
      check_error(msg != 0, "no memory for new debug message",
                  DebugErr::DebugMemoryError());
      m_msgs.push_back(msg);
      if (!msg->isEnabled()) {
        typedef std::vector<DebugPattern>::iterator LDPI;
        LDPI iter = std::find_if(m_patterns.begin(),
                                 m_patterns.end(),
                                 PatternMatches<DebugPattern>(*msg));
        if (iter != m_patterns.end())
          msg->enable();
      }
    }
    return(msg);
  }

  DebugMessage* findMsg(const std::string &file,
                        const std::string &pattern) {
    typedef std::vector<DebugMessage*>::const_iterator LDMPCI;
    LDMPCI iter = std::find_if(m_msgs.begin(),
                               m_msgs.end(),
                               MatchesPattern<DebugMessage*>(file, pattern));
    if (iter == m_msgs.end())
      return(0);
    return(*iter);
  }

  void findMatchingMsgs(const std::string &file,
                        const std::string &pattern,
                        std::list<DebugMessage*> &matches) {
    std::for_each(m_msgs.begin(), m_msgs.end(), GetMatches(file, pattern, matches));
  }

 private:
  bool m_allEnabled;
  std::vector<DebugMessage*> m_msgs;
  std::vector<DebugPattern> m_patterns;
};


namespace {

static DebugMessage::DebugInternals debugInternals;
#ifdef __APPLE__
static pthread_mutex_t debugMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
static pthread_mutex_t debugMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

typedef std::pair<EUROPA::MutexGrabber, 
                  boost::reference_wrapper<DebugMessage::DebugInternals> >
internals_accessor;

internals_accessor internals() {
  EUROPA::MutexGrabber grabber(debugMutex);
  return std::make_pair<EUROPA::MutexGrabber,
                        boost::reference_wrapper<DebugMessage::DebugInternals> >(grabber,
                                                                                    boost::ref(debugInternals));
}

}

DebugMessage::DebugMessage(const std::string& file,
                           const int& line,
                           const std::string& marker)
	  : m_file(file),
	  m_line(line),
	  m_marker(marker),
	  m_enabled(false) {
  internals_accessor i = internals();
  m_enabled = i.second.get().allEnabled();
}

DebugMessage *DebugMessage::addMsg(const std::string &file, const int& line,
                                   const std::string &marker) {
  return internals().second.get().addMsg(file, line, marker);
}

DebugMessage *DebugMessage::findMsg(const std::string &file,
                                    const std::string &pattern) {
  return internals().second.get().findMsg(file, pattern);
}

void DebugMessage::findMatchingMsgs(const std::string &file,
                                    const std::string &pattern,
                                    std::list<DebugMessage*> &matches) {
  internals().second.get().findMatchingMsgs(file, pattern, matches);
}

std::list<DebugMessage*> DebugMessage::getAllMsgs() {
  internals_accessor i = internals();
  std::list<DebugMessage*> retval(i.second.get().allMsgs().begin(),
                                  i.second.get().allMsgs().end());

  return retval;
}

void DebugMessage::enableAll() {
  internals().second.get().enableAll();
}

void DebugMessage::disableAll() {
  internals().second.get().disableAll();
}

void DebugMessage::enableMatchingMsgs(const std::string& file,
                                      const std::string& pattern) {
  internals().second.get().enableMatchingMsgs(file, pattern);
}

void DebugMessage::disableMatchingMsgs(const std::string& file,
				       const std::string& pattern) {
  internals().second.get().disableMatchingMsgs(file, pattern);
}
bool DebugMessage::matches(const DebugPattern& pattern) const {
  return markerMatches(getFile(), pattern.m_file) &&
      markerMatches(getMarker(), pattern.m_pattern);
}

bool DebugMessage::readConfigFile(std::istream& is) {
  check_error(is.good(), "cannot read debug config from invalid/error'd stream",
              DebugErr::DebugConfigError());
  internals_accessor a = internals();

  std::string input;
  while (is.good() && !is.eof()) {
    getline(is, input);
    if (input.empty())
      continue;
    std::string::size_type i = 0;
    std::string::size_type len = input.length();
    while (i < len && isascii(input[i]) && isspace(input[i]))
      i++;
    if (input[i] == ';' || input[i] == '#' || input[i] == '/')
      continue; // input is a comment
    input = input.substr(i); // after white space
    i = input.find_first_of(";#/");
    input = input.substr(0, i); // chop off comment
    i = input.length();
    while (i > 0 && isascii(input[i - 1]) && isspace(input[i - 1]))
      --i;
    if (i <= 0)
      continue; // should be impossible
    input = input.substr(0, i);
    i = input.find(":");
    std::string pattern;
    if (i < input.length() && input[i] == ':') {
      pattern = input.substr(i + 1);
      input = input.substr(0, i);
    }
    a.second.get().enableMatchingMsgs(input, pattern);
  }
  check_error(is.eof(), "error while reading debug config file",
              DebugErr::DebugConfigError());
  return(is.eof());
}

#endif /* DEBUG_MESSAGE_SUPPORT */
