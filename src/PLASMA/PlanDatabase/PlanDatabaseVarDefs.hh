#ifndef PLANDATABASEVARDEFS_H_
#define PLANDATABASEVARDEFS_H_

#include "Id.hh"
#include "Variable.hh"

namespace EUROPA {
  template<class DomainType> class TokenVariable;
  class StateDomain;
  typedef Id< TokenVariable<StateDomain> > StateVarId;

  typedef Id< Variable<IntervalIntDomain> > TimeVarId;
  typedef Id< TokenVariable<IntervalIntDomain> > TempVarId;

  class ObjectDomain;
  typedef Id< TokenVariable<ObjectDomain> > ObjectVarId;


}

#endif /* PLANDATABASEVARDEFS_H_ */
