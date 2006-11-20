package dsa;

import java.util.List;

public interface DSA 
{
    public void loadModel(String model) throws InvalidSourceException;
    
    /*
     * intepreted is a temporary flag to switch between interpreted and code-generated implementations
     * when everything can be interpreted it will go away
     */
    public void addPlan(String txSource, boolean interpreted) throws InvalidSourceException, NoModelException;
    
    public List<Component>    getComponents();
    public List<Attribute>    getAttributes();
 
	public Action             getAction(int actionKey);
    public List<Action>       getActions();
    public List<Proposition>  getPropositions();
    
    public List<Resource> getResources();    
}
