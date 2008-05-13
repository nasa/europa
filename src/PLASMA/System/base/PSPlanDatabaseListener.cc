#include "PSPlanDatabaseListener.hh"
#include "PSPlanDatabase.hh"
#include "PlanDatabase.hh"
#include "PSEngine.hh"

namespace EUROPA {
	  
PSPlanDatabaseListener::PSPlanDatabaseListener(PSEngine* engine)
	: PlanDatabaseListener(engine->getPlanDatabase()), 
	  m_psengine(*engine)	
{}

// Methods to convert notifications involving internal Europa types to notifications involving 'PS' types:
void PSPlanDatabaseListener::notifyAdded(const ObjectId& object)
{
	notifyAdded(m_psengine.getObjectByKey(object->getKey()));
}
}
