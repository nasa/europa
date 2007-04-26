#ifndef _H_TokenFactory
#define _H_TokenFactory

#include "PlanDatabaseDefs.hh"
#include "LabelStr.hh"
#include <map>
#include <vector>

/**
 * @file Factory class for allocation of tokens.
 * @author Conor McGann, March, 2004
 * @see DbClient
 */
namespace EUROPA {

  class ConcreteTokenFactory;
  typedef Id<ConcreteTokenFactory> ConcreteTokenFactoryId;

  /**
   * @brief Singleton, abstract factory which provides main point for token allocation. It relies
   * on binding to concrete token factories for eacy distinct class.
   * @see ConcreteTokenFactory
   */
  class TokenFactory {
  public:
    /**
     * @brief Should be private, but breaks with Andrews compiler if it is.
     */
    ~TokenFactory();

    /**
     * @brief Create a root token instance
     * @see DbClient::createToken(const LabelStr& type, const LabelStr& name)
     */
    static TokenId createInstance(const PlanDatabaseId& planDb,
                                  const LabelStr& predicateName,
                                  bool rejectable = false,
                                  bool isFact = false);

    /**
     * @brief Create a slave token.
     */
    static TokenId createInstance(const TokenId& master,
				  const LabelStr& predicateName,
				  const LabelStr& relation);

    static void purgeAll();

    /**
     * @brief Test if any factories are registered.
     */
    static bool hasFactory();

  private:
    friend class ConcreteTokenFactory; /*!< Requires access to registerFactory */

    /**
     * @brief Add a factory to provide instantiation of particular concrete types based on a label.
     */
    static void registerFactory(const ConcreteTokenFactoryId& factory);

    /**
     * @brief Obtain the factory based on the predicate name
     */ 
    static ConcreteTokenFactoryId getFactory(const SchemaId& schema, const LabelStr& predicateName);

    static TokenFactory& getInstance();

    TokenFactory();

    std::map<double, ConcreteTokenFactoryId> m_factoriesByPredicate;
    std::set<ConcreteTokenFactoryId> m_factories;
  };

  /**
   * @brief Each concrete class must provide an implementation for this.
   */
  class ConcreteTokenFactory {
  public:
    /**
     * @brief Prefer to be protected, but required to use cleanup utilities
     */
    virtual ~ConcreteTokenFactory();

  protected:
    friend class TokenFactory;

    /**
     * @brief Create a root token instance
     * @see DbClient::createToken(const LabelStr& type, const LabelStr& name)
     */
    virtual TokenId createInstance(const PlanDatabaseId& planDb,
                                   const LabelStr& name,
                                   bool rejectable = false,
                                   bool isFact = false) const = 0;


    /**
     * @brief Create a slave token
     */
    virtual TokenId createInstance(const TokenId& master, const LabelStr& name, const LabelStr& relation) const = 0;

    const ConcreteTokenFactoryId& getId() const;

    /**
     * @brief Return the type for which this factory is registered.
     */
    const LabelStr& getSignature() const;

    ConcreteTokenFactory(const LabelStr& signature);

  private:
    ConcreteTokenFactoryId m_id;
    LabelStr m_signature;
  };

}

#endif
