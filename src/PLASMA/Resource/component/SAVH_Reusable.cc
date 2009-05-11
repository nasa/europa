#include "SAVH_Reusable.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_DurativeTokens.hh"
#include "ConstraintEngine.hh"
#include "Domains.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"

namespace EUROPA {
  namespace SAVH {

    Reusable::Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, const LabelStr& detectorName, const LabelStr& profileName,
			 double initCapacityLb, double initCapacityUb, double lowerLimit, double maxInstConsumption,
			 double maxConsumption) :
      Resource(planDatabase, type, name, detectorName, profileName, initCapacityLb, initCapacityUb, lowerLimit, initCapacityUb, maxInstConsumption,
	       maxInstConsumption, maxConsumption, maxConsumption)
	{
	}

    Reusable::Reusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open) :
      Resource(planDatabase, type, name, open)
    {
    }

    Reusable::Reusable(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open) :
      Resource(parent, type, localName, open)
    {
    }

    Reusable::~Reusable()
    {
    }

    void Reusable::getOrderingChoices(const TokenId& token,
				      std::vector<std::pair<TokenId, TokenId> >& results,
				      unsigned int limit) {
      checkError(m_tokensToTransactions.find(token) != m_tokensToTransactions.end(), "Token " << token->getPredicateName().toString() <<
                 "(" << token->getKey() << ") not in profile for " << toString());
      Resource::getOrderingChoices(token, results, limit);
    }

    void Reusable::addToProfile(const TokenId& tok) {
      if(m_tokensToTransactions.find(tok) != m_tokensToTransactions.end()) {
	debugMsg("Reusable:addToProfile",
		 "Token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ") is already in the profile.");
	return;
      }
      ReusableTokenId t(tok);
      debugMsg("Reusable:addToProfile", "Adding token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ")");
      //here's the major difference between Reusable and Reservoir:  always consume the quantity at the start
      //and produce it again at the end
      TransactionId t1 = (new Transaction(t->start(), t->getQuantity(), true))->getId();
      TransactionId t2 = (new Transaction(t->end(), t->getQuantity(), false))->getId();
      m_transactionsToTokens.insert(std::make_pair(t1, tok));
      m_transactionsToTokens.insert(std::make_pair(t2, tok));
      m_tokensToTransactions.insert(std::make_pair(tok, std::make_pair(t1, t2)));
      m_profile->addTransaction(t1);
      m_profile->addTransaction(t2);
    }

    void Reusable::removeFromProfile(const TokenId& tok) {
      if(m_tokensToTransactions.find(tok) == m_tokensToTransactions.end())
        return;

      debugMsg("Reusable:removeFromProfile", "Removing token " << tok->getPredicateName().toString() << "(" << tok->getKey() << ")");
      std::pair<TransactionId, TransactionId> trans = m_tokensToTransactions.find(tok)->second;
      debugMsg("Reusable:removeFromProfile", "Removing transaction for time " << trans.first->time()->toString() << " with quantity " << trans.first->quantity()->toString());
      m_profile->removeTransaction(trans.first);
      debugMsg("Reusable:removeFromProfile", "Removing transaction for time " << trans.second->time()->toString() << " with quantity " << trans.second->quantity()->toString());
      m_profile->removeTransaction(trans.second);
      m_tokensToTransactions.erase(tok);
      m_transactionsToTokens.erase(trans.first);
      m_transactionsToTokens.erase(trans.second);
      std::map<TokenId, std::set<InstantId> >::iterator it = m_flawedTokens.find(tok);
      if(it != m_flawedTokens.end()) {
        m_flawedTokens.erase(it);
        notifyOrderingNoLongerRequired(tok);
      }
      delete (Transaction*) trans.first;
      delete (Transaction*) trans.second;

    }

    UnaryTimeline::UnaryTimeline(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
      : Reusable(planDatabase, type, name, open) {init(1, 1, 0, 1, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, "ClosedWorldFVDetector", "IncrementalFlowProfile");}

    UnaryTimeline::UnaryTimeline(const ObjectId& parent, const LabelStr& type, const LabelStr& name, bool open)
      : Reusable(parent, type, name, open) {init(1, 1, 0, 1, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, PLUS_INFINITY, "ClosedWorldFVDetector", "IncrementalFlowProfile");}


    UnaryTimeline::~UnaryTimeline()
    {
    }

    CBReusable::CBReusable(const PlanDatabaseId& planDatabase,
                           const LabelStr& type,
                           const LabelStr& name,
                           const LabelStr& detectorName,
                           const LabelStr& profileName,
                           double initCapacityLb,
                           double initCapacityUb,
                           double lowerLimit,
                           double maxInstConsumption,
                           double maxConsumption)
        : Resource(planDatabase,
                   type,
                   name,
                   detectorName,
                   profileName,
                   initCapacityLb,
                   initCapacityUb,
                   lowerLimit,
                   initCapacityUb,
                   maxInstConsumption,
                   maxInstConsumption,
                   maxConsumption,
                   maxConsumption)
    {
    }

    CBReusable::CBReusable(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
        : Resource(planDatabase, type, name, open)
    {
    }

    CBReusable::CBReusable(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open)
        : Resource(parent, type, localName, open)
    {
    }

    CBReusable::~CBReusable()
    {
    }

    void CBReusable::addToProfile(const ConstraintId& gc)
    {
        UsesId c = gc;
        if(m_constraintsToTransactions.find(c) != m_constraintsToTransactions.end()) {
            debugMsg("CBReusable:constraints",
                    "Constraint " << c->toString() << " is already in the profile. Ignoring addToProfile().");
            return;
        }

        debugMsg("CBReusable:constraints", "Resource :" << toString() << " adding constraint:" << c->toString());

        // here's the major difference between Reusable and Reservoir:  always consume the quantity at the start and produce it again at the end
        TransactionId t1 = c->getTransaction(Uses::START_VAR);
        TransactionId t2 = c->getTransaction(Uses::END_VAR);
        addToProfile(t1);
        addToProfile(t2);
        m_constraintsToTransactions.insert(std::make_pair(c, std::make_pair(t1, t2)));

        debugMsg("CBReusable:constraints","Resource :" << toString() << " added constraint:" << c->toString());
    }

    void CBReusable::removeFromProfile(const ConstraintId& gc)
    {
        UsesId c = gc;

        if(m_constraintsToTransactions.find(c) == m_constraintsToTransactions.end()) {
          debugMsg("CBReusable:constraints","No Transactions found for :" << c->toString() << " . Ignoring removeFromProfile()");
          return;
        }

        debugMsg("CBReusable:constraints","Resource :" << toString() << " removing constraint:" << c->toString());

        std::pair<TransactionId, TransactionId> trans = m_constraintsToTransactions.find(c)->second;
        removeFromProfile(trans.first);
        removeFromProfile(trans.second);
        m_constraintsToTransactions.erase(c);

        debugMsg("CBReusable:constraints","Resource :" << toString() << " removed constraint:" << c->toString());
    }

    void CBReusable::addToProfile(TransactionId& t)
    {
        m_profile->addTransaction(t);
        // TODO: this is the way to add transactions to the resource, not clean because in this case there is no associated token
        m_transactionsToTokens.insert(std::make_pair(t, TokenId::noId()));
        debugMsg("CBReusable:constraints", "Added transaction for time " << t->time()->toLongString() << " with quantity " << t->quantity()->toString());
    }

    void CBReusable::removeFromProfile(TransactionId& t)
    {
        debugMsg("CBReusable:constraints", "Removing transaction " << t << " for time " << t->time()->toLongString() << " with quantity " << t->quantity()->toString());
        m_profile->removeTransaction(t);
        m_transactionsToTokens.erase(t); // TODO: see note in addToProfile above about relying on map to null tokens
    }

    // TODO: only needed for backwards compatibility with Resource API, rework hierarchy to fix this.
    void CBReusable::addToProfile(const TokenId& tok)
    {
        debugMsg("CBReusable","Ignored addToProfile for Token:" << tok->toString());
    }
    void CBReusable::removeFromProfile(const TokenId& tok)
    {
        debugMsg("CBReusable","Ignored removeFromProfile for Token:" << tok->toString());
    }

    double getLb(ConstrainedVariableId v)
    {
        if (v->lastDomain().isSingleton())
            return v->lastDomain().getSingletonValue();

        return v->lastDomain().getLowerBound();
    }

    double getUb(ConstrainedVariableId v)
    {
        if (v->lastDomain().isSingleton())
            return v->lastDomain().getSingletonValue();

        return v->lastDomain().getUpperBound();
    }


    std::set<UsesId> CBReusable::getConstraintsForInstant(const InstantId& instant)
    {
        std::set<UsesId> retval;

        std::map<UsesId, std::pair<TransactionId, TransactionId> >::const_iterator it = m_constraintsToTransactions.begin();

        for (;it != m_constraintsToTransactions.end(); ++it) {
            UsesId c = it->first;
            double lb = getLb(c->getScope()[Uses::START_VAR]);
            double ub = getUb(c->getScope()[Uses::END_VAR]);
            int t = instant->getTime();
            if ((lb <= t) && (t <= ub))
                retval.insert(c->getId());
        }

        return retval;
    }

    void CBReusable::notifyViolated(const InstantId inst, Resource::ProblemType problem)
    {
        check_error(inst.isValid());
        check_error(inst->isViolated());
        check_error(!inst->getTransactions().empty());

        debugMsg("CBReusable:violations", "Received notification of violation at time " << inst->getTime());

        TransactionId txn = *(inst->getTransactions().begin());
        ConstraintEngineId ce = txn->quantity()->getConstraintEngine(); // TODO: keep track of constraint engine more cleanly?
        if (ce->getAllowViolations()) { // TODO: move this test to the constraint?
            std::set<UsesId> constraints = getConstraintsForInstant(inst);
            std::set<UsesId>::const_iterator it = constraints.begin();
            for(;it != constraints.end(); ++it) {
                UsesId c = *it;
                c->notifyViolated(problem,inst);
            }
        }
        else {
            const_cast<AbstractDomain&>(txn->quantity()->lastDomain()).empty();
        }
    }

    void CBReusable::notifyNoLongerViolated(const InstantId inst)
    {
        debugMsg("CBReusable:violations", "Received notification of violation removed at time " << inst->getTime());

        std::set<UsesId> constraints = getConstraintsForInstant(inst);
        std::set<UsesId>::const_iterator it = constraints.begin();
        for(;it != constraints.end(); ++it) {
            UsesId c = *it;
            c->notifyNoLongerViolated(inst);
        }
    }

    void CBReusable::notifyFlawed(const InstantId inst)
    {
        check_error(inst.isValid());
        checkError(inst->isFlawed(), "Instant at time " << inst->getTime() << " claims to be flawed, but resource didn't know it.");
        check_error(!inst->getTransactions().empty());


        if(m_flawedInstants.find(inst->getTime()) == m_flawedInstants.end()) {
            m_flawedInstants.insert(std::pair<int, InstantId>(inst->getTime(), inst));
            debugMsg("CBReusable:flaws", "Received notification of flaw at time " << inst->getTime());
        }
        else {
            debugMsg("CBReusable:flaws", "Ignored redundant notification of flaw at time " << inst->getTime());
        }
    }

    void CBReusable::notifyNoLongerFlawed(const InstantId inst)
    {
        check_error(inst.isValid());

        if(m_flawedInstants.find(inst->getTime()) != m_flawedInstants.end()) {
            m_flawedInstants.erase(inst->getTime());
            debugMsg("CBReusable:flaws", "Removed instant " << inst->getTime() << " from the set of flawed instants.");
        }
        else {
            debugMsg("CBReusable:flaws", "Ignored notification that instant " << inst->getTime() << " is no longer flawed. It wasn't marked as flawed in the first place.");
        }
    }

    Uses::Uses(const LabelStr& name,
            const LabelStr& propagatorName,
            const ConstraintEngineId& ce,
            const std::vector<ConstrainedVariableId>& scope)
        : Constraint(name, propagatorName, ce, scope)
    {
        checkError(scope.size() == 4, "Uses constraint requires resource,qty,start,end");

        m_txns.push_back((new Transaction(scope[Uses::START_VAR], scope[Uses::QTY_VAR], true, getId()))->getId());
        m_txns.push_back((new Transaction(scope[Uses::END_VAR],   scope[Uses::QTY_VAR], false, getId()))->getId());

        if(scope[RESOURCE_VAR]->lastDomain().isSingleton()) {
            m_resource = CBReusableId(scope[RESOURCE_VAR]->lastDomain().getSingletonValue());
            check_error(m_resource.isValid());
            debugMsg("Uses:Uses", "Adding constraint " << toString() << " to resource-profile of resource " << m_resource->toString() );
            m_resource->addToProfile(getId());
        }
    }

    const TransactionId& Uses::getTransaction(int var) const
    {
        if (var == Uses::START_VAR)
            return m_txns[0];
        else
            return m_txns[1];
    }

    void Uses::handleDiscard()
    {
        if (!Entity::isPurging()) {
            // TODO: object deletions are only assumed to happen when purging, it'll probably be hard to migrate later if we want more intelligent memory mgmt.
            if (m_resource.isId()) {
                m_resource->removeFromProfile(getId());
                debugMsg("Uses:Uses", "Removed " << toString() << " from profile for resource " << m_resource->toString());
                m_resource = CBReusableId::noId();
            }
        }

        // TODO: make sure Resource destructor doesn't get to these first
        for (unsigned int i=0;i<m_txns.size();i++) {
            TransactionId txn = m_txns[i];
            delete (Transaction*) txn;
        }
        m_txns.clear();

        Constraint::handleDiscard();
    }

    bool Uses::canIgnore(const ConstrainedVariableId& variable,
            int argIndex,
            const DomainListener::ChangeType& changeType)
    {
        ConstrainedVariableId res = m_variables[RESOURCE_VAR];

        // if this is a singleton message see if we can bind the resource
        if(changeType == DomainListener::RESTRICT_TO_SINGLETON ||
                changeType == DomainListener::SET_TO_SINGLETON ||
                variable->lastDomain().isSingleton()) {

            if(m_resource.isNoId() && res->lastDomain().isSingleton()) {
                m_resource = CBReusableId(res->lastDomain().getSingletonValue());
                check_error(m_resource.isValid());
                m_resource->addToProfile(getId());
                debugMsg("Uses:Uses", "Added " << toString() << " to profile for resource " << m_resource->toString());
            }
        }
        // if this is a relax/reset message, see if we need to unbind the resource
        else if((changeType == DomainListener::RESET || changeType == DomainListener::RELAXED)
                && m_resource.isValid()) {

            if((variable->getKey() == res->getKey()) && !(res->lastDomain().isSingleton())) {
                m_resource->removeFromProfile(getId());
                debugMsg("Uses:Uses", "Removed " << toString() << " from profile for resource " << m_resource->toString());
                m_resource = CBReusableId::noId();
            }
        }

        // Since we don't do bounds prop, always return true
        return true;
    }

    void Uses::handleExecute()
    {
        // TODO: not doing bounds prop for now, but we could
    }

    std::string Uses::getViolationExpl() const
    {
        std::ostringstream os;

        std::map<InstantId,Resource::ProblemType>::const_iterator it = m_violationProblems.begin();
        for(;it != m_violationProblems.end();++it) {
            os << Resource::getProblemString(it->second)
               << " for resource " << m_resource->getName().toString()
               << " at instant " << (it->first->getTime());
        }

        return os.str();
    }

    void Uses::notifyViolated(Resource::ProblemType problem, const InstantId inst)
    {
        m_violationProblems[inst] = problem;
        if (m_violationProblems.size() == 1) {
            Constraint::notifyViolated();
            debugMsg("Uses:violations", "Marked constraint as violated : " << toString());
        }
    }

    void Uses::notifyNoLongerViolated(const InstantId inst)
    {
        if (m_violationProblems.find(inst) == m_violationProblems.end()) {
            debugMsg("Uses:violations", "Unrecognized instant " << inst << " ignoring notifyNoLongerViolated");
            return;
        }

        bool wasViolated = (m_violationProblems.size() > 0);
        m_violationProblems.erase(inst);
        if (wasViolated && m_violationProblems.size() == 0) {
            Constraint::notifyNoLongerViolated();
            debugMsg("Uses:violations", "Marked constraint as NoLongerViolated : " << toString());
        }
    }

    void Uses::notifyViolated()
    {
        check_error(ALWAYS_FAILS,"unqualified notifyViolated() should never be called for Uses constraint");
    }

    // This can be called when variables attached to the constraint are relaxed
    void Uses::notifyNoLongerViolated()
    {
        m_violationProblems.clear();
        Constraint::notifyNoLongerViolated();
    }

    const LabelStr& Uses::CONSTRAINT_NAME() {
        static const LabelStr sl_const("uses");
        return sl_const;
    }

    const LabelStr& Uses::PROPAGATOR_NAME() {
        static const LabelStr sl_const("SAVH_Resource");
        return sl_const;
    }


  }
}
