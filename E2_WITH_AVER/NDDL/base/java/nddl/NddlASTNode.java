package nddl;

/**
 * Creation date: (4/9/2000 2:53:45 AM)
 * @author: Andrew Bachmann
 */
public class NddlASTNode extends antlr.CommonAST {
	private String filename = "";
	private int line = -1;
	private int column = -1;
	public NddlASTNode() {
		super();
	}
	public NddlASTNode(antlr.Token tok) {
		initialize(tok);
	}
	/**
	 * Creation date: (4/9/2000 3:01:47 AM)
	 * @author: Andrew Bachmann
	 * 
	 * @return int
	 */
	public int getColumn() {
		return column;
	}
	/**
	 * Creation date: (4/9/2000 2:58:31 AM)
	 * @author: Andrew Bachmann
	 * 
	 * @return java.lang.String
	 */
	public java.lang.String getFilename() {
		return filename;
	}
	/**
	 * Creation date: (4/9/2000 2:58:31 AM)
	 * @author: Andrew Bachmann
	 * 
	 * @return int
	 */
	public int getLine() {
		return line;
	}
	public void initialize(int t, String txt) {
		super.initialize(t,txt);
	}
	public void initialize(antlr.collections.AST t) {
		super.initialize(t);
		setLine(((NddlASTNode)t).getLine());
		setColumn(((NddlASTNode)t).getColumn());
		setFilename(((NddlASTNode)t).getFilename());
	}
	public void initialize(antlr.Token tok) {
		super.initialize(tok);
		setLine(tok.getLine());
		setColumn(tok.getColumn());
	}
	/**
	 * Creation date: (4/9/2000 3:01:47 AM)
	 * @author: Andrew Bachmann
	 * 
	 * @param newColumn int
	 */
	protected void setColumn(int newColumn) {
		column = newColumn;
	}
	/**
	 * Creation date: (4/9/2000 2:58:31 AM)
	 * @author: Andrew Bachmann
	 * 
	 * @param newFilename java.lang.String
	 */
	protected void setFilename(java.lang.String newFilename) {
		filename = newFilename;
	}
	/**
	 * Creation date: (4/9/2000 2:58:31 AM)
	 * @author: Andrew Bachmann
	 * 
	 * @param newLine int
	 */
	protected void setLine(int newLine) {
		line = newLine;
	}
}
