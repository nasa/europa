package org.ops.ui.schemabrowser.swing;

import java.awt.Dimension;

import javax.swing.JScrollPane;
import javax.swing.JTree;

import org.ops.ui.main.swing.EuropaInternalFrame;
import org.ops.ui.schemabrowser.model.SchemaSource;
import org.ops.ui.solver.model.SolverAdapter;
import org.ops.ui.solver.model.SolverModel;

public class SchemaView extends EuropaInternalFrame {

	private JTree tree;
	private SchemaTreeModel treeModel;

	public SchemaView(SolverModel model) {
		super("Schema browser");

		// Data
		this.treeModel = new SchemaTreeModel(
				new SchemaSource(model));
		model.addSolverListener(new SolverAdapter() {
			@Override
			public void solverStarted() {
				treeModel.reloadFromSchema();
			}
		});

		// Widgets
		this.tree = new JTree(treeModel);
		this.add(new JScrollPane(tree));
		this.tree.setRootVisible(false);
		this.tree.setCellRenderer(new SchemaNodeCellRenderer());
	}

	@Override
	public Dimension getFavoriteSize() {
		return new Dimension(200, 400);
	}
}
