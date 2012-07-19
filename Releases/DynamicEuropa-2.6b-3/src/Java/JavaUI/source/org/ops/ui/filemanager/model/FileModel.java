package org.ops.ui.filemanager.model;

import java.io.File;
import java.util.ArrayList;

import psengine.PSEngine;

/**
 * File model gets AST from a file. To do so, it creates a brand new copy of
 * engine, and deletes it when it is done. Loading of models into an engine for
 * running is done in SolverModel.
 * 
 * @author Tatiana Kichkaylo
 */
public class FileModel {
	/** Property name in the engine config: list of search paths for includes */
	private static final String INCLUDE_PATH = "nddl.includePath";

	/** List of errors */
	private ArrayList<ErrorRecord> errors = new ArrayList<ErrorRecord>();

	/** Root AST node */
	private AstNode rootNode;

	/** Make this file purely abstract */
	private FileModel() {
		rootNode = new AstNode();
	}

	/** AST parser does not actually load data into the database */
	public static FileModel getModel(String fname) {
		PSEngine engine = PSEngine.makeInstance();
		engine.start();
		String oldPath = engine.getConfig().getProperty(INCLUDE_PATH);
		FileModel model = new FileModel();
		String astString = null;
		try {
			File file = new File(fname);
			if (!file.exists()) {
				System.err.println("Cannot open non-existing file " + file);
				return null;
			}
			fname = file.getAbsolutePath();
			String newPath = file.getParent();
			if (oldPath != null)
				newPath = newPath + ":" + oldPath;
			engine.getConfig().setProperty(INCLUDE_PATH, newPath);
			astString = engine.executeScript("nddl-ast", fname, true);
			// System.out.println(astString);

			// Pick out error messages and stuff
			int offset = 0;
			while (offset < astString.length()
					&& (astString.charAt(offset) == 'L' || astString
							.charAt(offset) == 'P')) {
				offset = model.readError(astString, offset + 1);
			}

			// Should be looking at AST now
			String tmp = astString.substring(offset, offset + 4);
			assert ("AST ".equals(tmp));
			offset += 4;
			offset = model.rootNode.readTreeFrom(astString, offset);
			// root.print(System.out, "");
			assert (offset == astString.length());
		} catch (Exception e) {
			System.err.println("Cannot parse NDDL file?\nAST string: "
					+ astString + "\n" + e);
			model = null;
		}
		engine.shutdown();
		engine.delete();
		return model;
	}

	/** Looking at the error structure right after the marker */
	private int readError(String string, int offset) {
		if (string.charAt(offset) != '\"')
			throw new IllegalStateException("At " + offset
					+ " expected \", got " + string.charAt(offset));

		int end = string.indexOf("\":", offset + 1);
		String fileName = string.substring(offset + 1, end);
		offset = end + 2;

		end = string.indexOf(":", offset);
		int line = new Integer(string.substring(offset, end));
		offset = end + 1;
		end = string.indexOf(":", offset);
		int off = new Integer(string.substring(offset, end));
		offset = end + 1;
		end = string.indexOf(" ", offset);
		int len = new Integer(string.substring(offset, end));
		offset = end + 1;

		end = string.indexOf("$\n", offset + 1);
		String message = string.substring(offset, end);
		offset = end + 2;

		this.errors.add(new ErrorRecord(fileName, line, off, len, message));
		return offset;
	}

	public AstNode getAST() {
		return rootNode;
	}

	public ArrayList<ErrorRecord> getErrors() {
		return errors;
	}
}
