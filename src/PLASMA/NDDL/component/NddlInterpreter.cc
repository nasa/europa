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



NddlFunction::NddlFunction(const char* name, const char* constraint, const char* returnType, unsigned int argumentCount)
{
    m_name = name;
    m_constraint = constraint;
    m_returnType = returnType;
    m_argumentCount = argumentCount;
}

NddlFunction::NddlFunction(NddlFunction &copy)
{
    m_name = copy.getName();
    m_constraint = copy.getConstraint();
    m_returnType = copy.getReturnType();
    m_argumentCount = copy.getArgumentCount();
}

NddlFunction::~NddlFunction()
{
}

const char* NddlFunction::getName()
{
    return m_name.c_str();
}

const char* NddlFunction::getConstraint()
{
    return m_constraint.c_str();
}
const char* NddlFunction::getReturnType()
{
    return m_returnType.c_str();
}
unsigned int NddlFunction::getArgumentCount()
{
    return m_argumentCount;
}






NddlInterpreter::NddlInterpreter(EngineId& engine)
  : m_engine(engine), m_varcount(1)
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

bool NddlInterpreter::queryIncludeGuard(const std::string& f)
{
    for (unsigned int i = 0; i < m_filesread.size(); i++) {
      if (m_filesread[i] == f) { //Not the best. Fails if the paths differ in 'absoluteness'
	    return true;
        }
    }
    return false;
}
 
void NddlInterpreter::addInclude(const std::string &f) 
{
    m_filesread.push_back(f);
}

void NddlInterpreter::addInputStream(pANTLR3_INPUT_STREAM in) 
{
    m_inputstreams.push_back(in);
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

    std::vector<std::string> includePath = getIncludePath();

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

char* NddlInterpreter::createImplicitVariable() {
    char *buff = new char[255];
    sprintf(buff, "implicit_var_%u", m_varcount++);
    check_error(m_varcount != 0, "Somehow we got zero vars or we rolled over the number of implicit variables. This is very bad.");
    return buff;
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NddlInterpreter::isImplicitVar(std::string name) {
  if (name.substr(0, strlen("implicit_var_")) == "implicit_var_") {
    return true;
  }
  return false;
}

struct varNaming {
  std::string name;
  ANTLR3_BASE_TREE_struct* renameOf;
  bool renamesImplicit;
};


//Returns "" if the tree is not a declaration, or the name 
std::string isDeclaration(ANTLR3_BASE_TREE_struct* child) {
  if (std::string((char*)child->getText(child)->chars) == "VARIABLE") {
    if (child->getChildCount(child) == 2) {
      ANTLR3_BASE_TREE_struct* name = (ANTLR3_BASE_TREE_struct*)child->getChild(child, 1);
      return (char*)name->getText(name)->chars;
    }
  }
  return "";
}


void searchAndReplace(ANTLR3_BASE_TREE_struct* tree, std::string replace, ANTLR3_BASE_TREE_struct* replaceWith) {
  for (unsigned int i = 0; i < tree->getChildCount(tree); i++) {
    ANTLR3_BASE_TREE_struct* child = (ANTLR3_BASE_TREE_struct*)tree->getChild(tree, i);
    if ((char*)(child->getText(child)->chars) == replace) {
      tree->replaceChildren(tree, i, i, replaceWith);
      //printf("Replace!\n");
    } else {
      searchAndReplace(child, replace, replaceWith);
    }
  }
}

//Tests if child is an eq constraint, if it is then the arguments are set to left and right.
bool isEqConstraint(ANTLR3_BASE_TREE_struct* child, ANTLR3_BASE_TREE_struct* &left, ANTLR3_BASE_TREE_struct* &right) {
  if (std::string((char*)child->getText(child)->chars) == "CONSTRAINT_INSTANTIATION") {
    if (child->getChildCount(child) >= 2) {
      ANTLR3_BASE_TREE_struct* name = (ANTLR3_BASE_TREE_struct*)child->getChild(child, 0);
      ANTLR3_BASE_TREE_struct* vars = (ANTLR3_BASE_TREE_struct*)child->getChild(child, 1);
      if (std::string((char*)name->getText(name)->chars) == "eq" && vars->getChildCount(vars) == 2) {
	left = (ANTLR3_BASE_TREE_struct*)vars->getChild(vars, 0);
	right = (ANTLR3_BASE_TREE_struct*)vars->getChild(vars, 1);
	return true;
      }
    }
  }
  return false;
}


//Tests if child is a three argument constraint, if it is then the arguments are set to arg1, arg2, arg3. The constriant name is set to "con".
bool isThreeArgConstraint(ANTLR3_BASE_TREE_struct* child, ANTLR3_BASE_TREE_struct* &con, 
			  ANTLR3_BASE_TREE_struct* &arg1, ANTLR3_BASE_TREE_struct* &arg2, ANTLR3_BASE_TREE_struct* &arg3) {
  if (std::string((char*)child->getText(child)->chars) == "CONSTRAINT_INSTANTIATION") {
    if (child->getChildCount(child) >= 2) {
      con = (ANTLR3_BASE_TREE_struct*)child->getChild(child, 0);
      ANTLR3_BASE_TREE_struct* vars = (ANTLR3_BASE_TREE_struct*)child->getChild(child, 1);
      if (vars->getChildCount(vars) == 3) {
	arg1 = (ANTLR3_BASE_TREE_struct*)vars->getChild(vars, 0);
	arg2 = (ANTLR3_BASE_TREE_struct*)vars->getChild(vars, 1);
	arg3 = (ANTLR3_BASE_TREE_struct*)vars->getChild(vars, 2);
	return true;
      }
    }
  }
  return false;
}


/*
 * This optimizer recursivley goes through the AST and removes slowdowns of various types. It removes:
 *      * Needless implicit variables
 *      * Tautologies (such as eq(true, true))
 *      * Tests with fixed outcomes (such as testEq(a, b, true) becomes eq(a, b)) (it current does not do this one)
 *
 * It iterates over the same node multiple times. First, the optimizer tries to remove as many implicit
 * variables that "rename" non-implicit variables and constants. After removing as many a possible, it
 * then moves on to implicit variables that equal other implict variables. These must not be removed first,
 * because a variety of strange situations could get created. As iterates over the variables, the system removes
 * as many tautologies and tests with fixed outcomes as possible.
 */
void NddlInterpreter::optimizeTree(ANTLR3_BASE_TREE_struct* tree, unsigned int tabs, void* ctx) {
  //#define TABS for (unsigned int tabi = 0; tabi < tabs; tabi++) { putchar('\t'); }
  //unsigned int me = counter++;
  //TABS; printf("%s (%u)\n", (char*)(tree->getText(tree)->chars), me);


  std::vector<varNaming*> implicit_vars;
  bool changeMade = false; //This is true if a change is found the needs to be made.
  bool renameImplicitImplicits = true; //This is true if we remove implicit variables that are a new name for other implicit variables
  bool variableRenamed = false; //This does not allow variables to be added to the rename list if it is true.

  for (unsigned int i = 0; i < tree->getChildCount(tree); i++) {
    ANTLR3_BASE_TREE_struct* child = (ANTLR3_BASE_TREE_struct*)tree->getChild(tree, i);

    //Detect the creation of implicit variables 
    std::string varName = isDeclaration(child);
    if (isImplicitVar(varName)) {
      varNaming *re = new varNaming;
      re->name = varName;
      re->renameOf = NULL;
      re->renamesImplicit = false;
      implicit_vars.push_back(re);
      //printf("This is an implicit variable sub %u! Name: %s\n", me, varName.c_str());
    }

    //Detect the re-naming of implicit variables
    ANTLR3_BASE_TREE_struct *first, *second;
    if (isEqConstraint(child, first, second) && !variableRenamed) {
      bool firstImp = isImplicitVar((char*)first->getText(first)->chars);
      bool secondImp = isImplicitVar((char*)second->getText(second)->chars);
      if (firstImp || secondImp) { //If one of the variables is implicit, this is a renaming statement here.
	//printf("This is an equality constraint that is a renaming\n");
	ANTLR3_BASE_TREE_struct* oldName = NULL;
	std::string newName = "NULL";
	if (firstImp) {
	  //printf("%s is a new name for %s\n", (char*)first->getText(first)->chars, (char*)second->getText(second)->chars);
	  oldName = second;
	  newName = (char*)first->getText(first)->chars;
	} else if(secondImp) {
	  //printf("%s is a new name for %s\n", (char*)second->getText(second)->chars, (char*)first->getText(first)->chars);
	  oldName = first;
	  newName = (char*)second->getText(second)->chars;
	} else {
	  checkError(false, "Error in the parser (how in the world did it get here?)");
	}
	checkError(oldName && newName != "NULL", "Did not set the names here.");
	changeMade = true;
	bool variableFound = false;
	for (unsigned int t = 0; t < implicit_vars.size(); t++) {
	  if (implicit_vars[t]->name == newName) {
	    variableFound = true;
	    if (!implicit_vars[t]->renameOf) {
	      implicit_vars[t]->renameOf = oldName;
	      if (firstImp && secondImp) {
		if (renameImplicitImplicits) { variableRenamed = true; }
		implicit_vars[t]->renamesImplicit = true;
		//if (renameImplicitImplicits) { printf("This is implicit-implicit renaming\n"); }
	      } else {
		renameImplicitImplicits = false; //We don't want to remove implicit-implicit renaming situations until all others are removed.
		//printf("Implicit-implicit renaming disabled\n");
	      }
	    }
	  }
	}
	checkError(variableFound == true, "An undeclared implicit variable was found in the code: \""
		   << (char*)oldName->getText(oldName)->chars << "\" -> \"" << newName << "\".");
      }
    }

  }


  //Fix function calls FIXME TODO: can we do this in antlr rewrite? If not, we should walk the tree for this outside of the optimizer.
  //This just goes trough and replaces the comma delimited list with a list of IDs.
  for (unsigned int i = 0; i < tree->getChildCount(tree); i++) {
    ANTLR3_BASE_TREE_struct* child = (ANTLR3_BASE_TREE_struct*)tree->getChild(tree, i);
    if (std::string((char*)child->getText(child)->chars) == "FUNCTION_CALL"
	&& child->getChildCount(child) == 2) { //Is a function call
      ANTLR3_BASE_TREE_struct* argumentsList = (ANTLR3_BASE_TREE_struct*)child->getChild(child, 1);
      if (argumentsList->getChildCount(argumentsList) == 2) {
	ANTLR3_BASE_TREE_struct* arguments = (ANTLR3_BASE_TREE_struct*)argumentsList->getChild(argumentsList, 1);
	std::string argumentsText = (char*)arguments->getText(arguments)->chars;
	if (argumentsText.find(",") != std::string::npos) { //Is a comma-delimited list
	  //printf("Function splitting start: %s.\n", argumentsText.c_str());
	  std::vector<std::string> argumentNames;
	  unsigned int limit = 0;
	  while(argumentsText != "") {
	    unsigned int end = argumentsText.find(",");
	    argumentNames.push_back(argumentsText.substr(0, end));
	    argumentsText = argumentsText.substr(end + 1, std::string::npos);
	    //printf("Function splitting: %s.\n", argumentsText.c_str());
	    checkError(limit < 1000, "Insane number of arguments to a function (limit: 1000).");
	  }
	  argumentsList->deleteChild(argumentsList, 1);//Remove the string comma delimited list of arguments
	  
	  
	  for (unsigned int a = 0; a < argumentNames.size(); a++) {
	    //printf("Function argument: %s\n", argumentNames[a].c_str());
	    ANTLR3_BASE_TREE_struct* newVar = (ANTLR3_BASE_TREE_struct*)((NDDL3Parser*)ctx)->adaptor
	      ->createTypeText(((NDDL3Parser*)ctx)->adaptor, IDENT, (pANTLR3_UINT8)(argumentNames[a].c_str()));
	    argumentsList->addChild(argumentsList, newVar);
	  }
	  
	}
      }
    }
  }


  //Now do the optimizations if a change needs to be made, otherwise optimize the children
  if (changeMade) {
    for (unsigned int t = 0; t < implicit_vars.size(); t++) {
      if (implicit_vars[t]->renameOf && (!implicit_vars[t]->renamesImplicit || renameImplicitImplicits)) {
	//printf("Removing implict var: %s, replacing it with %s.\n", implicit_vars[t]->name.c_str(), 
	//       (char*)implicit_vars[t]->renameOf->getText(implicit_vars[t]->renameOf)->chars);
	for (unsigned int i = 0; i < tree->getChildCount(tree); i++) {
	  ANTLR3_BASE_TREE_struct* child = (ANTLR3_BASE_TREE_struct*)tree->getChild(tree, i);
	  std::string name = isDeclaration(child);
	  if (name == implicit_vars[t]->name) {
	    tree->deleteChild(tree, i);
	    i--;
	  }
	}
	searchAndReplace(tree, implicit_vars[t]->name, implicit_vars[t]->renameOf);
      }
    }

    //Some final optimizations here.
    for (unsigned int i = 0; i < tree->getChildCount(tree); i++) {
      ANTLR3_BASE_TREE_struct* child = (ANTLR3_BASE_TREE_struct*)tree->getChild(tree, i);

      //Remove tautologies
      ANTLR3_BASE_TREE_struct *first, *second, *third, *con;
      if (isEqConstraint(child, first, second)) {
	if (std::string((char*)first->getText(first)->chars) == std::string((char*)second->getText(second)->chars)) {
	  tree->deleteChild(tree, i);
	  i--;
	}
      }

      //Remove test contraints with singleton outcomes
      if (isThreeArgConstraint(child, con, first, second, third)) {
	std::string con_name = (char*)con->getText(con)->chars;
	std::string input = (char*)first->getText(first)->chars;
	//printf("Three arg: %s : %s\n", (char*)con->getText(con)->chars, input.c_str());

	bool removeArg = false;
	std::string rename = "";

	if (con_name == "testEQ") {
	  if (input == "true") {
	    removeArg = true;
	    rename = "eq";
	  } else if (input == "false") {
	  }
	}

	if (removeArg) {
	  ANTLR3_BASE_TREE_struct* vars = (ANTLR3_BASE_TREE_struct*)child->getChild(child, 1);
	  vars->deleteChild(vars, 0); //Remove it.
	}
	if (rename != "") {
	  ANTLR3_BASE_TREE_struct* target = (ANTLR3_BASE_TREE_struct*)((NDDL3Parser*)ctx)->adaptor->createTypeText(((NDDL3Parser*)ctx)->adaptor, IDENT, (pANTLR3_UINT8)rename.c_str());
	  child->replaceChildren(child, 0, 0, target);  
	}
	
      }

    }



    optimizeTree(tree, tabs, ctx);
  } else {
    for (unsigned int i = 0; i < tree->getChildCount(tree); i++) {
      ANTLR3_BASE_TREE_struct* child = (ANTLR3_BASE_TREE_struct*)tree->getChild(tree, i);
      optimizeTree(child, tabs + 1, ctx);
    }
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string NddlInterpreter::interpret(std::istream& ins, const std::string& source)
{
    if (queryIncludeGuard(source)) 
    {
      debugMsg("NddlInterpreter:error", "Ignoring root file: " << source << ". Bug?");
        return "";
    }
    addInclude(source);

    pANTLR3_INPUT_STREAM input = getInputStream(ins,source);

    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    lexer->parserObj = this;
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);
    parser->parserObj = this;

    // Build he AST
    NDDL3Parser_nddl_return result = parser->nddl(parser);
    int errorCount = parser->pParser->rec->state->errorCount + lexer->pLexer->rec->state->errorCount;
    if (errorCount > 0) {
        std::ostringstream os;
        os << "The parser returned " << errorCount << " errors" << std::endl;

        // Since errors are no longer printed during parsing, print them here
        // to stderr and to the return stream
        std::vector<NddlParserException> *lerrors = lexer->lexerErrors;
        std::vector<NddlParserException> *perrors = parser->parserErrors;
        for (std::vector<NddlParserException>::const_iterator it = lerrors->begin(); it != lerrors->end(); ++it) {
        	std::cerr << *it << std::endl;
        	os << *it << std::endl;
        }
        for (std::vector<NddlParserException>::const_iterator it = perrors->begin(); it != perrors->end(); ++it) {
        	std::cerr << *it << std::endl;
        	os << *it << std::endl;
        }
        parser->free(parser);
        tstream->free(tstream);
        lexer->free(lexer);
        input->close(input);
        debugMsg("NddlInterpreter:interpret",os.str());
        return os.str();
    }

    debugMsg("NddlInterpreter:interpret","NDDL AST Pre-optimization:\n" << result.tree->toStringTree(result.tree)->chars);


    //printf("\nTREE:\n");
    optimizeTree(result.tree, 0, parser);

    debugMsg("NddlInterpreter:interpret","NDDL AST Post-optimization:\n" << result.tree->toStringTree(result.tree)->chars);

    // Walk the AST to create nddl expr to evaluate
    pANTLR3_COMMON_TREE_NODE_STREAM nodeStream = antlr3CommonTreeNodeStreamNewTree(result.tree, ANTLR3_SIZE_HINT);
    pNDDL3Tree treeParser = NDDL3TreeNew(nodeStream);
    treeParser->parserObj = this;

    NddlSymbolTable symbolTable(m_engine);
    treeParser->SymbolTable = &symbolTable;
    treeParser->allowEval = (getEngine()->getConfig()->getProperty("nddl.eval") != "false");

    debugMsg("NddlInterpreter", "Allow Eval: " << treeParser->allowEval);

    try {
        treeParser->nddl(treeParser);
    }
    catch (const std::string& error) {
        debugMsg("NddlInterpreter:error","nddl parser halted on error:" << symbolTable.getErrors());
        //std::cerr << symbolTable.getErrors() << std::endl;
    }
    catch (const Error& internalError) {
        symbolTable.reportError(treeParser,internalError.getMsg());
        debugMsg("NddlInterpreter:error","nddl parser halted on error:" << symbolTable.getErrors());
        //std::cerr << symbolTable.getErrors() << std::endl;
    }

    // Free everything
    treeParser->free(treeParser);
    nodeStream->free(nodeStream);

    while(!m_inputstreams.empty()) {
      m_inputstreams[0]->close(m_inputstreams[0]);
      m_inputstreams.erase(m_inputstreams.begin());
    }

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
    m_functions.push_back(new NddlFunction("equalTestFunction", "testEQ", "bool", 2));
    m_functions.push_back(new NddlFunction("isSingleton", "testSingleton", "bool", 1));
    m_functions.push_back(new NddlFunction("isSpecified", "testSpecified", "bool", 1));
}

NddlSymbolTable::~NddlSymbolTable()
{
    while(!m_functions.empty()) 
    {
	delete m_functions[0];
	m_functions.erase(m_functions.begin());
    }
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

void NddlSymbolTable::addFunction(NddlFunction* func)
{
    m_functions.push_back(func);
}

NddlFunction* NddlSymbolTable::getFunction(const char* name) const
{
    for (unsigned int i = 0; i < m_functions.size(); i++)
    {
	if (!strcmp(m_functions[i]->getName(), name)) {
            return m_functions[i];
	}
    }
    if (m_parentST) 
    {
        return m_parentST->getFunction(name);
    }
    return NULL;
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
    if (m_parentST != NULL)
        return m_parentST->getObjectType(name);
    else {
        SchemaId s = ((Schema*)(engine()->getComponent("Schema")))->getId();

        if (s->isObjectType(name))
            return s->getObjectType(name);
    }

    return ObjectTypeId::noId();
}

TokenFactoryId NddlSymbolTable::getTokenType(const char* name) const
{
    if (m_parentST != NULL)
        return m_parentST->getTokenType(name);
    else {
        SchemaId s = ((Schema*)(engine()->getComponent("Schema")))->getId();

        if (s->isPredicate(name))
            return s->getTokenFactory(name);
    }

    return TokenFactoryId::noId();
}


void NddlSymbolTable::addLocalVar(const char* name,const DataTypeId& type)
{
    m_localVars[name]=type;
    debugMsg("NddlSymbolTable:addLocalVar","Added local var "+std::string(name));
}

void NddlSymbolTable::addLocalToken(const char* name,const TokenFactoryId& type)
{
    m_localTokens[name]=type;
    debugMsg("NddlSymbolTable:addLocalToken","Added local token "+std::string(name));
}

DataTypeId NddlSymbolTable::getTypeForVar(const char* name)
{
    if (m_localVars.find(name) != m_localVars.end())
        return m_localVars[name];

    if (m_parentST != NULL)
        return m_parentST->getTypeForVar(name);
    else if (getPlanDatabase()->isGlobalVariable(name))
        return getPlanDatabase()->getGlobalVariable(name)->getDataType();

    return DataTypeId::noId();
}

DataTypeId NddlSymbolTable::getTypeForVar(const char* qualifiedName,std::string& errorMsg)
{
    std::string parentName;
    std::vector<std::string> vars;
    std::string fullName(qualifiedName);
    tokenize(fullName,vars,".");

    if (vars.size() > 1) {
      parentName = vars[0];
      fullName = fullName.substr(parentName.length()+1);
      vars.erase(vars.begin());
      debugMsg("NddlSymbolTable:getTypeForVar","Split " << qualifiedName << " into " << parentName << " and " << fullName);
    }
    else {
      parentName = "";
      debugMsg("NddlSymbolTable:getTypeForVar","Didn't split " << qualifiedName);
    }

    DataTypeId dt;

    if (parentName == "") {
        dt = getTypeForVar(qualifiedName);
        if (dt.isNoId()) {
            // We failed to find a variable, let's try to find a token
            TokenFactoryId tt = getTypeForToken(qualifiedName);

            if (tt.isId())
                dt =  FloatDT::instance(); // TODO : return data type for state var?
            else
                errorMsg = fullName + " is not defined";
        }

        return dt;
    }
    else {
        unsigned int idx = 0;
        std::string curVarName=parentName;
        std::string curVarType;

        TokenFactoryId tt = getTypeForToken(parentName.c_str());
        if (tt.isNoId()) {
            dt = getTypeForVar(parentName.c_str(),errorMsg);
            if (dt.isNoId())
                return dt;
        }
        else {
            dt = tt->getArgType(vars[idx].c_str());
            if (dt.isNoId()) {
                errorMsg = curVarName+"("+tt->getPredicateName().toString()+") doesn't have a parameter called "+vars[idx];
                return dt;
            }
            curVarName = vars[idx];
            idx++;
        }
        curVarType = dt->getName().toString();

        for (;idx<vars.size();idx++) {
            ObjectTypeId ot = getObjectType(curVarType.c_str());
            if (ot.isNoId()) {
                errorMsg = curVarName+"("+curVarType+") is not a reference to an object";
                return DataTypeId::noId();
            }

            dt = ot->getMemberType(vars[idx].c_str());
            if (dt.isNoId()) {
                errorMsg = curVarName+"("+curVarType+") doesn't have a member called "+vars[idx];
                return DataTypeId::noId();
            }

            curVarName = vars[idx];
            curVarType = dt->getName().toString();
        }
    }

    return dt;

}

TokenFactoryId NddlSymbolTable::getTypeForToken(const char* name)
{
    if (m_localTokens.find(name) != m_localTokens.end())
        return m_localTokens[name];

    if (m_parentST != NULL)
        return m_parentST->getTypeForToken(name);
    else if (getPlanDatabase()->isGlobalToken(name)) {
        // TODO: modify Token to keep a handle on its Token Type
        TokenId t = getPlanDatabase()->getGlobalToken(name);
        return getTokenType(t->getPredicateName().c_str());
    }

    return TokenFactoryId::noId();
}


TokenFactoryId NddlSymbolTable::getTypeForToken(const char* qualifiedName,std::string& errorMsg)
{
    std::string parentName;
    std::string tokenType;
    std::vector<std::string> vars;
    std::string fullName(qualifiedName);
    tokenize(fullName,vars,".");

    if (vars.size() > 1) {
      parentName = vars[0];
      vars.erase(vars.begin());
      fullName = fullName.substr(parentName.length()+1);

      tokenType = vars.back();
      vars.erase(--vars.end());
      fullName = fullName.substr(0,fullName.length()-(tokenType.length()));

      debugMsg("NddlSymbolTable:getTypeForToken","Split " << qualifiedName
                                                           << " into " << parentName
                                                           << " , " << tokenType
                                                           << " and " << fullName);
    }
    else {
      parentName = "";
      debugMsg("NddlSymbolTable:getTypeForToken","Didn't split " << qualifiedName);
    }

    TokenFactoryId tt;

    if (parentName == "") {
        tt = getTokenType(qualifiedName);
        if (tt.isNoId())
            errorMsg = fullName + " is not a predicate type";

        return tt;
    }
    else {
        DataTypeId dt;
        ObjectTypeId ot = getObjectType(parentName.c_str());
        if (ot.isId()) {
            if (vars.size() > 1) {
                errorMsg = std::string(qualifiedName)+" is not a predicate type";
                return tt;
            }
            return getTokenType(qualifiedName);
        }
        else {
            dt = getTypeForVar(parentName.c_str());
            if (dt.isNoId()) {
                errorMsg = parentName + " is not defined";
                return tt;
            }
            ot = getObjectType(dt->getName().c_str());
            if (ot.isNoId()) {
                errorMsg = parentName+"("+dt->getName().c_str()+") is not a reference to an object";
                return tt;
            }
        }

        std::string curVarName=parentName;
        std::string curVarType = dt->getName().toString();
        unsigned int idx = 0;
        for (;idx<vars.size();idx++) {
            dt = ot->getMemberType(vars[idx].c_str());
            if (dt.isNoId()) {
                errorMsg = curVarName+"("+curVarType+") doesn't have a member called "+vars[idx];
                return TokenFactoryId::noId();
            }

            ot = getObjectType(dt->getName().c_str());
            if (ot.isNoId()) {
                errorMsg = curVarName+"("+curVarType+") is not a reference to an object";
                return TokenFactoryId::noId();
            }

            curVarName = vars[idx];
            curVarType = dt->getName().toString();
        }

        tokenType = ot->getName().toString()+"."+tokenType;
        debugMsg("NddlSymbolTable:getTypeForToken","looking for tokenType " << tokenType);
        return getTokenType(tokenType.c_str());
    }

    return tt;
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
        return EvalContext::getVar(name);
}

TokenId NddlSymbolTable::getToken(const char* name)
{
    if (getPlanDatabase()->isGlobalToken(name))
        return getPlanDatabase()->getGlobalToken(name);
    else
        return EvalContext::getToken(name);
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

    return new ExprConstant(enumType.c_str(),ad);
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


NddlTokenSymbolTable::NddlTokenSymbolTable(NddlSymbolTable* parent,
                                           const TokenFactoryId& tt,
                                           const ObjectTypeId& ot)
    : NddlSymbolTable(parent)
    , m_tokenType(tt)
    , m_objectType(ot)
{
}

NddlTokenSymbolTable::~NddlTokenSymbolTable()
{
}

DataTypeId NddlTokenSymbolTable::getTypeForVar(const char* varName)
{
    if (std::string(varName)=="object")
        return m_objectType->getVarType();

    DataTypeId dt = m_tokenType->getArgType(varName);
    if (dt.isId())
        return dt;

    return NddlSymbolTable::getTypeForVar(varName);
}

TokenFactoryId NddlTokenSymbolTable::getTokenType(const char* name) const
{
    TokenFactoryId tt= NddlSymbolTable::getTokenType(name);

    if (tt.isNoId()) {
        // Try implicit qualification
        std::string qualifiedName = m_objectType->getName().toString()+"."+name;
        tt= NddlSymbolTable::getTokenType(qualifiedName.c_str());
    }

    return tt;
}

TokenFactoryId NddlTokenSymbolTable::getTypeForToken(const char* name)
{
    if (std::string(name)=="this")
        return m_tokenType;

    return NddlSymbolTable::getTypeForToken(name);
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
    NDDL3Parser_nddl_return result = parser->nddl(parser);

    // The result
    std::ostringstream os;

    // Add errors, if any
    std::vector<NddlParserException> *lerrors = lexer->lexerErrors;
    std::vector<NddlParserException> *perrors = parser->parserErrors;

    for (int i=0; i<lerrors->size(); i++)
    	os << "L" << (*lerrors)[i] << "$\n";
    for (int i=0; i<perrors->size(); i++)
    	os << "P" << (*perrors)[i] << "$\n";
    // Warnings, if any, should go here

    // Calling static helper functions to get a verbose version of AST
    const char* ast = (char*)(toVerboseStringTree(result.tree)->chars);
    os << "AST " << ast;

    debugMsg("NddlToASTInterpreter:interpret",os.str());

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);

    return os.str();
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

NddlParserException::NddlParserException(const char *fileName, int line,
		int offset, int length, const char *message) :
  m_line(line), m_offset(offset), m_length(length), m_message(message) {
  if (fileName) m_fileName = fileName;
  else fileName = "No_File";
}

ostream &operator<<(ostream &os, const NddlParserException &ex) {
	os << "\"" << ex.m_fileName << "\":" <<ex.m_line << ":" << ex.m_offset << ":"
		<< ex.m_length << " " << ex.m_message;
}

std::string NddlParserException::asString() const {
	std::ostringstream os;
	os << *this;
	return os.str();
}


void reportParserError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames) {
	pANTLR3_EXCEPTION ex = recognizer->state->exception;

	const char *fileName = NULL;
	if (ex->streamName)
		fileName = (const char *)(ex->streamName->to8(ex->streamName)->chars);

	int line = recognizer->state->exception->line;
	const char* message = static_cast<const char *>(recognizer->state->exception->message);

	int offset = -1; // to signal something is wrong (like recognizer type)
	int length = 0; // in case there is no token
	if (recognizer->type == ANTLR3_TYPE_PARSER) {
		offset = recognizer->state->exception->charPositionInLine;

		// Look for a token
		pANTLR3_COMMON_TOKEN token = (pANTLR3_COMMON_TOKEN)(recognizer->state->exception->token);
		line = token->getLine(token);
		pANTLR3_STRING text = token->getText(token);
		if (text != NULL) {
			// It looks like when an extra token is present, message is
			// empty and the token points to the actual thing. When a token
			// is missing, the token text contains the message and the length
			// is irrelevant
			if (message == NULL || !message[0])
				length = text->len;
			message = (const char *)(text->chars);
		}
	}

	NddlParserException exception(fileName, line, offset, length, message);

	pANTLR3_PARSER parser = (pANTLR3_PARSER) recognizer->super;
	pNDDL3Parser ctx = (pNDDL3Parser) parser->super;
	std::vector<NddlParserException> *errors = ctx->parserErrors;
	errors->push_back(exception);

	// std::cout << errors->size() << "; " << (*errors)[errors->size()-1];
}

void reportLexerError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames) {
    pANTLR3_LEXER lexer = (pANTLR3_LEXER)(recognizer->super);
    pANTLR3_EXCEPTION ex = lexer->rec->state->exception;

	const char *fileName = NULL;
	if (ex->streamName)
		fileName = (const char *)(ex->streamName->to8(ex->streamName)->chars);

	int line = recognizer->state->exception->line;
	const char* message = static_cast<const char *>(recognizer->state->exception->message);
	int offset = ex->charPositionInLine+1;

	NddlParserException exception(fileName, line, offset, 1, message);

	pNDDL3Lexer ctx = (pNDDL3Lexer) lexer->ctx;
	std::vector<NddlParserException> *errors = ctx->lexerErrors;
	errors->push_back(exception);

	// std::cout << errors->size() << "; " << (*errors)[errors->size()-1];
}
}
