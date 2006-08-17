#include "ResourceProblem.hh"
#include "Instant.hh"

namespace EUROPA {

  /* Methods common for Flaw and Violation */
  ResourceProblem::ResourceProblem(Type type, const InstantId& instant)
    : m_type(type), m_instant(instant), m_id(this) {
    check_error(m_id.isValid());
  }

  ResourceProblem::ResourceProblem(Type type )
    : m_type(type), m_id(this) {
	m_instant = InstantId::noId();
    check_error(m_id.isValid());
  }

  ResourceProblem::~ResourceProblem() {
    m_id.remove();
  }

  void ResourceProblem::print(ostream& os) {
    os << getString();
	if ( m_instant==InstantId::noId() ) {
	  os << " global";
	} else {
	  m_instant->print(os);
    }
  }

}
