package nddl;

/**
 * Creation date: (7/2/2000 5:55:06 PM)
 * @author: Andrew Bachmann
 */
public class NddlToken extends antlr.CommonToken {
	// most tokens will want column and filename information
	private java.lang.String filename = null;
	/**
	 * NddlToken constructor comment.
	 */
	public NddlToken() {
		super();
	}
	/**
	 * NddlToken constructor comment.
	 * @param t int
	 * @param txt java.lang.String
	 */
	public NddlToken(int t, String txt) {
		super(t, txt);
	}
	/**
	 * NddlToken constructor comment.
	 * @param s java.lang.String
	 */
	public NddlToken(String s) {
		super(s);
	}
	/**
	 * Creation date: (7/2/2000 6:46:48 PM)
	 * @author: Andrew Bachmann
	 * 
	 * @return java.lang.String
	 */
	public java.lang.String getFilename() {
		return filename;
	}
	/**
	 * Creation date: (7/2/2000 6:46:48 PM)
	 * @author: Andrew Bachmann
	 * 
	 * @param newFilename java.lang.String
	 */
	public void setFilename(java.lang.String newFilename) {
		filename = newFilename;
	}
}
