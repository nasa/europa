
#include "ResourceTransactionConstraint.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
//#include "BoolDomain.hh"
#include "Utils.hh"
//#include "ResourceDefs.hh"
#include "Resource.hh"

namespace Prototype
{
  
  ResourceTransactionConstraint::ResourceTransactionConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){
    check_error(variables.size() == (unsigned int) ARG_COUNT);
    //@todo add type checking of each variable in the constraint
  }

  void ResourceTransactionConstraint::handleExecute()
  {
    ObjectDomain& domTO = static_cast<ObjectDomain&> (getCurrentDomain(m_variables[TO]));
    IntervalIntDomain& domTH = static_cast<IntervalIntDomain&> (getCurrentDomain(m_variables[TH]));
    IntervalDomain& domTQ = static_cast<IntervalDomain&> (getCurrentDomain(m_variables[TQ]));

    std::list<double> objs;
    domTO.getValues(objs);

    // Initialize aggregate horizon bounds to propagate time of resource transaction. Only want the least
    // commitment bounds. This interval will eventually span the max and min bounds for all remaining objects.
    int hMin = PLUS_INFINITY;
    int hMax = MINUS_INFINITY;

    // Initialize quantity bounds
    double qMin = -MINUS_INFINITY;
    double qMax = PLUS_INFINITY;

    for(std::list<double>::const_iterator it = objs.begin(); it != objs.end(); ++it) {
      ResourceId resource = *it;
      check_error(resource.isValid());
      int oMin = resource->getHorizonStart();
      int oMax = resource->getHorizonEnd();
      IntervalIntDomain oHorizon(oMin, oMax);

      // Intersect the transaction horizon with that of the resource. If there is no intersection, then
      // remove the resource.
      if (oHorizon.intersect(domTH.getLowerBound(), domTH.getUpperBound()) && oHorizon.isEmpty()){
	domTO.remove(resource);
	if(domTO.isEmpty())
	  return;
	else
	  continue;
      }

      // Now test the quantity to see of the resource is compatible with the transaction
      double oQtyMin = resource->getConsumptionRateMax();
      check_error(oQtyMin <= 0);
      double oQtyMax = resource->getProductionRateMax();
      check_error(oQtyMax >= 0);
      IntervalDomain oQty(oQtyMin, oQtyMax);

      // Intersect the transaction quantity for compliance with rate limits
      if (oQty.intersect(domTQ.getLowerBound(), domTQ.getUpperBound()) && oQty.isEmpty()){
	domTO.remove(resource);
	if(domTO.isEmpty())
	  return;
	else
	  continue;
      }

      // If the resource is still a candidate, Update the horizon bounds using this resurce
      if ( oMin < hMin)
	hMin = oMin;
      if (hMax < oMax)
	hMax = oMax;

      // Also update the quantity bounds
      if ( oQtyMin < qMin)
	qMin = oQtyMin;
      if (qMax < oQtyMax)
	qMax = oQtyMax;
    }

    // The new horizon and quantity data is intersected with the transaction. This policy may change based
    // on our future views about controllability of such things
    domTH.intersect(hMin, hMax);
    if(!domTH.isEmpty())
      domTQ.intersect(qMin, qMax);
  }

  bool ResourceTransactionConstraint::canIgnore(const ConstrainedVariableId& variable, 
				     int argIndex, 
				     const DomainListener::ChangeType& changeType){
    return(false);
  }

}//namespace prototype
