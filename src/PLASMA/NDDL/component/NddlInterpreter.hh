/*
 * NddlInterpreter.h
 *
 *  Created on: Jan 21, 2009
 *      Author: javier
 */

#ifndef NDDLINTERPRETER_H_
#define NDDLINTERPRETER_H_

#include <antlr3.h>
#include <antlr3interfaces.h>
#include "Interpreter.hh"

namespace EUROPA {

class NddlSymbolTable : public EvalContext {
private:
  NddlSymbolTable(const NddlSymbolTable&);
  NddlSymbolTable& operator=(const NddlSymbolTable&);
 public:
  NddlSymbolTable(const EngineId engine);
  NddlSymbolTable(NddlSymbolTable* parent);
  virtual ~NddlSymbolTable();

  NddlSymbolTable* getParentST();

  const PlanDatabaseId getPlanDatabase() const;

  virtual void addLocalVar(const std::string& name,const DataTypeId type);
  virtual void addLocalToken(const std::string& name,const TokenTypeId type);

  virtual DataTypeId getDataType(const std::string& name) const;
  virtual ObjectTypeId getObjectType(const std::string& name) const;
  virtual TokenTypeId getTokenType(const std::string& name) const;

  virtual DataTypeId getTypeForVar(const std::string& name);
  virtual DataTypeId getTypeForVar(const std::string& qualifiedName,std::string& errorMsg);

  virtual TokenTypeId getTypeForToken(const std::string& name);
  virtual TokenTypeId getTypeForToken(const std::string& qualifiedName,std::string& errorMsg);

  virtual MethodId getMethod(const std::string& methodName,Expr* target,const std::vector<Expr*>& args);

  virtual CFunctionId getCFunction(const std::string& name, const std::vector<CExpr*>& args);

  Domain* makeNumericDomainFromLiteral(const std::string& type,const std::string& value);

  void checkConstraint(const std::string& name,const std::vector<Expr*>& args);
  void checkObjectFactory(const std::string& name,const std::vector<Expr*>& args);

  // Error reporting methods
  void reportError(void* treeWalker, const std::string& msg);
  void addError(const std::string& msg);
  std::string getErrors() const;

  // EvalContext methods
  virtual ConstrainedVariableId getVar(const std::string& name);
  virtual TokenId getToken(const std::string& name);
  virtual void* getElement(const std::string& name) const;

  // Enum support methods
  bool isEnumValue(const std::string& value) const;
  const std::string& getEnumForValue(const std::string& value) const;
  Expr* makeEnumRef(const std::string& value) const;

 protected:
  NddlSymbolTable* m_parentST;

  EngineId m_engine;
  std::vector<std::string> m_errors;
  std::map<std::string,DataTypeId> m_localVars;
  std::map<std::string,TokenTypeId> m_localTokens;

  const EngineId engine() const;
  std::vector<std::string>& errors();
  const std::vector<std::string>& errors() const;
};

class NddlClassSymbolTable : public NddlSymbolTable {
 private:
  NddlClassSymbolTable(const NddlClassSymbolTable&);
  NddlClassSymbolTable& operator=(const NddlClassSymbolTable&);
 public:
  NddlClassSymbolTable(NddlSymbolTable* parent, ObjectType* ot);
  virtual ~NddlClassSymbolTable();

  virtual DataTypeId getDataType(const std::string& name) const;
  virtual ObjectTypeId getObjectType(const std::string& name) const;

  virtual DataTypeId getTypeForVar(const std::string& varName);

 protected:
  ObjectType* m_objectType; // Object type being declared
};

class NddlTokenSymbolTable : public NddlSymbolTable
{
public:
    NddlTokenSymbolTable(NddlSymbolTable* parent, const TokenTypeId tt, const ObjectTypeId ot);
    virtual ~NddlTokenSymbolTable();

    virtual DataTypeId getTypeForVar(const std::string& varName);
    virtual TokenTypeId getTokenType(const std::string& name) const;

    virtual TokenTypeId getTypeForToken(const std::string& name);

protected:
    TokenTypeId m_tokenType;
    ObjectTypeId m_objectType;
};

class NddlInterpreter : public LanguageInterpreter
{
public:
    NddlInterpreter(EngineId engine);
    virtual ~NddlInterpreter();
    virtual std::string interpret(std::istream& input, const std::string& source);

    std::string getFilename(const std::string& f);
    bool queryIncludeGuard(const std::string& f);
    void addInclude(const std::string &f);

    std::vector<std::string> getIncludePath();
    void addInputStream(pANTLR3_INPUT_STREAM in);

protected:
    EngineId m_engine;
    std::vector<std::string> m_filesread;
    std::vector<pANTLR3_INPUT_STREAM> m_inputstreams;
};

// An Interpreter that just returns the AST
class NddlToASTInterpreter : public NddlInterpreter
{
public:
    NddlToASTInterpreter(EngineId engine);
    virtual ~NddlToASTInterpreter();
    virtual std::string interpret(std::istream& input, const std::string& source);
};

class PSLanguageException {
 public:
  PSLanguageException(const char *fileName, unsigned int line, int offset,
                      unsigned int length, const char *message);
  friend ostream &operator<<(ostream &, const PSLanguageException &);
  /** Prepare this exception for shipping with AST */
  std::string asString() const;

  const std::string& getFileName() const { return m_fileName; }
  unsigned int getLine() const { return m_line; }
  int getOffset() const { return m_offset; }
  unsigned int getLength() const { return m_length; }
  const std::string& getMessage() const { return m_message; }
 protected:
  std::string m_fileName;
  unsigned int m_line;
  int m_offset;
  unsigned int m_length;
  std::string m_message;
};

class PSLanguageExceptionList
{
public:
  PSLanguageExceptionList(const std::vector<PSLanguageException>& exceptions);
  friend std::ostream &operator<<(std::ostream &, const PSLanguageExceptionList &);
  long getExceptionCount() const { return static_cast<long>(m_exceptions.size()); }
  const PSLanguageException& getException(int index) const {
    return m_exceptions[static_cast<unsigned>(index)];
  }
protected:
	std::vector<PSLanguageException> m_exceptions;
};

void reportParserError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames);
void reportLexerError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 * tokenNames);

pANTLR3_STRING toVerboseStringTree(pANTLR3_BASE_TREE tree);
pANTLR3_INPUT_STREAM getInputStream(std::istream& input, const std::string& source, std::string& strInput);


}


#endif /* NDDLINTERPRETER_H_ */
