#ifndef _H_IntervalIntDomain
#define _H_IntervalIntDomain

#include "IntervalDomain.hh"

namespace Prototype{
  class IntervalIntDomain: public IntervalDomain {
  public:
    IntervalIntDomain(int lb = -MAX_INT, 
		      int ub = MAX_INT, 
		      bool finite = true, 
		      bool closed = true, 
		      const DomainListenerId& listener = DomainListenerId::noId());
    IntervalIntDomain(const DomainListenerId& listener);
    IntervalIntDomain(const IntervalIntDomain& org);
    IntervalIntDomain(Europa::Domain& org);
    IntervalIntDomain& operator=(const IntervalIntDomain& org);
    bool operator==(const IntervalIntDomain& dom) const;
    void intersect(const IntervalIntDomain& dom);
    bool isSubsetOf(const IntervalIntDomain& dom) const;
    const Europa::Domain makeDomain() const;
    const DomainType& getType() const;

  private:
    void testPrecision(const double& value) const;
  };
}
#endif
