package org.ops.ui.mouse.swing;


import psengine.PSEngine;
import psengine.PSToken;
import psengine.PSVariable;
import psengine.PSVariableList;

public class ActionDetailsPanel 
    extends MouseListenerPanel 
{
	private static final long serialVersionUID = 7779941401503562818L;

	protected PSEngine psengine_ = null;
	
    public ActionDetailsPanel(PSEngine pse)
    {
    	psengine_ = pse;
    }
	
	@Override
	public void mouseMoved(Object key) 
	{
		if (key == null) 
			text_.setText("");
		else 
		    text_.setText(getDetails((Integer)key));
	}
	
	protected String getDetails(Integer key)
	{
		return tokenDetails(psengine_.getTokenByKey(key));
	}
		
	protected String tokenDetails(PSToken t)
	{
		StringBuffer buf = new StringBuffer();

        buf.append("ID     : ").append(t.getEntityKey()).append("\n")
           .append("Name   : ").append(t.getEntityName()).append("\n")
           .append("isFact  : ").append(t.isFact()).append("\n")
        ;
        
        if (t.getParameters().size() > 0) {
            buf.append("Parameters:").append("\n");
            PSVariableList parameters = t.getParameters();
            for (int i = 0; i< parameters.size(); i++) {
            	PSVariable p = parameters.get(i);
                buf.append(p.getEntityName()).append(" : ").append(p.toString()).append("\n");
            }
        }
           
		return buf.toString();
	}	
}
