#ifndef _H_TokenType
#define _H_TokenType

#include "PlanDatabaseDefs.hh"
#include "LabelStr.hh"
//#include "TokenTypeMgr.hh"
#include <map>
#include <vector>

/**
 * @file Type class for allocation of tokens.
 * @author Conor McGann, March, 2004
 * @modified Mark Roberts, Aug 2009
 * @see DbClient
 */
namespace EUROPA {

  /**
   * @brief Each concrete class must provide an implementation for this.
   */
  class TokenType {
  public:
    TokenType(const ObjectTypeId& ot,const LabelStr& signature);

    virtual ~TokenType();

    void addArg(const DataTypeId& type, const LabelStr& name);

    const LabelStr& getPredicateName() const;
    const std::map<LabelStr,DataTypeId>& getArgs() const;
    const DataTypeId& getArgType(const char* argName) const;

    const TokenTypeId& getId() const;

    const TokenTypeId& getParentType() const;

    /**
     * @brief Return the type for which this type is registered.
     */
    const LabelStr& getSignature() const;

    /**
     * @brief Create a root token instance
     * @see DbClient::createInstance(const LabelStr& type, const LabelStr& name)
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
    TokenTypeId m_id;
    ObjectTypeId m_objType;
    LabelStr m_signature;
    LabelStr m_predicateName;
    std::map<LabelStr,DataTypeId> m_args;
  };
}

#endif //  H_TokenType
