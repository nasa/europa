package org.ops.ui.schemabrowser.model;

import java.util.HashMap;

import org.ops.ui.filemanager.model.AstNode;
import org.ops.ui.filemanager.model.AstNodeTypes;
import org.ops.ui.filemanager.model.FileModel;
import org.ops.ui.solver.model.SolverModel;

import psengine.PSObjectType;

/**
 * Accessor class for EUROPA schema. Uses PSEngine to get actual data
 * 
 * TODO JRB: this class shouldn't be needed, the PSSchema API should be extended to provide file location info
 * which is the only thing this class adds
 * 
 */
public class SchemaSolverModel extends SchemaModelImpl 
{

	/** Solver model pointing to loaded files and PSEngine to do all the work */
	protected SolverModel model;
	protected HashMap<String, AstNode> astMap = null;
	
	public SchemaSolverModel(SolverModel model) 
	{
		super(model != null ? model.getEngine() : null);
		this.model = model;
	}

	/** Make a node for object types */
	public SchemaNode getObjectTypesNode() 
	{
		if (!isInitialized())
			return null;

		// Parse type definitions from loaded files
		astMap = new HashMap<String, AstNode>();
		FileModel fm = FileModel.getModel(model.getModelFile().getAbsolutePath());
		collectClasses(fm.getAST(), astMap);

		return super.getObjectTypesNode();
	}
	
	protected SchemaNode makeObjectTypeNode(PSObjectType type)
	{
		SchemaNode typeNode = super.makeObjectTypeNode(type);
		
		AstNode ast = astMap.get(typeNode.getName());
		if (ast != null)
			typeNode.setFileLocation(new SchemaNode.FileLocation(
					ast.getFileName(),
					ast.getLine(),
					ast.getEndLine()));
		
		return typeNode;
	}
	
	private void collectClasses(AstNode ast, HashMap<String, AstNode> map) 
	{
		if (ast.getType() == AstNodeTypes.CLASS_DEF) {
			map.put(ast.getChildren().get(0).getText(), ast);
		} else
			for (AstNode c : ast.getChildren()) {
				collectClasses(c, map);
			}
	}

	public boolean isInitialized() 
	{
		return super.isInitialized() && (model != null) && !model.isTerminated();
	}
}
