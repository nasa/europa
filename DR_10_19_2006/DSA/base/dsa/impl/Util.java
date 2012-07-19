package dsa.impl;

import java.util.List;
import java.util.Vector;
import java.util.Enumeration;

import dsa.Action;
import dsa.Component;
import dsa.Attribute;
import net.n3.nanoxml.IXMLElement;
import net.n3.nanoxml.IXMLParser;
import net.n3.nanoxml.IXMLReader;
import net.n3.nanoxml.StdXMLReader;
import net.n3.nanoxml.XMLParserFactory;

class Util
{
	public static List<Action> getActionsFromXML(String xml)
	{
		List<Action> actions = new Vector<Action>();

		try{
		    IXMLElement response = toXML(xml);
		    
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
	
    public static IXMLElement toXML(String xmlStr) 
        throws Exception 
    {
    	IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
    	IXMLReader reader = StdXMLReader.stringReader(xmlStr);
    	parser.setReader(reader);
    	return (IXMLElement) parser.parse();
    }	
}