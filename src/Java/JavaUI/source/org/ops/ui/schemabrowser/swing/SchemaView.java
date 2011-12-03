package org.ops.ui.schemabrowser.swing;

import java.awt.BorderLayout;
import java.awt.Dimension;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;

import org.ops.ui.schemabrowser.model.SchemaModel;

public class SchemaView extends JPanel {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	
	private JTree tree;
	private SchemaTreeModel treeModel;

	public SchemaView(SchemaModel model) {
		// Data
		this.treeModel = new SchemaTreeModel(model);

		setLayout(new BorderLayout());
		
		// Widgets
		this.tree = new JTree(treeModel);
		this.add(new JScrollPane(tree));
		this.tree.setRootVisible(false);
		this.tree.setCellRenderer(new SchemaNodeCellRenderer());
		this.setSize(new Dimension(200, 400));
	}
}
