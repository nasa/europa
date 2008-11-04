#include "nddl-test-module.hh"
#include "NDDL3Lexer.h"
#include "NDDL3Parser.h"

void NDDLModuleTests::syntaxTests()
{
    std::string filename="parser.nddl";

    pANTLR3_INPUT_STREAM input = antlr3AsciiFileStreamNew((pANTLR3_UINT8)filename.c_str());
    pNDDL3Lexer lexer = NDDL3LexerNew(input);
    pANTLR3_COMMON_TOKEN_STREAM tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    pNDDL3Parser parser = NDDL3ParserNew(tstream);
    parser->nddl(parser);

    parser->free(parser);
    tstream->free(tstream);
    lexer->free(lexer);
    input->close(input);
}


