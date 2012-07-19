package org.ops.ui.gantt;

import org.ops.ui.PSComponentBase;

public abstract class PSGantt 
    extends PSComponentBase
{
	protected PSGanttModel model_;
	
	public PSGantt(PSGanttModel model)
	{
	    model_ = model;
	}
}
