package org.ops.ui.solver.model;

/**
 * Listener to Solver Model
 * 
 * @author Tatiana Kichkaylo
 */
public interface SolverListener {
	/** Solver started/restarted */
	public void solverStarted();
	
	/** Solver stopped */
	public void solverStopped();
	
	/** Called before the solver starts running */
	public void beforeStepping();

	/**
	 * Called each time new statistics are available, even mid-run
	 * 
	 * @param time
	 *            stepping time in ms
	 */
	public void afterOneStep(long time);

	/** Called after all required steps are done, or there is nowhere else to go */
	public void afterStepping();
}
