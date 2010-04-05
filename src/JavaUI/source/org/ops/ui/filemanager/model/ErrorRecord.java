package org.ops.ui.filemanager.model;

/**
 * Record for representing error messages accompanying AST. All fields are read
 * only.
 * 
 * @author Tatiana Kichkaylo
 */
public class ErrorRecord {
	private String fileName, message;
	private int line, offset, length;

	public ErrorRecord(String fileName, int line, int offset, int length,
			String message) {
		this.fileName = fileName;
		this.line = line;
		this.offset = offset;
		this.length = length;
		this.message = message;
	}

	@Override
	public String toString() {
		return fileName + ":" + line + ":" + offset + ":" + length + " "
				+ message;
	}

	public String getFileName() {
		return fileName;
	}

	public String getMessage() {
		return message;
	}

	public int getLine() {
		return line;
	}

	public int getOffset() {
		return offset;
	}

	public int getLength() {
		return length;
	}
}
