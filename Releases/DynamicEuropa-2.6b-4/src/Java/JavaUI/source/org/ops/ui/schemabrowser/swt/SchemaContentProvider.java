package org.ops.ui.schemabrowser.swt;

import org.eclipse.jface.viewers.ITreeContentProvider;
import org.eclipse.jface.viewers.Viewer;
import org.ops.ui.schemabrowser.model.SchemaNode;
import org.ops.ui.schemabrowser.model.SchemaSource;

/**
 * Wrapping SchemaSource as a SWT content provider
 * 
 * @author Tatiana Kichkaylo
 */
public class SchemaContentProvider implements ITreeContentProvider {

	/** The actual source of data, common between SWT and Swing versions */
	private SchemaSource model;

	/** Root node to be fed as input to the viewer */
	private SchemaNode rootNode;

	public SchemaContentProvider(SchemaSource model) {
		this.model = model;
		this.rootNode = new SchemaNode(SchemaNode.Type.CATEGORY, "Schema Root");
	}

	public void dispose() {
	}

	/**
	 * This method should not really be called, ever, since we never reassign
	 * the input to the view.
	 */
	public void inputChanged(Viewer viewer, Object oldInput, Object newInput) {
		rootNode.clear();
		viewer.refresh();
	}

	public SchemaNode[] getChildren(Object parent) {
		assert (parent instanceof SchemaNode) : "Not a schema node in the tree";
		return ((SchemaNode) parent).getArray();
	}

	public Object getParent(Object element) {
		// TODO Auto-generated method stub
		return null;
	}

	public boolean hasChildren(Object parent) {
		assert (parent instanceof SchemaNode) : "Not a schema node in the tree";
		return !((SchemaNode) parent).getChildren().isEmpty();
	}

	/** This method is called with whatever is set of view input to get started */
	public SchemaNode[] getElements(Object inputElement) {
		if (rootNode.getChildren().isEmpty())
			initialize();
		return rootNode.getArray();
	}

	/** @return one and only root node */
	public SchemaNode getRootNode() {
		return rootNode;
	}

	/** Recompute the whole tree from the model */
	public void initialize() {
		rootNode.clear();
		if (!model.isInitialized())
			return;
		rootNode.add(model.getObjectTypesNode());
	}
}
