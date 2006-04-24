#ifndef _H_SAVH_ResourceDefs
#define _H_SAVH_ResourceDefs

#include "ConstraintEngineDefs.hh"

namespace EUROPA {
  namespace SAVH {
    //typedef std::pair<ConstrainedVariableId, ConstrainedVariableId> Transaction;
    class Transaction;
    typedef Id<Transaction> TransactionId;
    
    class Profile;
    typedef Id<Profile> ProfileId;
    
    class ProfileIterator;
    typedef Id<ProfileIterator> ProfileIteratorId;
    
    class Instant;
    typedef Id<Instant> InstantId;
    
    class FVDetector;
    typedef Id<FVDetector> FVDetectorId;

    class Resource;
    typedef Id<Resource> ResourceId;

    class Reservoir;
    typedef Id<Reservoir> ReservoirId;

    class ReservoirToken;
    typedef Id<ReservoirToken> ReservoirTokenId;
  }

  class TimetableToken;
  typedef Id<TimetableToken> TimetableTokenId;
}

#endif
