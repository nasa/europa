#include "SAVH_Reusable.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Transaction.hh"
#include "SAVH_DurativeTokens.hh"
#include "ConstraintEngine.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
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
                    "Constraint " << c->toString() << " is already in the profile.");
            return;
        }

        debugMsg("CBReusable:constraints", "Resource :" << toString() << " adding constraint:" << c->toString());
        // here's the major difference between Reusable and Reservoir:  always consume the quantity at the start and produce it again at the end
        TransactionId t1 = (new Transaction(c->getScope()[Uses::START_VAR], c->getScope()[Uses::QTY_VAR], true))->getId();
        TransactionId t2 = (new Transaction(c->getScope()[Uses::END_VAR], c->getScope()[Uses::QTY_VAR], false))->getId();

        // TODO: this is the way to add transactions to the resource, not clean because in this case there is no associated token
        m_transactionsToTokens.insert(std::make_pair(t1, TokenId::noId()));
        m_transactionsToTokens.insert(std::make_pair(t2, TokenId::noId()));

        m_constraintsToTransactions.insert(std::make_pair(c, std::make_pair(t1, t2)));
        m_profile->addTransaction(t1);
        debugMsg("CBReusable:constraints", "Added transaction for time " << t1->time()->toLongString() << " with quantity " << t1->quantity()->toString());
        m_profile->addTransaction(t2);
        debugMsg("CBReusable:constraints", "Added transaction for time " << t2->time()->toLongString() << " with quantity " << t2->quantity()->toString());
        debugMsg("CBReusable:constraints","Resource :" << toString() << " added constraint:" << c->toString());
    }

    void CBReusable::removeFromProfile(const ConstraintId& gc)
    {
        UsesId c = gc;

        if(m_constraintsToTransactions.find(c) == m_constraintsToTransactions.end())
          return;

        debugMsg("CBReusable:constraints","Resource :" << toString() << " removing constraint:" << c->toString());

        std::pair<TransactionId, TransactionId> trans = m_constraintsToTransactions.find(c)->second;
        debugMsg("CBReusable:constraints", "Removing transaction " << trans.first << " for time " << trans.first->time()->toLongString() << " with quantity " << trans.first->quantity()->toString());
        m_profile->removeTransaction(trans.first);
        debugMsg("CBReusable:constraints", "Removing transaction " << trans.second << " for time " << trans.second->time()->toLongString() << " with quantity " << trans.second->quantity()->toString());
        m_profile->removeTransaction(trans.second);
        m_constraintsToTransactions.erase(c);

        // TODO: see note in addToProfile above about relying on map to null tokens
        m_transactionsToTokens.erase(trans.first);
        m_transactionsToTokens.erase(trans.second);

        delete (Transaction*) trans.first;
        delete (Transaction*) trans.second;

        debugMsg("CBReusable:constraints","Resource :" << toString() << " removed constraint:" << c->toString());
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

    void CBReusable::notifyViolated(const InstantId inst, ResourceProblem::Type problem)
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

        if(scope[RESOURCE_VAR]->lastDomain().isSingleton()) {
            m_resource = CBReusableId(scope[RESOURCE_VAR]->lastDomain().getSingletonValue());
            check_error(m_resource.isValid());
            debugMsg("Uses:Uses", "Adding constraint " << toString() << " to resource-profile of resource " << m_resource->toString() );
            m_resource->addToProfile(getId());
        }
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
                debugMsg("Uses:canIgnore", "Added " << toString() << " to profile for resource " << m_resource->toString());
            }
        }
        // if this is a relax/reset message, see if we need to unbind the resource
        else if((changeType == DomainListener::RESET || changeType == DomainListener::RELAXED)
                && m_resource.isValid()) {

            if((variable->getKey() == res->getKey()) && !(res->lastDomain().isSingleton())) {
                m_resource->removeFromProfile(getId());
                debugMsg("Uses:canIgnore", "Removed " << toString() << " from profile for resource " << m_resource->toString());
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

        std::map<InstantId,ResourceProblem::Type>::const_iterator it = m_violationProblems.begin();
        for(;it != m_violationProblems.end();++it) {
            os << ResourceProblem::getString(it->second)
               << " for resource " << m_resource->getName().toString()
               << " at instant " << (it->first->getTime());
        }

        return os.str();
    }

    void Uses::notifyViolated(ResourceProblem::Type problem, const InstantId inst)
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

  }
}
