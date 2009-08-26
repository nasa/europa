#include "DomainListener.hh"

#include <ostream>

namespace EUROPA {
  std::ostream& operator<<(std::ostream& str, const DomainListener::ChangeType ct) {
    str << DomainListener::toString(ct);
    return str;
  }
}
