package org.ops.ui.gantt.swing;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;

import org.ops.ui.gantt.model.IGanttResource;

/**
 * Single resource profile panel, includes upper and lower lines.
 * 
 * @author Tatiana Kichkaylo
 */
public class ResourcePanel extends LinePanel {

	private IGanttResource resource;
	private double pixelPerPoint = 1;
	private int stepSize;
	private int[] horizon;
	private static final int margin = 2;

	public ResourcePanel(IGanttResource resource) {
		super(resource.getName());
		this.resource = resource;

		// TODO Compute pixel per point

		this.addMouseMotionListener(new MouseMotionAdapter() {
			@Override
			public void mouseMoved(MouseEvent event) {
				updateTooltip(event.getX(), event.getY());
			}
		});
	}

	protected void updateTooltip(int x, int y) {
		if (horizon == null)
			return;
		// Compute time point
		int time = x / stepSize + horizon[0];
		this.setToolTipText("Time " + time + " "
				+ asInterval(resource.getLow(time), resource.getHigh(time))
				+ " of "
				+ asInterval(resource.getActualMin(), resource.getActualMax()));
	}

	protected String asInterval(double a, double b) {
		return "[" + a + ", " + b + "]";
	}

	@Override
	public int getHeight() {
		double range = resource.getActualMax() - resource.getActualMin();
		return (int) (range * pixelPerPoint) + margin * 2;
	}

	@Override
	public void layout(int stepSize, int[] horizon) {
		// TODO Auto-generated method stub

		// Remember for painting
		this.stepSize = stepSize;
		this.horizon = horizon;
	}

	@Override
	public void paint(Graphics g) {
		super.paint(g);

		double bottom = resource.getActualMin();
		int h = getHeight();
		for (int i = horizon[0] + 1; i < horizon[1]; i++) {
			int x1 = (i - 1 - horizon[0]) * stepSize;
			int x2 = (i - horizon[0]) * stepSize;

			int y1 = (int) ((resource.getLow(i - 1) - bottom) * pixelPerPoint)
					+ margin;
			y1 = h - y1;
			// int y2 = (int) ((resource.getLow(i) - bottom) * pixelPerPoint)
			// + margin;
			g.setColor(Color.red);
			g.drawLine(x1, y1, x2, y1);
			// g.drawLine(x2, y1, x2, y2);

			y1 = (int) ((resource.getHigh(i - 1) - bottom) * pixelPerPoint)
					+ margin;
			y1 = h - y1;
			// y2 = (int) ((resource.getHigh(i) - bottom) * pixelPerPoint)
			// + margin;
			g.setColor(Color.green);
			g.drawLine(x1, y1, x2, y1);
			// g.drawLine(x2, y1, x2, y2);
		}
	}
}
