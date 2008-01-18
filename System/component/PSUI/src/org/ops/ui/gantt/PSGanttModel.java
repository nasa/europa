package org.ops.ui.gantt;

import java.util.Calendar;
import java.util.Iterator;


public interface PSGanttModel 
{
	public String[] getResourceColumnNames();
	public String getResourceColumn(int resource, int column);
	
	public int getResourceCount();
	public Iterator<PSGanttActivity> getActivities(int resource);
	public void setActivityStart(Object key, Calendar start);
	public void setActivityFinish(Object key, Calendar finish);	
}
