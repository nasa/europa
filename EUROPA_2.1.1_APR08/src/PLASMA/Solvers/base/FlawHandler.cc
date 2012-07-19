/**
 * @author Conor McGann
 */

#include "FlawHandler.hh"
#include "FlawManager.hh"
#include "Schema.hh"
#include "Debug.hh"
#include "ConstrainedVariable.hh"
#include "Token.hh"
#include "Object.hh"
#include "RuleInstance.hh"
#include "PlanDatabase.hh"
#include "Utils.hh"
#include "XMLUtils.hh"
#include <math.h>

namespace EUROPA {
  namespace SOLVERS {


    TiXmlElement* FlawHandler::s_element = NULL;

    FlawHandler::FlawHandler(const TiXmlElement& configData):
      MatchingRule(static_cast<const TiXmlElement&>(*(makeConfigData(configData)))),
      m_configData(s_element),
      m_guards(readGuards(configData, false)),
      m_masterGuards(readGuards(configData, true)){

      // Establish the priority
      const char* priorityStr = m_configData->Attribute("priority");
      m_priority = atof(priorityStr);

      checkError(m_priority >= 0 && m_priority < WEIGHT_BASE(), 
                 m_priority << " outside valid range of [0 " << WEIGHT_BASE()-1 << "]");
 
      // Establish the max choices to cut a decision
      const char* maxChoicesStr =  m_configData->Attribute("maxChoices");
      m_maxChoices = static_cast<unsigned int>(atof(maxChoicesStr));

      checkError(m_maxChoices >= 0, maxChoicesStr << " must be positive");

      // The base uses a number that exceeds the max absolute value priority allowed.
      // It also multiplies by a minimum of 1 to ensure that 0 guards are handled as low weights.
      // Note also that we make it 2 so that defaul t compatibility heuristics 
      // can discount the weight without getting into the zero territory
      m_weight = fabs(m_priority - (2+staticFilterCount() + m_guards.size() + m_masterGuards.size()) * WEIGHT_BASE());
    }
    
    /**
     * @brief Process the input element to pull defaults from the parent
     */
    TiXmlElement* FlawHandler::makeConfigData(const TiXmlElement& configData){
      checkError(strcmp(configData.Value(), "FlawHandler") == 0, "Invalid Tag of " << configData.Value());
      checkError(configData.Parent() != NULL, "Must have a parent to get the default properties.");

      s_element = static_cast<TiXmlElement*>(configData.Clone());
      TiXmlElement* parent = (TiXmlElement*) configData.Parent();

      if(s_element->Attribute("priority") == NULL){
        if(parent->Attribute("defaultPriority") != NULL)
          s_element->SetAttribute("priority", parent->Attribute("defaultPriority"));
        else
          s_element->SetAttribute("priority", "99999");
      }

      if(s_element->Attribute("maxChoices") == NULL){
	// Set a default of 0 which implies no cut to be applied
        s_element->SetAttribute("maxChoices", "0");
      }

      return s_element;
    }
  
    FlawHandler::~FlawHandler(){
      delete m_configData;
    }

    Priority FlawHandler::getPriority(const EntityId&) { return m_priority;}
 
    double FlawHandler::getWeight() const {return m_weight;}
    
    unsigned int FlawHandler::getMaxChoices() const {return m_maxChoices;}

    std::string FlawHandler::toString() const{
      std::stringstream sstr;

      sstr << MatchingRule::toString();

      // if we have gaurds
      if(!m_guards.empty() || !m_masterGuards.empty()){
        static const std::string sl_indent("       ");
        sstr << " WHEN  { " << std::endl;

        for(unsigned int index = 0; index < m_guards.size(); index++){
          const GuardEntry& entry = m_guards[index];
          sstr << sl_indent << toString(entry) << std::endl;
        }

        for(unsigned int index = 0; index < m_masterGuards.size(); index++){
          const GuardEntry& entry = m_masterGuards[index];
          sstr << sl_indent << "MASTER." << toString(entry) << std::endl;
        }

        sstr << "}";
      }

      sstr << std::endl;

      sstr << "SET PRIORITY == " << m_priority << std::endl;

      sstr << "SET WEIGHT == " << m_weight << std::endl;
      return sstr.str();
    }

    std::string FlawHandler::toString(const GuardEntry& entry){
      std::stringstream sstr;
      LabelStr varName = entry.first;
      sstr << varName.toString() << " == ";

      if(LabelStr::isString(entry.second))
        sstr << LabelStr(entry.second).toString();
      else
        sstr << entry.second;

      sstr << std::endl;

      return sstr.str();
    }

    double FlawHandler::convertValueIfNecessary(const PlanDatabaseId& db,
                                                const ConstrainedVariableId& guardVar,
                                                const double& testValue){
      // Convert if an object variable. Make it the object id.
      if(Schema::instance()->isObjectType(guardVar->baseDomain().getTypeName())){
        checkError(LabelStr::isString(testValue), "Should be a declared string since it must be the object name.");
        return db->getObject(LabelStr(testValue));
      }

      return testValue;
    }

    const std::vector<GuardEntry>& FlawHandler::readGuards(const TiXmlElement& configData, bool forMaster){
      static std::vector<GuardEntry> sl_guards;
      static const char* sl_guardKey = "Guard";
      static const char* sl_masterKey = "MasterGuard";
      const char* guardKey = (forMaster ? sl_masterKey : sl_guardKey);

      // Clear vector to reuse it. Allows return without copying.
      sl_guards.clear();

      // Populate guard data
      for (TiXmlElement * child = configData.FirstChildElement(); 
           child != NULL; 
           child = child->NextSiblingElement()) {
        if(strcmp(child->Value(), guardKey) == 0){
          checkError(child->Attribute("name") != NULL, "'name' is not provided for " << *child);
          checkError(child->Attribute("value") != NULL, "'value' is not provided for " << *child);
          GuardEntry entry(child->Attribute("name"), readValue(child->Attribute("value")));
          sl_guards.push_back(entry);
        }
      }

      return sl_guards;
    }

    /**
     * @note Treating bools as numbers
     */
    double FlawHandler::readValue(const char* data) {
      double value;

      if(!isNumber(data, value)){
        if(strcmp(data, "true") == 0 || strcmp(data, "TRUE") == 0 || strcmp(data, "True") == 0)
          value = 1;
        else if(strcmp(data, "false") == 0 || strcmp(data, "FALSE") == 0 || strcmp(data, "False") == 0)
          value = 0;
        else {
          LabelStr lblStr(data);
          // Cast to a double
          value = (double) lblStr;
        }
      }

      return value;
    }

    bool FlawHandler::hasGuards() const {return !m_guards.empty() || !m_masterGuards.empty();}

    const std::vector< GuardEntry >& FlawHandler::noGuards(){
      static const std::vector< GuardEntry > sl_noGuards;
      return sl_noGuards;
    }

    bool FlawHandler::makeConstraintScope(const EntityId& entity, std::vector<ConstrainedVariableId>& scope) const {
      checkError(hasGuards(), "Should not call unless there are guards on the handler.");

      TokenId token = getTokenFromEntity(entity);

      // If we cannot get the token, then it is not a token or rule variable and so cannot have guards.
      if(token.isNoId())
        return false;

      if(!m_guards.empty()){
        for (std::vector< GuardEntry >::const_iterator it = m_guards.begin(); it != m_guards.end(); ++it){
          const GuardEntry& entry = *it;
          const LabelStr& guardName = entry.first;
          ConstrainedVariableId guard = token->getVariable(guardName);

          // If it does not have the required variable, return false
          if(guard.isNoId())
            return false;

          scope.push_back(guard);
        }
      }

      if(!m_masterGuards.empty()){
        TokenId master = token->getMaster();

        // If no master and we expect it, retrun false
        if(master.isNoId())
          return false;

        for (std::vector< GuardEntry >::const_iterator it = m_masterGuards.begin(); it != m_masterGuards.end(); ++it){
          const GuardEntry& entry = *it;
          const LabelStr& guardName = entry.first;
          ConstrainedVariableId guard = master->getVariable(guardName);

          // If it does not have the required variable, return false
          if(guard.isNoId())
            return false;

          scope.push_back(guard);
        }
      }

      return true;
    }

    bool FlawHandler::test(const std::vector<ConstrainedVariableId>& scope){
      debugMsg("FlawHandler:test", std::endl << toString());

      checkError(scope.size() == m_guards.size() + m_masterGuards.size(),
                 scope.size() << "!=" << m_guards.size() << " + " << m_masterGuards.size());

      unsigned int scopeIndex = 0;

      for(unsigned int index = 0; index <  m_guards.size(); index++){
        if(!matches(scope[scopeIndex], m_guards[index].second))
          return false;
        scopeIndex++;
      }

      for(unsigned int index = 0; index < m_masterGuards.size(); index++){
        if(!scope[scopeIndex]->lastDomain().isSingleton() || 
           !matches(scope[scopeIndex], m_masterGuards[index].second))
          return false;
        scopeIndex++;
      }

      return true;
    }

    /* Have to convert if it is an object variable */
    bool FlawHandler::matches(const ConstrainedVariableId& guardVar, const double& testValue){
      if(!guardVar->lastDomain().isSingleton())
        return false;

      double convertedValue = convertValueIfNecessary(getPlanDatabase(guardVar), guardVar, testValue);

      condDebugMsg(Schema::instance()->isObjectType(guardVar->baseDomain().getTypeName()),"FlawHandler:matches",
                   "Comparing " << guardVar->lastDomain() << " with " << LabelStr(testValue).toString());
      condDebugMsg(LabelStr::isString(testValue), "FlawHandler:matches", 
                   "Comparing " << guardVar->lastDomain() << " with " << LabelStr(testValue).toString());
      condDebugMsg(!LabelStr::isString(testValue), "FlawHandler:matches", 
                   "Comparing " << guardVar->lastDomain() << " with " << testValue);

      return guardVar->lastDomain().getSingletonValue() == convertedValue;
    }

    const PlanDatabaseId& FlawHandler::getPlanDatabase(const ConstrainedVariableId& tokenVar){
      if(m_db.isNoId()){
        checkError(tokenVar->getParent().isId() && TokenId::convertable(tokenVar->getParent()),
                   tokenVar->toString() << " should have a parent token.");
        m_db = TokenId(tokenVar->getParent())->getPlanDatabase();
      }

      checkError(m_db.isValid(), m_db);
      return m_db;
    }

    TokenId FlawHandler::getTokenFromEntity(const EntityId& entity){
      if(TokenId::convertable(entity))
        return entity;

      checkError(ConstrainedVariableId::convertable(entity), entity->toString());
      ConstrainedVariableId var = entity;
      EntityId parent = var->getParent();
      if(parent.isNoId() || ObjectId::convertable(parent))
        return TokenId::noId();

      if(RuleInstanceId::convertable(parent))
        return RuleInstanceId(parent)->getToken();

      return parent;
    }
    

    /** FlawHandler::VariableListener **/

    FlawHandler::VariableListener::VariableListener(const LabelStr& name,
                                                    const LabelStr& propagatorName,
                                                    const ConstraintEngineId& constraintEngine, 
                                                    const std::vector<ConstrainedVariableId>& variables)
      : Constraint(name, propagatorName, constraintEngine, variables), m_isApplied(false) {}

    FlawHandler::VariableListener::VariableListener(const ConstraintEngineId& ce,
                                                    const EntityId& target,
                                                    const FlawManagerId& flawManager,
                                                    const FlawHandlerId& flawHandler,
                                                    const std::vector<ConstrainedVariableId>& scope)
      : Constraint(CONSTRAINT_NAME(), PROPAGATOR_NAME(), ce, scope),
        m_target(target), m_flawManager(flawManager), m_flawHandler(flawHandler), m_isApplied(false) {}

    void FlawHandler::VariableListener::handleExecute() {
      // If the handler is not set, we can ignore this.
      if(m_flawHandler.isNoId())
        return;

      checkError(m_flawHandler.isValid(), m_flawHandler);

      // If a Reset has occurred, and the rule has been fired, we may have to do something right now
      bool shouldBeApplied = m_flawHandler->test(getScope());
      if(isApplied() && !shouldBeApplied)
        undo();
      else if(!isApplied() && shouldBeApplied)
        apply();
    }

    bool FlawHandler::VariableListener::isApplied() const {return m_isApplied;}

    void FlawHandler::VariableListener::apply(){
      checkError(m_target.isValid(),"Target is invalid: " << m_target);
      m_isApplied = true;
      m_flawManager->notifyActivated(m_target, m_flawHandler);
    }

    void FlawHandler::VariableListener::undo(){
      checkError(m_target.isValid(),"Target is invalid: " << m_target);
      m_isApplied = false;
      m_flawManager->notifyDeactivated(m_target, m_flawHandler);
    }
  }
}
