package nddl;

import java.util.List;

public class SignatureOperation implements NddlTokenTypes
{
	int oper = DAMP;
	Object left;
	Object right;
	public SignatureOperation(int oper, Object left, Object right)
	{
		this.oper = oper;
		this.left = left;
		this.right = right;
	}

	public boolean eval(List signature, List candidate)
	{
		if(left instanceof String && right instanceof NddlType)
		{
			if(oper != IS_A) throw new RuntimeException("Internal singature checking error [1]");
			return ((NddlType)right).isAssignableFrom((NddlType)candidate.get(signature.indexOf(left)));
		}
		if(left instanceof String && right instanceof String)
		{
			if(oper != IS_A) throw new RuntimeException("Internal singature checking error [1]");
			return ((NddlType)candidate.get(signature.indexOf(right))).isAssignableFrom((NddlType)candidate.get(signature.indexOf(left)));
		}
		else if(left instanceof SignatureOperation && right instanceof SignatureOperation)
		{
			switch(oper)
			{
				case DAMP: return ((SignatureOperation)left).eval(signature, candidate) && ((SignatureOperation)right).eval(signature, candidate);
				case DPIPE: return ((SignatureOperation)left).eval(signature, candidate) || ((SignatureOperation)right).eval(signature, candidate);
				default: throw new RuntimeException("Internal singature checking error [2]");
			}
		}
		else throw new ClassCastException("Internal signature checking error [3]");
	}

}
