package org.ops.ui.rchart.model;


// TODO: introduce UI specific classes?

import psengine.PSResourceProfile;

public interface PSResourceChartModel 
{
    public PSResourceProfile getCapacity();
    public PSResourceProfile getUsage();
    public PSResourceProfile getLimit();
    public PSResourceProfile getFDLevel();
    public PSResourceProfile getVDLevel();
}
