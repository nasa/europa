package org.ops.ui.editor.swing.nddl;

/*
 * NddlTokenMarker.java - Nddl Shell token marker
 */

import org.ops.ui.editor.swing.ash.KeywordMap;
import org.ops.ui.editor.swing.ash.AshTokenMarker;
import org.ops.ui.editor.swing.ash.Token;

/**
 * Nddl Shell token marker.
 *
 * @author Matthew E. Boyce
 * @version $Id: NddlTokenMarker.java,v 1.4 2007-03-01 22:25:30 meboyce Exp $
 */
public class NddlTokenMarker extends AshTokenMarker {
  public NddlTokenMarker() {
		super("Nddl % ");
    this.keywords = getKeywords();
  }

  public static KeywordMap getKeywords() {
    if(nddlKeywords == null) {
      nddlKeywords = new KeywordMap(false);

      nddlKeywords.add("#include", Token.PREPROC);

      // functions (cyan)
      nddlKeywords.add("close", Token.KEYWORD3);
      nddlKeywords.add("free", Token.KEYWORD3);
      nddlKeywords.add("constrain", Token.KEYWORD3);
      nddlKeywords.add("merge", Token.KEYWORD3);
      nddlKeywords.add("reject", Token.KEYWORD3);
      nddlKeywords.add("activate", Token.KEYWORD3);
      nddlKeywords.add("cancel", Token.KEYWORD3);
      nddlKeywords.add("reset", Token.KEYWORD3);
      nddlKeywords.add("specify", Token.KEYWORD3);
      // conditionals (yellow)
      nddlKeywords.add("if", Token.KEYWORD1);
      nddlKeywords.add("else", Token.KEYWORD1);
      nddlKeywords.add("foreach", Token.KEYWORD1);
      nddlKeywords.add("filterOnly", Token.KEYWORD1);
      nddlKeywords.add("in", Token.KEYWORD1);
      // class related (yellow)
      nddlKeywords.add("new", Token.KEYWORD1);
      nddlKeywords.add("this", Token.KEYWORD1);
      nddlKeywords.add("super", Token.KEYWORD1);
      // types (green)
      nddlKeywords.add("bool", Token.KEYWORD2);
      nddlKeywords.add("string", Token.KEYWORD2);
      nddlKeywords.add("int", Token.KEYWORD2);
      nddlKeywords.add("float", Token.KEYWORD2);
      nddlKeywords.add("numeric", Token.KEYWORD2);
      // class (green)
      nddlKeywords.add("class", Token.KEYWORD2);
      nddlKeywords.add("enum", Token.KEYWORD2);
      nddlKeywords.add("typedef", Token.KEYWORD2);
      nddlKeywords.add("extends", Token.KEYWORD2);
      nddlKeywords.add("predicate", Token.KEYWORD2);
      // goals (cyan)
      nddlKeywords.add("goal", Token.KEYWORD3);
      nddlKeywords.add("rejectable", Token.KEYWORD3);
      // subgoal (cyan)

      nddlKeywords.add("contains", Token.KEYWORD3);
      nddlKeywords.add("any", Token.KEYWORD3);
      nddlKeywords.add("starts", Token.KEYWORD3);
      nddlKeywords.add("ends", Token.KEYWORD3);
      nddlKeywords.add("equals", Token.KEYWORD3);
      nddlKeywords.add("equal", Token.KEYWORD3);
      nddlKeywords.add("before", Token.KEYWORD3);
      nddlKeywords.add("after", Token.KEYWORD3);
      nddlKeywords.add("contained_by", Token.KEYWORD3);
      nddlKeywords.add("ends_before", Token.KEYWORD3);
      nddlKeywords.add("ends_after", Token.KEYWORD3);
      nddlKeywords.add("starts_before_end", Token.KEYWORD3);
      nddlKeywords.add("ends_after_start", Token.KEYWORD3);
      nddlKeywords.add("contains_start", Token.KEYWORD3);
      nddlKeywords.add("starts_during", Token.KEYWORD3);
      nddlKeywords.add("contains_end", Token.KEYWORD3);
      nddlKeywords.add("ends_during", Token.KEYWORD3);
      nddlKeywords.add("meets", Token.KEYWORD3);
      nddlKeywords.add("met_by", Token.KEYWORD3);
      nddlKeywords.add("parallels", Token.KEYWORD3);
      nddlKeywords.add("paralleled_by", Token.KEYWORD3);
      nddlKeywords.add("starts_before", Token.KEYWORD3);
      nddlKeywords.add("starts_after", Token.KEYWORD3);

      // literals
      // bools
      nddlKeywords.add("true", Token.LITERAL3);
      nddlKeywords.add("false", Token.LITERAL3);
      // infs
      nddlKeywords.add("+inf", Token.LITERAL3);
      nddlKeywords.add("-inf", Token.LITERAL3);
      nddlKeywords.add("+inff", Token.LITERAL3);
      nddlKeywords.add("-inff", Token.LITERAL3);
      // states
      nddlKeywords.add("INACTIVE", Token.LITERAL1);
      nddlKeywords.add("ACTIVE", Token.LITERAL1);
      nddlKeywords.add("MERGED", Token.LITERAL1);
      nddlKeywords.add("REJECTED", Token.LITERAL1);

      // common stuff
      // classes
      nddlKeywords.add("Object", Token.LITERAL1);
      nddlKeywords.add("TokenStates", Token.LITERAL1);
      nddlKeywords.add("Timeline", Token.LITERAL1);
      nddlKeywords.add("Resource", Token.LITERAL1);
      nddlKeywords.add("UnaryResource", Token.LITERAL1);
      nddlKeywords.add("StringData", Token.LITERAL1);
      nddlKeywords.add("PlannerConfig", Token.LITERAL1);
      // constraints
      nddlKeywords.add("eq", Token.OPERATOR);
      nddlKeywords.add("neq", Token.OPERATOR);
      nddlKeywords.add("leq", Token.OPERATOR);
      nddlKeywords.add("addEq", Token.OPERATOR);
      nddlKeywords.add("mulEq", Token.OPERATOR);
      nddlKeywords.add("multEq", Token.OPERATOR);
      nddlKeywords.add("allDiff", Token.OPERATOR);
    }
    return nddlKeywords;
  }

  // private members
  private static KeywordMap nddlKeywords;
}
