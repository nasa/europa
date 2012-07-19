package org.ops.ui.gantt.model;

/**
 * Interface for a resource profile to be used with Gantt chart.
 * 
 * @author tatiana
 */
public interface IGanttResource {
	/** @return name of the resource for display purposes */
	public String getName();

	/** @return the actual minimum value for the whole profile */
	public double getActualMin();

	/** @return the actual maximum value for the whole profile */
	public double getActualMax();

	/** @return the lower bound value for the given time step */
	public double getLow(int time);

	/** @return the upper bound value for the given time step */
	public double getHigh(int time);

}