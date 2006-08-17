#include "TokenFactory.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "Utils.hh"

namespace EUROPA {

  /**
   * First try a hit for the predicate name as provided. If that does not work, extract the object,
   * and try each parent object untill we get a hit.
   */
  ConcreteTokenFactoryId TokenFactory::getFactory(const SchemaId& schema, const LabelStr& predicateName){
    check_error(schema->isPredicate(predicateName), predicateName.toString() + " is undefined.");

    // Confirm it is present
    const std::map<double, ConcreteTokenFactoryId>::const_iterator pos =  
      getInstance().m_factoriesByPredicate.find(predicateName.getKey());

    if (pos != getInstance().m_factoriesByPredicate.end()) // We have found what we are looking for
      return(pos->second);

    // If we are here, we have not found it, so build up a list of parents, and try each one. We have to use the schema
    // for this.

    // Call recursively if we have a parent
    if (schema->hasParent(predicateName)) {
      ConcreteTokenFactoryId factory =  getFactory(schema, schema->getParent(predicateName));

      check_error(factory.isValid(), "No factory found for " + predicateName.toString());

      // Log the mapping in this case, from the original predicate, to make it faster the next time around
      getInstance().m_factoriesByPredicate.insert(std::pair<double, ConcreteTokenFactoryId>(predicateName, factory));
      return(factory);
    }

    // If we get here, it is an error
    check_error(ALWAYS_FAILS, "Failed in TokenFactory::getFactory for " + predicateName.toString());

    return ConcreteTokenFactoryId::noId();
  }

  TokenFactory::TokenFactory() {
  }

  TokenFactory& TokenFactory::getInstance() {
    static TokenFactory sl_instance;
    return(sl_instance);
  }

  TokenFactory::~TokenFactory() {
    cleanup(m_factories);
  }

  void TokenFactory::registerFactory(const ConcreteTokenFactoryId& factory) {
    check_error(factory.isValid());

    // Ensure it is not present already
    check_error(getInstance().m_factories.find(factory) == getInstance().m_factories.end()) ;

    getInstance().m_factories.insert(factory);
    getInstance().m_factoriesByPredicate.insert(std::pair<double, ConcreteTokenFactoryId>(factory->getSignature().getKey(), factory));
  }

  TokenId TokenFactory::createInstance(const PlanDatabaseId& planDb,
                                       const LabelStr& predicateName,
                                       bool rejectable) {
    check_error(planDb.isValid());

    // Obtain the factory 
    ConcreteTokenFactoryId factory = getFactory(planDb->getSchema(), predicateName);

    check_error(factory.isValid());

    TokenId token = factory->createInstance(planDb, predicateName, rejectable);

    check_error(token.isValid());
    return(token);
  }

  TokenId TokenFactory::createInstance(const TokenId& master,
				       const LabelStr& predicateName,
				       const LabelStr& relation) {
    check_error(master.isValid());

    // Obtain the factory 
    ConcreteTokenFactoryId factory = getFactory(master->getPlanDatabase()->getSchema(), predicateName);
    check_error(factory.isValid());

    TokenId token = factory->createInstance(master, predicateName, relation);

    check_error(token.isValid());
    return(token);
  }

  bool TokenFactory::hasFactory(){
    return (!getInstance().m_factories.empty());
  }

  void TokenFactory::purgeAll(){
    cleanup(getInstance().m_factories);
    getInstance().m_factoriesByPredicate.clear();
  }
  ConcreteTokenFactory::ConcreteTokenFactory(const LabelStr& signature)
    : m_id(this), m_signature(signature){
    TokenFactory::registerFactory(m_id);
  }

  ConcreteTokenFactory::~ConcreteTokenFactory(){
    m_id.remove();
  }

  const ConcreteTokenFactoryId& ConcreteTokenFactory::getId() const {return m_id;}

  const LabelStr& ConcreteTokenFactory::getSignature() const {return m_signature;}
}
