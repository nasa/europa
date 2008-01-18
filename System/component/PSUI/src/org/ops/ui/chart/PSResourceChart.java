package org.ops.ui.chart;

import org.ops.ui.PSComponentBase;

// TODO: do we really need a specialized ResourceChart, or just a generic time series chart will suffice?

public abstract class PSResourceChart 
    extends PSComponentBase
{
	protected PSResourceChartModel model_;
	protected String resourceName_;
	
	public PSResourceChart(PSResourceChartModel model, String resourceName)
	{
	    model_ = model;
	    resourceName_ = resourceName;
	}
}
