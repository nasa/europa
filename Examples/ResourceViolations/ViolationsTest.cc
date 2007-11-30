
#include "PSEngine.hh"
#include "Debug.hh"

using namespace EUROPA;

void testViolations(PSEngine& psengine)
{
	/*
	PSObject* res = psengine.getObjectsByType("CapacityResource").get(0);
	PSList<PSToken*> toks = res->getTokens();
	PSToken* t1 = toks.get(0);
	PSToken* t2 = toks.get(1);
    */
	
	PSObject* act_obj1 = psengine.getObjectsByType("Activity").get(0);
	PSObject* act_obj2 = psengine.getObjectsByType("Activity").get(1);
	PSObject* act_obj3 = psengine.getObjectsByType("Activity").get(2);

	PSToken* act1 = act_obj1->getTokens().get(0);
	PSToken* act2 = act_obj2->getTokens().get(0);	
	PSToken* act3 = act_obj3->getTokens().get(0);	

	PSVariable* s1 = act1->getParameter("start");
	PSVariable* s2 = act2->getParameter("start");
	PSVariable* s3 = act3->getParameter("start");

	PSVarValue vv5 = PSVarValue::getInstance(5);
	PSVarValue vv11 = PSVarValue::getInstance(11);
	PSVarValue vv18 = PSVarValue::getInstance(18);
	PSVarValue vv20 = PSVarValue::getInstance(20);

	// Cause Violation
	s1->specifyValue(vv5);
	s2->specifyValue(vv11);
	s2->specifyValue(vv20);
	debugMsg("testViolations",psengine.getViolation());
	debugMsg("testViolations",psengine.getViolationExpl());

	// Remove Violation
	s3->specifyValue(vv18);
	debugMsg("testViolations",psengine.getViolation());
	debugMsg("testViolations",psengine.getViolationExpl());
}

