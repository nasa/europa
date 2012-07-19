#include <iostream>
#include <fstream>
#include "Error.hh"

// Support for default setup
#include "ANMLParser.hpp"

void outputAST2Dot(std::ostream& out, const antlr::RefAST ast, const int parent, int& node) {
  int self = node;

  out << "  node_" << self;
  out <<  " [label=\"" << ast->toString() << "\"];" << std::endl;
  if(parent != -1)
    out << "  node_" << parent << " -> node_" << self << ";" << std::endl;

  if(ast->getFirstChild())
    outputAST2Dot(out, ast->getFirstChild(), self, ++node);

  if(ast->getNextSibling())
    outputAST2Dot(out, ast->getNextSibling(), parent, ++node);
}

/**
 * Interface for testing ANML models for parsability.
 * Called with two filenames: [ANML model] [Output Digraph AST].
 */
int main(int argc, char** argv) {
	std::cout << "Starting ANML Test Parser" << std::endl;
	for(int i=0; i< argc; ++i) {
		std::cout << "arg[" << i << "] = " << argv[i] << std::endl;
	}
  assertTrue(argc == 3, "Expected exactly two filename arguments: [ANML model] [Output Digraph AST]");

  ANTLR_USE_NAMESPACE(antlr)RefAST ast =
    ANMLParser::parse(".", argv[1]);

  assertTrue(ast != ANTLR_USE_NAMESPACE(antlr)nullAST, "Parse failed to return an AST");

	std::ofstream digraph(argv[2]);

  int node = 0;
  digraph << "digraph ANMLAst {" << std::endl;
  outputAST2Dot(digraph, ast, -1, node);
  digraph << "}" << std::endl;

  return 0;
}
