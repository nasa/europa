package org.ops.ui.main.swing;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JCheckBoxMenuItem;
import javax.swing.JInternalFrame;
import javax.swing.WindowConstants;
import javax.swing.event.InternalFrameAdapter;
import javax.swing.event.InternalFrameEvent;

/**
 * Base class for internal frames of Swing version of GUI. This class takes care
 * of various open/close behavior and supplies check box menu item
 * 
 * @author Tatiana Kichkaylo
 */
public class EuropaInternalFrame extends JInternalFrame {
	private JCheckBoxMenuItem cbMenuItem = null;

	public EuropaInternalFrame(String name) {
		this.setMaximizable(true);
		this.setResizable(true);
		this.setClosable(true);
		this.setMinimumSize(getFavoriteSize());
		this.setSize(getFavoriteSize());
		this.setTitle(name);
		this.setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
		this.addInternalFrameListener(new InternalFrameAdapter() {
			@Override
			public void internalFrameClosing(InternalFrameEvent e) {
				setVisible(false);
				if (cbMenuItem != null)
					cbMenuItem.setSelected(false);
			}
		});
	}
	
	/** Minimum/initial size. Made into a method for overriding */
	public Dimension getFavoriteSize() {
		return new Dimension(100, 100);
	}

	/** Get a menu item with all listeners already attached */
	public JCheckBoxMenuItem getToggleMenuItem() {
		if (cbMenuItem == null) {
			cbMenuItem = new JCheckBoxMenuItem(this.getTitle(), this
					.isVisible());
			cbMenuItem.addActionListener(new ActionListener() {
				public void actionPerformed(ActionEvent e) {
					setVisible(cbMenuItem.isSelected());
				}
			});
		}
		return cbMenuItem;
	}
}
