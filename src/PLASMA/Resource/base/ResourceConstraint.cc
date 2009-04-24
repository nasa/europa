#include "ResourceConstraint.hh"
#include "Utils.hh"
#include "Resource.hh"
#include "ConstraintEngine.hh"
#include "Domains.hh"
#include "Utils.hh"
#include "Debug.hh"

namespace EUROPA
{

  ResourceConstraint::ResourceConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == (unsigned int) ARG_COUNT);
    checkError(variables[OBJECT]->getName() == LabelStr("object"), variables[OBJECT]->toString());
    checkError(variables[TIME]->baseDomain().isInterval() && variables[TIME]->getName() == LabelStr("time"),
	       variables[TIME]->toString());
    checkError(variables[USAGE]->baseDomain().isInterval() && variables[USAGE]->getName() == LabelStr("quantity"),
	       variables[USAGE]->toString());
  }

  AbstractDomain&  ResourceConstraint::getCurrentDomain(const ConstrainedVariableId& var){
    return Constraint::getCurrentDomain(var);
  }

  void ResourceConstraint::handleExecute(){
    ObjectDomain& domTO = static_cast<ObjectDomain&> (getCurrentDomain(m_variables[OBJECT]));
    IntervalDomain& domTQ = static_cast<IntervalDomain&> (getCurrentDomain(m_variables[USAGE]));

    // Do nothing if the domain is open
    if(domTO.isOpen())
      return;

    std::list<double> objs;
    domTO.getValues(objs);

    // Initialize quantity bounds
    double qMin = -MINUS_INFINITY;
    double qMax = PLUS_INFINITY;

    for(std::list<double>::const_iterator it = objs.begin(); it != objs.end(); ++it) {
      ResourceId resource = *it;
      check_error(resource.isValid());

      // Now test the quantity to see of the resource is compatible with the transaction
      double oQtyMin = resource->getConsumptionRateMax();
      check_error(oQtyMin <= 0);
      double oQtyMax = resource->getProductionRateMax();
      check_error(oQtyMax >= 0);
      IntervalDomain oQty(oQtyMin, oQtyMax);

      debugMsg("ResourceConstraint:handleExecute",
	       "Evaluating " << resource->getName().toString() << " as a candidate." <<
	       " Rate limits of " << oQty.toString() << " must be adhered to  by " << domTQ.toString());

      // Intersect the transaction quantity for compliance with rate limits
      if (oQty.intersect(domTQ.getLowerBound(), domTQ.getUpperBound()) && oQty.isEmpty()){
	debugMsg("ResourceConstraint:handleExecute",
		 "Removing " << resource->getName().toString() << " as a candidate.");

	domTO.remove(resource);
	if(domTO.isEmpty())
	  return;
	else
	  continue;
      }

      // Also update the quantity bounds
      if ( oQtyMin < qMin)
	qMin = oQtyMin;
      if (qMax < oQtyMax)
	qMax = oQtyMax;
    }

    // The new quantity data is intersected with the transaction. This policy may change based
    // on our future views about controllability of such things
    domTQ.intersect(qMin, qMax);

    // If the resource variable is a singleton, mark the resource as dirty.
    if(domTO.isSingleton()){
      ResourceId resource = (ResourceId) domTO.getSingletonValue();
      resource->markDirty();
    }
  }

}//namespace EUROPA
