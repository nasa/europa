#include "Filters.hh"
#include "SolverUtils.hh"
#include "Debug.hh"
#include "PlanDatabase.hh"
#include "RuleVariableListener.hh"
#include "TokenVariable.hh"

#include <set>

namespace EUROPA {
  namespace SOLVERS {

    GuardFilter::GuardFilter(const TiXmlElement& configData) : FlawFilter(configData, true) {
      setExpression(toString() + ":guard");
      debugMsg("GuardFilter:constructor", "Constructing a guard filter.");
    }

    bool GuardFilter::test(const EntityId& entity) {
      if(!ConstrainedVariableId::convertable(entity))
        return false;
      ConstrainedVariableId var = entity;
      std::set<ConstraintId> constraints;
      var->constraints(constraints);
      for(std::set<ConstraintId>::iterator it = constraints.begin(); it != constraints.end(); ++it) {
        ConstraintId constr = *it;
        //indicate a match if this variable is a guard (will be filtered out)
        if(constr->getName() == RuleVariableListener::CONSTRAINT_NAME())
          return true;
      }
      return false;
    }

    NotGuardFilter::NotGuardFilter(const TiXmlElement& configData) : GuardFilter(configData) {
      setExpression(toString() + ":in");
      debugMsg("NotGuardFilter:constructor", "Constructing a not guard filter.");
    }

    bool NotGuardFilter::test(const EntityId& entity) {
      if(!ConstrainedVariableId::convertable(entity))
        return false;
      return !GuardFilter::test(entity);
    }

    InfiniteDynamicFilter::InfiniteDynamicFilter(const TiXmlElement& configData)
      : FlawFilter(configData, true) {
      debugMsg("InfiniteDynamicFilter:constructor", "Constructing an InfiniteDynamicFilter.");
      setExpression(toString() + ":infinite/dynamic");
    }

    bool InfiniteDynamicFilter::test(const EntityId& entity){
      if(!ConstrainedVariableId::convertable(entity))
        return false;

      ConstrainedVariableId var = entity;  
      debugMsg("InfiniteDynamicFilter:test", "Evaluating " << var->toString() << " for dynamic/infinite filter.");
      debugMsg("InfiniteDynamicFilter:test", var->lastDomain() << " isOpen : " << var->lastDomain().isOpen());
      debugMsg("InfiniteDynamicFilter:test", var->lastDomain() << " isInfinite : " << var->lastDomain().isInfinite());
      return (var->lastDomain().isOpen() || var->lastDomain().isInfinite()) && FlawFilter::test(entity);
    }

    SingletonFilter::SingletonFilter(const TiXmlElement& configData)
      : FlawFilter(configData, true) {
      setExpression(toString() + ":singleton");
    }

    bool SingletonFilter::test(const EntityId& entity) {
      if(!ConstrainedVariableId::convertable(entity))
        return false;

      ConstrainedVariableId var = entity;
      debugMsg("SingletonFilter:test", "Evaluating " << var->toString() << " for singleton filter.");

      // Indicate a match if it is not a singleton
      return !var->lastDomain().isSingleton() && FlawFilter::test(entity);
    }

    /** TokenMustBeAssignedFilter **/
    TokenMustBeAssignedFilter::TokenMustBeAssignedFilter(const TiXmlElement& configData)
      : FlawFilter(configData, true) {
      setExpression(toString() + ":tokenMustBeAssigned");
    }

    bool TokenMustBeAssignedFilter::test(const EntityId& entity) {
      checkError(ConstrainedVariableId::convertable(entity), 
                 "Configuration Error. Cannot apply to " << entity->toString());

      ConstrainedVariableId var = entity;

      debugMsg("TokenMustBeAssignedFilter:test", "Evaluating " << var->toString() << " for token assignment filter.");

      if(var->getParent().isNoId() || ObjectId::convertable(var->getParent()))
        return false;

      TokenId parentToken;
      if(RuleInstanceId::convertable(var->getParent()))
        parentToken = (RuleInstanceId(var->getParent())->getToken());
      else
        parentToken = var->getParent();
      
      // Indicate a match if it is not an assigned token
      return !parentToken->isAssigned();
    }
    
    MasterMustBeAssignedFilter::MasterMustBeAssignedFilter(const TiXmlElement& configData)
      : FlawFilter(configData, true) {
      setExpression(toString() + ":masterMustBeAssigned");
    }

    bool MasterMustBeAssignedFilter::test(const EntityId& entity) {
      checkError(TokenId::convertable(entity), 
                 "Configuration error.  Cannot apply to " << entity->toString());
      TokenId tok = entity;
      debugMsg("MasterMustBeAssignedFilter:test", "Evaluation " << tok->toString() << " for master assignment filter.");
      
      if(tok->getMaster().isNoId())
        return false;
      
      return !tok->getMaster()->isAssigned();
    }

    /** HORIZON FILTERING **/
    IntervalIntDomain& HorizonFilter::getHorizon() {
      static IntervalIntDomain sl_instance;
      return sl_instance;
    }

    HorizonFilter::HorizonFilter(const TiXmlElement& configData)
      : FlawFilter(configData, true) {
      static const LabelStr sl_defaultPolicy("PartiallyContained");
      const char* argData = NULL;
      argData = configData.Attribute("policy");
      if(argData != NULL){
        checkError(policies().contains(argData), argData << " is not a valid policy. Choose one of " << policies().toString());
        m_policy = LabelStr(argData);
      }
      else
        m_policy = sl_defaultPolicy;
      setExpression(toString() + ":horizonFilter:" + m_policy.toString());
    }

    bool HorizonFilter::test(const EntityId& entity) {
      static const LabelStr sl_possiblyContained("PossiblyContained");
      static const LabelStr sl_partiallyContained("PartiallyContained");
      static const LabelStr sl_totallyContained("TotallyContained");

      TokenId token;
      if(ConstrainedVariableId::convertable(entity)){
        EntityId parent = ConstrainedVariableId(entity)->getParent();
        if(parent.isNoId() || ObjectId::convertable(parent))
          return false;

        if(RuleInstanceId::convertable(parent))
          token = RuleInstanceId(parent)->getToken();
        else
          token = parent;
      }
      else
        token = entity;

      const IntervalIntDomain& horizon = getHorizon();
      checkError(horizon.isFinite(), "Infinite Horizon not permitted." << horizon.toString());
      const IntervalIntDomain& startTime = token->getStart()->lastDomain();
      const IntervalIntDomain& endTime = token->getEnd()->lastDomain();

      bool withinHorizon = false;

      debugMsg("HorizonFilter:test",
               "Evaluating: " << token->toString() << 
               " Start=" << startTime.toString() << ", End=" << endTime.toString() <<
               ", Policy='" << m_policy.toString() << "', Horizon =" << horizon.toString());

      if(m_policy == sl_possiblyContained)
        withinHorizon = startTime.intersects(horizon) && endTime.intersects(horizon);
      else if (m_policy == sl_partiallyContained)
        withinHorizon = (endTime.getLowerBound() > horizon.getLowerBound() &&
                         startTime.getUpperBound() < horizon.getUpperBound()); //&&
                         //(startTime.intersects(horizon) || endTime.intersects(horizon)));
      else
        withinHorizon = horizon.isMember(startTime.getLowerBound()) && horizon.isMember(endTime.getUpperBound());

      debugMsg("HorizonFilter:test", 
               token->toString() << " is " << (withinHorizon ? " inside " : " outside ") << " the horizon.");

      return !withinHorizon && FlawFilter::test(entity);
    }


    std::string HorizonFilter::toString() const {
      const IntervalIntDomain& horizon = getHorizon();
      std::string expr = FlawFilter::toString();
      expr = expr + " Policy='" + m_policy.toString() + "' Horizon=" + horizon.toString();
      return expr;
    }

    HorizonVariableFilter::HorizonVariableFilter(const TiXmlElement& configData)
      : FlawFilter(configData, true), m_horizonFilter(configData){
      setExpression(toString() + ":variable");
    }

    bool HorizonVariableFilter::test(const EntityId& entity) {
      if(!ConstrainedVariableId::convertable(entity))
        return false;

      ConstrainedVariableId var = entity;

      debugMsg("HorizonVariableFilter:test", "Evaluating " << var->toString() << " for horizon filter.");

      EntityId parent = var->getParent();

      if(parent.isNoId() || ObjectId::convertable(parent))
        return false;

      TokenId token;

      if(RuleInstanceId::convertable(parent))
        token = RuleInstanceId(parent)->getToken();
      else {
        checkError(TokenId::convertable(parent), 
                   "If we have covered our bases, it must be a token, but it ain't:" << var->toString());
        token = (TokenId) parent;
      }

      // Now simply delegate to the filter stored internally.
      return m_horizonFilter.test(token) && FlawFilter::test(entity);
    }

    std::string HorizonVariableFilter::toString() const {
      return m_horizonFilter.toString();
    }
  }
}
