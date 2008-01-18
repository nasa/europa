package org.ops.ui.mouse;

import java.util.List;

import psengine.PSEngine;
import psengine.PSToken;

public class ActionViolationsPanel 
    extends MouseListenerPanel 
{
	private static final long serialVersionUID = 7462693069863073725L;

	protected PSEngine psengine_ = null;
	
    public ActionViolationsPanel(PSEngine pse)
    {
    	psengine_ = pse;
    }
	
	@Override
	public void mouseMoved(Object key) 
	{
		if (key == null) 
			text_.setText("");
		else 
		    text_.setText(getViolations((Integer)key));
	}
		
	protected String getViolations(Integer key)
	{
		return tokenViolations(psengine_.getTokenByKey(key));
	}
		
	protected String tokenViolations(PSToken t)
	{
		return t.getViolationExpl();
	}	
}
