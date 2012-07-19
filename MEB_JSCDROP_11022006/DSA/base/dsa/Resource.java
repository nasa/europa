package dsa;

public interface Resource 
    extends Entity
{
    public ResourceProfile getCapacity();
    public ResourceProfile getUsage();    
}
