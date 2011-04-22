package org.ops.ui.solver.swt;

import org.eclipse.ui.part.ViewPart;
import org.ops.ui.solver.model.SolverListener;

/**
 * Common class for view parts that also listen to the solver model events. See
 * issue 123.
 * 
 * Provides empty implementations for the methods of SolverListener in case
 * deriving classes want only some of them. Also provides empty implementation
 * of setFocus() from ViewPart.
 * 
 * @author Tatiana Kichkaylo
 */
public abstract class SolverModelViewImpl extends ViewPart implements
		SolverListener, SolverModelView {
	/** Currently active solver model, or NULL */
	protected SolverModelSWT model;

	@Override
	public void setModel() {
		if (this.model != null)
			this.model.removeSolverListener(this);
		this.model = SolverModelSWT.getCurrent();
		if (model != null)
			model.addSolverListener(this);
	}

	@Override
	public void solverStarted() {
	}

	@Override
	public void solverStopped() {
	}

	@Override
	public void beforeStepping() {
	}

	@Override
	public void afterOneStep(long time) {
	}

	@Override
	public void afterStepping() {
	}

	@Override
	public void setFocus() {
	}

	@Override
	public void dispose() {
		if (model != null)
			model.removeSolverListener(this);
		super.dispose();
	}
}
