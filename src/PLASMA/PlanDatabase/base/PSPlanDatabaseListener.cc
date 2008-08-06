#include "PSPlanDatabaseListener.hh"
#include "Object.hh"
#include "Token.hh"

namespace EUROPA {

// Methods to convert notifications involving internal Europa types to notifications involving 'PS' types:
void PSPlanDatabaseListener::notifyAdded(const ObjectId& object) {
	notifyAdded((PSObject *) object);
}

void PSPlanDatabaseListener::notifyRemoved(const ObjectId& object) {
	notifyRemoved((PSObject *) object);
}

void PSPlanDatabaseListener::notifyActivated(const TokenId& token) {
	notifyActivated((PSToken *) token);
}

void PSPlanDatabaseListener::notifyDeactivated(const TokenId& token) {
	notifyDeactivated((PSToken *) token);
}

void PSPlanDatabaseListener::notifyRejected(const TokenId& token) {
	notifyRejected((PSToken *) token);
}

void PSPlanDatabaseListener::notifyMerged(const TokenId& token) {
	notifyMerged((PSToken *) token);
}

void PSPlanDatabaseListener::notifySplit(const TokenId& token) {
	notifySplit((PSToken *) token);
}

void PSPlanDatabaseListener::notifyAdded(const ObjectId& object, const TokenId& token) {
	notifyAdded((PSObject *) object, (PSToken *) token);
}

void PSPlanDatabaseListener::notifyRemoved(const ObjectId& object, const TokenId& token) {
	notifyRemoved((PSObject *) object, (PSToken *) token);

}

}
