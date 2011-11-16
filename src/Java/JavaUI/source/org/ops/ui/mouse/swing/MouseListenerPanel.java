package org.ops.ui.mouse.swing;

import javax.swing.JPanel;
import javax.swing.JTextArea;
import org.ops.ui.main.swing.PSMouseListener;

public abstract class MouseListenerPanel
    extends JPanel
    implements PSMouseListener 
{
	private static final long serialVersionUID = 1L;
	
	protected JTextArea text_;
	
    public MouseListenerPanel()
    {
    	text_ = new JTextArea("");
        add(text_);	
    }
    
	public abstract void mouseMoved(Object key);
}
