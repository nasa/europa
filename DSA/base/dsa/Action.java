package dsa;

import java.util.*;
import net.n3.nanoxml.IXMLElement;

public class Action extends Entity {

    protected Action(int key, String actionType){
	super(key);
	m_type = actionType;
    }

    public String getType() {return m_type;}

    public boolean hasViolations(){ return false; }

    public List<Violation> getViolations() {
	List<Violation> violations =  new Vector<Violation>();
	JNI.getViolations(getKey());

	try{
	    IXMLElement response = DSA.readResponse();
	    
	    Enumeration children = response.enumerateChildren();
	    while(children.hasMoreElements()){
		IXMLElement violationXml = (IXMLElement) children.nextElement();
		//int key = violationXml.getAttribute("key", 0);
		//String name = violationXml.getAttribute("name", "NO_NAME");
		violations.add(new Violation());
	    }
	}
	catch(Exception e){
	    e.printStackTrace();
	}

	return violations;
    }
    private String m_type;
}
