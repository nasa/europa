package dsa.impl;

import java.util.*;

import dsa.AbstractType;
import dsa.Attribute;
import dsa.Component;
import dsa.Slot;

public class AttributeImpl 
    extends EntityBase 
    implements Attribute 
{
    public AttributeImpl(int key, String name)
    {
    	super(key, name);
    }

    public List<Component> getComponents(){
	return new Vector<Component>();
    }

    public List<Slot> getValues(){
	return new Vector<Slot>();
    }

    public List<Slot> getConstraints(){
	return new Vector<Slot>();
    }

    public void setValue(int from, int to, AbstractType value){}
}
