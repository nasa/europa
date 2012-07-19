package org.ops.ui.filemanager.model;

import java.io.PrintStream;
import java.util.ArrayList;

/**
 * Java version of Antlr3 AST node as passed through verbose string
 * 
 * @author Tatiana Kichkaylo
 */
public class AstNode {
	/** Text of this node's token, or NULL */
	private String text = null;
	/** Type of this token, see /Europa/src/PLASMA/NDDL/base/NDDL3.tokens */
	private int type = -1;
	/** Name of the file, or NULL. Intern names to save memory */
	private String fileName = null;
	/** Line number within the file, or -1 */
	private int line = -1;
	/** Offset withing the line */
	private int offset = -1;
	/** End line */
	private int endLine = -1;
	/** Children */
	private ArrayList<AstNode> children = new ArrayList<AstNode>();

	/**
	 * Read from the string/offset. Expect to see
	 * "text":token-type:"file":line:offset-in-line or "text":token-type
	 * 
	 * @return offset past this node
	 */
	public int readNodeFrom(String astString, int offset) {
		// Token text
		checkChar(astString, offset, '\"');
		offset++;
		int end = astString.indexOf('\"', offset);
		if (end < 0)
			throw new IllegalStateException("No closing \" after " + offset);

		// Special case: string literals get double quotes
		if (end == offset && astString.charAt(end + 1) != ':') {
			end = astString.indexOf('\"', offset + 1);
			if (end < 0)
				throw new IllegalStateException("No closing \" after " + offset);
			// Check for double quote at the end
			if (astString.charAt(end + 1) == '\"')
				end++;
		}
		text = astString.substring(offset, end);

		// Token type
		checkChar(astString, end + 1, ':');
		offset = end + 2; // skip :
		end = nextStop(astString, offset, ':', ' ', ')');
		if (end == astString.length() - 1) {
			type = new Integer(astString.substring(offset));
			return end + 1;
		} else
			type = new Integer(astString.substring(offset, end));

		// If it was not :, we are done
		if (astString.charAt(end) != ':')
			return end;

		// File name
		checkChar(astString, end + 1, '\"');
		offset = end + 2;
		end = astString.indexOf('\"', offset);
		if (end < 0)
			throw new IllegalStateException("No closing \" after " + offset);
		fileName = astString.substring(offset, end);

		// Line number
		checkChar(astString, end + 1, ':');
		offset = end + 2; // skip :
		end = astString.indexOf(':', offset);
		line = new Integer(astString.substring(offset, end));
		endLine = line;

		// Offset in line
		checkChar(astString, end, ':');
		offset = end + 1; // skip :
		end = nextStop(astString, offset, ')', ' ');
		this.offset = new Integer(astString.substring(offset, end));

		return end;
	}

	/** Minimum of the two positions, unless one of them is -1 */
	public int nextStop(String str, int offset, char... cs) {
		int p = str.length() - 1;
		for (char c : cs) {
			int p1 = str.indexOf(c, offset);
			if (p1 >= 0 && p1 < p)
				p = p1;
		}
		return p;
	}

	public int readTreeFrom(String astString, int offset) {
		boolean hasChildren = false;
		if (astString.charAt(offset) == '(') {
			hasChildren = true;
			offset++;
		}

		// This node
		offset = readNodeFrom(astString, offset);
		// end of input
		if (offset == astString.length())
			return offset;

		// skip a space
		if (astString.charAt(offset) == ' ')
			offset++;

		// if no children, this is it
		if (!hasChildren)
			return offset;

		// Read children
		while (astString.charAt(offset) != ')') {
			AstNode child = new AstNode();
			offset = child.readTreeFrom(astString, offset);
			// Assume child is fully constructed
			if (this.line < 0) {
				this.line = child.line; // should be >=0
				this.offset = child.offset;
			}
			if (child.endLine > this.endLine)
				this.endLine = child.endLine;
			this.children.add(child);
		}
		offset++; // skip over )
		// skip a space
		if (offset < astString.length() && astString.charAt(offset) == ' ')
			offset++;
		return offset;
	}

	public String getText() {
		return text;
	}

	public int getType() {
		return type;
	}

	public String getFileName() {
		return fileName;
	}

	public int getLine() {
		return line;
	}

	public int getEndLine() {
		return endLine;
	}

	public int getOffset() {
		return offset;
	}

	public ArrayList<AstNode> getChildren() {
		return children;
	}

	public AstNode getSafe(int index) {
		if (index < 0 || index >= children.size())
			return null;
		return children.get(index);
	}

	/** Error check moved into a separate method */
	private static void checkChar(String string, int position, char value) {
		if (string.charAt(position) != value)
			throw new IllegalStateException("At " + position + " expected "
					+ value + ", got " + string.charAt(position));
	}

	@Override
	public String toString() {
		return "\""
				+ text
				+ "\":"
				+ type
				+ (fileName == null ? "" : (":\"" + fileName + "\":" + line
						+ ":" + offset)) + "[" + children.size() + "]";
	}

	public void print(PrintStream out, String prefix) {
		out.println(prefix + this);
		if (!children.isEmpty())
			prefix = prefix + "  ";
		for (AstNode child : children)
			child.print(out, prefix);
	}
}
