package org.ops.ui;

import java.util.List;
import java.util.ArrayList;
import javax.swing.JPanel;

public abstract class PSComponentBase 
    extends JPanel 
    implements PSComponent 
 {
    List<PSMouseListener> mouseListeners_;
    
    public PSComponentBase()
    {
    	mouseListeners_ = new ArrayList<PSMouseListener>();
    }
	public void addMouseListener(PSMouseListener l) 
	{
		mouseListeners_.add(l);
	}

	public void removeMouseListener(PSMouseListener l) 
	{
		mouseListeners_.remove(l);
	}
	
	protected void notifyMouseMoved(Object key)
	{
		if (key != null) System.out.println("MouseMovedOver{"+key+"}");
		
		for (PSMouseListener l : mouseListeners_) {
			l.mouseMoved(key);
		}
	}
}
