#include "ResourceTokenRelation.hh"
#include "Constraint.hh"
#include "Resource.hh"
#include "Token.hh"
#include "ConstrainedVariable.hh"
#include "TokenVariable.hh"
#include "Instant.hh"
#include "Propagator.hh"

namespace EUROPA {

ResourceTokenRelation::ResourceTokenRelation(const ConstraintEngineId constraintEngine,
                                             const std::vector<ConstrainedVariableId>& scope,
                                             const TokenId tok)
    : Constraint(CONSTRAINT_NAME(), PROPAGATOR_NAME(), constraintEngine, scope)
    , m_token(tok)
    , m_resource()
    , m_violationTime()
    , m_violationProblem(Resource::NoProblem)
{
		checkError(scope.size() == 2, "Require both state and object variables, in that order.");
		checkError(scope[0] == tok->getState() && scope[1] == tok->getObject(), "Require both state and object variables, in that order.");

		safeConnect();
	}

	ResourceTokenRelation::~ResourceTokenRelation()
	{
		discard(false);
	}

	void ResourceTokenRelation::handleDiscard() {
		if(!Entity::isPurging()) {
			check_error(m_token.isValid());
			if(m_resource.isId())
				disconnect();
		}
		Constraint::handleDiscard();
	}

	void ResourceTokenRelation::safeConnect()
	{
		if(m_token->isDeleted())
			return;

		ConstrainedVariableId state = m_variables[STATE_VAR];
		ConstrainedVariableId object = m_variables[OBJECT_VAR];

		if(object->lastDomain().isSingleton() &&
				state->isSpecified() && state->getSpecifiedValue() == Token::ACTIVE &&
				m_resource.isNoId()) {
			connect();
		}
	}

	void ResourceTokenRelation::connect()
	{
		check_error(m_resource.isNoId());
		m_resource = Entity::getTypedEntity<Resource>(m_variables[OBJECT_VAR]->lastDomain().getSingletonValue());
		check_error(m_resource.isValid());
		debugMsg("ResourceTokenRelation:connect", "Adding " << m_token->toString() << " to profile for resource " << m_resource->toString());
		m_resource->addToProfile(m_token);

	}

	void ResourceTokenRelation::safeDisconnect()
	{
		if(m_token->isDeleted() || m_resource.isNoId())
			return;

		ConstrainedVariableId state = m_variables[STATE_VAR];
		ConstrainedVariableId object = m_variables[OBJECT_VAR];

		if(!(object->lastDomain().isSingleton() && object->lastDomain().isMember(m_resource->getKey())) ||
				!(state->isSpecified() && state->getSpecifiedValue() == Token::ACTIVE)) {
			disconnect();
		}
	}

	void ResourceTokenRelation::disconnect()
	{
		check_error(m_resource.isId());
		debugMsg("ResourceTokenRelation:disconnect", "Removing " << m_token->toString() << " from profile for resource " << m_resource->toString());
		m_resource->removeFromProfile(m_token);
		m_resource = ResourceId::noId();
	}

bool ResourceTokenRelation::canIgnore(const ConstrainedVariableId variable,
                                      unsigned int ,
                                      const DomainListener::ChangeType& changeType) {
  debugMsg("ResourceTokenRelation:canIgnore",
           m_token->toString() << " Received notification of change type " << changeType << " on variable " <<
           variable->toString());

  ConstrainedVariableId state = m_variables[STATE_VAR];
  ConstrainedVariableId object = m_variables[OBJECT_VAR];

  debugMsg("ResourceTokenRelation:canIgnore", "Current state: " << std::endl <<
           "  " << object->toString() << std::endl <<
           "  " << state->toString());

  // if this is a singleton message
  if(changeType == DomainListener::RESTRICT_TO_SINGLETON ||
     changeType == DomainListener::SET_TO_SINGLETON ||
     variable->lastDomain().isSingleton()) {
    safeConnect();
  }
  else if (changeType == DomainListener::RESET || changeType == DomainListener::RELAXED) {
    safeDisconnect();
  }

  return true;
}

    // TODO: these should be handleDeactivate and handleActivate, but need to fix ViolationManager first
    void ResourceTokenRelation::enable()
    {
    	undoDeactivation();
    	safeConnect();
    }

    void ResourceTokenRelation::disable()
    {
    	deactivate();
    	if (m_resource.isId())
    		disconnect();
    }

void ResourceTokenRelation::notifyViolated() {Constraint::notifyViolated();}

void ResourceTokenRelation::notifyViolated(Resource::ProblemType problem,
                                           const InstantId inst) {
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
		<< " for resource " << m_resource->getName()
		<< " at instant " << m_violationTime;

		return os.str();
	}

	TokenId ResourceTokenRelation::getToken() const { return m_token; }

	ResourceId ResourceTokenRelation::getResource() const { return m_resource; }

	std::pair<eint,Resource::ProblemType> ResourceTokenRelation::getViolationInfo() const
	{
		return std::pair<eint,Resource::ProblemType>(m_violationTime,m_violationProblem);
	}

const std::vector<ConstrainedVariableId>&
ResourceTokenRelation::getModifiedVariables(const ConstrainedVariableId) const {
  static std::vector<ConstrainedVariableId> s_emptyScope;
  return s_emptyScope;
}

const std::vector<ConstrainedVariableId>&
ResourceTokenRelation::getModifiedVariables() const {
  static std::vector<ConstrainedVariableId> s_emptyScope;
  return s_emptyScope;
}
}
