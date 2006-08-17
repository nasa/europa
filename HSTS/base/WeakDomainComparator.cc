#include "WeakDomainComparator.hh"
#include "Entity.hh"
#include "PlanDatabase.hh"

namespace EUROPA {

  WeakDomainComparator::WeakDomainComparator(){}

  WeakDomainComparator::~WeakDomainComparator(){}

  /**
   * @brief Implements the default tests for comparison, but relaxes type checking on members
   * of enum types. Useful for the WeakEq constraint.
   */
  bool WeakDomainComparator::canCompare(const AbstractDomain& domx, const AbstractDomain& domy) const {
    // std::cerr << "In WeakDomainComparator::canCompare\n";
    // If either is numeric, both must be numeric. Assumes the type names don't matter
    if(domx.isNumeric() || domy.isNumeric())
      return(domx.isNumeric() && domy.isNumeric());
   
    const SchemaId& schema = Schema::instance();
    if(schema->isObjectType(domx.getTypeName()) || schema->isObjectType(domy.getTypeName()))
      return(schema->isObjectType(domx.getTypeName()) && schema->isObjectType(domy.getTypeName()));

    // Otherwsie we permit the comparison
    return true;
  }

}
