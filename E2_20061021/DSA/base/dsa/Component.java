package dsa;

import java.util.*;
import net.n3.nanoxml.IXMLElement;

public class Component extends Entity {
    protected Component(int key, String name){
	super(key);
    }

    public String getName(){return m_name;}


    public List<Action> getActions() {
	List<Action> actions = new Vector<Action>();
	String responseStr = JNI.getActions(getKey());

	try{
	    IXMLElement response = DSA.toXML(responseStr);
	    
	    Enumeration children = response.enumerateChildren();
	    while(children.hasMoreElements()){
		IXMLElement componentXml = (IXMLElement) children.nextElement();
		String type = componentXml.getAttribute("type", "NO_NAME");
		int key = componentXml.getAttribute("key", 0);
		int startLb = componentXml.getAttribute("startLb", 0);
		int startUb = componentXml.getAttribute("startUb", 0);
		int endLb = componentXml.getAttribute("endLb", 0);
		int endUb = componentXml.getAttribute("endUb", 0);
		int durationLb = componentXml.getAttribute("durationLb", 0);
		int durationUb = componentXml.getAttribute("durationUb", 0);
		actions.add(new ActionImpl(type, key, startLb, startUb, endLb, endUb, durationLb, durationUb));
	    }
	}
	catch(Exception e){
	    e.printStackTrace();
	}

	return actions;
    }

    public List<Attribute> getAttributes() {
	return new Vector<Attribute>();
    }

    private String m_name;
}
