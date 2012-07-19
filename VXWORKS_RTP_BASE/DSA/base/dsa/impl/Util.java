package dsa.impl;

import java.util.List;
import java.util.Vector;
import java.util.Enumeration;

import dsa.Action;
import dsa.Component;
import dsa.Parameter;
import net.n3.nanoxml.IXMLElement;
import net.n3.nanoxml.IXMLParser;
import net.n3.nanoxml.IXMLReader;
import net.n3.nanoxml.StdXMLReader;
import net.n3.nanoxml.XMLParserFactory;

class Util
{
	public static List<Action> xmlToActions(String xml)
	{
		List<Action> actions = new Vector<Action>();

		try{
		    IXMLElement response = toXML(xml);
		    
		    Enumeration children = response.enumerateChildren();
		    while(children.hasMoreElements()){
		    	IXMLElement componentXml = (IXMLElement) children.nextElement();
		    	String type = componentXml.getAttribute("type", "NO_TYPE");
		    	String name = componentXml.getAttribute("name", "NO_NAME");
		    	int key = componentXml.getAttribute("key", 0);
		    	int startLb = componentXml.getAttribute("startLb", 0);
		    	int startUb = componentXml.getAttribute("startUb", 0);
		    	int endLb = componentXml.getAttribute("endLb", 0);
		    	int endUb = componentXml.getAttribute("endUb", 0);
		    	int durationLb = componentXml.getAttribute("durationLb", 0);
		    	int durationUb = componentXml.getAttribute("durationUb", 0);
		    	ActionImpl a = new ActionImpl(type, name, key, startLb, startUb, endLb, endUb, durationLb, durationUb);
		    	actions.add(a);
		    	
		    	Enumeration parameters = componentXml.enumerateChildren();
			    while(parameters.hasMoreElements()){
			    	IXMLElement parameterXml = (IXMLElement) parameters.nextElement();
			    	String pname = parameterXml.getAttribute("name","NO_NAME");
			    	String pvalue = parameterXml.getAttribute("value","NO_NAME");
			    	Parameter p = new ParameterImpl(pname,pvalue);
			    	a.addParameter(p);
			    }
		    }
		}
		catch(Exception e){
		    e.printStackTrace();
		}

		return actions;
	}
	
    public static List<Component> xmlToComponents(String xml)
    {
    	List<Component> components =  new Vector<Component>();

    	try{
    	    IXMLElement response = Util.toXML(xml);
    	    
    	    Enumeration children = response.enumerateChildren();
    	    while(children.hasMoreElements()){
    		IXMLElement componentXml = (IXMLElement) children.nextElement();
    		int key = componentXml.getAttribute("key", 0);
    		String name = componentXml.getAttribute("name", "NO_NAME");
    		components.add(new ComponentImpl(key, name));
    	    }
    	}
    	catch(Exception e){
    	    e.printStackTrace();
    	}

    	return components;    	
    }
	
    public static IXMLElement toXML(String xmlStr) 
        throws Exception 
    {
    	IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
    	IXMLReader reader = StdXMLReader.stringReader(xmlStr);
    	parser.setReader(reader);
    	return (IXMLElement) parser.parse();
    }	
}