package org.ops.ui.gantt.model;

import java.util.List;

/** Generic model providing access to activities and resource profiles.
 * 
 * @author Tatiana
 */
public interface IGanttModel {
	
	public int getResourceCount();

	public List<IGanttActivity> getActivities(int resource);

	public IGanttResource getResource(int resource);

	public int getStart();

	public int getEnd();

	public String getResourceName(int index);

}