#include "CommonDefs.hh"

namespace EUROPA {

  static std::string & testLoadLibraryPath() {
    static std::string sl_testLoadLibraryPath("");
    return sl_testLoadLibraryPath;
  }

  void setTestLoadLibraryPath(std::string path) {
    testLoadLibraryPath() = path;
  }

  std::string getTestLoadLibraryPath() {
    return testLoadLibraryPath();
  }
}
