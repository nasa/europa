package dsa;

import java.util.List;

import dsa.impl.AttributeImpl;

public interface Component 
    extends Entity
{
	public List<Action> getActions();
	public List<Attribute> getAttributes();
}