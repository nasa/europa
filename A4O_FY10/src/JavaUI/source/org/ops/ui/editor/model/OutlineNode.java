package org.ops.ui.editor.model;

import java.util.ArrayList;

import org.ops.ui.filemanager.model.AstNode;
import org.ops.ui.filemanager.model.AstNodeTypes;

/**
 * Outline node. Outline tree is computed from AST tree. This class also
 * contains a bunch of static methods for the actual AST to Outline conversion.
 * 
 * @author Tatiana Kichkaylo
 */
public class OutlineNode {
	private OutlineNodeType nodeType;
	private String text;
	private ArrayList<OutlineNode> children = new ArrayList<OutlineNode>();
	private AstNode ast;

	public OutlineNode(OutlineNodeType type, String text, AstNode ast) {
		this.nodeType = type;
		this.text = text;
		this.ast = ast;
	}

	public ArrayList<OutlineNode> getChildren() {
		return children;
	}

	public void addChild(OutlineNode child) {
		this.children.add(child);
	}

	public String getText() {
		return text;
	}

	public OutlineNodeType getType() {
		return nodeType;
	}

	@Override
	public String toString() {
		return nodeType + " " + text;
	}

	public String getFileName() {
		if (ast == null)
			return null;
		return ast.getFileName();
	}

	public AstNode getAst() {
		return ast;
	}

	/** Construct an Outline node for the given AST node. Recursive */
	public static OutlineNode makeTree(AstNode ast) {
		OutlineNode res = null;

		AstNode c0 = ast.getSafe(0); // first child, or null
		AstNode c1 = ast.getSafe(1); // second child, or null
		String args = "()";

		switch (ast.getType()) {
		case AstNodeTypes.ERROR:
			return null;
		case AstNodeTypes.NDDL:
			res = new OutlineNode(OutlineNodeType.NDDL, null, ast);
			addChildren(res, ast);
			break;
		case AstNodeTypes.CLASS_DEF:
			// Class definition. First child is type name
			assert (ast.getChildren().size() >= 1);
			assert (c0.getType() == AstNodeTypes.IDENT) : "Expected IDENT, got "
					+ c0.getType();
			assert (c0.getChildren().isEmpty());
			// Then there may be 'extends' with a single id child
			if ("extends".equals(c1.getText())) {
				c1 = c1.getSafe(0);
				assert (c1.getType() == AstNodeTypes.IDENT);
				res = new OutlineNode(OutlineNodeType.CLASS_DEF, c0.getText()
						+ " < " + c1.getText(), ast);
				c1 = ast.getSafe(2);
			} else
				res = new OutlineNode(OutlineNodeType.CLASS_DEF, c0.getText(),
						ast);
			// Finally there should be { with fields and predicates
			assert (c1.getType() == AstNodeTypes.LBRACE);
			addChildren(res, c1);
			break;
		case AstNodeTypes.VARIABLE:
			// Variable definition. 1st child is type, second name
			if (ast.getChildren().size() == 2
					&& c1.getType() == AstNodeTypes.IDENT)
				res = new OutlineNode(OutlineNodeType.VARIABLE, c0.getText()
						+ " " + c1.getText(), ast);
			else if ("=".equals(c1.getText()))
				res = new OutlineNode(OutlineNodeType.VARIABLE_INST, c0
						.getText()
						+ " " + c1.getSafe(0).getText(), ast);
			break;
		case AstNodeTypes.CONSTRUCTOR:
			// Constructor. 1st child is name, 2nd is (, 3rd is {
			if (c1.getType() == AstNodeTypes.LPAREN)
				args = buildArgTypes(c1);
			res = new OutlineNode(OutlineNodeType.CONSTRUCTOR, c0.getText()
					+ args, ast);
			break;
		case AstNodeTypes.PREDICATE_KEYWORD:
			// Predicate definition. Has name, possible args, and { with body
			if (c1.getType() == AstNodeTypes.LPAREN)
				args = buildArgTypes(c1);
			res = new OutlineNode(OutlineNodeType.PREDICATE_IN_CLASS, c0
					.getText()
					+ args, ast);
			break;
		case AstNodeTypes.PREDICATE_DEF:
			// Code for predicate: token type id, predicate id, {
			if (c0.getType() == AstNodeTypes.IDENT
					&& c1.getType() == AstNodeTypes.IDENT)
				;
			res = new OutlineNode(OutlineNodeType.PREDICATE_DEF, c0.getText()
					+ "::" + c1.getText(), ast);
			break;
		case AstNodeTypes.FACT_KEYWORD:
			if (c0.getType() == AstNodeTypes.LPAREN
					&& c0.getSafe(0).getType() == AstNodeTypes.PREDICATE_INSTANCE)
				res = buildPredicateInst(OutlineNodeType.FACT, c0.getSafe(0));
			break;
		case AstNodeTypes.GOAL_KEYWORD:
			if (c0.getType() == AstNodeTypes.LPAREN
					&& c0.getSafe(0).getType() == AstNodeTypes.PREDICATE_INSTANCE)
				res = buildPredicateInst(OutlineNodeType.GOAL, c0.getSafe(0));
			break;
		case AstNodeTypes.CONSTRAINT_INSTANCE:
			// Constraint instance. First child is name, 2nd is ( with args
			res = new OutlineNode(OutlineNodeType.CONSTRAINT_INST, c0.getText()
					+ "(..)", ast);
			break;
		case AstNodeTypes.ENUM_KEYWORD:
			res = new OutlineNode(OutlineNodeType.ENUM, c0.getText(), ast);
			break;
		}
		// There may be some cases we did not catch. Uncomment to see them
		// if (res == null)
		// ast.print(System.out, "");
		return res;
	}

	/**
	 * Create node for predicate instance. First child is . with two ids, second
	 * child is the token name
	 */
	private static OutlineNode buildPredicateInst(OutlineNodeType type,
			AstNode ast) {
		AstNode a1 = ast.getSafe(0);
		if (!".".equals(a1.getText()))
			return null;
		AstNode a2 = ast.getSafe(1);
		return new OutlineNode(type, makeMember(a1) + " " + a2.getText(), ast);
	}

	/** Get string a.b out of an AST node */
	private static String makeMember(AstNode ast) {
		assert (".".equals(ast.getText()));
		return ast.getSafe(0).getText() + "." + ast.getSafe(1).getText();
	}

	private static void addChildren(OutlineNode out, AstNode ast) {
		for (AstNode achild : ast.getChildren()) {
			OutlineNode ochild = makeTree(achild);
			if (ochild != null)
				out.addChild(ochild);
		}
	}

	private static String buildArgTypes(AstNode parenNode) {
		String res = null;
		for (AstNode c : parenNode.getChildren()) {
			AstNode c0 = c.getSafe(0);
			AstNode c1 = c.getSafe(1);
			String cur = c1.getText() + " " + c0.getText();
			if (res == null)
				res = "(" + cur;
			else
				res = res + ", " + cur;
		}
		if (res == null)
			return "()";
		return res + ")";
	}
}
