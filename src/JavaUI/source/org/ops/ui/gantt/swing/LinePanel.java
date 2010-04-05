package org.ops.ui.gantt.swing;

import java.awt.Color;

import javax.swing.JLabel;
import javax.swing.JPanel;

/**
 * Parent class for TimelinePanel and ResourcePanel.
 * 
 * @author Tatiana Kichkaylo
 */
public abstract class LinePanel extends JPanel {

	private String name;

	private JLabel labelWidget;

	public LinePanel(String resourceName) {
		this.name = resourceName;
		this.labelWidget = new JLabel(" " + resourceName);
		labelWidget.setOpaque(true);
	}

	@Override
	public void setBackground(Color color) {
		super.setBackground(color);
		// It looks like this method is called from super constructor, so we
		// need to check that labelWidget has been initialized already
		if (labelWidget != null)
			labelWidget.setBackground(color);
	}

	@Override
	public void setBounds(int x, int y, int width, int height) {
		super.setBounds(x, y, width, height);
		if (labelWidget != null)
			labelWidget.setBounds(0, y, labelWidget.getBounds().width, height);
	}

	@Override
	public void doLayout() {
		// Noop
	}

	@Override
	public String toString() {
		return name + getBounds();
	}

	public JLabel getLabel() {
		return labelWidget;
	}

	@Override
	public abstract int getHeight();

	public abstract void layout(int stepSize, int[] horizon);
}
