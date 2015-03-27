#ifndef H_TokenType
#define H_TokenType

#include "PlanDatabaseDefs.hh"
#include "PSList.hh"
#include <map>
#include <vector>

/**
 * @file Type class for allocation of tokens.
 * @author Conor McGann, March, 2004
 * @modified Mark Roberts, Aug 2009
 * @see DbClient
 */
namespace EUROPA {
class PSDataType;

  /** Version of TokenType for communication with other languages */
  class PSTokenType {
  public:
    enum TokenAttribute {
      ACTION=1,
      PREDICATE=2,
      CONDITION=4,
      EFFECT=8
    };

    virtual ~PSTokenType() {}
    virtual const std::string& getName() const = 0;
    virtual PSList<std::string> getParameterNames() const = 0;
    virtual PSDataType* getParameterType(int index) const = 0;
    virtual PSDataType* getParameterType(const std::string& name) const = 0;

    virtual int getAttributes() const = 0;
    virtual void setAttributes(int attrs) = 0;
    virtual void addAttributes(int attrMask) = 0;
    virtual bool hasAttributes( int attrMask ) const = 0;

    virtual PSList<PSTokenType*> getSubgoalsByAttr( int attrMask ) const = 0;

    virtual std::string toString() const = 0;
    virtual std::string toLongString() const = 0;

  };

  /**
   * @brief Each concrete class must provide an implementation for this.
   */
class TokenType: public PSTokenType {
 public:
  TokenType(const ObjectTypeId ot,const std::string& signature);

  virtual ~TokenType();

  void addArg(const DataTypeId type, const std::string& name);

  const std::string& getPredicateName() const;
  std::string toString() const;
  std::string toLongString() const;

  // From PSTokenType
  const std::string& getName() const { return getPredicateName(); }
  const std::map<std::string,DataTypeId>& getArgs() const;
  const DataTypeId getArgType(const std::string& argName) const;

  const TokenTypeId getId() const;

  const TokenTypeId getParentType() const;

  const ObjectTypeId getObjectType() const;

  /**
   * @brief Return the type for which this type is registered.
   */
  const std::string& getSignature() const;

  /**
   * @brief Create a root token instance
   * @see DbClient::createInstance(const std::string& type, const std::string& name)
   */
  virtual TokenId createInstance(const PlanDatabaseId planDb,
                                 const std::string& name,
                                 bool rejectable = false,
                                 bool isFact = false) const = 0;

  /**
   * @brief Create a slave token
   */
  virtual TokenId createInstance(const TokenId master, const std::string& name,
                                 const std::string& relation) const = 0;

  // From PSTokenType
  virtual PSList<std::string> getParameterNames() const;
  virtual PSDataType* getParameterType(int index) const;
  virtual PSDataType* getParameterType(const std::string& name) const;

  // See PSTokenType::TokenAttribute
  virtual int getAttributes() const;
  virtual void setAttributes(int attrs);
  virtual void addAttributes(int attrMask);
  virtual bool hasAttributes( int attrMask ) const;

  virtual PSList<PSTokenType*> getSubgoalsByAttr( int attrMask ) const;

 protected:

  void addSubgoalByAttr( TokenTypeId type, int attr );

  TokenTypeId m_id;
  ObjectTypeId m_objType;
  std::string m_signature;
  std::string m_predicateName;
  int m_attributes;
  std::map<std::string,DataTypeId> m_args;

  std::map< int, PSList<PSTokenType*> > m_subgoalsByAttr;

};
}

#endif //  H_TokenType
