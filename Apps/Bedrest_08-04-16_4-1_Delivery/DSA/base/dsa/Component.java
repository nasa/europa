package dsa;

import java.util.List;

public interface Component 
    extends Entity
{
	public List<Action> getActions();
	public List<Attribute> getAttributes();
}