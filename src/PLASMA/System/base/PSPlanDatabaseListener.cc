#include "PSPlanDatabaseListener.hh"
#include "Object.hh"

namespace EUROPA {

PSPlanDatabaseListener::PSPlanDatabaseListener()
: PlanDatabaseListener()
{}

// Methods to convert notifications involving internal Europa types to notifications involving 'PS' types:
void PSPlanDatabaseListener::notifyAdded(const ObjectId& object) {
	notifyAdded((PSObject *) object);
}

}
