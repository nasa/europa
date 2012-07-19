package org.ops.ui.chart;

import psengine.PSResource;
import psengine.PSResourceProfile;

public class PSResourceChartPSEModel 
    implements PSResourceChartModel 
{
	protected PSResource resource_;
	
	public PSResourceChartPSEModel(PSResource resource)
	{
	    resource_ = resource;
	}

	public PSResourceProfile getCapacity() 
	{
		return resource_.getLimits();
	}

	public PSResourceProfile getUsage() 
	{
		return resource_.getLevels();
	}
}
