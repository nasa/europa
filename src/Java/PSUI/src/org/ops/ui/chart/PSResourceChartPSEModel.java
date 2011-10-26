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

	public PSResourceProfile getLimit() 
	{
		return resource_.getLimits();
	}

	public PSResourceProfile getFDLevel() 
	{
		return resource_.getFDLevels();
	}
	
	public PSResourceProfile getVDLevel() 
	{
		return resource_.getVDLevels();
	}	
}
