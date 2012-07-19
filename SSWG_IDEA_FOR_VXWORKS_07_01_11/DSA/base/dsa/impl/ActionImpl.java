package dsa.impl;

import java.util.*;

import dsa.Action;
import dsa.Component;
import dsa.Proposition;
import dsa.Violation;

public class ActionImpl 
    extends TokenImpl 
    implements Action 
{
    public ActionImpl(String type,
    		          String name,
    		          int key, 
    		          int startLb, 
    		          int startUb, 
    		          int endLb, 
    		          int endUb, 
    		          int durationLb, 
    		          int durationUb)
    {
	    super(type, name, key, startLb, startUb, endLb, endUb, durationLb, durationUb);
    }

    public boolean hasViolations(){return getViolation() != 0;}

    public List<Action> getChildActions(){return null;}

    public List<Proposition> getConditions(){return null;}

    public List<Proposition> getEffects(){return null;}

    public List<Violation> getViolations(){return new ArrayList<Violation>();}

	public double getViolation() 
	{
		// TODO Auto-generated method stub
		return 0;
	}
	
	public Component getComponent() 
	{
		String xml = JNI.getComponentForAction(m_key);
		List<Component> components = Util.xmlToComponents(xml);
		
		if (components.size() > 0)
			return components.get(0);
		
		return null;
	}
	
	public Action getMaster() 
	{
		String xml = JNI.getMaster(m_key);
		List<Action> components = Util.xmlToActions(xml);
		
		if (components.size() > 0)
			return components.get(0);
		
		return null;
	}	
}
