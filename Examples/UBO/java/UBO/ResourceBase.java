package UBO;

import psengine.*;

public abstract class ResourceBase implements Resource 
{
	protected PSEngine psengine_;
	protected PSResource res_;
	protected int capacity_;
	
    public ResourceBase(PSEngine pse,PSResource r, int capacity)
    {
       psengine_ = pse;
       capacity_ = capacity;
       res_ = r;
    }
    
    public int getCapacity() { return capacity_; }
    public PSResource getPSResource() { return res_; }
    public String getName() { return res_.getEntityName(); }
    
    public ResourceProfile getLevels()
    {
    	return new ResourceProfile(this);
    }
    
	public String toString()
	{
		StringBuffer buf = new StringBuffer();
		buf.append(res_.getEntityName()).append(" ").append(getLevels().toString());
		return buf.toString();
	}    
}
