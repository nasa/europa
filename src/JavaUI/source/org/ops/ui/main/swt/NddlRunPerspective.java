package org.ops.ui.main.swt;

import org.eclipse.ui.IFolderLayout;
import org.eclipse.ui.IPageLayout;
import org.eclipse.ui.IPerspectiveFactory;
import org.eclipse.ui.console.IConsoleConstants;
import org.ops.ui.gantt.swt.GanttView;
import org.ops.ui.schemabrowser.swt.SchemaView;
import org.ops.ui.solver.swt.DecisionsView;
import org.ops.ui.solver.swt.SolverView;
import org.ops.ui.solver.swt.StatisticsChartsView;

/**
 * Perspective for running NDDL model. This is similar to PSDesktop.
 */
public class NddlRunPerspective implements IPerspectiveFactory {
	/** Perspective id from plugin.xml */
	public static final String PESPECTIVE_ID = "org.ops.ui.main.swt.NddlRunPerspective";

	private IPageLayout factory;

	public NddlRunPerspective() {
		super();
	}

	public void createInitialLayout(IPageLayout factory) {
		this.factory = factory;
		addViews();
		addActionSets();
		addPerspectiveShortcuts();
		addViewShortcuts();
	}

	/**
	 * Should have solver dialog in top left part, editor in top right, and the
	 * payload views in bottom half of the screen
	 */
	private void addViews() {
		IFolderLayout bottom = factory.createFolder("bottomRight", // NON-NLS-1
				IPageLayout.BOTTOM, 0.5f, factory.getEditorArea());
		bottom.addView(StatisticsChartsView.VIEW_ID);
		bottom.addView(DecisionsView.VIEW_ID);
		bottom.addView(SchemaView.VIEW_ID);
		bottom.addView(GanttView.VIEW_ID);
		bottom.addPlaceholder(IConsoleConstants.ID_CONSOLE_VIEW);

		IFolderLayout topLeft = factory.createFolder("topLeft", // NON-NLS-1
				IPageLayout.LEFT, 0.3f, factory.getEditorArea());
		topLeft.addView(SolverView.VIEW_ID);
		topLeft.addView(IPageLayout.ID_RES_NAV);

		// Little buttons at the bottom. Left from the template
		factory.addFastView("org.eclipse.team.ccvs.ui.RepositoriesView", 0.50f); // NON-NLS-1
		factory.addFastView("org.eclipse.team.sync.views.SynchronizeView",
				0.50f); // NON-NLS-1
	}

	private void addActionSets() {
		factory.addActionSet(IPageLayout.ID_NAVIGATE_ACTION_SET); // NON-NLS-1
	}

	private void addPerspectiveShortcuts() {
		factory
				.addPerspectiveShortcut("org.eclipse.team.ui.TeamSynchronizingPerspective"); // NON-NLS-1
		factory
				.addPerspectiveShortcut("org.eclipse.team.cvs.ui.cvsPerspective"); // NON-NLS-1
	}

	private void addViewShortcuts() {
		factory.addShowViewShortcut(IConsoleConstants.ID_CONSOLE_VIEW);
		factory.addShowViewShortcut(IPageLayout.ID_RES_NAV);
		factory.addShowViewShortcut(IPageLayout.ID_PROBLEM_VIEW);
		factory.addShowViewShortcut(IPageLayout.ID_OUTLINE);
	}
}
