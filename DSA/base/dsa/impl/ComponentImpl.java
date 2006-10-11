package dsa.impl;

import java.util.*;

import dsa.Action;
import dsa.Component;
import dsa.Attribute;
import net.n3.nanoxml.IXMLElement;

public class ComponentImpl 
    extends EntityBase 
    implements Component 
{
    protected ComponentImpl(int key, String name)
    {
	   super(key,name);
    }

    /* (non-Javadoc)
	 * @see dsa.Component#getActions()
	 */
    public List<Action> getActions() 
    {
    	String xml = JNI.getActions(getKey());
    	return Util.getActionsFromXML(xml);
    }

    /* (non-Javadoc)
	 * @see dsa.Component#getAttributes()
	 */
    public List<Attribute> getAttributes() {
	return new Vector<Attribute>();
    }
}
