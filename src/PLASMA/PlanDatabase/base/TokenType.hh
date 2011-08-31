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
  };

  /**
   * @brief Each concrete class must provide an implementation for this.
   */
  class TokenType: public PSTokenType {
  public:
    TokenType(const ObjectTypeId& ot,const LabelStr& signature);

    virtual ~TokenType();

    void addArg(const DataTypeId& type, const LabelStr& name);

    const LabelStr& getPredicateName() const;

    // From PSTokenType
    const std::string& getName() const { return getPredicateName().toString(); }
    const std::map<LabelStr,DataTypeId>& getArgs() const;
    const DataTypeId& getArgType(const char* argName) const;

    const TokenTypeId& getId() const;

    const TokenTypeId& getParentType() const;

    const ObjectTypeId& getObjectType() const;

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

    // See TokenType::TokenAttribute
    virtual int getAttributes() const;
    virtual void setAttributes(int attrs);
    virtual void addAttributes(int attrMask);

    // From PSTokenType
	virtual PSList<std::string> getParameterNames() const;
	virtual PSDataType* getParameterType(int index) const;
	virtual PSDataType* getParameterType(const std::string& name) const;

  protected:
    TokenTypeId m_id;
    ObjectTypeId m_objType;
    LabelStr m_signature;
    LabelStr m_predicateName;
    int m_attributes;
    std::map<LabelStr,DataTypeId> m_args;
  };
}

#endif //  H_TokenType
