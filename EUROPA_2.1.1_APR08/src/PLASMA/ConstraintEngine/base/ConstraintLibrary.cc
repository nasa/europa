#include "ConstraintLibrary.hh"
#include "Debug.hh"

/**
 * @file ConstraintLibrary.cc
 */


namespace EUROPA {

  ConstraintId ConstraintLibrary::createConstraint(const LabelStr& name, 
						   const ConstraintEngineId constraintEngine, 
						   const std::vector<ConstrainedVariableId>& scope) {
    check_error(getInstance().getFactory(name).isValid());
    ConstraintId constraint = getInstance().getFactory(name)->createConstraint(constraintEngine, scope);
    return(constraint);
  }

  void ConstraintLibrary::registerFactory(ConstraintFactory* factory) {
    getInstance().registerFactory(factory, factory->getName());
  }

  ConstraintLibrary& ConstraintLibrary::getInstance() {
    static ConstraintLibrary s_lib;
    return(s_lib);
  }

  /**
   * @brief Deallocation will be handled as process is killed. No need to do anything
   */
  ConstraintLibrary::~ConstraintLibrary() {}

  const void ConstraintLibrary::registerFactory(ConstraintFactory* factory, const LabelStr& name) {
    if(isRegistered(name)){
      debugMsg("ConstraintLibrary:registerFactory", "Over-riding prior registration for " << name);
      ConstraintFactoryId oldFactory = getInstance().getFactory(name);
      std::map<double, ConstraintFactoryId>& factories = getInstance().m_constraintsByName;
      factories.erase(name.getKey());
      oldFactory.release();
    }

    check_error(isNotRegistered(name), "Constraint factory for '" + name.toString() + "' should not be registered, and yet it is....");
    m_constraintsByName.insert(std::pair<double, ConstraintFactoryId>(name.getKey(),
								      factory->getId()));
    debugMsg("ConstraintLibrary:registerFactory", "Registered factory " << factory->getName().toString());
  }

  const ConstraintFactoryId& ConstraintLibrary::getFactory(const LabelStr& name) {
    check_error(isRegistered(name), "Factory for constraint '" + name.toString() + "' is not registered.");
    std::map< double, ConstraintFactoryId >::const_iterator it = m_constraintsByName.find(name.getKey());
    return(it->second);
  }

  bool ConstraintLibrary::isRegistered(const LabelStr& name, const bool& warn) {
    std::map<double, ConstraintFactoryId>& factories = getInstance().m_constraintsByName;
    std::map<double, ConstraintFactoryId >::const_iterator it = factories.find(name.getKey());
    if (it == factories.end()) {
      if (warn)
        std::cerr << "\nConstraint <" << name.toString() << "> has not been registered\n";
      return(false);
    }
    return(true);
  }

  bool ConstraintLibrary::isNotRegistered(const LabelStr& name) {
    std::map< double, ConstraintFactoryId >::const_iterator it = m_constraintsByName.find(name.getKey());
    if (it != m_constraintsByName.end()) {
      std::cerr << "\nConstraint <" << name.toString() << "> has already been registered\n";
      return(false);
    }
    return(true);
  }

  void ConstraintLibrary::purgeAll() {
    std::map<double, ConstraintFactoryId >& factories = getInstance().m_constraintsByName;
    std::map<double, ConstraintFactoryId >::iterator it = factories.begin();
    while (it != factories.end()){
      ConstraintFactoryId factory = it->second;
      check_error(factory.isValid());
      debugMsg("ConstraintLibrary:purgeAll", "Removing constraint factory " << factory->getName().toString());
      if(!factory->isSystemDefined()){
	factories.erase(it++);
	factory.release();
      }
      else
	++it;
    }
  }

}
