package dsa.impl;

import java.util.Enumeration;
import java.util.Iterator;
import java.util.SortedMap;
import java.util.TreeMap;

import net.n3.nanoxml.IXMLElement;

import dsa.Instant;
import dsa.ResourceProfile;

public class ResourceProfileImpl 
    implements ResourceProfile 
 {
	protected SortedMap<Integer,Double> values_;
	
	public ResourceProfileImpl(String xml)
	{
	    loadFromXML(xml);	
	}
	
	protected void loadFromXML(String xml)
	{
	    values_ = new TreeMap<Integer,Double>();
		try{
		    IXMLElement response = Util.toXML(xml);
		    
		    Enumeration children = response.enumerateChildren();
		    while(children.hasMoreElements()) {
		    	IXMLElement componentXml = (IXMLElement) children.nextElement();
		    	int time = componentXml.getAttribute("key", 0);
		    	double lb = new Double(componentXml.getAttribute("lb", "0.0"));
		    	double ub = new Double(componentXml.getAttribute("ub", "0.0"));
		    	
		    	// TODO: support both bounds
		    	values_.put(time,ub);
		    }
		}
		catch(Exception e){
		    e.printStackTrace();
		}	    
	}

	public Iterator<Integer> getTimes() 
	{
		return values_.keySet().iterator();
	}

	public double getValue(int time) 
	{
		return values_.get(time);
	}
}
