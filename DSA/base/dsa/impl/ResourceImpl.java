package dsa.impl;

import dsa.Resource;
import dsa.ResourceProfile;

public class ResourceImpl 
    extends EntityBase
    implements Resource 
{
	public ResourceImpl(int key,String name)
	{
		super(key,name);
	}

	// TODO: cache profiles?
	
	public ResourceProfile getCapacity() 
	{
		String xml = JNI.getResourceCapacityProfile(m_key);
		return new ResourceProfileImpl(xml);
	}

	public ResourceProfile getUsage() 
	{
		String xml = JNI.getResourceUsageProfile(m_key);
		return new ResourceProfileImpl(xml);
	}
}
