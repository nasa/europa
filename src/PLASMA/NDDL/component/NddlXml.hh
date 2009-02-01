#ifndef _H_NddlXml
#define _H_NddlXml

#include <set>
#include <string>
#include <vector>
#include "DbClientTransactionPlayer.hh"
#include "RulesEngineDefs.hh"
#include "Engine.hh"
#include "ObjectType.hh"

namespace EUROPA {

  class Expr;
  class ObjectFactory;

  class NddlXmlInterpreter : public DbClientTransactionPlayer, public LanguageInterpreter
  {
    public:
      NddlXmlInterpreter(const DbClientId & client, const RuleSchemaId& ruleSchema);
      virtual ~NddlXmlInterpreter();

      virtual std::string interpret(std::istream& input, const std::string& source);

      void addNativeClass(const std::string& className, const std::vector<std::string>& nativeTokens);

    protected:
      virtual void playDeclareClass(const TiXmlElement &);
      virtual void playDefineClass(const TiXmlElement &);
      virtual void playDefineCompat(const TiXmlElement &);
      virtual void playDefineEnumeration(const TiXmlElement &);
      virtual void playDefineType(const TiXmlElement &);

      void buildRuleBody(const char* className, const std::string& predName, const TiXmlElement* element, std::vector<Expr*>& ruleBody,std::map<std::string,std::string>& localVars);
      void defineClassMember (ObjectType* objType, const TiXmlElement* element);
      int  defineConstructor (ObjectType* objType, const TiXmlElement* element);
      void declarePredicate  (ObjectType* objType, const TiXmlElement* element);
      void defineEnum(const SchemaId& schema, const char* className,  const TiXmlElement* element);

      Expr* valueToExpr(const TiXmlElement* element,bool isRule=true);

      bool isClass(const LabelStr& className) const;

      LabelStr predicateInstanceToType(const char* className,
                                       const char* predicateName,
                                       const char* predicateInstance,
                                       std::map<std::string,std::string>& localVars) const;

      LabelStr getObjectVarClass(const LabelStr& className,const LabelStr& var) const;
      LabelStr getTokenVarClass(const LabelStr& className,const LabelStr& predName,const LabelStr& var) const;
      LabelStr checkPredicateType(const LabelStr& type) const;

      RuleSchemaId m_ruleSchema;

      // TODO: move these to schema
      std::set<std::string> m_nativeClasses;
      std::set<std::string> m_nativeTokens;
  };

}

#endif // _H_NddlXml
