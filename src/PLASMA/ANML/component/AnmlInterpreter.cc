/*
 * AnmlInterpreter.cc
 *
 */

#include "AnmlInterpreter.hh"

#include "ANMLLexer.h"
#include "ANMLParser.h"
//#include "ANMLTree.h"
#include "antlr3exception.h"
#include "NddlInterpreter.hh"

namespace EUROPA {

AnmlInterpreter::AnmlInterpreter(EngineId& engine)
	: m_engine(engine)
{
}

AnmlInterpreter::~AnmlInterpreter()
{
}

std::string AnmlInterpreter::interpret(std::istream& ins, const std::string& source)
{
	std::string strInput;
    pANTLR3_INPUT_STREAM input = getInputStream(ins,source,strInput);

    pANMLLexer lexer = ANMLLexerNew(input);
    //lexer->parserObj = this;
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pANMLParser parser = ANMLParserNew(tstream);

    // Build he AST
    ANMLParser_anml_return result = parser->anml(parser);

    // The result
    std::ostringstream os;

    // Add errors, if any
    /*
    std::vector<PSLanguageException> *lerrors = lexer->lexerErrors;
    std::vector<PSLanguageException> *perrors = parser->parserErrors;

    for (unsigned int i=0; i<lerrors->size(); i++)
    	os << "L" << (*lerrors)[i] << "$\n";
    for (unsigned int i=0; i<perrors->size(); i++)
    	os << "P" << (*perrors)[i] << "$\n";
    	*/
    // Warnings, if any, should go here

    // Calling static helper functions to get a verbose version of AST
    const char* ast = (char*)(toVerboseStringTree(result.tree)->chars);
    os << "AST " << ast;

    debugMsg("AnmlToASTInterpreter:interpret",os.str());

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);

    return os.str();
}


}
