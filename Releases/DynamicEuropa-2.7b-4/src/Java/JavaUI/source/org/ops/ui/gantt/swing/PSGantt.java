package org.ops.ui.gantt.swing;

import org.ops.ui.main.swing.PSComponentBase;
import org.ops.ui.gantt.model.PSGanttModel;

public abstract class PSGantt 
    extends PSComponentBase
{
	private static final long serialVersionUID = 1L;
	
	protected PSGanttModel model_;
	
	public PSGantt(PSGanttModel model)
	{
	    model_ = model;
	}
}
