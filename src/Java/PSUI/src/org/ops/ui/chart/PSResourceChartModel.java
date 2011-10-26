package org.ops.ui.chart;


// TODO: introduce UI specific classes?

import psengine.PSResourceProfile;

public interface PSResourceChartModel 
{
    public PSResourceProfile getLimit();
    public PSResourceProfile getFDLevel();
    public PSResourceProfile getVDLevel();
}
