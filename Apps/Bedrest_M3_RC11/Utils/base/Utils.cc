#include "Utils.hh"
#include <sstream>

DEFINE_GLOBAL_CONST(bool, g_alwaysFails, false);

namespace EUROPA {

  std::string toString(float value) {
    std::stringstream s;
    s << value;
    return(s.str());
  }

  void tokenize(const std::string& str,
		std::vector<std::string>& tokens,
		const std::string& delimiters)  {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
      {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
      }
  }


  //DEFINE_GLOBAL_CONST(int, g_maxInt, (LONG_MAX/8)); /*!< Same as max finite time */
  DEFINE_GLOBAL_CONST(int, g_maxInt, (std::numeric_limits<int>::max() / 8));
  DEFINE_GLOBAL_CONST(int, g_infiniteTime, (g_maxInt() + 1));
  DEFINE_GLOBAL_CONST(int, g_noTime, 0);
  DEFINE_GLOBAL_CONST(double, g_epsilon, 0.00001);
}
