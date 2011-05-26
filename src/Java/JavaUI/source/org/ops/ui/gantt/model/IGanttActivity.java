package org.ops.ui.gantt.model;

/**
 * Interface for the data structure representing an activity in Gantt chart
 * 
 * @author Tatiana
 */
public interface IGanttActivity {
	/** Get minimum start time */
	public int getStartMin();

	/** Get maximum start time */
	public int getStartMax();

	/** Get minimum end time */
	public int getEndMin();

	/** Get maximum end time */
	public int getEndMax();

	/** Get minimum duration */
	public int getDurMin();

	/** Get maximum duration time */
	public int getDurMax();

	/** Get label to draw on the activity */
	public String getText();

	/**
	 * Get underlying object. This can be used to provide access to the
	 * underlying data
	 */
	public Object getData();
}