#include "TokenFactory.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "Utils.hh"

namespace EUROPA {

    TokenTypeMgr::TokenTypeMgr()
        : m_id(this)
    {
    }

    TokenTypeMgr::~TokenTypeMgr() 
    {
        cleanup(m_factories);
        m_id.remove();    
    }
    
    const TokenTypeMgrId& TokenTypeMgr::getId() const {return m_id;}   

    void TokenTypeMgr::registerFactory(const TokenFactoryId& factory) {
      check_error(factory.isValid());

      // Ensure it is not present already
      check_error(m_factories.find(factory) == m_factories.end()) ;

      m_factories.insert(factory);
      m_factoriesByPredicate.insert(std::pair<double, TokenFactoryId>(factory->getSignature().getKey(), factory));
    }

    /**
     * First try a hit for the predicate name as provided. If that does not work, extract the object,
     * and try each parent object untill we get a hit.
     */
    TokenFactoryId TokenTypeMgr::getFactory(const SchemaId& schema, const LabelStr& predicateName){
        check_error(schema->isPredicate(predicateName), predicateName.toString() + " is undefined.");

        // Confirm it is present
        const std::map<double, TokenFactoryId>::const_iterator pos =  
            m_factoriesByPredicate.find(predicateName.getKey());

        if (pos != m_factoriesByPredicate.end()) // We have found what we are looking for
            return(pos->second);

        // If we are here, we have not found it, so build up a list of parents, and try each one. We have to use the schema
        // for this.

        // Call recursively if we have a parent
        if (schema->hasParent(predicateName)) {
            TokenFactoryId factory =  getFactory(schema, schema->getParent(predicateName));

            check_error(factory.isValid(), "No factory found for " + predicateName.toString());

            // Log the mapping in this case, from the original predicate, to make it faster the next time around
            m_factoriesByPredicate.insert(std::pair<double, TokenFactoryId>(predicateName, factory));
            return(factory);
        }

        // If we get here, it is an error
        check_error(ALWAYS_FAILS, "Failed in TokenTypeMgr::getFactory for " + predicateName.toString());

        return TokenFactoryId::noId();
    }

    bool TokenTypeMgr::hasFactory()
    {
        return (!m_factories.empty());
    }

    void TokenTypeMgr::purgeAll()
    {
        cleanup(m_factories);
        m_factoriesByPredicate.clear();
    }


    TokenFactory::TokenFactory(const LabelStr& signature)
        : m_id(this)
        , m_signature(signature)
    {
    }

    TokenFactory::~TokenFactory()
    {
        m_id.remove();
    }

    const TokenFactoryId& TokenFactory::getId() const {return m_id;}

    const LabelStr& TokenFactory::getSignature() const {return m_signature;}
}
