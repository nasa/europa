package UBO;

import psengine.*;

public abstract class ResourceBase implements Resource 
{
	protected PSEngineWithResources psengine_;
	protected PSResource res_;
	protected int capacity_;
	
    public ResourceBase(PSEngineWithResources pse,PSResource r, int capacity)
    {
       psengine_ = pse;
       capacity_ = capacity;
       res_ = r;
    }
    
    public int getCapacity() { return capacity_; }
    public PSResource getPSResource() { return res_; }
    public String getName() { return res_.getName(); }
    
    public ResourceProfile getLevels()
    {
    	return new ResourceProfile(this);
    }
    
	public String toString()
	{
		StringBuffer buf = new StringBuffer();
		buf.append(res_.getName()).append(" ").append(getLevels().toString());
		return buf.toString();
	}    
}
