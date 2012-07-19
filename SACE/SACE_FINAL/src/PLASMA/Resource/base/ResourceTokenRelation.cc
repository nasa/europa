#include "ResourceTokenRelation.hh"
#include "Constraint.hh"
#include "Resource.hh"
#include "Token.hh"
#include "ConstrainedVariable.hh"
#include "TokenVariable.hh"
#include "Instant.hh"

namespace EUROPA {

   ResourceTokenRelation::ResourceTokenRelation(const ConstraintEngineId& constraintEngine,
                                                 const std::vector<ConstrainedVariableId>& scope,
                                                 const TokenId& tok)
      : Constraint(CONSTRAINT_NAME(), PROPAGATOR_NAME(), constraintEngine, scope)
      , m_token(tok)
      , m_violationProblem(Resource::NoProblem)
    {
      //       debugMsg("Resource:ResourceTokenRelation", "Checking resource " << m_resource->getKey() << " against variable " << scope[0]->toString());
      //       condDebugMsg(scope[0]->lastDomain().isSingleton(), "Resource:ResourceTokenRelation",
      // 		   "Value of variable: " << scope[0]->lastDomain().getSingletonValue());
      checkError(scope.size() == 2, "Require both state and object variables, in that order.");
      checkError(scope[0] == tok->getState() && scope[1] == tok->getObject(),
                 "Require both state and object variables, in that order.");
      if(scope[STATE_VAR]->isSpecified() && scope[STATE_VAR]->getSpecifiedValue() == Token::ACTIVE &&
         scope[OBJECT_VAR]->lastDomain().isSingleton()) {
          m_resource = ResourceId(scope[OBJECT_VAR]->lastDomain().getSingletonValue());
          check_error(m_resource.isValid());
          debugMsg("ResourceTokenRelation:ResourceTokenRelation", "Adding token " << m_token->toString() << " to resource-profile of resource " << m_resource->toString() );

          m_resource->addToProfile(m_token);
      }
    }

    bool ResourceTokenRelation::canIgnore(const ConstrainedVariableId& variable,
                                          int argIndex, const DomainListener::ChangeType& changeType) {
      debugMsg("ResourceTokenRelation:canIgnore", "Received notification of change type " << changeType << " on variable " << variable->toString());
      if(m_token->isDeleted())
        return true;
      ConstrainedVariableId state = m_variables[STATE_VAR];
      ConstrainedVariableId object = m_variables[OBJECT_VAR];

      debugMsg("ResourceTokenRelation:canIgnore", "Current state: " << std::endl <<
               "  " << object->toString() << std::endl <<
               "  " << state->toString());
      //if this is a singleton message
      if(changeType == DomainListener::RESTRICT_TO_SINGLETON ||
         changeType == DomainListener::SET_TO_SINGLETON ||
         variable->lastDomain().isSingleton()) {
        //if the object is singleton and the state has the singleton value ACTIVE
        if(object->lastDomain().isSingleton() &&
           state->isSpecified() && state->getSpecifiedValue() == Token::ACTIVE) {
          m_resource = ResourceId(object->lastDomain().getSingletonValue());
          check_error(m_resource.isValid());
          debugMsg("ResourceTokenRelation:canIgnore", "Adding " << m_token->toString() << " to profile for resource " << m_resource->toString());
          m_resource->addToProfile(m_token);
        }
      }
      else if((changeType == DomainListener::RESET ||
               changeType == DomainListener::RELAXED) && m_resource.isValid()) {
        if(!(object->lastDomain().isSingleton() && object->lastDomain().isMember(m_resource->getKey())) ||
           !(state->isSpecified() && state->getSpecifiedValue() == Token::ACTIVE)) {
            debugMsg("ResourceTokenRelation:canIgnore", "Removing " << m_token->toString() << " from profile for resource " << m_resource->toString());
          m_resource->removeFromProfile(m_token);
          m_resource = ResourceId::noId();
        }
      }
      return true;
    }

    void ResourceTokenRelation::notifyViolated(Resource::ProblemType problem, const InstantId inst)
    {
    	m_violationProblem = problem;
    	m_violationTime = inst->getTime();
    	Constraint::notifyViolated();
    }

    void ResourceTokenRelation::notifyNoLongerViolated()
    {
        m_violationProblem = Resource::NoProblem;
        Constraint::notifyNoLongerViolated();
    }

    std::string ResourceTokenRelation::getViolationExpl() const
    {
        if (m_violationProblem == Resource::NoProblem)
            return "";

    	std::ostringstream os;

    	os << Resource::getProblemString(m_violationProblem)
    	      << " for resource " << m_resource->getName().toString()
    	      << " at instant " << m_violationTime;

    	return os.str();
    }
}
