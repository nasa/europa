#include <iostream>
#include <fstream>
#include "Error.hh"
#include "Debug.hh"

// Support for default setup
#include "ANMLParser.hpp"
#include "ANML2NDDL.hpp"

int max(int a, int b) { return (a<b)? b : a; }

void parse        (const std::string& filename, antlr::RefAST& ast);
void translate    (const std::string& filename, antlr::RefAST& ast);
void dumpDigraph  (const std::string& filename, const antlr::RefAST& ast);
int outputAST2Dot(std::ostream& out, const antlr::RefAST& ast, const int depth, const int parent, int& node); 



/**
 * Interface for testing ANML models for parsability.
 * Called with one filename: [ANML model] without the extension (.anml)
 */
int main(int argc, char** argv) 
{
  assertTrue(argc == 2, "Expected exactly one filename pattern argument: [ANML Model], without the extension");
  std::string filename(argv[1]);  

  try {
      antlr::RefAST ast;
  
      parse(filename,ast);
      translate(filename,ast);
  }
  catch (Error& e) {
      std::cerr << "Failed translating " << filename << " : " << e.getMsg() << std::endl;
      return -1;	  
  }
  
  return 0;
}

void parse(const std::string& filename, antlr::RefAST& ast)
{
  debugMsg("ANMLTest:parser", "Phase 1: parsing \"" << filename << ".anml\"");
  
  ast = ANMLParser::parse(".", filename + ".anml");
  assertTrue(ast != antlr::nullAST, "Parse failed to return an AST");
  
  // debug output
  dumpDigraph(filename,ast);
  
  debugMsg("ANMLTest:parser", "Phase 1 complete");
}

void translate(const std::string& filename, antlr::RefAST& ast)
{
  debugMsg("ANMLTest:translator", "Phase 2: Walking parse tree");
  std::ofstream nddl((filename + "-auto.nddl").c_str());

  ANML2NDDL* treeParser = new ANML2NDDL();
  treeParser->anml(ast);

  debugMsg("ANMLTest:translator", "Phase 2 complete");
  debugMsg("ANMLTest:translator", "\n" << treeParser->getTranslator().toString());
  
  treeParser->getTranslator().toNDDL(nddl);  
  delete treeParser;
}

void dumpDigraph(const std::string& filename, const antlr::RefAST& ast)
{
  debugMsg("ANMLTest:parser", "Phase 1: dumping parse tree to \"" << filename << ".dot\"");

  std::ofstream digraph((filename + ".dot").c_str());

  int node = 0;
  digraph << "digraph ANMLAst {" << std::endl;
  outputAST2Dot(digraph, ast, 1, -1, node);
  digraph << "}" << std::endl;
  digraph.flush();	
}

int outputAST2Dot(std::ostream& out, 
                  const antlr::RefAST& ast, 
                  const int depth, 
                  const int parent, 
                  int& node) 
{
  if(ast == antlr::nullAST)
    return depth-1;
  int maxdepth = depth;

  // saved for iteration over children.
  int currParent = node;

  antlr::RefAST curr = ast;
  while(curr != antlr::nullAST) {
    out << "  node_" << node;
    out <<  " [label=\"" << curr->toString() << "\"];" << std::endl;
    if(curr != ast) {
      out << "  node_" << node-1 << " -> node_" << node << " [style=invis];" << std::endl;
    }
    if(parent != -1)
      out << "  node_" << parent << " -> node_" << node << ";" << std::endl;
    curr = curr->getNextSibling();
    ++node;
  }

  out << "  { rank = same; depth_" << depth;
  for(int i=currParent; i<node; ++i)
    out << ";"<< std::endl << "      node_" << i;
  out << "}" << std::endl;

  curr = ast;
  while(curr != antlr::nullAST) {
    maxdepth = max(maxdepth,outputAST2Dot(out, curr->getFirstChild(), depth+1, currParent, node));
    curr = curr->getNextSibling();
    ++currParent;
  }

  if(depth == 1) {
    out << "  {" << std::endl;
    for(int i=1; i<=maxdepth; ++i)
      out << "    depth_" << i << " [shape=plaintext, label=\"Tree Depth " << i << "\"];" << std::endl;
    out << "    depth_1";
    for(int i=2; i<=maxdepth; ++i)
      out << " -> depth_" << i;
    out << ";" << std::endl << "  }" << std::endl;
  }
  return maxdepth;
}

