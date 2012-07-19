package org.ops.ui.schemabrowser.model;

import java.util.HashMap;

import org.ops.ui.filemanager.model.AstNode;
import org.ops.ui.filemanager.model.AstNodeTypes;
import org.ops.ui.filemanager.model.FileModel;
import org.ops.ui.schemabrowser.model.SchemaNode.Type;
import org.ops.ui.solver.model.SolverModel;

import psengine.PSDataType;
import psengine.PSObjectType;
import psengine.PSObjectTypeList;
import psengine.PSSchema;
import psengine.PSStringList;
import psengine.PSTokenType;
import psengine.PSTokenTypeList;

/**
 * Accessor class for EUROPA schema. Uses PSEngine to get actual data
 * 
 * @author Tatiana Kichkaylo
 */
public class SchemaSource {

	/** Solver model pointing to loaded files and PSEngine to do all the work */
	private SolverModel model;

	public SchemaSource(SolverModel model) {
		this.model = model;
	}

	/** Make a node for object types */
	public SchemaNode getObjectTypesNode() {
		if (!isInitialized())
			return null;

		// Parse type definitions from loaded files
		HashMap<String, AstNode> map = new HashMap<String, AstNode>();
		// for (File file : model.getLoadedFiles()) {
		// FileModel fm = FileModel.getModel(file.getAbsolutePath());
			FileModel fm = FileModel.getModel(model.getModelFile().getAbsolutePath());
			collectClasses(fm.getAST(), map);
		//}

		SchemaNode node = new SchemaNode(Type.CATEGORY, "Object types");
		PSSchema schema = model.getEngine().getPSSchema();
		PSObjectTypeList types = schema.getAllPSObjectTypes();
		for (int i = 0; i < types.size(); i++) {
			PSObjectType type = types.get(i);
			SchemaNode typeNode = new SchemaNode(Type.OBJECT_TYPE, type
					.getNameString(), type.getParentName());
			AstNode ast = map.get(typeNode.getName());
			if (ast != null)
				typeNode.setAst(ast);
			node.add(typeNode);

			PSStringList members = type.getMemberNames();
			for (int j = 0; j < members.size(); j++) {
				String name = members.get(j);
				PSDataType mtype = type.getMemberTypeRef(name);
				typeNode.add(new SchemaNode(Type.OBJECT_TYPE_MEMBER, name,
						mtype));
			}

			PSTokenTypeList preds = type.getPredicates();
			for (int j = 0; j < preds.size(); j++) {
				PSTokenType ttype = preds.get(j);
				SchemaNode pnode = new SchemaNode(Type.TOKEN_TYPE, ttype
						.getName());
				PSStringList argNames = ttype.getParameterNames();
				for (int k = 0; k < argNames.size(); k++) {
					PSDataType atype = ttype.getParameterType(k);
					pnode.add(new SchemaNode(Type.TOKEN_TYPE_PARAMETER,
							argNames.get(k), atype));
				}
				typeNode.add(pnode);
			}
		}

		return node;
	}

	private void collectClasses(AstNode ast, HashMap<String, AstNode> map) {
		if (ast.getType() == AstNodeTypes.CLASS_DEF) {
			map.put(ast.getChildren().get(0).getText(), ast);
		} else
			for (AstNode c : ast.getChildren()) {
				collectClasses(c, map);
			}
	}

	public boolean isInitialized() {
		return model != null && !model.isTerminated();
	}

	public void setModel(SolverModel smodel) {
		this.model = smodel;
	}
}
