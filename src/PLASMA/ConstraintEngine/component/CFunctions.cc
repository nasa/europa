#include "CFunctions.hh"

namespace EUROPA {
void IsSingleton::checkArgTypes(const std::vector<DataTypeId>& args) {
  checkRuntimeError(args.size() == 1,
             "isSingleton expected 1 argument, but got " << args.size());
}

void IsSpecified::checkArgTypes(const std::vector<DataTypeId>& ) {}
}
