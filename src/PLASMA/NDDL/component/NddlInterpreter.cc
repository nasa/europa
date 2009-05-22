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
#include "antlr3exception.h"

#include "Debug.hh"
#include "Utils.hh"

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

    // TODO: dump includePath to log
    return includePath;
}


std::string NddlInterpreter::getFilename(const std::string& f)
{
    std::string fname = f.substr(1,f.size()-2); // remove quotes

    std::vector<std::string> includePath=getIncludePath();

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
    catch (const Error& internalError) {
        symbolTable.reportError(treeParser,internalError.getMsg());
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


NddlSymbolTable::NddlSymbolTable(NddlSymbolTable* parent)
    : EvalContext(parent)
    , m_parentST(parent)
{
}

NddlSymbolTable::NddlSymbolTable(const EngineId& engine)
    : EvalContext(NULL)
    , m_parentST(NULL)
    , m_engine(engine)
{
}

NddlSymbolTable::~NddlSymbolTable()
{
}

NddlSymbolTable* NddlSymbolTable::getParentST() { return m_parentST; }

const EngineId& NddlSymbolTable::engine() const { return (m_parentST==NULL ? m_engine : m_parentST->engine()); }
std::vector<std::string>& NddlSymbolTable::errors() { return (m_parentST==NULL ? m_errors : m_parentST->errors()); }
const std::vector<std::string>& NddlSymbolTable::errors() const { return (m_parentST==NULL ? m_errors : m_parentST->errors()); }
std::map<std::string,std::string>& NddlSymbolTable::enumValues() { return (m_parentST==NULL ? m_enumValues : m_parentST->enumValues()); }
const std::map<std::string,std::string>& NddlSymbolTable::enumValues() const { return (m_parentST==NULL ? m_enumValues : m_parentST->enumValues()); }

void NddlSymbolTable::addError(const std::string& msg)
{
    errors().push_back(msg);
    debugMsg("NddlInterpreter:SymbolTable","SEMANTIC ERROR reported : " << msg);
}

std::string NddlSymbolTable::getErrors() const
{
    std::ostringstream os;

    for (unsigned int i=0; i<errors().size(); i++)
        os << errors()[i] << std::endl;

    return os.str();
}

void* NddlSymbolTable::getElement(const char* name) const
{
    EngineComponent* component = engine()->getComponent(name);

    if (component != NULL)
        return component;

    return EvalContext::getElement(name);
}

const PlanDatabaseId& NddlSymbolTable::getPlanDatabase() const
{
    return ((PlanDatabase*)getElement("PlanDatabase"))->getId();
}

DataTypeId NddlSymbolTable::getDataType(const char* name) const
{
    CESchemaId ces = ((CESchema*)getElement("CESchema"))->getId();

    if (ces->isDataType(name))
        return ces->getDataType(name);

    debugMsg("NddlInterpreter:SymbolTable","Unknown type " << name);
    return DataTypeId::noId();
}

ObjectTypeId NddlSymbolTable::getObjectType(const char* name) const
{
    SchemaId s = ((Schema*)getElement("Schema"))->getId();

    return s->getObjectType(name);
}

TokenFactoryId NddlSymbolTable::getTokenType(const char* name) const
{
    SchemaId s = ((Schema*)getElement("Schema"))->getId();

    return s->getTokenFactory(name);
}


void NddlSymbolTable::addLocalVar(const char* name,const DataTypeId& type)
{
    m_localVars[name]=type;
}

DataTypeId NddlSymbolTable::getTypeForVar(const char* varName)
{
    if (m_localVars.find(varName) != m_localVars.end())
        return m_localVars[varName];

    if (getPlanDatabase()->isGlobalVariable(varName))
        return getPlanDatabase()->getGlobalVariable(varName)->getDataType();

    if (m_parentST != NULL)
        return m_parentST->getTypeForVar(varName);

    return DataTypeId::noId();
}

DataTypeId NddlSymbolTable::getTypeForVar(const char* varName,std::string& errorMsg)
{
    std::string parentName;
    std::vector<std::string> vars;
    std::string fullName(varName);
    tokenize(fullName,vars,".");

    if (vars.size() > 1) {
      parentName = vars[0];
      fullName = fullName.substr(parentName.length()+1);
      vars.erase(vars.begin());
      debugMsg("NddlSymbolTable::getTypeForVar","Split " << varName << " into " << parentName << " and " << varName);
    }
    else {
      parentName = "";
      debugMsg("NddlSymbolTable::getTypeForVar","Didn't split " << varName);
    }

    DataTypeId dt;

    if (parentName == "") {
        dt = getTypeForVar(varName);
        if (dt.isNoId()) {
            // We failed to find a variable, let's try to find a token
            TokenFactoryId tf = getTypeForToken(parentName.c_str());

            if (tf.isId())
                dt =  FloatDT::instance(); // TODO : return data type for state var?
            else
                errorMsg = fullName + " is not defined";
        }

        return dt;
    }
    else {
        TokenFactoryId tf = getTypeForToken(parentName.c_str());
        if (tf.isNoId()) {
            dt = getTypeForVar(parentName.c_str(),errorMsg);
            if (dt.isNoId())
                return dt;
        }

        std::string curVarName=vars[0];
        unsigned int idx = 0;

        if (tf.isId()) {
            dt = tf->getArgType(curVarName.c_str());
            if (dt.isNoId()) {
                errorMsg = tf->getPredicateName().toString() + " doesn't have a parameter called "+curVarName;
                return dt;
            }
            idx++;
        }

        for (;idx<vars.size();idx++) {
            std::string curVarType = dt->getName().toString();

            ObjectTypeId ot = getObjectType(dt->getName().c_str());
            if (ot.isNoId()) {
                errorMsg = curVarName+"("+curVarType+") doesn't have a member called "+vars[idx];
                return DataTypeId::noId();
            }

            dt = ot->getMemberType(vars[idx].c_str());
            if (ot.isNoId()) {
                errorMsg = curVarName+"("+curVarType+") doesn't have a member called "+vars[idx];
                return DataTypeId::noId();
            }

            curVarName = vars[idx];
        }
    }

    return dt;

}

TokenFactoryId NddlSymbolTable::getTypeForToken(const char* name)
{

    return TokenFactoryId::noId();
}


AbstractDomain* NddlSymbolTable::makeNumericDomainFromLiteral(const std::string& type,
                                                              const std::string& value)
{
    // TODO: only one copy should be kept for each literal, domains should be marked as constant
    CESchemaId ces = ((CESchema*)getElement("CESchema"))->getId();
    AbstractDomain* retval = ces->baseDomain(type.c_str()).copy();
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
        enumValues()[values[i]]=type;
}

bool NddlSymbolTable::isEnumValue(const char* value) const
{
    return enumValues().find(value) != enumValues().end();
}

Expr* NddlSymbolTable::makeEnumRef(const char* value) const
{
    std::string enumType = enumValues().find(value)->second;
    EnumeratedDomain* ad = dynamic_cast<EnumeratedDomain*>(
            getPlanDatabase()->getSchema()->getCESchema()->baseDomain(enumType.c_str()).copy());
    double v = LabelStr(value);
    ad->set(v);

    return new ExprConstant(getPlanDatabase()->getClient(),enumType.c_str(),ad);
}

std::string getErrorLocation(pNDDL3Tree treeWalker);
void NddlSymbolTable::reportError(void* tw, const std::string& msg)
{
    pNDDL3Tree treeWalker = (pNDDL3Tree) tw;
    addError(getErrorLocation(treeWalker) + "\n" + msg);
}


NddlClassSymbolTable::NddlClassSymbolTable(NddlSymbolTable* parent, ObjectType* ot)
    : NddlSymbolTable(parent)
    , m_objectType(ot)
{
}

NddlClassSymbolTable::~NddlClassSymbolTable()
{
}

DataTypeId NddlClassSymbolTable::getDataType(const char* name) const
{
    if (m_objectType->getName().toString()==name)
        return m_objectType->getVarType();

    return NddlSymbolTable::getDataType(name);
}

ObjectTypeId NddlClassSymbolTable::getObjectType(const char* name) const
{
    if (m_objectType->getName().toString()==name)
        return m_objectType->getId();

    return NddlSymbolTable::getObjectType(name);
}

DataTypeId NddlClassSymbolTable::getTypeForVar(const char* varName)
{
    DataTypeId dt = m_objectType->getMemberType(varName);
    if (dt.isId())
        return dt;

    return NddlSymbolTable::getTypeForVar(varName);
}


NddlTokenSymbolTable::NddlTokenSymbolTable(NddlSymbolTable* parent, const TokenFactoryId& tf)
    : NddlSymbolTable(parent)
    , m_tokenType(tf)
{
}

NddlTokenSymbolTable::~NddlTokenSymbolTable()
{
}

DataTypeId NddlTokenSymbolTable::getTypeForVar(const char* varName)
{
    DataTypeId dt = m_tokenType->getArgType(varName);
    if (dt.isId())
        return dt;

    return NddlSymbolTable::getTypeForVar(varName);
}

NddlToASTInterpreter::NddlToASTInterpreter(EngineId& engine)
    : NddlInterpreter(engine)
{
}

NddlToASTInterpreter::~NddlToASTInterpreter()
{
}

pANTLR3_STRING toVerboseString(pANTLR3_BASE_TREE tree);
pANTLR3_STRING toVerboseStringTree(pANTLR3_BASE_TREE tree);

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
        // Calling static helper functions to get a verbose version of AST.
        // Old non-verbose code: result.tree->toStringTree(result.tree)->chars;
        retval = (char*)(toVerboseStringTree(result.tree)->chars);
        debugMsg("NddlToASTInterpreter:interpret","NDDL AST:\n" << retval);
    }

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);

    return retval;
}

// Antlr functions
std::string getErrorLocation(pNDDL3Tree treeWalker)
{
    std::ostringstream os;

    // get location. see displayRecognitionError() in antlr3baserecognizer.c
    pANTLR3_BASE_RECOGNIZER rec = treeWalker->pTreeParser->rec;
    if (rec->state->exception == NULL) {
        antlr3RecognitionExceptionNew(rec);
        //rec->state->exception->type = ANTLR3_RECOGNITION_EXCEPTION;
        //rec->state->exception->message = (char*)msg.c_str();
    }
    //rec->reportError(rec);

    pANTLR3_EXCEPTION ex = rec->state->exception;
    if  (ex->streamName == NULL) {
        if  (((pANTLR3_COMMON_TOKEN)(ex->token))->type == ANTLR3_TOKEN_EOF)
            os << "-end of input-(";
        else
            os << "-unknown source-(";
    }
    else {
        pANTLR3_STRING ftext = ex->streamName->to8(ex->streamName);
        os << ftext->chars << "(";
    }

    // Next comes the line number
    os << "line:" << rec->state->exception->line << ")";

    pANTLR3_BASE_TREE theBaseTree = (pANTLR3_BASE_TREE)(rec->state->exception->token);
    pANTLR3_STRING ttext       = theBaseTree->toStringTree(theBaseTree);

    os << ", at offset " << theBaseTree->getCharPositionInLine(theBaseTree);
    os << ", near " <<  ttext->chars;

    return os.str();
}

/**
 *  Create a verbose string for a single tree node:
 *  "text":token-type:"file":line:offset-in-line
 */
pANTLR3_STRING toVerboseString(pANTLR3_BASE_TREE tree)
{
	if  (tree->isNilNode(tree) == ANTLR3_TRUE)
	{
		pANTLR3_STRING  nilNode;
		nilNode	= tree->strFactory->newPtr(tree->strFactory, (pANTLR3_UINT8)"nil", 3);
		return nilNode;
	}

	pANTLR3_COMMON_TOKEN ptoken = tree->getToken(tree);
	pANTLR3_INPUT_STREAM pstream = ptoken->input;
	pANTLR3_STRING  string = tree->strFactory->newRaw(tree->strFactory);

	// "text":token-type:"file":line:offset-in-line
	string->append8 (string, "\""); // "text"
	string->appendS	(string, ((pANTLR3_COMMON_TREE)(tree->super))->token->
			getText(((pANTLR3_COMMON_TREE)(tree->super))->token));
	string->append8	(string, "\"");
	string->append8 (string, ":");
	string->addi (string, tree->getType(tree)); // type

	// if no file (e.g., root NDDL node), last three items are dropped
	if (pstream) {
		string->append8 (string, ":");
		string->append8	(string, "\""); // "file", full path
		string->appendS(string, pstream->fileName);
		string->append8	(string, "\"");
		string->append8 (string, ":");
		string->addi (string, tree->getLine(tree)); // line
		string->append8 (string, ":");
		string->addi (string, ptoken->charPosition); // offset in line
	}

	return string;
}

/** Create a verbose string for the whole tree */
pANTLR3_STRING toVerboseStringTree(pANTLR3_BASE_TREE tree)
{
	pANTLR3_STRING  string;
	ANTLR3_UINT32   i;
	ANTLR3_UINT32   n;
	pANTLR3_BASE_TREE   t;

	if	(tree->children == NULL || tree->children->size(tree->children) == 0)
		return	toVerboseString(tree);

	/* Need a new string with nothing at all in it.
	*/
	string	= tree->strFactory->newRaw(tree->strFactory);

	if	(tree->isNilNode(tree) == ANTLR3_FALSE)
	{
		string->append8	(string, "(");
		string->appendS	(string, toVerboseString(tree));
		string->append8	(string, " ");
	}
	if	(tree->children != NULL)
	{
		n = tree->children->size(tree->children);

		for	(i = 0; i < n; i++)
		{
			t   = (pANTLR3_BASE_TREE) tree->children->get(tree->children, i);

			if  (i > 0)
			{
				string->append8(string, " ");
			}
			string->appendS(string, toVerboseStringTree(t));
		}
	}
	if	(tree->isNilNode(tree) == ANTLR3_FALSE)
	{
		string->append8(string,")");
	}

	return  string;
}

}
