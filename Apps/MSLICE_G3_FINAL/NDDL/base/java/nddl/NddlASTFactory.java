package nddl;

import antlr.CharScanner;
import antlr.ASTFactory;
import antlr.collections.AST;

/**
 * Creation date: (4/9/2000 3:13:25 AM)
 * @author: Andrew Bachmann
 */
public class NddlASTFactory extends ASTFactory {
	CharScanner stream;
	/**
	 * Creation date: (4/9/2000 3:25:07 AM)
	 * @author: Andrew Bachmann
	 * 
	 * @param location Object
	 */
	public NddlASTFactory(CharScanner stream) {
		this.stream = stream;
		setASTNodeClass("nddl.NddlASTNode");
	}
	//must override all creates that do not delegate to other creates!
	
	/**
	 * Create a new empty AST node of type NddlASTNode
	 */
	public AST create(Class c) {
		AST t = super.create(c);
		if(t instanceof NddlASTNode)
		{
			if(stream.getFilename() != null)
				((NddlASTNode)t).setFilename(stream.getFilename());
			else
				((NddlASTNode)t).setFilename("!FILENAME ERROR!");
		}
		return t;
	}
}
