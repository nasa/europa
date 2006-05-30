#include "rs-test-module.hh"
#include "rs-flow-test-module.hh"

int main(int argc, const char** argv) {
  ResourceModuleTests::runTests(".");
  FlowProfileModuleTests::runTests(".");
  return 0;
}
