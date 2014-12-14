#include "SearchListener.hh"

namespace EUROPA {
namespace SOLVERS {
void SearchListener::notifyCreated(DecisionPointId) {};

void SearchListener::notifyDeleted(DecisionPointId) {};

void SearchListener::notifyUndone(DecisionPointId) {};

void SearchListener::notifyStepSucceeded(DecisionPointId) {};

void SearchListener::notifyStepFailed(DecisionPointId) {};

void SearchListener::notifyRetractSucceeded(DecisionPointId) {};

void SearchListener::notifyRetractNotDone(DecisionPointId) {};
}
}

