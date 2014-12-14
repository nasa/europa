#include "PSPlanDatabaseListener.hh"
#include "Object.hh"
#include "Token.hh"

namespace EUROPA {

void PSPlanDatabaseListener::notifyAdded(PSObject*) {}
void PSPlanDatabaseListener::notifyRemoved(PSObject*) {}
void PSPlanDatabaseListener::notifyActivated(PSToken*) {}
void PSPlanDatabaseListener::notifyDeactivated(PSToken*) {}
void PSPlanDatabaseListener::notifyRejected(PSToken*) {}
void PSPlanDatabaseListener::notifyMerged(PSToken*) {}
void PSPlanDatabaseListener::notifySplit(PSToken*) {}
void PSPlanDatabaseListener::notifyAdded(PSObject*, PSToken*) {}
void PSPlanDatabaseListener::notifyRemoved(PSObject*, PSToken*) {}


// Methods to convert notifications involving internal Europa types to notifications involving 'PS' types:
void PSPlanDatabaseListener::notifyAdded(const ObjectId object) {
  notifyAdded(id_cast<PSObject>(object));
}

void PSPlanDatabaseListener::notifyRemoved(const ObjectId object) {
	notifyRemoved(id_cast<PSObject>(object));
}

void PSPlanDatabaseListener::notifyActivated(const TokenId token) {
	notifyActivated(id_cast<PSToken>(token));
}

void PSPlanDatabaseListener::notifyDeactivated(const TokenId token) {
	notifyDeactivated(id_cast<PSToken>(token));
}

void PSPlanDatabaseListener::notifyRejected(const TokenId token) {
	notifyRejected(id_cast<PSToken>(token));
}

void PSPlanDatabaseListener::notifyMerged(const TokenId token) {
	notifyMerged(id_cast<PSToken>(token));
}

void PSPlanDatabaseListener::notifySplit(const TokenId token) {
	notifySplit(id_cast<PSToken>(token));
}

void PSPlanDatabaseListener::notifyAdded(const ObjectId object, const TokenId token) {
	notifyAdded(id_cast<PSObject>(object), id_cast<PSToken>(token));
}

void PSPlanDatabaseListener::notifyRemoved(const ObjectId object, const TokenId token) {
	notifyRemoved(id_cast<PSObject>(object), id_cast<PSToken>(token));

}

}
