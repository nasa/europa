package org.ops.ui.schemabrowser.swing;

import java.util.ArrayList;

import javax.swing.event.TreeModelEvent;
import javax.swing.event.TreeModelListener;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

import org.ops.ui.schemabrowser.model.SchemaNode;
import org.ops.ui.schemabrowser.model.SchemaModel;

/**
 * Convert schema nodes into Swing tree model
 * 
 * @author Tatiana Kichkaylo
 */
public class SchemaTreeModel implements TreeModel {
	private SchemaModel model;
	private SchemaNode rootNode = new SchemaNode(SchemaNode.Type.CATEGORY,
			"Root");
	private ArrayList<TreeModelListener> listeners = new ArrayList<TreeModelListener>();

	public SchemaTreeModel(SchemaModel m) {
		this.model = m;
		reloadFromSchema();
	}

	public void reloadFromSchema() {
		rootNode.clear();

		rootNode.add(model.getObjectTypesNode());
		TreeModelEvent event = new TreeModelEvent(this,
				new Object[] { rootNode });
		for (TreeModelListener lnr : this.listeners)
			lnr.treeStructureChanged(event);
	}

	public SchemaNode getRoot() {
		return rootNode;
	}

	public void addTreeModelListener(TreeModelListener lnr) {
		if (!this.listeners.contains(lnr))
			this.listeners.add(lnr);
	}

	public void removeTreeModelListener(TreeModelListener lnr) {
		this.listeners.remove(lnr);
	}

	public SchemaNode getChild(Object node, int index) {
		SchemaNode n = (SchemaNode) node;
		return n.getChildren().get(index);
	}

	public int getChildCount(Object node) {
		SchemaNode n = (SchemaNode) node;
		return n.getChildren().size();
	}

	public int getIndexOfChild(Object parent, Object child) {
		SchemaNode n = (SchemaNode) parent;
		return n.getChildren().indexOf(child);
	}

	public boolean isLeaf(Object node) {
		SchemaNode n = (SchemaNode) node;
		return n.getChildren().isEmpty();
	}

	public void valueForPathChanged(TreePath path, Object newValue) {
		// Should not be called for this model
	}
}
