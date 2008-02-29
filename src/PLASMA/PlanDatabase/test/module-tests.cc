#include "db-test-module.hh"
#include <string>

int main() {
  PlanDatabaseModuleTests::runTests(std::string("."));
  return 0;
}
