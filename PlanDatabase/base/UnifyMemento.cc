#include "Id.hh"
#include "UnifyMemento.hh"

namespace EUROPA {
  
  UnifyMemento::~UnifyMemento() {
    if (!m_mm.isNoId())
      m_mm.release();
    if (!m_sm.isNoId())
      m_sm.release();
  }
  
  UnifyMemento::UnifyMemento() {}

  UnifyMemento::UnifyMemento(const TokenId& inactiveToken, const TokenId& activeToken) {
    static const char* TRUE_VALUE = "1";

    const char *envStr = getenv("EUROPA_USE_STACK_METHOD");

    m_method = mergeMethod;

    // If the environment variable has been set, and is 1, return true, else false
    if (envStr != NULL && strcmp(envStr, TRUE_VALUE) == 0)
      m_method = stackMethod;

    if (m_method == mergeMethod) {
      m_mm = MergeMementoId(new MergeMemento(inactiveToken, activeToken));
    }
    else {
      m_sm = StackMementoId(new StackMemento(inactiveToken, activeToken));
    }
  }

  void UnifyMemento::undo(bool activeTokenDeleted) {
    check_error(m_mm.isNoId() || m_sm.isNoId());
    check_error(!m_mm.isNoId() || !m_sm.isNoId());
    if (m_mm.isNoId()) {
      check_error(m_sm.isValid());
      m_sm->undo(activeTokenDeleted);
    }
    else
      m_mm->undo(activeTokenDeleted);
  }

  void UnifyMemento::handleRemovalOfInactiveConstraint(const ConstraintId& constraint) {
    check_error(m_mm.isNoId() || m_sm.isNoId());
    check_error(!m_mm.isNoId() || !m_sm.isNoId());
    if (m_mm.isNoId())
      m_sm->handleRemovalOfInactiveConstraint(constraint);
    else
      m_mm->handleRemovalOfInactiveConstraint(constraint);
  }
}
