package aver;

import antlr.*;
import antlr.collections.AST;

public class LineNumberAST extends CommonAST {
  private int lineNo;
  public LineNumberAST(){}
  public LineNumberAST(Token tok){super(tok);}
  public void initialize(AST t) {
    super.initialize(t);
    if(t instanceof LineNumberAST) {
      lineNo = ((LineNumberAST)t).lineNo;
    }
  }
  public void initialize(Token t) {
    super.initialize(t);
    lineNo = t.getLine();
  }
  public int getLine() {
    return lineNo;
  }
}
