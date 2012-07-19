package dsa;

import java.util.List;

public interface Component 
    extends Entity
{

	public abstract List<Action> getActions();

	public abstract List<Attribute> getAttributes();

}