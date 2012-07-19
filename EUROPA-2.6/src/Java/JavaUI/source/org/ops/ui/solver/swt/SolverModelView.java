package org.ops.ui.solver.swt;

/**
 * Any view that depends on selection of solver model.
 * 
 * @author tatiana
 */
public interface SolverModelView {
	/**
	 * Notify the view that the current model accessible statically through
	 * SolverModelSWT has changed. The model can be NULL.
	 */
	public void setModel();
}
