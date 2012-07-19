package org.ops.ui.mouse;

import javax.swing.JPanel;
import javax.swing.JTextArea;
import org.ops.ui.PSMouseListener;

public abstract class MouseListenerPanel
    extends JPanel
    implements PSMouseListener 
{
	protected JTextArea text_;
	
    public MouseListenerPanel()
    {
    	text_ = new JTextArea("");
        add(text_);	
    }
    
	public abstract void mouseMoved(Object key);
}
