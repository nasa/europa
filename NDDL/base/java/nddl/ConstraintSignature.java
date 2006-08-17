package nddl;

import java.util.ArrayList;
import java.util.List;
import antlr.SemanticException;
import antlr.collections.AST;

public class ConstraintSignature implements NddlTokenTypes
{
	protected String name;       ///< The name that will cause this signiture to fire (matching the constraint name unless there's a uses)
	protected List signature;    ///< The other half of the signature, a list of strings.
	protected SignatureOperation root;       ///< The test that will be performed for verifying constraint type semantics.
	protected ConstraintSignature uses; ///< The parent test.

	public ConstraintSignature(String name, List signature, ConstraintSignature uses)
	{
		this.name = name;
		this.signature = signature;
		this.uses = uses;
		// first validate the signature itself:
		for(int i=0;i<signature.size();i++)
		{
			String curr = (String)signature.get(i);

		}
		// here we should validate the operations, they shouldn't make reference to variables we don't know about
	}

	public void setBlock(AST root, NddlParserState state) throws SemanticException
	{
		this.root = (SignatureOperation)convert(root,state);
	}
	private Object convert(AST root, NddlParserState state) throws SemanticException
	{
		if(root != null)
		switch(root.getType())
		{
			case LPAREN: case LBRACE: return convert(root.getFirstChild(),state);
			case DPIPE: case DAMP: case IS_A:
			             return new SignatureOperation(root.getType(),convert(root.getFirstChild(),state),convert(root.getFirstChild().getNextSibling(),state));
			case IDENT: return root.getText();
			case TYPE: return state.getType(root.getText());
			default: state.error(new RuntimeException("Error converting AST to signature operation tree"));
		}
		return null;
	}
	public boolean matches(List candidate)
	{
		if(signature.size() != candidate.size()) return false;
		if(uses != null)
		{
			List subcall = new ArrayList(uses.signature.size());
			for(int i=0;i<uses.signature.size();i++)
				subcall.add(candidate.get(signature.indexOf(uses.signature.get(i))));
			return uses.matches(subcall) && (root == null || root.eval(signature, candidate));
		}
		else if(root != null)
			return root.eval(signature, candidate);
		return true;
	}
	public String getSignature()
	{
		return name + "[" + signature.size() + "]";
	}

}
