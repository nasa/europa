package org.ops.ui.editor.swt;

import org.eclipse.jface.text.rules.*;

/**
 * Partition scanner cuts text into portions with different meaning so that
 * coloring and other modules know what to do with it
 */
public class NddlPartitionScanner extends RuleBasedPartitionScanner {
	public final static String NDDL_COMMENT = "__nddl_comment";

	public NddlPartitionScanner() {

		IToken nddlComment = new Token(NDDL_COMMENT);

		IPredicateRule[] rules = new IPredicateRule[2];

		rules[0] = new EndOfLineRule("//", nddlComment);
		rules[1] = new MultiLineRule("/*", "*/", nddlComment);

		setPredicateRules(rules);
	}
}
