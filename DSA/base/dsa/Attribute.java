package dsa;

import java.util.*;

public class Attribute extends Entity {

    public Attribute(int key, String name){
	super(key);
	m_name = name;
    }

    List<Component> getComponents(){
	return new Vector<Component>();
    }

    List<Slot> getValues(){
	return new Vector<Slot>();
    }

    List<Slot> getConstraints(){
	return new Vector<Slot>();
    }

    void setValue(int from, int to, AbstractType value){}

    String m_name;
}
