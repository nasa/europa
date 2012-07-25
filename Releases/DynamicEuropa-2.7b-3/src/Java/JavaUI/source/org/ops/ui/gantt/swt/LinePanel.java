package org.ops.ui.gantt.swt;

import org.eclipse.draw2d.Figure;
import org.eclipse.draw2d.Label;
import org.eclipse.draw2d.PositionConstants;
import org.eclipse.draw2d.geometry.Rectangle;
import org.eclipse.swt.graphics.Color;

public abstract class LinePanel extends Figure {

	private String name;

	private Label labelWidget;

	public LinePanel(String resourceName) {
		this.name = resourceName;
		this.labelWidget = new Label(" " + resourceName);
		this.setOpaque(true);
		labelWidget.setLabelAlignment(PositionConstants.LEFT);
		labelWidget.setOpaque(true);
	}

	@Override
	public void setBackgroundColor(Color color) {
		super.setBackgroundColor(color);
		// It looks like this method is called from super constructor, so we
		// need to check that labelWidget has been initialized already
		if (labelWidget != null)
			labelWidget.setBackgroundColor(color);
	}

	@Override
	public void setBounds(Rectangle rect) {
		super.setBounds(rect);
		if (labelWidget != null) {
			int w = labelWidget.getParent().getBounds().width;
			labelWidget.setBounds(new Rectangle(0, rect.y, w - 2, rect.height));
		}
	}

	@Override
	public String toString() {
		return name + getBounds();
	}

	public Label getLabel() {
		return labelWidget;
	}

	public abstract int getHeight();

	public abstract void layout(int stepSize, int[] horizon);
}
