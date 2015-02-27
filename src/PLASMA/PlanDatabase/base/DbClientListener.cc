#include "DbClientListener.hh"

namespace EUROPA {
void DbClientListener::notifyObjectCreated(const ObjectId) {}
void DbClientListener::notifyObjectCreated(const ObjectId,
                                           const std::vector<const Domain*>&){}
void DbClientListener::notifyObjectDeleted(const ObjectId) {}
void DbClientListener::notifyClosed(){}
void DbClientListener::notifyClosed(const std::string&){}
void DbClientListener::notifyTokenCreated(const TokenId){}
void DbClientListener::notifyTokenDeleted(const TokenId, const std::string&){}
void DbClientListener::notifyConstrained(const ObjectId, const TokenId,
                                         const TokenId){}
void DbClientListener::notifyFreed(const ObjectId, const TokenId,
                                   const TokenId){}
void DbClientListener::notifyActivated(const TokenId){}
void DbClientListener::notifyMerged(const TokenId, const TokenId){}
void DbClientListener::notifyMerged(const TokenId){}
void DbClientListener::notifyRejected(const TokenId){}
void DbClientListener::notifyCancelled(const TokenId){}
void DbClientListener::notifyConstraintCreated(const ConstraintId){}
void DbClientListener::notifyConstraintDeleted(const ConstraintId){}
void DbClientListener::notifyVariableCreated(const ConstrainedVariableId){}
void DbClientListener::notifyVariableDeleted(const ConstrainedVariableId){}
void DbClientListener::notifyVariableSpecified(const ConstrainedVariableId){}
void DbClientListener::notifyVariableRestricted(const ConstrainedVariableId){}
void DbClientListener::notifyVariableClosed(const ConstrainedVariableId){}
void DbClientListener::notifyVariableReset(const ConstrainedVariableId){}
}
