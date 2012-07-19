#ifndef _H_WeakDomainComparator
#define _H_WeakDomainComparator

/**
 * @file WeakDomainComparator.hh
 * @author Patrick Daley
 * @date January, 2005
 * @brief Declares custom Domain Comparator with weak type checking that extend default DomainComparator.
 *
 * Operations among domains are type checked according the compatibility rules enforced by a DomainComparator.
 *
 * @see AbstractDomain, DomainComparator
 */
#include "ConstraintEngineDefs.hh"
#include "LabelStr.hh"
#include "AbstractDomain.hh"
#include <list>
#include <map>
#include <string>
#include <set>

namespace EUROPA {

  /**
   * @class WeakDomainComparator
   * @brief Class for testng if 2 domains can be compared with relaxed type checking 
   *
   * @see AbstractDomain::canBeCompared
   */
  class WeakDomainComparator : public DomainComparator {
  public:
    WeakDomainComparator();
    virtual ~WeakDomainComparator();

    /**
     * @brief Tests if domains can be compared. Relaxes the check on enumerated and string domains.
     */
    virtual bool canCompare(const AbstractDomain& domx, const AbstractDomain& domy) const;

  };

}
#endif
