package org.ops.ui.solver.model;

/**
 * Convenience class: empty implementation of SolverListener. The class is made
 * abstract to force clients to provide at least some methods.
 * 
 * @author Tatiana Kichkaylo
 */
public abstract class SolverAdapter implements SolverListener {
	public void solverStarted() {
	}

	public void solverStopped() {
	}

	public void afterOneStep(long time) {
	}

	public void afterStepping() {
	}

	public void beforeStepping() {
	}
}
