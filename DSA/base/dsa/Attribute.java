package dsa;

import java.util.*;

public class Attribute extends Entity {

    public Attribute(int key, String name){
	super(key);
	m_name = name;
    }

    List<Slot> getSlots(){
	return new Vector<Slot>();
    }

    void setValue(int from, int to, ParameterCollection value){}

    String m_name;
}
