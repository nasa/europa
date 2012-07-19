package dsa.impl;

import java.util.Enumeration;
import java.util.Iterator;
import java.util.SortedMap;
import java.util.TreeMap;

import net.n3.nanoxml.IXMLElement;

import dsa.ResourceProfile;

public class ResourceProfileImpl 
    implements ResourceProfile 
 {
	protected SortedMap<Integer,Double> upperBounds_;
	protected SortedMap<Integer,Double> lowerBounds_;
	
	public ResourceProfileImpl(String xml)
	{
	    loadFromXML(xml);	
	}
	
	protected void loadFromXML(String xml)
	{
	    upperBounds_ = new TreeMap<Integer,Double>();
	    lowerBounds_ = new TreeMap<Integer,Double>();

	    try{
		    IXMLElement response = Util.toXML(xml);
		    
		    Enumeration children = response.enumerateChildren();
		    while(children.hasMoreElements()) {
		    	IXMLElement componentXml = (IXMLElement) children.nextElement();
		    	int time = componentXml.getAttribute("time", 0);
		    	double lb = new Double(componentXml.getAttribute("lb", "0.0"));
		    	double ub = new Double(componentXml.getAttribute("ub", "0.0"));
		    	
		    	upperBounds_.put(time,ub);
		    	lowerBounds_.put(time,lb);
		    }
		}
		catch(Exception e){
		    e.printStackTrace();
		}	    
	}

	public Iterator<Integer> getTimes() 
	{
		return upperBounds_.keySet().iterator();
	}

	public double getUpperBound(int time) 
	{
		return upperBounds_.get(time);
	}
	
	public double getLowerBound(int time) 
	{
		return lowerBounds_.get(time);
	}
}
