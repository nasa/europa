package dsa;

import java.util.*;

public class Component extends Entity {
    protected Component(int key, String name){
	super(key);
    }

    public String getName(){return m_name;}

    public List<Action> getActions() throws NoActivePlanException, InvalidKeyException {
	return new Vector<Action>();
    }
    private String m_name;
}
