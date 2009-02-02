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

  class TokenTypeMgr;
  typedef Id<TokenTypeMgr> TokenTypeMgrId;

  class TokenFactory;
  typedef Id<TokenFactory> TokenFactoryId;

  /**
   * @brief Singleton, abstract factory which provides main point for token allocation. It relies
   * on binding to concrete token factories for eacy distinct class.
   * @see TokenFactory
   */
  class TokenTypeMgr {
  public:
      
    TokenTypeMgr();  
    ~TokenTypeMgr();

    const TokenTypeMgrId& getId() const;   
    
    void purgeAll();

    /**
     * @brief Test if any factories are registered.
     */
    bool hasFactory();

     /**
     * @brief Add a factory to provide instantiation of particular concrete types based on a label.
     */
    void registerFactory(const TokenFactoryId& factory);

    /**
     * @brief Obtain the factory based on the predicate name
     */ 
    TokenFactoryId getFactory(const SchemaId& schema, const LabelStr& predicateName);

  protected:
    TokenTypeMgrId m_id;
    std::map<edouble, TokenFactoryId> m_factoriesByPredicate;
    std::set<TokenFactoryId> m_factories;
  };

  /**
   * @brief Each concrete class must provide an implementation for this.
   */
  class TokenFactory {
  public:
    TokenFactory(const LabelStr& signature);

    virtual ~TokenFactory();

    const TokenFactoryId& getId() const;

    /**
     * @brief Return the type for which this factory is registered.
     */
    const LabelStr& getSignature() const;
    
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

  protected:
    TokenFactoryId m_id;
    LabelStr m_signature;
  };
}

#endif
