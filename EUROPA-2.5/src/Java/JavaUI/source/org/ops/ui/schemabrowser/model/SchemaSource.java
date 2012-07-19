package org.ops.ui.schemabrowser.model;

import org.ops.ui.schemabrowser.model.SchemaNode.Type;

import psengine.PSDataType;
import psengine.PSEngine;
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

	/** PSEngine to do all the work */
	private PSEngine engine;

	public SchemaSource(PSEngine engine) {
		this.engine = engine;
	}

	/** Make a node for object types */
	public SchemaNode getObjectTypesNode() {
		if (engine == null)
			return null;
		SchemaNode node = new SchemaNode(Type.CATEGORY, "Object types");
		PSSchema schema = engine.getPSSchema();
		PSObjectTypeList types = schema.getAllPSObjectTypes();
		for (int i = 0; i < types.size(); i++) {
			PSObjectType type = types.get(i);
			SchemaNode typeNode = new SchemaNode(Type.OBJECT_TYPE, type
					.getNameString(), type.getParentName());
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

	public void setEngine(PSEngine engine) {
		this.engine = engine;
	}

	public boolean isInitialized() {
		return engine != null;
	}
}
