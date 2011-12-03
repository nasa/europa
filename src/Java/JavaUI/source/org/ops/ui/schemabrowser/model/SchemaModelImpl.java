package org.ops.ui.schemabrowser.model;

import java.util.HashMap;

import org.ops.ui.filemanager.model.AstNode;
import org.ops.ui.filemanager.model.AstNodeTypes;
import org.ops.ui.filemanager.model.FileModel;
import org.ops.ui.schemabrowser.model.SchemaNode.Type;
import org.ops.ui.solver.model.SolverModel;

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
 */
public class SchemaModelImpl implements SchemaModel 
{

	/** Solver model pointing to loaded files and PSEngine to do all the work */
	protected PSEngine engine_;
	
	public SchemaModelImpl(PSEngine e) 
	{
		engine_ = e;
	}

	/** Make a node for object types */
	public SchemaNode getObjectTypesNode() 
	{
		if (!isInitialized())
			return null;

		SchemaNode rootNode = new SchemaNode(Type.CATEGORY, "Object types");
		PSObjectTypeList types = engine_.getPSSchema().getAllPSObjectTypes();
		for (int i = 0; i < types.size(); i++) 
			rootNode.add(makeObjectTypeNode(types.get(i)));

		return rootNode;
	}
	
	protected SchemaNode makeObjectTypeNode(PSObjectType type)
	{
		SchemaNode typeNode = new SchemaNode(
			Type.OBJECT_TYPE, 
			type.getNameString(), 
			type.getParentName());
		
		PSStringList members = type.getMemberNames();
		for (int j = 0; j < members.size(); j++) 
			typeNode.add(makeObjectMemberNode(type,members.get(j)));

		PSTokenTypeList preds = type.getPredicates();
		for (int j = 0; j < preds.size(); j++) 
			typeNode.add(makeTokenTypeNode(preds.get(j)));
		
		return typeNode;
	}
	
	protected SchemaNode makeObjectMemberNode(PSObjectType type,String memberName)
	{
		PSDataType mtype = type.getMemberTypeRef(memberName);		
		return new SchemaNode(
				Type.OBJECT_TYPE_MEMBER, 
				memberName,
				mtype);
	}
	
	protected SchemaNode makeTokenTypeNode(PSTokenType ttype)
	{
		SchemaNode pnode = new SchemaNode(
				Type.TOKEN_TYPE, 
				ttype.getName());
		
		PSStringList argNames = ttype.getParameterNames();
		for (int k = 0; k < argNames.size(); k++) {
			PSDataType atype = ttype.getParameterType(k);
			pnode.add(new SchemaNode(
					Type.TOKEN_TYPE_PARAMETER,
					argNames.get(k), 
					atype));
		}

		return pnode;
	}
	
	public boolean isInitialized() 
	{
		return engine_ != null;
	}
}
