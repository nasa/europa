package dsa;

import java.io.*;
import java.util.*;
import java.net.*;

// nanoxml support
import net.n3.nanoxml.StdXMLReader;
import net.n3.nanoxml.XMLParserFactory;
import net.n3.nanoxml.IXMLReader;
import net.n3.nanoxml.XMLWriter;
import net.n3.nanoxml.IXMLParser;
import net.n3.nanoxml.IXMLElement;

public class DSA {

    /** Static Data Members **/
    private static DSA s_instance = null;

    private String m_model = null;

    private DSA(){}

    static public DSA instance(){
	if(s_instance == null){
	    s_instance = new DSA();
	    System.loadLibrary("DSA_g");
	}
	return s_instance;
    }

    public void loadModel(String model) throws InvalidSourceException {
	m_model = model;
	JNI.load(model);
    }

    public void loadTransactions(String txSource) throws InvalidSourceException, NoModelException {
	if(m_model == null)
	    throw new NoModelException();

	JNI.loadTransactions(txSource);
    }

    public List<Component> getComponents() {
	List<Component> components =  new Vector<Component>();
	JNI.getComponents();

	try{
	    IXMLElement response = readResponse();
	    
	    Enumeration children = response.enumerateChildren();
	    while(children.hasMoreElements()){
		IXMLElement componentXml = (IXMLElement) children.nextElement();
		int key = componentXml.getAttribute("key", 0);
		String name = componentXml.getAttribute("name", "NO_NAME");
		components.add(new Component(key, name));
	    }
	}
	catch(Exception e){
	    e.printStackTrace();
	}

	return components;
    }

    public List<Resource> getResources() {
	return new Vector<Resource>();
    }

    private void handleUpdates(){

    }

    public static IXMLElement readResponse() throws Exception {
	IXMLParser parser = XMLParserFactory.createDefaultXMLParser();
	IXMLReader reader = StdXMLReader.fileReader("RESPONSE_FILE");
	parser.setReader(reader);
	return (IXMLElement) parser.parse();
    }
}
