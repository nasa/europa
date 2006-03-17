#ifndef _H_ResourceDefs
#define _H_ResourceDefs

#include "ConstraintEngineDefs.hh"

namespace EUROPA {
  namespace SAVH {
    typedef std::pair<ConstrainedVariableId, ConstrainedVariableId> Transaction;
    typedef Id<Transaction> TransactionId;
    
    class Profile;
    typedef Id<Profile> ProfileId;
    
    class ProfileIterator;
    typedef Id<ProfileIterator> ProfileIteratorId;
    
    class Instant;
    typedef Id<Instant> InstantId;
  }
}

#endif
