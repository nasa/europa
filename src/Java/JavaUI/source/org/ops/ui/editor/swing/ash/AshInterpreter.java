package org.ops.ui.editor.swing.ash;

public abstract class AshInterpreter {
  private String grammar = "Antlr";

  public AshInterpreter(String grammar) {
    this.grammar = grammar;
  }

  public String getPrompt() {
    return grammar + " % ";
  }

  public String getBanner() {
    return "!"+grammar+"Shell 1.0 - Matthew E. Boyce (meboyce@email.arc.nasa.gov)";
  }

  // returns true if parse didn't encounter a premature end of string.
  public abstract boolean eval(String toEval);

  public abstract void source(String toEval);
}
