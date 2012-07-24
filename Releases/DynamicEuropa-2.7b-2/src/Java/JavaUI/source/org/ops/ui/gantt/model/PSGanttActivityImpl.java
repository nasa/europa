package org.ops.ui.gantt.model;

import java.util.Calendar;

public class PSGanttActivityImpl 
    implements PSGanttActivity 
 {
	protected Object key_;
	protected Calendar start_;
	protected Calendar finish_;
	protected double violation_;
	
	public PSGanttActivityImpl(Object key, 
			                   Calendar start, 
			                   Calendar finish,
			                   double violation)
	{
		key_ = key;
		start_ = start;
		finish_ = finish;
		violation_ = violation;
	}

	public Calendar getFinish() { return finish_; }
	public Object getKey() { return key_; }
    public Calendar getStart() {  return start_; }
    public double getViolation() { return violation_; }    
}
