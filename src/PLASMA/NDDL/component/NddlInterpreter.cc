/*
 * NddlInterpreter.cpp
 *
 *  Created on: Jan 21, 2009
 *      Author: javier
 */

#include "NddlInterpreter.hh"

#include <sys/stat.h>

#include "NDDL3Lexer.h"
#include "NDDL3Parser.h"
#include "NDDL3Tree.h"

#include "Debug.hh"

namespace EUROPA {

NddlInterpreter::NddlInterpreter(EngineId& engine)
    : m_engine(engine)
{
}

NddlInterpreter::~NddlInterpreter()
{
}

pANTLR3_INPUT_STREAM getInputStream(std::istream& input, const std::string& source)
{
    if (source == "<eval>") {
        // TODO: this is kind of a hack, see if it can be made more robust & efficient
        std::istringstream* is = dynamic_cast<std::istringstream*>(&input);
        std::string strInput = is->str(); // This makes a copy of the original string that could be avoided

        return antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8)strInput.c_str(),(ANTLR3_UINT32)strInput.size(),(pANTLR3_UINT8)source.c_str());
    }
    else {
        return antlr3AsciiFileStreamNew((pANTLR3_UINT8)source.c_str());
    }
}

bool isFile(const std::string& filename)
{
    struct stat my_stat;
    return (stat(filename.c_str(), &my_stat) == 0);
}

std::vector<std::string> NddlInterpreter::getIncludePath()
{
    // TODO: cache this
    std::vector<std::string> includePath;

    // Add overrides from config;
    const std::string& configPathStr = getEngine()->getConfig()->getProperty("nddl.includePath");
    if (configPathStr.size() > 0) {
        LabelStr configPath=configPathStr;
        unsigned int cnt = configPath.countElements(":");
        for (unsigned int i=0;i<cnt;i++)
            includePath.push_back(configPath.getElement(i,":").c_str());
    }

    // Look in current dir first
    includePath.push_back(".");

    // otherwise, look in include path, starting with $EUROPA_HOME, then $PLASMA_HOME
    const char* europaHome = std::getenv("EUROPA_HOME");
    if (europaHome != NULL)
        includePath.push_back(std::string(europaHome)+"/include");
    else // TODO: this should be at least INFO, possibly WARNING
        debugMsg("NddlInterpreter","$EUROPA_HOME is not defined, therefore not added to NddlInterpreter include path");

    const char* plasmaHome = std::getenv("PLASMA_HOME");
    if (plasmaHome != NULL) {
        includePath.push_back(std::string(plasmaHome)+"/src/PLASMA/NDDL/base");
        includePath.push_back(std::string(plasmaHome)+"/src/PLASMA/Resource/component/NDDL");
    }

    return includePath;
}


std::string NddlInterpreter::getFilename(const std::string& f)
{
    std::string fname = f.substr(1,f.size()-2); // remove quotes

    std::vector<std::string> includePath=getIncludePath();

    // Look in current dir first
    includePath.push_back(".");

    // otherwise, look in include path, starting with $EUROPA_HOME, then $PLASMA_HOME
    const char* europaHome = std::getenv("EUROPA_HOME");
    if (europaHome != NULL)
        includePath.push_back(std::string(europaHome)+"/include");
    else // TODO: this should be at least INFO, possibly WARNING
        debugMsg("NddlInterpreter","$EUROPA_HOME is not defined, therefore not added to NddlInterpreter include path");

    const char* plasmaHome = std::getenv("PLASMA_HOME");
    if (plasmaHome != NULL) {
        includePath.push_back(std::string(plasmaHome)+"/src/PLASMA/NDDL/base");
        includePath.push_back(std::string(plasmaHome)+"/src/PLASMA/Resource/component/NDDL");
    }

    // TODO: dump includePath to log
    for (unsigned int i=0; i<includePath.size();i++) {
        // TODO: this may not be portable to all OSs
        std::string fullName = includePath[i]+"/"+fname;
        if (isFile(fullName)) {
            debugMsg("NddlInterpreter","Found:" << fullName);
            return fullName;
        }
        else
            debugMsg("NddlInterpreter",fullName << " doesn't exist");
    }

    return "";
}

std::string NddlInterpreter::interpret(std::istream& ins, const std::string& source)
{
    pANTLR3_INPUT_STREAM input = getInputStream(ins,source);

    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    lexer->parserObj = this;
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);

    // Build he AST
    NDDL3Parser_nddl_return result = parser->nddl(parser);
    if (parser->pParser->rec->state->errorCount > 0) {
        std::ostringstream os;
        os << "The parser returned " << parser->pParser->rec->state->errorCount << " errors";
        // TODO: get error messages as well!
        parser->free(parser);
        tstream->free(tstream);
        lexer->free(lexer);
        input->close(input);
        debugMsg("NddlInterpreter:interpret",os.str());
        return os.str();
    }
    else {
        debugMsg("NddlInterpreter:interpret","NDDL AST:\n" << result.tree->toStringTree(result.tree)->chars);
    }

    // Walk the AST to create nddl expr to evaluate
    pANTLR3_COMMON_TREE_NODE_STREAM nodeStream = antlr3CommonTreeNodeStreamNewTree(result.tree, ANTLR3_SIZE_HINT);
    pNDDL3Tree treeParser = NDDL3TreeNew(nodeStream);

    NddlSymbolTable symbolTable(m_engine);
    treeParser->SymbolTable = &symbolTable;

    try {
        treeParser->nddl(treeParser);
    }
    catch (const std::string& error) {
        debugMsg("NddlInterpreter:interpret","nddl parser halted on error:" << symbolTable.getErrors());
    }

    // Free everything
    treeParser->free(treeParser);
    nodeStream->free(nodeStream);

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);

    return symbolTable.getErrors();
}


NddlSymbolTable::NddlSymbolTable(const EngineId& engine)
    : EvalContext(NULL)
    , m_engine(engine)
    , m_currentObjectType(NULL)
{
}

NddlSymbolTable::~NddlSymbolTable()
{
}

void NddlSymbolTable::addError(const std::string& msg)
{
    m_errors.push_back(msg);
    debugMsg("NddlInterpreter:SymbolTable","SEMANTIC ERROR reported : " << msg);
}

std::string NddlSymbolTable::getErrors() const
{
    std::ostringstream os;

    for (unsigned int i=0; i<m_errors.size(); i++)
        os << m_errors[i] << std::endl;

    return os.str();
}

void* NddlSymbolTable::getElement(const char* name) const
{
    EngineComponent* component = m_engine->getComponent(name);

    if (component != NULL)
        return component;

    return EvalContext::getElement(name);
}

const PlanDatabaseId& NddlSymbolTable::getPlanDatabase() const
{
    return ((PlanDatabase*)getElement("PlanDatabase"))->getId();
}

void NddlSymbolTable::setCurrentObjectType(ObjectType* ot)
{
    m_currentObjectType = ot;
}


DataTypeId NddlSymbolTable::getVarType(const char* name) const
{
    CESchemaId ces = ((CESchema*)getElement("CESchema"))->getId();

    if (ces->isDataType(name))
        return ces->getDataType(name);
    else {
        if (m_currentObjectType!=NULL && m_currentObjectType->getName().toString()==name)
            return m_currentObjectType->getVarType();

        debugMsg("NddlInterpreter:SymbolTable","Unknown type " << name);
        return DataTypeId::noId();
    }
}

AbstractDomain* NddlSymbolTable::makeNumericDomainFromLiteral(const std::string& type,
                                                              const std::string& value)
{
    // TODO: only one copy should be kept for each literal, domains should be marked as constant
    AbstractDomain* retval = getPlanDatabase()->getSchema()->getCESchema()->baseDomain(type.c_str()).copy();
    double v = getPlanDatabase()->getClient()->createValue(type.c_str(), value);
    retval->set(v);

    return retval;
}


ConstrainedVariableId NddlSymbolTable::getVar(const char* name)
{
    if (getPlanDatabase()->isGlobalVariable(name))
        return getPlanDatabase()->getGlobalVariable(name);
    else
        return ConstrainedVariableId::noId();
}


void NddlSymbolTable::addEnumValues(const char* enumName,const std::vector<std::string>& values)
{
    std::string type(enumName);

    for (unsigned int i=0;i<values.size();i++)
        m_enumValues[values[i]]=type;
}

bool NddlSymbolTable::isEnumValue(const char* value) const
{
    return m_enumValues.find(value) != m_enumValues.end();
}

Expr* NddlSymbolTable::makeEnumRef(const char* value) const
{
    std::string enumType = m_enumValues.find(value)->second;
    EnumeratedDomain* ad = dynamic_cast<EnumeratedDomain*>(
            getPlanDatabase()->getSchema()->getCESchema()->baseDomain(enumType.c_str()).copy());
    double v = LabelStr(value);
    ad->set(v);

    return new ExprConstant(getPlanDatabase()->getClient(),enumType.c_str(),ad);
}


NddlToASTInterpreter::NddlToASTInterpreter(EngineId& engine)
    : NddlInterpreter(engine)
{
}

NddlToASTInterpreter::~NddlToASTInterpreter()
{
}

std::string NddlToASTInterpreter::interpret(std::istream& ins, const std::string& source)
{
    pANTLR3_INPUT_STREAM input = getInputStream(ins,source);

    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    lexer->parserObj = this;
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);

    // Build he AST
    std::string retval;
    NDDL3Parser_nddl_return result = parser->nddl(parser);
    if (parser->pParser->rec->state->errorCount > 0) {
        std::ostringstream os;
        os << "The parser returned " << parser->pParser->rec->state->errorCount << " errors";
        // TODO: get error messages as well!
        debugMsg("NddlToASTInterpreter:interpret",os.str());
        retval = os.str();
    }
    else {
    	retval = (char*)(result.tree->toStringTree(result.tree)->chars);
        debugMsg("NddlToASTInterpreter:interpret","NDDL AST:\n" << retval);
    }

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);

    return retval;
}


}
