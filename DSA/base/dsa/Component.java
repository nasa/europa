package dsa;

import java.util.*;

public class Component extends Entity {
    protected Component(int key, String name){
	super(key);
    }

    public String getName(){return m_name;}

    public List<Action> getActions() {
	return new Vector<Action>();
    }

    public List<Attribute> getAttributes() {
	return new Vector<Attribute>();
    }

    private String m_name;
}
