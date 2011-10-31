
#include "GenericFVDetector.hh"
#include <cmath>
#include "Instant.hh"
#include "Profile.hh"

namespace EUROPA {

    GenericFVDetector::GenericFVDetector(const ResourceId res) : FVDetector(res) {
      m_maxInstConsumption = res->getMaxInstConsumption();
      m_maxInstProduction = res->getMaxInstProduction();
      m_maxCumulativeConsumption = res->getMaxConsumption();
      m_maxCumulativeProduction = res->getMaxProduction();
      debugMsg("GenericFVDetector:GenericFVDetector", "Created FVDetector for " << res->toString());
      debugMsg("GenericFVDetector:GenericFVDetector", "Got values: "
    		  << " max instantaneous consumption(" << m_maxInstConsumption << ")"
    		  << " max instantaneous production(" << m_maxInstProduction << ")"
    		  << " max consumption(" << m_maxCumulativeConsumption << ")"
    		  << " max production(" << m_maxCumulativeProduction << ")");
    }

    Resource::ProblemType GenericFVDetector::getResourceViolation(const InstantId inst) const
    {
    	if (inst->getMinCumulativeConsumption() > m_maxCumulativeConsumption)
    	{
    		debugMsg("GenericFVDetector:detect", "Cumulative consumption violation.");
    		return Resource::ConsumptionSumExceeded;
    	}

    	if (inst->getMinCumulativeProduction() > m_maxCumulativeProduction)
    	{
    		debugMsg("GenericFVDetector:detect", "Cumulative production violation.");
    		return Resource::ProductionSumExceeded;
    	}

    	if (inst->getMinInstantConsumption() > m_maxInstConsumption)
    	{
    		debugMsg("GenericFVDetector:detect", "Instantaneous consumption violation.");
    		return Resource::ConsumptionRateExceeded;
    	}

    	if (inst->getMinInstantProduction() > m_maxInstProduction)
    	{
    		debugMsg("GenericFVDetector:detect", "Instantaneous production violation.");
    		return Resource::ProductionRateExceeded;
    	}
    	return getResourceLevelViolation(inst);
    }
    /* Reporting these flaws is new, and is not fully supported.  To get full functionality, we need:
     *
     * 1) To implement the ?? methods as is done for ??={Lower,Upper}and used in the handleResourceLevelFlaws subclass methods.
     *    The variables set in that case are used by the threat manager so that one can decide to prefer one type of flaw
     *    over another, or to order flaws by magnitude.
     *
     * 2) To handle these flaws differently.  A flaw could be resolved by either bounding consumption/production variables OR
     *    by adding an action (I think) and current flaw handling will simply try rescheduling things, which could easily not
     *    help.  We probably need a specialized DecisionPoint.
     *
     * Alternatives to full functionality:
     * a) Ignore these flaws (currently we cannot filter flaws based on type, but we may in the future)
     * b) Be conservative and report flaws as violations.
     *
     */
    void GenericFVDetector::handleResourceFlaws(const InstantId inst)
    {
    	if (inst->getMaxCumulativeConsumption() > m_maxCumulativeConsumption)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(std::abs(m_maxCumulativeConsumption - inst->getMaxCumulativeConsumption()));
       		debugMsg("GenericFVDetector:detect", "Cumulative consumption flaw.");
    	}
    	if (inst->getMaxCumulativeProduction() > m_maxCumulativeProduction)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(std::abs(m_maxCumulativeProduction - inst->getMaxCumulativeProduction()));
       		debugMsg("GenericFVDetector:detect", "Cumulative production flaw.");
    	}
    	if (inst->getMaxInstantConsumption() > m_maxInstConsumption)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(std::abs(m_maxInstConsumption - inst->getMaxInstantConsumption()));
       		debugMsg("GenericFVDetector:detect", "Instantaneous consumption flaw.");
    	}
    	if (inst->getMaxInstantProduction() > m_maxInstProduction)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(std::abs(m_maxInstProduction - inst->getMaxInstantProduction()));
       		debugMsg("GenericFVDetector:detect", "Instantaneous production flaw.");
    	}
    	handleResourceLevelFlaws(inst);
    }

    bool GenericFVDetector::detect(const InstantId inst) {
        debugMsg("GenericFVDetector:detect", "Detecting flaws and violations at time " << inst->getTime());
     	debugMsg("GenericFVDetector:detect", "Max cumulative consumption: " << m_maxCumulativeConsumption <<
    			".  At instant: " << inst->getMaxCumulativeConsumption() << ".");
    	debugMsg("GenericFVDetector:detect", "Max cumulative production: " << m_maxCumulativeConsumption <<
    			".  At instant: " << inst->getMaxCumulativeProduction() << ".");
    	debugMsg("GenericFVDetector:detect", "Max instantaneous consumption: " << m_maxInstConsumption <<
    			".  At instant: " << inst->getMaxInstantConsumption() << ".");
    	debugMsg("GenericFVDetector:detect", "Max instantaneous production: " << m_maxInstConsumption <<
    			".  At instant: " << inst->getMaxInstantProduction() << ".");

    	// A: Look for violations (note that only the first one found is returned)
   	 	bool wasViolated = inst->isViolated();
    	inst->setViolated(false);

    	Resource::ProblemType violation = getResourceViolation(inst);

    	if(violation != Resource::NoProblem) {
    		debugMsg("GenericFVDetector:detect", "Flagging violation at instant " << inst->getTime() << " : ");
    		inst->setViolated(true);
    		notifyOfViolation(inst, violation);
    	}
    	else if(wasViolated)
    	{
    		notifyNoLongerViolated(inst);
    		debugMsg("GenericFVDetector:detect", "Clearing Violation");
    	}

    	// B:  Look for flaws (multiple flaws might be found)
    	bool wasFlawed = inst->isFlawed();
    	inst->setFlawed(false); // new

    	handleResourceFlaws(inst);

    	if(inst->isFlawed())
    	{
    		debugMsg("GenericFVDetector:detect", "Flagging (at least one) flaw:");
    		notifyOfFlaw(inst);
    	}
    	else if(wasFlawed)
    	{
    		debugMsg("GenericFVDetector:detect", "Clearing flaw.");
    		notifyNoLongerFlawed(inst);
    	}

    	// Return true if there was a violation and we're not allowing violations:
    	return(inst->isViolated() && !allowViolations());
    }

    PSResourceProfile* GenericFVDetector::getFDLevelProfile()
    {
    	return new GenericFVProfile(this,m_res->getProfile(),true);
    }

    PSResourceProfile* GenericFVDetector::getVDLevelProfile()
    {
    	return new GenericFVProfile(this,m_res->getProfile(),false);
    }

    Resource::ProblemType GenericFVDetector::getResourceLevelViolation(const InstantId inst) const
    {
    	edouble limitLb, limitUb;
    	getLimitBounds(inst,limitLb,limitUb);
    	edouble levelLb, levelUb;
    	getVDLevelBounds(inst,levelLb,levelUb);

    	if (levelUb < limitLb)
    	{
    		debugMsg("GenericFVDetector:detect",
    				"Lower limit violation.  Limit: " << limitLb << " Upper level: " << levelUb);
    		return Resource::LevelTooLow;
    	}
    	if (levelLb > limitUb)
    	{
        	debugMsg("GenericFVDetector:detect",
        			"Upper limit violation.  Limit: " << limitUb << " Lower level: " << levelLb);
        	return Resource::LevelTooHigh;
    	}
    	return Resource::NoProblem;
    }

    void GenericFVDetector::handleResourceLevelFlaws(const InstantId inst)
    {
    	edouble limitLb, limitUb;
    	getLimitBounds(inst,limitLb,limitUb);
    	edouble levelLb, levelUb;
    	getFDLevelBounds(inst,levelLb,levelUb);

    	if(levelLb < limitLb)
    	{
    		inst->setFlawed(true);
    		inst->setLower(true);
    		inst->setLowerMagnitude(std::abs(limitLb - levelLb));
    		debugMsg("GenericFVDetector:detect", "Lower limit flaw.");
    	}

    	if(levelUb > limitUb)
    	{
    		inst->setFlawed(true);
    		inst->setUpper(true);
    		inst->setUpperMagnitude(std::abs(limitUb - levelUb));
    		debugMsg("GenericFVDetector:detect", "Upper limit flaw.");
    	}
    }

    void GenericFVDetector::getLimitBounds(const InstantId& inst, edouble& lb, edouble& ub) const
    {
    	// TODO: make 1 call instead of 2?
    	lb = m_res->getLowerLimit(inst);
    	ub = m_res->getUpperLimit(inst);
    }

    // TODO: Level(t) = Capacity(t) - Usage(t)
    void GenericFVDetector::getDefaultLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
    {
    	const std::pair<edouble,edouble>& capacityBounds = m_res->getCapacityProfile()->getValue(inst->getTime());

    	// TODO: make 1 call instead of 2?
    	edouble usageLb = inst->getLowerLevel();
    	edouble usageUb = inst->getUpperLevel();

    	// Positive Usage is computed as a negative value by the profile, so add instead of subtract
    	lb = capacityBounds.first + usageLb;
    	ub = capacityBounds.second + usageUb;

    	debugMsg("GenericFVDetector:getDeafultLevelBounds",
    		m_res->getName().toString() << " - time:" << inst->getTime() << " "
    		<< "Capacity[" << capacityBounds.first << "," << capacityBounds.second << "] "
    		<< "Usage[" << usageLb << "," << usageUb << "] "
    		<< "Level[" << lb << "," << ub << "]");
    }

    GenericFVProfile::GenericFVProfile(GenericFVDetector* fvd, const ProfileId& profile, bool isFDProfile)
    	: m_detector(fvd)
    	, m_profile(profile)
    	, m_isFDProfile(isFDProfile)
    {
    	update();
    }

    void GenericFVProfile::update()
    {
    	m_times.clear();
    	m_bounds.clear();

		ProfileIterator it(m_profile);
		while(!it.done()) {
			TimePoint inst = cast_basis(it.getTime());
			m_times.push_back(inst);
			edouble lb,ub;
			if (m_isFDProfile)
				m_detector->getFDLevelBounds(it.getInstant(),lb,ub);
			else
				m_detector->getVDLevelBounds(it.getInstant(),lb,ub);

			m_bounds[inst] = std::pair<edouble,edouble>(lb,ub);
			it.next();
		}
    }

	PSList<TimePoint> GenericFVProfile::getTimes()
	{
		return m_times;
	}

	double GenericFVProfile::getLowerBound(TimePoint time)
	{
		checkError(m_bounds.find(time) != m_bounds.end(),"Time " << time << " doesn't exist in FVProfile");
		return cast_double(m_bounds[time].first);
	}

	double GenericFVProfile::getUpperBound(TimePoint time)
	{
		checkError(m_bounds.find(time) != m_bounds.end(),"Time " << time << " doesn't exist in FVProfile");
		return cast_double(m_bounds[time].second);
	}
}
