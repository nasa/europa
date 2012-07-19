#include "HeuristicInstance.hh"
#include "HeuristicsEngine.hh"
#include "Heuristic.hh"
#include "Token.hh"
#include "ConstraintLibrary.hh"
#include "Debug.hh"

#include <sstream>
/**
 * @author Conor McGann
 */

namespace EUROPA {

  class HeuristicInstanceLocalStatic {
  public:
    HeuristicInstanceLocalStatic(){
      static bool sl_boolean = false;
      check_error(sl_boolean == false, "Should only be called once");
      if(sl_boolean == false){
	// Register built in constraint with expected logical propagator name
	REGISTER_SYSTEM_CONSTRAINT(HeuristicInstance::VariableListener, 
				   HeuristicInstance::VariableListener::CONSTRAINT_NAME(),
				   HeuristicInstance::VariableListener::PROPAGATOR_NAME());
        sl_boolean = true;
      }
    }
  };

  HeuristicInstanceLocalStatic s_heuristicInstanceLocalStatic;

  HeuristicInstance::VariableListener::VariableListener(const LabelStr& name,
							const LabelStr& propagatorName,
							const ConstraintEngineId& constraintEngine, 
							const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {}

  HeuristicInstance::VariableListener::VariableListener(const ConstraintEngineId& ce,
							const HeuristicInstanceId& instance,
							const std::vector<ConstrainedVariableId>& scope)
    : Constraint(CONSTRAINT_NAME(), PROPAGATOR_NAME(), ce, scope), m_instance(instance){
    debugMsg("HeuristicInstance:VariableListener:VariableListener", "Allocating " << getKey());
  }

  HeuristicInstance::VariableListener::~VariableListener(){
    debugMsg("HeuristicInstance:VariableListener:~VariableListener", "Deleting " << getKey());
  }

  void HeuristicInstance::VariableListener::handleDiscard(){
    if(m_instance.isId())
      m_instance->resetListener();
    Constraint::handleDiscard();
  }

  void HeuristicInstance::VariableListener::handleExecute() {
    if(m_instance.isNoId())
      return;

    checkError(m_instance.isValid(), m_instance);

    // If a Reset has occurred, and the rule has been fired, we may have to do something right now
    if(m_instance->isExecuted() && !m_instance->test()){
      m_instance->undo();
      return;
    }

    // If not executed, and the specified domain is a singleton and the rule test passses,
    // then execute the rule.
    if(!m_instance->isExecuted() &&  m_instance->test())
      m_instance->execute();
  }

  /**
   * @brief Prevent processing  on dangling nstance
   */
  void HeuristicInstance::VariableListener::resetInstance(){
    m_instance = HeuristicInstanceId::noId();
  }

  HeuristicInstance::HeuristicInstance(const TokenId& token,
				       int target,
				       const HeuristicId& heuristic, 
				       const HeuristicsEngineId& he)
    : Entity(), m_id(this), m_token(token), m_guardSource(token), 
      m_heuristic(heuristic), m_he(he), m_isExecuted(false) {
    m_targets.push_back(target);
    commonInit();
  }

  HeuristicInstance::HeuristicInstance(const TokenId& token, 
				       const std::vector<int>& targets,
				       const TokenId& guardSource,
				       const HeuristicId& heuristic, 
				       const HeuristicsEngineId& he)
    : Entity(), m_id(this), m_token(token), m_targets(targets), m_guardSource(guardSource),
      m_heuristic(heuristic), m_he(he), m_isExecuted(false) {
    commonInit();
  }

  void HeuristicInstance::commonInit(){
    checkError(m_heuristic.isValid(), m_heuristic);
    checkError(validTargets(m_targets), "Targets contain invalid values" << targetsToString(m_targets));

    if(m_heuristic->hasGuards()){
      std::vector<ConstrainedVariableId> scope;
      m_heuristic->makeConstraintScope(m_guardSource, scope);
      m_variableListener = (new VariableListener(m_he->getConstraintEngine(), m_id, scope))->getId();
    }

    if(test())
      execute();
  }

  HeuristicInstance::~HeuristicInstance(){
    debugMsg("HeuristicInstance:~HeuristicInstance", "Deleting " << getKey() << " " << m_id );

    // Note that we do not need to delete the constraint. It will be cleaned up when deleting the token
    if(isExecuted())
      undo();

    if(m_variableListener.isId()){
      ((HeuristicInstance::VariableListener*) m_variableListener)->resetInstance();
      delete (Constraint*) m_variableListener;
    }

    debugMsg("HeuristicInstance:~HeuristicInstance", "Deleted " << getKey() << " " << m_id );
    m_id.remove();
  }


  /**
   * @brief Prevent processing  on dangling nstance
   */
  void HeuristicInstance::resetListener(){
    m_variableListener = ConstraintId::noId();
  }

  const HeuristicInstanceId& HeuristicInstance::getId() const {return m_id;}

  const std::vector<int>& HeuristicInstance::getTargets() const { return m_targets; }

  const TokenId& HeuristicInstance::getToken() const {return m_token;}

  bool HeuristicInstance::isExecuted() const {return m_isExecuted;}

  const Priority& HeuristicInstance::getPriority() const {return m_heuristic->getPriority();}

  double HeuristicInstance::getWeight() const {return m_heuristic->getWeight();}

  const HeuristicId& HeuristicInstance::getHeuristic() const{ return m_heuristic;}					   
  std::string HeuristicInstance::targetsToString(const std::vector<int>& targets) {
    std::stringstream sstr;

    for(std::vector<int>::const_iterator it = targets.begin(); it != targets.end(); ++it){
      int targetKey = *it;
      sstr << std::endl << "        ";
      EntityId targetEntity = Entity::getEntity(targetKey);

      if(targetEntity.isNoId())
	sstr << "BadKey(" << targetKey << ")";
      else {
	checkError(targetEntity.isValid(), targetEntity);
	checkError(ConstrainedVariableId::convertable(targetEntity) || TokenId::convertable(targetEntity), targetEntity);

	if(ConstrainedVariableId::convertable(targetEntity))
	  sstr << ConstrainedVariableId(targetEntity)->toString();
	else
	  sstr << TokenId(targetEntity)->toString();
      }
    }

    return sstr.str();
  }

  bool HeuristicInstance::validTargets(const std::vector<int>& targets) {
    for(std::vector<int>::const_iterator it = targets.begin(); it != targets.end(); ++it){
      int targetKey = *it;
      EntityId targetEntity = Entity::getEntity(targetKey);

      if(!targetEntity.isValid())
	return false;
    }

    return true;
  }

  std::string HeuristicInstance::toString() const {
    std::stringstream sstr;

    sstr << "HeuristicInstance(" << getKey() << ") FOR TARGETS { " << targetsToString(m_targets) << "}" << 
      std::endl << getHeuristic()->toString();

    if(m_variableListener.isId()){
      static const std::string sl_indent("       ");
      sstr << " WITH GUARD VALUES {" << std::endl;
      for(std::vector<ConstrainedVariableId>::const_iterator it = m_variableListener->getScope().begin();
	  it != m_variableListener->getScope().end();
	  ++it){
	ConstrainedVariableId guard = *it;
	sstr << sl_indent << guard->toString() << std::endl;
      }

      sstr << "}";
    }

    return sstr.str();
  }

  bool HeuristicInstance::test() const {
    checkError(m_id.isValid(), m_id);
    checkError(m_variableListener.isNoId() || m_variableListener.isValid(), m_variableListener);

    if(m_variableListener.isId())
      return m_heuristic->test(m_variableListener->getScope());
    else
      return true;
  }

  void HeuristicInstance::execute() {
    checkError(m_id.isValid(), m_id);
    m_isExecuted = true;
    m_he->notifyExecuted(m_id);
  }

  void HeuristicInstance::undo() {
    checkError(m_id.isValid(), m_id);
    m_isExecuted = false;
    m_he->notifyUndone(m_id);
  }

}
