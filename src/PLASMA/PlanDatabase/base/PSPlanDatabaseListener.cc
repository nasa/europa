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

void PSPlanDatabaseListener::notifyAdded(const TokenId& token) {
	notifyAdded((PSToken *) token);
}

void PSPlanDatabaseListener::notifyRemoved(const TokenId& token) {
	notifyRemoved((PSToken *) token);	
}

void PSPlanDatabaseListener::notifyActivated(const TokenId& token) {
	notifyActivated((PSToken *) token);
}

void PSPlanDatabaseListener::notifyDeactivated(const TokenId& token) {
	notifyDeactivated((PSToken *) token);
}

void PSPlanDatabaseListener::notifyMerged(const TokenId& token) {
	notifyMerged((PSToken *) token);	
}

void PSPlanDatabaseListener::notifySplit(const TokenId& token) {
	notifySplit((PSToken *) token);
}

void PSPlanDatabaseListener::notifyRejected(const TokenId& token) {
	notifyRejected((PSToken *) token);
}

void PSPlanDatabaseListener::notifyReinstated(const TokenId& token) {
	notifyReinstated((PSToken *) token);
}

void PSPlanDatabaseListener::notifyConstrained(const ObjectId& object, const TokenId& predecessor, const TokenId& successor) {
	notifyConstrained((PSObject *) object, (PSToken *) predecessor, (PSToken *) successor);
}

void PSPlanDatabaseListener::notifyFreed(const ObjectId& object, const TokenId& predecessor, const TokenId& successor) {
	notifyFreed((PSObject *) object, (PSToken *) predecessor, (PSToken *) successor);	
}

void PSPlanDatabaseListener::notifyAdded(const ObjectId& object, const TokenId& token) {
	notifyAdded((PSObject *) object, (PSToken *) token);	
}

void PSPlanDatabaseListener::notifyRemoved(const ObjectId& object, const TokenId& token) {
	notifyRemoved((PSObject *) object, (PSToken *) token);

}

void PSPlanDatabaseListener::notifyCommitted(const TokenId& token) {
	notifyCommitted((PSToken *) token);	
}

void PSPlanDatabaseListener::notifyTerminated(const TokenId& token) {
	notifyTerminated((PSToken *) token);	
}

}
