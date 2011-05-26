package org.ops.ui.gantt.swt;

import org.eclipse.swt.graphics.RGB;
import org.eclipse.swt.widgets.Composite;
import org.ops.ui.gantt.model.EuropaGanttModel;
import org.ops.ui.gantt.model.IGanttModel;
import org.ops.ui.main.swt.EuropaPlugin;
import org.ops.ui.solver.model.SolverListener;
import org.ops.ui.solver.swt.SolverModelSWT;
import org.ops.ui.solver.swt.SolverModelView;

/**
 * Gantt chart view - SWT version for the Eclipse plugin
 * 
 * @author Tatiana Kichkaylo, Tristan Smith
 */
public class GanttView extends GenericGanttView implements SolverListener,
		SolverModelView {
	public static final String VIEW_ID = "org.ops.ui.gantt.swt.GanttView";

	/** Currently active solver model, or NULL */
	protected SolverModelSWT model;

	// The plugin should be initialized before this class is ever loaded
	@Override
	protected void initializeColors() {
		if (oddBg != null)
			return; // already initialized
		EuropaPlugin pl = EuropaPlugin.getDefault();
		oddBg = pl.getColor(new RGB(250, 255, 250));
		evenBg = pl.getColor(new RGB(250, 250, 150));
		smallGrid = pl.getColor(new RGB(200, 200, 200));
		ActivityWidget.DEFAULT_COLOR = new TokenColor(pl.getColor(new RGB(100,
				255, 120)), pl.getColor(new RGB(100, 240, 255)),
				pl.getColor(new RGB(100, 140, 155)), pl.getColor(new RGB(0,
						100, 0)));
	}

	/** Switch this view to the given model, possibly NULL */
	@Override
	public void setModel() {
		if (this.model != null)
			this.model.removeSolverListener(this);
		this.model = SolverModelSWT.getCurrent();
		if (model != null)
			model.addSolverListener(this);
		updateChart();
	}

	@Override
	public void createPartControl(final Composite parent) {
		super.createPartControl(parent);
		// This will update the view
		setModel();
	}

	@Override
	public void afterStepping() {
		updateChart();
	}

	@Override
	public void solverStarted() {
		updateChart();
	}

	@Override
	public void solverStopped() {
		updateChart();
	}

	@Override
	public void beforeStepping() {
	}

	@Override
	public void afterOneStep(long time) {
	}

	@Override
	protected IGanttModel getGanttModel() {
		if (model == null || model.isTerminated())
			return null;
		return new EuropaGanttModel(model);
	}
}
