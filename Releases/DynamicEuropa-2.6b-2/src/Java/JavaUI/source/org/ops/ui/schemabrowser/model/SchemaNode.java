package org.ops.ui.schemabrowser.model;

import java.util.ArrayList;

import org.ops.ui.filemanager.model.AstNode;

import psengine.PSDataType;

/**
 * Node of the schema tree.
 * 
 * @author Tatiana Kichkaylo
 */
public class SchemaNode {
	/** Seed for conversion to array */
	private final static SchemaNode[] seedArray = {};

	/** Node types. To be extended */
	public enum Type {
		CATEGORY, OBJECT_TYPE, OBJECT_TYPE_MEMBER, TOKEN_TYPE, TOKEN_TYPE_PARAMETER
	};

	/** Name of the thing. Never NULL. */
	private String name;

	/**
	 * Name of the parent type, or NULL. If/when have more rarely used fields
	 * like this, should derive from SchemaNode
	 */
	private String parentName = null;

	/** Data type for members and arguments. May be NULL */
	private PSDataType dataType = null;

	/** Node type */
	private Type nodeType;

	/** Cached label for display */
	private String text;

	/** Child nodes */
	private ArrayList<SchemaNode> children = new ArrayList<SchemaNode>();

	/** Pointer to AST node, which in turn points to file and offset */
	private AstNode ast;

	public SchemaNode(Type type, String name) {
		this.name = name;
		this.nodeType = type;
		switch (type) {
		case CATEGORY:
			this.text = name;
			break;
		case TOKEN_TYPE:
			this.text = name + " ()";
			break;
		default:
			throw new UnsupportedOperationException(
					"Cannot create a node of type " + type + " with name only");
		}
	}

	public SchemaNode(Type type, String name, PSDataType dtype) {
		this.name = name;
		this.nodeType = type;
		this.dataType = dtype;
		switch (type) {
		case OBJECT_TYPE_MEMBER:
		case TOKEN_TYPE_PARAMETER:
			this.text = dtype.getNameString() + " " + name;
			break;
		default:
			throw new UnsupportedOperationException(
					"Cannot create a node of type " + type + " with data type");
		}
	}

	public SchemaNode(Type type, String name, String parentName) {
		this.name = name;
		this.nodeType = type;
		if ("".equals(parentName))
			this.parentName = null;
		else
			this.parentName = parentName;
		if (!Type.OBJECT_TYPE.equals(type))
			throw new IllegalStateException(
					"Only object types can have parent type. Got " + type
							+ " for " + name);
		this.text = name + (this.parentName == null ? "" : (" <" + parentName));
	}

	public String getName() {
		return name;
	}

	public PSDataType getDataType() {
		return dataType;
	}

	public Type getNodeType() {
		return nodeType;
	}

	public void add(SchemaNode child) {
		children.add(child);
	}

	public ArrayList<SchemaNode> getChildren() {
		return children;
	}

	public SchemaNode[] getArray() {
		return children.toArray(seedArray);
	}

	@Override
	public String toString() {
		return text;
	}

	/** Remove all children */
	public void clear() {
		children.clear();
	}

	public void setAst(AstNode ast) {
		this.ast = ast;
	}

	public AstNode getAst() {
		return ast;
	}
}
