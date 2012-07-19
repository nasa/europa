package dsa;

import java.util.List;

public interface Attribute 
{

	public List<Component> getComponents();

	public List<Slot> getValues();

	public List<Slot> getConstraints();

	public void setValue(int from, int to, AbstractType value);

}