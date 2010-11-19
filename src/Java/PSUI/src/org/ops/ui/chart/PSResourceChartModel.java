package org.ops.ui.chart;


// TODO: introduce UI specific classes?

import psengine.PSResourceProfile;

public interface PSResourceChartModel 
{
    public PSResourceProfile getCapacity();
    public PSResourceProfile getUsage();
}
