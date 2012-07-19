#include "Utils.hh"
#include <sstream>
#include <iomanip>
#include <cmath>

DEFINE_GLOBAL_CONST(bool, g_alwaysFails, false);

namespace EUROPA {

  std::string toString(double value) {
    std::stringstream s;
    s << std::setprecision(MAX_PRECISION) << value;
    return(s.str());
  }

  std::string toString(edouble value) {
    std::stringstream s;
    s << std::setprecision(MAX_PRECISION) << value;
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


  DEFINE_GLOBAL_CONST(eint, g_maxInt, std::numeric_limits<eint>::max());
  DEFINE_GLOBAL_CONST(eint, g_infiniteTime, std::numeric_limits<eint>::infinity());
  DEFINE_GLOBAL_CONST(eint, g_noTime, 0);
  DEFINE_GLOBAL_CONST(double, g_epsilon, cast_double(std::numeric_limits<edouble>::epsilon()));
}
