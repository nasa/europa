package dsa;

import java.util.List;

public interface DSA 
{
    public void loadModel(String model) throws InvalidSourceException;
    public void addPlan(String txSource) throws InvalidSourceException, NoModelException;
    
    public List<Component>    getComponents();
    public List<Attribute>    getAttributes();
 
	public Action             getAction(int actionKey);
    public List<Action>       getActions();
    public List<Proposition>  getPropositions();
    
    public List<Resource> getResources();    
}
