#ifndef EUROPA_TOKENTYPE_MGR_H

#define EUROPA_TOKENTYPE_MGR_H

#include "PlanDatabaseDefs.hh"
#include "LabelStr.hh"
#include "TokenType.hh"
#include "Utils.hh"

namespace EUROPA {

  /**
   * @brief Singleton, abstract class which provides main point for token allocation. It relies
   * on binding to concrete token types for easy distinct class.
   * @see TokenType
   */
  class TokenTypeMgr {
  public:

  TokenTypeMgr();
  ~TokenTypeMgr();

  const TokenTypeMgrId& getId() const;

  void purgeAll();

  /**
   * @brief Test if any types are registered.
   */
  bool hasType();

  /**
   * @brief Add a factory to provide instantiation of particular concrete types based on a label.
   */
  void registerType(const TokenTypeId& type );

  /**
   * @brief Obtain the factory based on the predicate name
   */
  TokenTypeId getType(const SchemaId& schema, const LabelStr& predicateName);

  protected:
	  TokenTypeMgrId m_id;
	  std::map<edouble, TokenTypeId> m_typesByPredicate;
	  std::set<TokenTypeId> m_types;
  };

} //namespace EUROPA

#endif //EUROPA_TOKENTYPE_MGR_H
