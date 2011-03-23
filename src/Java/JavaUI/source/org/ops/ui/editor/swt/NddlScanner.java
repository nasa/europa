package org.ops.ui.editor.swt;

import java.util.ArrayList;

import org.eclipse.jdt.internal.ui.text.CombinedWordRule;
import org.eclipse.jface.text.rules.*;
import org.eclipse.jface.text.*;
import org.eclipse.swt.SWT;

/** File-level scanner */
@SuppressWarnings("restriction")
public class NddlScanner extends BufferedRuleBasedScanner {

	private final static String[] KEY_WORDS = { "include", "enum", "typedef",
			"int", "float", "bool", "string", "true", "false",
			"inf", "inff", "constraint", "extends", "numeric", "class",
			"predicate", "this", "rejectable", "goal", "super", "if", "else",
			"foreach", "in", "new", "filter", "fact",

			"specify", "free", "constrain", "merge", "activate", "reset",
			"reject", "cancel", "close" };

	// "Operator"};

	/*
	 * temporalRelation: "any" | "ends" | "starts" | "equals" | "equal" |
	 * "before" | "after" | "contains" | "contained_by" | "ends_before" |
	 * "ends_after" | "starts_before_end" | "ends_after_start" |
	 * "contains_start" | "starts_during" | "contains_end" | "ends_during" |
	 * "meets" | "met_by" | "parallels" | "paralleled_by" | "starts_before" |
	 * "starts_after" ;
	 */

	public NddlScanner(ColorManager manager) {
		ArrayList<IRule> rules = new ArrayList<IRule>();

		// Add generic whitespace rule.
		rules.add(new WhitespaceRule(new WhitespaceDetector()));

		// Strings
		IToken stringToken = new Token(new TextAttribute(manager
				.getColor(INddlColorConstants.STRING)));
		rules.add(new SingleLineRule("\"", "\"", stringToken, '\\'));

		// A bunch of keywords

		IToken defaultToken = new Token(new TextAttribute(manager
				.getColor(INddlColorConstants.DEFAULT)));
		WordDetector wordDetector = new WordDetector();
		CombinedWordRule combinedWordRule = new CombinedWordRule(wordDetector,
				defaultToken);

		Token keywordToken = new Token(new TextAttribute(manager
				.getColor(INddlColorConstants.KEYWORD), null, SWT.BOLD));
		CombinedWordRule.WordMatcher keywordMatcher = new CombinedWordRule.WordMatcher();
		for (String s : KEY_WORDS)
			keywordMatcher.addWord(s, keywordToken);
		combinedWordRule.addWordMatcher(keywordMatcher);
		rules.add(combinedWordRule);

		Token bracketToken = new Token(new TextAttribute(manager
				.getColor(INddlColorConstants.BRACKET)));
		rules.add(new CharacterRule(bracketToken, "(){}[]"));

		IRule[] arr = new IRule[rules.size()];
		rules.toArray(arr);
		setRules(arr);
	}

	private class WhitespaceDetector implements IWhitespaceDetector {
		public boolean isWhitespace(char c) {
			return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
		}
	}

	private class WordDetector implements IWordDetector {
		public boolean isWordPart(char c) {
			return Character.isJavaIdentifierPart(c);
		}

		public boolean isWordStart(char c) {
			return Character.isJavaIdentifierStart(c);
		}
	}

	/**
	 * Rule to detect character-based things like operators and brackets.
	 * Adopted from JavaCodeScanner.
	 */
	private static final class CharacterRule implements IRule {
		/** A bunch of characters to be accepted. Treated individually */
		private final String fChars;
		/** Token to return for this rule */
		private final IToken fToken;

		/**
		 * Creates a new operator rule.
		 * 
		 * @param token
		 *            Token to use for this rule
		 */
		public CharacterRule(IToken token, String chars) {
			fToken = token;
			fChars = chars;
		}

		/*
		 * @see
		 * org.eclipse.jface.text.rules.IRule#evaluate(org.eclipse.jface.text
		 * .rules.ICharacterScanner)
		 */
		public IToken evaluate(ICharacterScanner scanner) {
			int character = scanner.read();
			if (fChars.indexOf(character) >= 0) {
				do {
					character = scanner.read();
				} while (fChars.indexOf(character) >= 0);
				scanner.unread();
				return fToken;
			}
			scanner.unread();
			return Token.UNDEFINED;
		}
	}

}
