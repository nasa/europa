/**
  @file debug.cc
  @author William R. Edgington (wedgingt@ptolemy.arc.nasa.gov)
  @brief Define and implement variables and functions related to
  debugging and profiling.
*/

#ifndef NO_DEBUG_MESSAGE_SUPPORT

#include <fstream>
#include <algorithm>
#include <functional>

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

DebugMessage::DebugMessage(const std::string& file,
                           const int& line,
                           const std::string& marker,
                           const bool& enabled)
	  : m_file(file),
	  m_line(line),
	  m_marker(marker),
	  m_enabled(enabled) {
}

DebugMessage *DebugMessage::addMsg(const std::string &file, const int& line,
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
    allMsgs().push_back(msg);
    if (!msg->isEnabled()) {
      typedef std::list<DebugPattern>::iterator LDPI;
      LDPI iter = std::find_if(enabledPatterns().begin(),
                               enabledPatterns().end(),
                               PatternMatches<DebugPattern>(*msg));
      if (iter != enabledPatterns().end())
        msg->enable();
    }
  }
  return(msg);
}

DebugMessage *DebugMessage::findMsg(const std::string &file,
                                    const std::string &pattern) {
  typedef std::list<DebugMessage*>::const_iterator LDMPCI;
  LDMPCI iter = std::find_if(allMsgs().begin(),
                             allMsgs().end(),
                             MatchesPattern<DebugMessage*>(file, pattern));
  if (iter == allMsgs().end())
    return(0);
  return(*iter);
}

void DebugMessage::findMatchingMsgs(const std::string &file,
                                    const std::string &pattern,
                                    std::list<DebugMessage*> &matches) {
  std::for_each(allMsgs().begin(), allMsgs().end(), GetMatches(file, pattern, matches));
}

const std::list<DebugMessage*>& DebugMessage::getAllMsgs() {
  return(allMsgs());
}

void DebugMessage::enableAll() {
  allEnabled() = true;
  enabledPatterns().clear();
  std::for_each(allMsgs().begin(),
                allMsgs().end(),
                std::mem_fun(&DebugMessage::enable));
}

void DebugMessage::disableAll() {
  allEnabled() = false;
  enabledPatterns().clear();
  std::for_each(allMsgs().begin(),
                allMsgs().end(),
                std::mem_fun(&DebugMessage::disable));
}

void DebugMessage::enableMatchingMsgs(const std::string& file,
                                      const std::string& pattern) {
  if (file.length() < 1 && pattern.length() < 1) {
    enableAll();
    return;
  }
  DebugPattern dp(file, pattern);
  enabledPatterns().push_back(dp);
  std::for_each(allMsgs().begin(),
                allMsgs().end(),
                EnableMatches(dp));
}

void DebugMessage::disableMatchingMsgs(const std::string& file,
				       const std::string& pattern) {
  if(file.length() < 1 && pattern.length() < 1)
    return;

  DebugPattern dp(file, pattern);
  enabledPatterns().erase(std::find(enabledPatterns().begin(), enabledPatterns().end(), dp));
  std::for_each(allMsgs().begin(),
		allMsgs().end(),
		DisableMatches(dp));
}

bool DebugMessage::readConfigFile(std::istream& is) {
  check_error(is.good(), "cannot read debug config from invalid/error'd stream",
              DebugErr::DebugConfigError());
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
    enableMatchingMsgs(input, pattern);
  }
  check_error(is.eof(), "error while reading debug config file",
              DebugErr::DebugConfigError());
  return(is.eof());
}

#endif /* DEBUG_MESSAGE_SUPPORT */
